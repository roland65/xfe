// File management class with progress dialog

#include "config.h"
#include "i18n.h"

#include <fcntl.h>
#include <utime.h>
#if defined(linux)
#include <sys/statfs.h>
#endif

// For Sun compatibility
#ifdef __sun
#include <alloca.h>
#endif


#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>
#include <FXUTF8Codec.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "XFileExplorer.h"
#include "OverwriteBox.h"
#include "MessageBox.h"
#include "CommandWindow.h"
#include "File.h"


// Delay before the progress bar should be shown (ms)
#define SHOW_PROGRESSBAR_DELAY      1000

// Progress dialog width
#define PROGRESSDIALOG_WIDTH        300

// Source size refresh delay  (ms)
#define SOURCESIZE_REFRESH_DELAY    500

// Global variables
extern FXMainWindow* mainWindow;


// Compute source size recursively (same algorithm as pathsize() function)
FXulong computeSourceSize(File* f, FXApp* app, char* path, FXuint* nbfiles, FXuint* nbsubdirs,
                          FXulong* totalsize, FXButton* cancelButton, FXbool* cancelled)
{
    struct stat statbuf;
    struct dirent* dirp;
    char* ptr;
    DIR* dp;
    FXulong dsize;
    int ret;

    ret = xf_lstat(path, &statbuf);
    if (ret < 0)
    {
        return 0;
    }

    dsize = (FXulong)statbuf.st_size;
    (*totalsize) += dsize;
    (*nbfiles)++;

    // Not a directory
    if (!S_ISDIR(statbuf.st_mode))
    {
        return dsize;
    }

    // Give cancel button an opportunity to be clicked
    if (cancelButton)
    {
        app->runModalWhileEvents(cancelButton);
    }

    // Repaint application
    if (app != NULL)
    {
        app->repaint();
    }

    if (*cancelled)
    {
        return 0;
    }

    // Directory
    (*nbsubdirs)++;

    ptr = (char*)path + strlen(path);
    if (ptr[-1] != '/')
    {
        *ptr++ = '/';
        *ptr = '\0';
    }

    if ((dp = opendir(path)) == NULL)
    {
        return 0;
    }

    while ((dirp = readdir(dp)))
    {
        if (xf_strequal(dirp->d_name, ".") || xf_strequal(dirp->d_name, ".."))
        {
            continue;
        }
        xf_strlcpy(ptr, dirp->d_name, strlen(dirp->d_name) + 1);

        // Recursive call
        dsize += computeSourceSize(f, app, path, nbfiles, nbsubdirs, totalsize, cancelButton, cancelled);
    }

    ptr[-1] = '\0'; // ??

    if (closedir(dp) < 0)
    {
        fprintf(stderr, _("Error: Can't close folder %s\n"), path);
    }

    f->setTotalSourceSize(*totalsize);

    return dsize;
}


// Message Map
FXDEFMAP(File) FileMap[] =
{
    FXMAPFUNC(SEL_COMMAND, File::ID_CANCEL_BUTTON, File::onCmdCancel),
    FXMAPFUNC(SEL_TIMEOUT, File::ID_PROGRESSBAR, File::onTimeout),
    FXMAPFUNC(SEL_TIMEOUT, File::ID_SOURCESIZE, File::onSourceSizeRefresh),
};


// Object implementation
FXIMPLEMENT(File, DialogBox, FileMap, ARRAYNUMBER(FileMap))


// Construct object
File::File(FXWindow* owner, FXString title, const FXuint operation, const FXuint num) :
DialogBox(owner, title, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE)
{
    // Progress window
    FXPacker* buttons = new FXPacker(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 10, 10,
                                     PROGRESSDIALOG_WIDTH, PROGRESSDIALOG_WIDTH, 5, 5);
    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Cancel Button
#if defined(linux)
    if (operation != MOUNT && operation != UNMOUNT)
#endif
    {
        cancelButton = new FXButton(buttons, _("&Cancel"), NULL, this, File::ID_CANCEL_BUTTON, FRAME_GROOVE |
                                    LAYOUT_CENTER_X, 0, 0, 0, 0, 20, 20);
        cancelButton->setFocus();
        cancelButton->addHotKey(KEY_Escape);        
    }
    cancelled = false;

    // Progress bar
    progressbar = NULL;

    // Progress bar colors (foreground and background)
    FXuint r, g, b, l;
    FXColor textcolor, textaltcolor;
    FXColor fgcolor = getApp()->reg().readColorEntry("SETTINGS", "pbarcolor", FXRGB(10, 36, 106));
    FXColor bgcolor = getApp()->reg().readColorEntry("SETTINGS", "backcolor", FXRGB(255, 255, 255));

    // Text color is white or black depending on the background luminance
    r = FXREDVAL(bgcolor);
    g = FXGREENVAL(bgcolor);
    b = FXBLUEVAL(bgcolor);
    l = (FXuint)(0.3 * r + 0.59 * g + 0.11 * b);
    if (l < 150)
    {
        textcolor = FXRGB(255, 255, 255);
    }
    else
    {
        textcolor = FXRGB(0, 0, 0);
    }

    // Alternate text color is white or black depending on the foreground luminance
    r = FXREDVAL(fgcolor);
    g = FXGREENVAL(fgcolor);
    b = FXBLUEVAL(fgcolor);
    l = (FXuint)(0.3 * r + 0.59 * g + 0.11 * b);
    if (l < 150)
    {
        textaltcolor = FXRGB(255, 255, 255);
    }
    else
    {
        textaltcolor = FXRGB(0, 0, 0);
    }

    // Copy suffix
    copysuffix = getApp()->reg().readStringEntry("OPTIONS", "copysuffix", DEFAULT_COPY_SUFFIX);
    copysuffix_pos = getApp()->reg().readUnsignedEntry("OPTIONS", "copysuffix_pos", 0);

    // Progress dialog depends on the file operation
    switch (operation)
    {
    case COPY:
        // Labels and progress bar
        uplabel = new FXLabel(contents, _("Source:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("Target:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        progressbar = new FXProgressBar(contents, NULL, 0, LAYOUT_FILL_X | TEXTFIELD_NORMAL | PROGRESSBAR_PERCENTAGE,
                                        0, 0, 0, 0, 0, 0, 0, 0); // 0 top and bottom pads
        progressbar->setBarColor(fgcolor);
        progressbar->setTextColor(textcolor);
        progressbar->setTextAltColor(textaltcolor);
        datatext = _("Copied data:");
        datalabel = new FXLabel(contents, datatext, NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        timetext = _("Remaining time:");
        timelabel = new FXLabel(contents, timetext, NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;

    case MOVE:
        // Labels and progress bar
        uplabel = new FXLabel(contents, _("Source:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("Target:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        progressbar = new FXProgressBar(contents, NULL, 0, LAYOUT_FILL_X | TEXTFIELD_NORMAL | PROGRESSBAR_PERCENTAGE,
                                        0, 0, 0, 0, 0, 0, 0, 0); // 0 top and bottom pads
        progressbar->setBarColor(fgcolor);
        progressbar->setTextColor(textcolor);
        progressbar->setTextAltColor(textaltcolor);
        datatext = _("Moved data:");
        datalabel = new FXLabel(contents, datatext, NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        timetext = _("Remaining time:");
        timelabel = new FXLabel(contents, timetext, NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;

    case DELETE:
        // Labels
        uplabel = new FXLabel(contents, _("Delete:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("From:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        datalabel = NULL;
        timelabel = NULL;

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;

    case CHMOD:
        // Labels
        uplabel = new FXLabel(contents, _("Changing permissions..."), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("File: "), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        datalabel = NULL;
        timelabel = NULL;
        break;

    case CHOWN:
        // Labels
        uplabel = new FXLabel(contents, _("Changing owner..."), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("File: "), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        datalabel = NULL;
        timelabel = NULL;

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;

#if defined(linux)
    case MOUNT:
        // Labels
        uplabel = new FXLabel(contents, _("Mount file system..."), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("Mounting folder:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        datalabel = NULL;
        timelabel = NULL;

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;

    case UNMOUNT:
        // Labels
        uplabel = new FXLabel(contents, _("Unmount file system..."), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        downlabel = new FXLabel(contents, _("Unmounting folder:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_X);
        datalabel = NULL;
        timelabel = NULL;

        // Timer on
        getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
        break;
#endif

    default: // Other : RENAME, SYMLINK, ARCHIVE, EXTRACT, PKG_INSTALL, PKG_UNINSTALL
        // Progress dialog not used
        uplabel = NULL;
        downlabel = NULL;
        datalabel = NULL;
        timelabel = NULL;
    }

    FXbool confirm_overwrite = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_overwrite", true);

    // Initialize the overwrite flags
    if (confirm_overwrite)
    {
        overwrite = false;
        overwrite_all = false;
        skip_all = false;
    }
    else
    {
        overwrite = true;
        overwrite_all = true;
        skip_all = false;
    }

    // Total read data
    totaldataread = 0;

    // Owner window
    ownerwin = owner;

    // Operation
    op = operation;

    // Number of selected items
    numsel = num;

    // Error message box
    mbox = new MessageBox(ownerwin, _("Error"), "", bigerroricon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);

    // Total data size
    totalsourcesize = 0;

    // Refresh source size label
    if (op == COPY || op == MOVE)
    {
        getApp()->addTimeout(this, File::ID_SOURCESIZE, SOURCESIZE_REFRESH_DELAY);
    }
}


// Destructor
File::~File()
{
    if (op == COPY || op == MOVE)
    {
        getApp()->removeTimeout(this, File::ID_SOURCESIZE);
    }
    getApp()->removeTimeout(this, File::ID_PROGRESSBAR);
    delete progressbar;
    delete mbox;
}


// Create and initialize
void File::create()
{
    DialogBox::create();
}


// Force timeout for progress dialog (used before opening confirmation or error dialogs)
void File::forceTimeout(void)
{
    getApp()->removeTimeout(this, File::ID_PROGRESSBAR);
    hide();
    getApp()->forceRefresh();
    getApp()->flush();
}


// Restart timeout for progress dialog  (used after closing confirmation or error dialogs)
void File::restartTimeout(void)
{
    getApp()->addTimeout(this, File::ID_PROGRESSBAR, SHOW_PROGRESSBAR_DELAY);
}


// Read bytes
FXlong File::fullread(int fd, FXuchar* ptr, FXlong len)
{
    FXlong nread;

#ifdef EINTR
    do
    {
        nread = read(fd, ptr, len);
    }
    while (nread < 0 && errno == EINTR);
#else
    nread = read(fd, ptr, len);
#endif
    return nread;
}


// Write bytes
FXlong File::fullwrite(int fd, const FXuchar* ptr, FXlong len)
{
    FXlong nwritten, ntotalwritten = 0;

    while (len > 0)
    {
        nwritten = write(fd, ptr, len);
        if (nwritten < 0)
        {
#ifdef EINTR
            if (errno == EINTR)
            {
                continue;
            }
#endif
            return -1;
        }
        ntotalwritten += nwritten;
        ptr += nwritten;
        len -= nwritten;
    }
    return ntotalwritten;
}


// Construct overwrite dialog and get user answer
FXuint File::getOverwriteAnswer(FXString srcpath, FXString tgtpath, FXbool restart_timeout)
{
    // Message string
    FXString msg;

    if (xf_isdirectory(tgtpath))
    {
        msg.format(_("Folder %s already exists.\nOverwrite?\n=> Caution, files within this folder could be overwritten!"),
                   tgtpath.text());
    }
    else
    {
        msg.format(_("File %s already exists.\nOverwrite?"), tgtpath.text());
    }

    // Read time format
    FXString timeformat = getApp()->reg().readStringEntry("SETTINGS", "time_format", DEFAULT_TIME_FORMAT);

    // Get the size and mtime of the source and target
    struct stat linfo;
    FXString srcsize, srcmtime, tgtsize, tgtmtime;
    FXbool statsrc = false, stattgt = false;
    if (xf_lstat(srcpath.text(), &linfo) == 0)
    {
        statsrc = true;
        srcmtime = FXSystem::time(timeformat.text(), linfo.st_mtime);
        char buf[MAXPATHLEN];
        if (S_ISDIR(linfo.st_mode)) // Folder
        {
            FXulong dirsize = 0;
            FXuint nbfiles = 0, nbsubfolders = 0;
            FXulong totalsize = 0;
            xf_strlcpy(buf, srcpath.text(), srcpath.length() + 1);
            dirsize = xf_pathsize(buf, &nbfiles, &nbsubfolders, &totalsize);
#if __WORDSIZE == 64
            snprintf(buf, sizeof(buf), "%lu", dirsize);
#else
            snprintf(buf, sizeof(buf), "%llu", dirsize);
#endif
        }
        else // File
        {
#if __WORDSIZE == 64
            snprintf(buf, sizeof(buf), "%lu", (FXulong)linfo.st_size);
#else
            snprintf(buf, sizeof(buf), "%llu", (FXulong)linfo.st_size);
#endif
        }
        srcsize = xf_humansize(buf);
    }
    if (xf_lstat(tgtpath.text(), &linfo) == 0)
    {
        stattgt = true;
        tgtmtime = FXSystem::time(timeformat.text(), linfo.st_mtime);
        char buf[64];
        if (S_ISDIR(linfo.st_mode)) // Folder
        {
            FXulong dirsize = 0;
            FXuint nbfiles = 0, nbsubfolders = 0;
            FXulong totalsize = 0;
            xf_strlcpy(buf, tgtpath.text(), tgtpath.length() + 1);
            dirsize = xf_pathsize(buf, &nbfiles, &nbsubfolders, &totalsize);
#if __WORDSIZE == 64
            snprintf(buf, sizeof(buf), "%lu", dirsize);
#else
            snprintf(buf, sizeof(buf), "%llu", dirsize);
#endif
        }
        else // File
        {
#if __WORDSIZE == 64
            snprintf(buf, sizeof(buf), "%lu", (FXulong)linfo.st_size);
#else
            snprintf(buf, sizeof(buf), "%llu", (FXulong)linfo.st_size);
#endif
        }
        tgtsize = xf_humansize(buf);
    }

    // Overwrite dialog
    OverwriteBox* dlg;
    if (statsrc && stattgt)
    {
        if (numsel == 1)
        {
            dlg = new OverwriteBox(ownerwin, _("Confirm Overwrite"), msg, srcsize, srcmtime, tgtsize, tgtmtime,
                                   OVWBOX_SINGLE_FILE);
        }
        else
        {
            dlg = new OverwriteBox(ownerwin, _("Confirm Overwrite"), msg, srcsize, srcmtime, tgtsize, tgtmtime);
        }
    }
    else
    {
        if (numsel == 1)
        {
            dlg = new OverwriteBox(ownerwin, _("Confirm Overwrite"), msg, OVWBOX_SINGLE_FILE);
        }
        else
        {
            dlg = new OverwriteBox(ownerwin, _("Confirm Overwrite"), msg);
        }
    }

    FXuint answer = dlg->execute(PLACEMENT_OWNER);
    delete dlg;

    if (restart_timeout)
    {
        restartTimeout();
    }

    return answer;
}


// Copy ordinary file
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
// Return -2 when an error has occurred during the copy
int File::copyfile(const FXString& source, const FXString& target, const FXString& hsourcesize,
                   const FXulong sourcesize,
                   const FXulong tstart, const FXbool preserve_date)
{
    FXString destfile;
    FXuchar buffer[BUFFER_COPY_SIZE];
    struct stat info;
    struct utimbuf timbuf;
    FXlong nread, nwritten;
    int src, dst;
    int ok = false;

    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "preserve_date_warn", true);

    if ((src = ::open(source.text(), O_RDONLY)) >= 0)
    {
        if (xf_stat(source.text(), &info) == 0)
        {
            // If destination is a directory
            if (xf_isdirectory(target))
            {
                destfile = target + PATHSEPSTRING + FXPath::name(source);
            }
            else
            {
                destfile = target;
            }

            // File is on an MTP mount
            if (destfile.contains("mtp:host="))
            {
                // Force timeout checking for progress dialog
                checkTimeout();

                // Update read data size
                if (xf_lstat(source.text(), &info) == 0)
                {
                    totaldataread += (FXulong)info.st_size;
                }

                // Force timeout checking for progress dialog
                checkTimeout();

                if (progressbar)
                {
                    // Percentage
                    int pct = (sourcesize == 0 ? 0 : (100.0 * totaldataread) / sourcesize);
                    progressbar->setProgress(pct);

                    char size[64];

                    // Copy speed
                    double copyspeed = totaldataread / (double)(xf_getcurrenttime() - tstart);
                    snprintf(size, sizeof(size), "%f", copyspeed);
                    FXString hspeed = xf_humansize(size);

                    // Remaining time in seconds
                    FXuint remtime = (FXuint)((sourcesize - totaldataread) / copyspeed);
                    FXString hremtime = xf_secondstotimestring(remtime);

                    // Total data copied
                    FXString hsize;


#if __WORDSIZE == 64
                    snprintf(size, sizeof(size), "%ld", totaldataread);
#else
                    snprintf(size, sizeof(size), "%lld", totaldataread);
#endif
                    hsize = xf_humansize(size);
                    snprintf(size, sizeof(size), "%s %s / %s (%s/s)", datatext.text(), hsize.text(),
                             hsourcesize.text(), hspeed.text());
                    datalabel->setText(size);
                    snprintf(size, sizeof(size), "%s %s", timetext.text(), hremtime.text());
                    timelabel->setText(size);
                }

                // Give cancel button an opportunity to be clicked
                if (cancelButton)
                {
                    getApp()->runModalWhileEvents(cancelButton);
                }

                // Set labels for progress dialog
                FXString label = _("Source: ") + source;
                if (uplabel)
                {
                    uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                label = _("Target: ") + target;
                if (downlabel)
                {
                    downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                getApp()->repaint();

                // If cancel button was clicked, close files and return
                if (cancelled)
                {
                    ::close(src);
                    return false;
                }

                // Copy using gio command (regular copy does not work on MTP mounts)
                FXString cmd = "gio copy " + xf_quote(source) + " " + xf_quote(destfile);
                FXString ret = xf_getcommandoutput(cmd);

                // An error has occurred
                if (ret != "")
                {
                    forceTimeout();
                    FXString str;
                    str.format(_("Can't copy file %s: %s"), target.text(), ret.text());
                    mbox->setText(str);
                    FXuint answer = mbox->execute(PLACEMENT_OWNER);
                    if (answer == BOX_CLICKED_CANCEL)
                    {
                        cancelled = true;
                        ::close(src);
                        
                        return false;
                    }
                    else
                    {
                        ok = -2; // An error has occurred
                    }
                }
                else
                {
                    ok = true;
                }
            }
    
            // File is on a regular file system
            else
            {
                // Copy file block by block
                if ((dst = ::open(destfile.text(), O_WRONLY | O_CREAT | O_TRUNC, info.st_mode)) >= 0)
                {
                    int error = false;

                    while (1)
                    {
                        errno = 0;
                        nread = File::fullread(src, buffer, sizeof(buffer));

                        int errcode = errno;
                        if (nread < 0)
                        {
                            forceTimeout();

                            FXString str;
                            if (errcode)
                            {
                                str.format(_("Can't copy file %s: %s"), target.text(), strerror(errcode));
                            }
                            else
                            {
                                str.format(_("Can't copy file %s"), target.text());
                            }
                            mbox->setText(str);
                            FXuint answer = mbox->execute(PLACEMENT_OWNER);

                            restartTimeout();
                            if (answer == BOX_CLICKED_CANCEL)
                            {
                                ::close(dst);
                                ::close(src);
                                cancelled = true;
                                return false;
                            }
                            else
                            {
                                error = true; // An error has occurred
                            }
                        }
                        if (nread == 0)
                        {
                            break;
                        }

                        // Force timeout checking for progress dialog
                        checkTimeout();

                        // Set percentage value for progress dialog
                        totaldataread += nread;

                        if (progressbar)
                        {
                            // Percentage
                            int pct = (sourcesize == 0 ? 0 : (100.0 * totaldataread) / sourcesize);
                            progressbar->setProgress(pct);

                            char size[64];

                            // Copy speed
                            double copyspeed = totaldataread / (double)(xf_getcurrenttime() - tstart);
                            snprintf(size, sizeof(size), "%f", copyspeed);
                            FXString hspeed = xf_humansize(size);

                            // Remaining time in seconds
                            FXuint remtime = (FXuint)((sourcesize - totaldataread) / copyspeed);
                            FXString hremtime = xf_secondstotimestring(remtime);

                            // Total data copied
                            FXString hsize;


#if __WORDSIZE == 64
                            snprintf(size, sizeof(size), "%ld", totaldataread);
#else
                            snprintf(size, sizeof(size), "%lld", totaldataread);
#endif
                            hsize = xf_humansize(size);
                            snprintf(size, sizeof(size), "%s %s / %s (%s/s)", datatext.text(), hsize.text(),
                                     hsourcesize.text(), hspeed.text());
                            datalabel->setText(size);
                            snprintf(size, sizeof(size), "%s %s", timetext.text(), hremtime.text());
                            timelabel->setText(size);
                        }

                        // Give cancel button an opportunity to be clicked
                        if (cancelButton)
                        {
                            getApp()->runModalWhileEvents(cancelButton);
                        }

                        // Set labels for progress dialog
                        FXString label = _("Source: ") + source;
                        if (uplabel)
                        {
                            uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                        }
                        label = _("Target: ") + target;
                        if (downlabel)
                        {
                            downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                        }
                        getApp()->repaint();

                        // If cancel button was clicked, close files and return
                        if (cancelled)
                        {
                            ::close(dst);
                            ::close(src);
                            return false;
                        }
                        errno = 0;
                        nwritten = File::fullwrite(dst, buffer, nread);
                        errcode = errno;
                        if (nwritten < 0)
                        {
                            forceTimeout();

                            FXString str;
                            if (errcode)
                            {
                                str.format(_("Can't copy file %s: %s"), target.text(), strerror(errcode));
                            }
                            else
                            {
                                str.format(_("Can't copy file %s"), target.text());
                            }
                            mbox->setText(str);
                            FXuint answer = mbox->execute(PLACEMENT_OWNER);

                            restartTimeout();
                            if (answer == BOX_CLICKED_CANCEL)
                            {
                                ::close(dst);
                                ::close(src);
                                cancelled = true;
                                return false;
                            }
                            else
                            {
                                error = true; // An error has occurred
                            }
                        }
                    }

                    // An error has occurred during the copy
                    if (error)
                    {
                        ok = -2;
                    }
                    else
                    {
                        ok = true;
                    }

                    ::close(dst);

                    // Keep original date if asked
                    if (preserve_date)
                    {
                        timbuf.actime = info.st_atime;
                        timbuf.modtime = info.st_mtime;
                        errno = 0;
                        int rc = utime(destfile.text(), &timbuf);
                        int errcode = errno;
                        if (warn && rc)
                        {
                            forceTimeout();

                            FXString str;
                            if (errcode)
                            {
                                str.format(_("Can't preserve date when copying file %s : %s"), target.text(),
                                           strerror(errcode));
                            }
                            else
                            {
                                str.format(_("Can't preserve date when copying file %s"), target.text());
                            }
                            mbox->setText(str);
                            FXuint answer = mbox->execute(PLACEMENT_OWNER);

                            restartTimeout();
                            if (answer == BOX_CLICKED_CANCEL)
                            {
                                ::close(src);
                                cancelled = true;
                                return false;
                            }
                        }
                    }
                }

#if defined(linux)
                // If source file is on a ISO9660 file system (CD or DVD, thus read-only)
                // then add to the target the write permission for the user
                if (ok)
                {
                    struct statfs fs;
                    if ((statfs(source.text(), &fs) == 0) && (fs.f_type == 0x9660))
                    {
                        ::chmod(target.text(), info.st_mode | S_IWUSR);
                    }
                }
#endif
            }
        }
        ::close(src);
    }
    // File cannot be opened
    else
    {
        forceTimeout();
        int errcode = errno;

        FXString str;
        if (errcode)
        {
            str.format(_("Can't copy file %s: %s"), target.text(), strerror(errcode));
        }
        else
        {
            str.format(_("Can't copy file %s"), target.text());
        }
        mbox->setText(str);
        FXuint answer = mbox->execute(PLACEMENT_OWNER);

        restartTimeout();
        if (answer == BOX_CLICKED_CANCEL)
        {
            cancelled = true;
            return false;
        }
        ok = -1;        // Prevent displaying an error message
                        // in the calling function
    }
    return ok;
}


// Copy directory
int File::copydir(const FXString& source, const FXString& target, const FXString& hsourcesize, const FXulong sourcesize,
                  const FXulong tstart, struct stat& parentinfo, inodelist* inodes, const FXbool preserve_date)
{
    DIR* dirp;
    struct dirent* dp;
    struct stat linfo;
    struct utimbuf timbuf;
    inodelist* in, inode;
    FXString destfile, oldchild, newchild;

    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "preserve_date_warn", true);

    // Destination file
    destfile = target;

    // See if visited this inode already
    for (in = inodes; in; in = in->next)
    {
        if (in->st_ino == parentinfo.st_ino)
        {
            return true;
        }
    }

    // Try make directory, if none exists yet
    if ((::mkdir(destfile.text(), parentinfo.st_mode | S_IWUSR) != 0) && (errno != EEXIST))
    {
        return false;
    }

    // Can we stat it
    if ((xf_lstat(destfile.text(), &linfo) != 0) || !S_ISDIR(linfo.st_mode))
    {
        return false;
    }

    // Try open directory to copy
    dirp = opendir(source.text());
    if (!dirp)
    {
        return false;
    }

    // Add this to the list
    inode.st_ino = linfo.st_ino;
    inode.next = inodes;

    // Copy stuff
    while ((dp = readdir(dirp)) != NULL)
    {
        if ((dp->d_name[0] != '.') || ((dp->d_name[1] != '\0') && ((dp->d_name[1] != '.') || (dp->d_name[2] != '\0'))))
        {
            oldchild = source;
            if (!ISPATHSEP(oldchild[oldchild.length() - 1]))
            {
                oldchild.append(PATHSEP);
            }
            oldchild.append(dp->d_name);
            newchild = destfile;
            if (!ISPATHSEP(newchild[newchild.length() - 1]))
            {
                newchild.append(PATHSEP);
            }
            newchild.append(dp->d_name);
            if (!copyrec(oldchild, newchild, hsourcesize, sourcesize, tstart, &inode, preserve_date))
            {
                // If the cancel button was pressed
                if (cancelled)
                {
                    closedir(dirp);
                    return false;
                }
                // Or a permission problem occured
                else
                {
                    FXString str;
                    if (xf_isdirectory(oldchild))
                    {
                        str.format(_("Can't copy folder %s : Permission denied"), oldchild.text());
                    }
                    else
                    {
                        str.format(_("Can't copy file %s : Permission denied"), oldchild.text());
                    }
                    forceTimeout();
                    mbox->setText(str);
                    FXuint answer = mbox->execute(PLACEMENT_OWNER);

                    restartTimeout();
                    if (answer == BOX_CLICKED_CANCEL)
                    {
                        closedir(dirp);
                        cancelled = true;
                        return false;
                    }
                }
            }
        }
    }

    // Close directory
    closedir(dirp);

    // Keep original date if asked
    if (preserve_date && !target.contains("mtp:host="))  // Don't report error if MTP mount
    {
        if (xf_lstat(source.text(), &linfo) == 0)
        {
            timbuf.actime = linfo.st_atime;
            timbuf.modtime = linfo.st_mtime;
            errno = 0;
            int rc = utime(destfile.text(), &timbuf);
            int errcode = errno;
            if (warn && rc)
            {
                forceTimeout();
                FXString str;
                if (errcode)
                {
                    str.format(_("Can't preserve date when copying folder %s: %s"), target.text(), strerror(errcode));
                }
                else
                {
                    str.format(_("Can't preserve date when copying folder %s"), target.text());
                }
                mbox->setText(str);
                FXuint answer = mbox->execute(PLACEMENT_OWNER);

                restartTimeout();
                if (answer == BOX_CLICKED_CANCEL)
                {
                    cancelled = true;
                    return false;
                }
            }
        }
    }

    // Success
    return true;
}


// Recursive copy
int File::copyrec(const FXString& source, const FXString& target, const FXString& hsourcesize, const FXulong sourcesize,
                  const FXulong tstart, inodelist* inodes, const FXbool preserve_date)
{
    struct stat linfo1, linfo2;

    // Source file or directory does not exist
    if (xf_lstat(source.text(), &linfo1) != 0)
    {
        return false;
    }

    // If target is not a directory, remove it if allowed
    if (xf_lstat(target.text(), &linfo2) == 0)
    {
        if (!S_ISDIR(linfo2.st_mode))
        {
            if (!(overwrite | overwrite_all))
            {
                return false;
            }
            if (unlink(target.text()) != 0)
            {
                return false;
            }
        }
    }

    // Source is directory: copy recursively
    if (S_ISDIR(linfo1.st_mode))
    {
        return File::copydir(source, target, hsourcesize, sourcesize, tstart, linfo1, inodes, preserve_date);
    }

    // Source is regular file: copy block by block
    if (S_ISREG(linfo1.st_mode))
    {
        return File::copyfile(source, target, hsourcesize, sourcesize, tstart, preserve_date);
    }

    // Remove target if it already exists
    if (xf_existfile(target))
    {
        int ret = File::remove(target);
        if (!ret)
        {
            return false;
        }
    }

    // Source is fifo: make a new one
    if (S_ISFIFO(linfo1.st_mode))
    {
        return mkfifo(target.text(), linfo1.st_mode);
    }

    // Source is device: make a new one
    if (S_ISBLK(linfo1.st_mode) || S_ISCHR(linfo1.st_mode) || S_ISSOCK(linfo1.st_mode))
    {
        return mknod(target.text(), linfo1.st_mode, linfo1.st_rdev) == 0;
    }

    // Source is symbolic link: make a new one
    if (S_ISLNK(linfo1.st_mode))
    {
        FXString lnkfile = xf_readlink(source);
        return ::symlink(lnkfile.text(), target.text()) == 0;
    }

    // This shouldn't happen
    return false;
}


// Copy file (with progress dialog)
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
int File::copy(const FXString& source, const FXString& target, const FXString& hsourcesize, const FXulong sourcesize,
               const FXulong tstart, const FXbool confirm_dialog, const FXbool preserve_date)
{
    FXString targetfile;

    // Source doesn't exist
    if (!xf_existfile(source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Source %s doesn't exist"), source.text());
        return -1;
    }

    // Source and target are identical
    if (xf_isidentical(target, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), target.text());
        return -1;
    }

    // Source path is included into target path
    FXString str = source + PATHSEPSTRING;
    if (target.left(str.length()) == str)
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Target %s is a sub-folder of source"), target.text());
        return -1;
    }

    // Target is an existing directory
    if (xf_isdirectory(target))
    {
        targetfile = target + PATHSEPSTRING + FXPath::name(source);
    }
    else
    {
        targetfile = target;
    }

    // Source and target are identical => add a suffix to the name
    if (xf_isidentical(source, targetfile))
    {
        FXString pathname = xf_cleanpath(targetfile);
        targetfile = xf_buildcopyname(pathname, xf_isdirectory(pathname), copysuffix, copysuffix_pos); // Remove trailing / if any
    }
    // Source and target file are identical
    if (xf_isidentical(targetfile, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), targetfile.text());
        return -1;
    }

    // Target already exists
    if (xf_existfile(targetfile))
    {
        // Overwrite dialog if necessary
        if ((!(overwrite_all | skip_all)) & confirm_dialog)
        {
            FXString label = _("Source: ") + source;
            if (uplabel)
            {
                uplabel->setText(xf_multilines(label, MAX_MESSAGE_LENGTH));
            }
            label = _("Target: ") + targetfile;
            if (downlabel)
            {
                downlabel->setText(xf_multilines(label, MAX_MESSAGE_LENGTH));
            }
            getApp()->repaint();
            forceTimeout();
            FXuint answer = getOverwriteAnswer(source, targetfile);
            switch (answer)
            {
            // Cancel
            case 0:
                forceTimeout();
                cancelled = true;
                return false;

                break;

            // Overwrite
            case 1:
                overwrite = true;
                break;

            // Overwrite all
            case 2:
                overwrite_all = true;
                break;

            // Skip
            case 3:
                overwrite = false;
                break;

            // Skip all
            case 4:
                skip_all = true;
                break;
            }
        }
        if ((!(overwrite | overwrite_all)) | skip_all)
        {
            return true;
        }

        // Remove targetfile if source is not a directory
        if (!xf_isdirectory(source))
        {
            if (File::remove(targetfile) == false)
            {
                forceTimeout();
                return false;
            }
        }
    }

    // Copy file or directory
    return File::copyrec(source, targetfile, hsourcesize, sourcesize, tstart, NULL, preserve_date);
}


// Remove file or directory (with progress dialog)
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
int File::remove(const FXString& file)
{
    FXString dirname;
    struct stat linfo;
    static FXbool ISDIR = false; // Caution! ISDIR is common to all File instances, is that we want?

    if (xf_lstat(file.text(), &linfo) == 0)
    {
        // It is a directory
        if (S_ISDIR(linfo.st_mode))
        {
            DIR* dirp = opendir(file.text());
            if (dirp)
            {
                struct dirent* dp;
                FXString child;

                // Used to display only one progress dialog when deleting a directory
                ISDIR = true;

                // Force timeout checking for progress dialog
                checkTimeout();

                // Give cancel button an opportunity to be clicked
                if (cancelButton)
                {
                    getApp()->runModalWhileEvents(cancelButton);
                }

                // Set labels for progress dialog
                FXString label = _("Delete folder: ") + file;
                if (uplabel)
                {
                    uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                dirname = FXPath::directory(FXPath::absolute(file));
                label = _("From: ") + dirname;
                if (downlabel)
                {
                    downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                getApp()->repaint();

                // If cancel button was clicked, return
                if (cancelled)
                {
                    closedir(dirp);
                    return false;
                }

                while ((dp = readdir(dirp)) != NULL)
                {
                    if ((dp->d_name[0] != '.') ||
                        ((dp->d_name[1] != '\0') && ((dp->d_name[1] != '.') || (dp->d_name[2] != '\0'))))
                    {
                        child = file;
                        if (!ISPATHSEP(child[child.length() - 1]))
                        {
                            child.append(PATHSEP);
                        }
                        child.append(dp->d_name);
                        if (!File::remove(child))
                        {
                            closedir(dirp);
                            return false;
                        }
                    }
                }
                closedir(dirp);
            }
            if (rmdir(file.text()) == -1)
            {
                int errcode = errno;
                forceTimeout();

                FXString str;
                if (errcode)
                {
                    str.format(_("Can't delete folder %s: %s"), file.text(), strerror(errcode));
                }
                else
                {
                    str.format(_("Can't delete folder %s"), file.text());
                }
                mbox->setText(str);
                FXuint answer = mbox->execute(PLACEMENT_OWNER);

                restartTimeout();
                if (answer == BOX_CLICKED_CANCEL)
                {
                    cancelled = true;
                    return false;
                }
                return -1; // To prevent displaying an error message
                // in the calling function
            }
            else
            {
                return true;
            }
        }
        else
        {
            // If it was not a directory
            if (!ISDIR)
            {
                // Force timeout checking for progress dialog
                checkTimeout();

                // Give cancel button an opportunity to be clicked
                if (cancelButton)
                {
                    getApp()->runModalWhileEvents(cancelButton);
                }

                // Set labels for progress dialog
                FXString label = _("Delete:") + file;
                if (uplabel)
                {
                    uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                dirname = FXPath::directory(FXPath::absolute(file));
                label = _("From: ") + dirname;
                if (downlabel)
                {
                    downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
                }
                getApp()->repaint();

                // If cancel button was clicked, return
                if (cancelled)
                {
                    return false;
                }
            }
            if (unlink(file.text()) == -1)
            {
                int errcode = errno;
                forceTimeout();

                FXString str;
                if (errcode)
                {
                    str.format(_("Can't delete file %s: %s"), file.text(), strerror(errcode));
                }
                else
                {
                    str.format(_("Can't delete file %s"), file.text());
                }
                mbox->setText(str);
                FXuint answer = mbox->execute(PLACEMENT_OWNER);

                restartTimeout();
                if (answer == BOX_CLICKED_CANCEL)
                {
                    cancelled = true;
                    return false;
                }
                return -1;      // To prevent displaying an error message
                                // in the calling function
            }
            else
            {
                return true;
            }
        }
    }
    return -1;
}


// Rename a file or a directory (no progress dialog)
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
int File::rename(const FXString& source, const FXString& target)
{
    // Source doesn't exist
    if (!xf_existfile(source))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Source %s doesn't exist"), source.text());
        return -1;
    }

    // Source and target are identical
    if (xf_isidentical(target, source))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), target.text());
        return -1;
    }

    // Target already exists => only allow overwriting destination if both source and target are files
    if (xf_existfile(target))
    {
        // Source or target are a directory
        if (xf_isdirectory(source) || xf_isdirectory(target))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s already exists"), target.text());
            return -1;
        }
        // Source and target are files
        else
        {
            // Overwrite dialog if necessary
            if (!(overwrite_all | skip_all))
            {
                FXuint answer = getOverwriteAnswer(source, target, false); // Don't restart timeout
                switch (answer)
                {
                // Cancel
                case 0:
                    cancelled = true;
                    return false;

                    break;

                // Overwrite
                case 1:
                    overwrite = true;
                    break;

                // Overwrite all
                case 2:
                    overwrite_all = true;
                    break;

                // Skip
                case 3:
                    overwrite = false;
                    break;

                // Skip all
                case 4:
                    skip_all = true;
                    break;
                }
            }

            if ((!(overwrite | overwrite_all)) | skip_all)
            {
                return true;
            }
        }
    }

    // File is on an MTP mount
    if (source.contains("mtp:host="))
    {       
        // Rename using gio command (regular rename does not work on MTP mounts)
        FXString cmd = "cd " + xf_quote(FXPath::directory(source)) +  "; gio rename " 
                       + xf_quote(FXPath::name(source)) + " " + xf_quote(FXPath::name(target));       
        FXString ret = xf_getcommandoutput(cmd);

        // An error has occurred
        if (ret != "")
        {
            forceTimeout();
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't rename to target %s: %s"), target.text(),
                              ret.text());

            return -1;
        }
        else
        {
            return true;
        }
    }    
    
    // File is on a regular file system
    else
    {
        // Rename file using the standard C function
        // This should only work for files that are on the same file system
        if (::rename(source.text(), target.text()) == 0)
        {
            return true;
        }

        int errcode = errno;
        if ((errcode != EXDEV) && (errcode != ENOTEMPTY))
        {
            forceTimeout();
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't rename to target %s: %s"), target.text(),
                              strerror(errcode));
            return -1;
        }
    }

    return -1;
}


// Move files
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
int File::fmove(const FXString& source, const FXString& target, const FXString& hsourcesize, const FXulong sourcesize,
                const FXulong tstart, const FXbool restore)
{
    // Source doesn't exist
    if (!xf_existfile(source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Source %s doesn't exist"), source.text());
        return -1;
    }

    // Source and target are identical
    if (xf_isidentical(target, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), target.text());
        return -1;
    }

    // Source path is included into target path
    FXString str = source + PATHSEPSTRING;
    if (target.left(str.length()) == str)
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Target %s is a sub-folder of source"), target.text());
        return -1;
    }

    // Target is an existing directory (don't do this in the restore case)
    FXString targetfile;
    if (!restore && xf_isdirectory(target))
    {
        targetfile = target + PATHSEPSTRING + FXPath::name(source);
    }
    else
    {
        targetfile = target;
    }

    // Source and target file are identical
    if (xf_isidentical(targetfile, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), targetfile.text());
        return -1;
    }

    // Force timeout checking for progress dialog
    checkTimeout();

    // Give cancel button an opportunity to be clicked
    if (cancelButton)
    {
        getApp()->runModalWhileEvents(cancelButton);
    }

    // Set labels for progress dialog
    FXString label = _("Source: ") + source;
    if (uplabel)
    {
        uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
    }
    label = _("Target: ") + target;
    if (downlabel)
    {
        downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
    }
    getApp()->repaint();

    // Target file already exists
    if (xf_existfile(targetfile))
    {
        // Overwrite dialog if necessary
        if (!overwrite_all & !skip_all)
        {
            forceTimeout();
            FXuint answer = getOverwriteAnswer(source, targetfile);
            restartTimeout();
            switch (answer)
            {
            // Cancel
            case 0:
                forceTimeout();
                cancelled = true;
                return false;

                break;

            // Overwrite
            case 1:
                overwrite = true;
                break;

            // Overwrite all
            case 2:
                overwrite_all = true;
                break;

            // Skip
            case 3:
                overwrite = false;
                break;

            // Skip all
            case 4:
                skip_all = true;
                break;
            }
        }
        if ((!(overwrite | overwrite_all)) | skip_all)
        {
            // Hide progress dialog and restart timer
            forceTimeout();
            restartTimeout();

            return true;
        }
    }

    // Get the size of the source
    FXulong srcsize = 0;
    struct stat linfo;
    if (xf_lstat(source.text(), &linfo) == 0)
    {
        char buf[MAXPATHLEN];
        if (S_ISDIR(linfo.st_mode)) // Folder
        {
            FXuint nbfiles = 0, nbsubfolders = 0;
            FXulong totalsize = 0;
            xf_strlcpy(buf, source.text(), source.length() + 1);
            srcsize = xf_pathsize(buf, &nbfiles, &nbsubfolders, &totalsize);
            totaldataread += srcsize;
        }
        else // File
        {
            srcsize = (FXulong)linfo.st_size;
            totaldataread += srcsize;
        }
    }

    if (progressbar)
    {
        // Percentage
        int pct = (sourcesize == 0 ? 0 : (100.0 * totaldataread) / sourcesize);
        progressbar->setProgress((int)pct);

        char size[64];

        // Move speed
        double movespeed = totaldataread / (double)(xf_getcurrenttime() - tstart);
        snprintf(size, sizeof(size), "%f", movespeed);
        FXString hspeed = xf_humansize(size);

        // Remaining time in seconds
        FXuint remtime = (FXuint)((sourcesize - totaldataread) / movespeed);
        FXString hremtime = xf_secondstotimestring(remtime);

        // Total data moved
        FXString hsize;

#if __WORDSIZE == 64
        snprintf(size, sizeof(size), "%ld", totaldataread);
#else
        snprintf(size, sizeof(size), "%lld", totaldataread);
#endif
        hsize = xf_humansize(size);

        snprintf(size, sizeof(size), "%s %s / %s (%s/s)", datatext.text(), hsize.text(), hsourcesize.text(),
                 hspeed.text());
        datalabel->setText(size);
        snprintf(size, sizeof(size), "%s %s", timetext.text(), hremtime.text());
        timelabel->setText(size);
    }

    // Rename file using the standard C function
    // This should only work for files that are on the same file system
    if (::rename(source.text(), targetfile.text()) == 0)
    {
        return true;
    }

    int errcode = errno;
    if ((errcode != EXDEV) && (errcode != ENOTEMPTY))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't rename to target %s: %s"), targetfile.text(),
                          strerror(errcode));
        return -1;
    }

    // If files are on different file systems, use the copy/delete scheme and preserve the original date
    totaldataread -= srcsize; // Avoid counting data twice

    int ret = this->copy(source, target, hsourcesize, sourcesize, tstart, false, true);

    // Success
    if (ret == true)
    {
        return remove(source.text()) == true;
    }
    // Error during copy
    else if (ret == -2)
    {
        return true;
    }
    // Operation cancelled
    else
    {
        return false;
    }
}


// Symbolic Link file (no progress dialog)
// Return  0 to allow displaying an error message in the calling function
// Return -1 to prevent displaying an error message in the calling function
int File::symlink(const FXString& source, const FXString& target)
{
    // Source doesn't exist
    if (!xf_existfile(source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Source %s doesn't exist"), source.text());
        return -1;
    }

    // Source and target are identical
    if (xf_isidentical(target, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), target.text());
        return -1;
    }

    // Target is an existing directory
    FXString targetfile;
    if (xf_isdirectory(target))
    {
        targetfile = target + PATHSEPSTRING + FXPath::name(source);
    }
    else
    {
        targetfile = target;
    }

    // Source and target are identical
    if (xf_isidentical(targetfile, source))
    {
        forceTimeout();
        MessageBox::error(this, BOX_OK, _("Error"), _("Destination %s is identical to source"), targetfile.text());
        return -1;
    }

    // Target already exists
    if (xf_existfile(targetfile))
    {
        // Overwrite dialog if necessary
        if (!(overwrite_all | skip_all))
        {
            FXuint answer = getOverwriteAnswer(source, targetfile);
            switch (answer)
            {
            // Cancel
            case 0:
                forceTimeout();
                return false;

                break;

            // Overwrite
            case 1:
                overwrite = true;
                break;

            // Overwrite all
            case 2:
                overwrite_all = true;
                break;

            // Skip
            case 3:
                overwrite = false;
                break;

            // Skip all
            case 4:
                skip_all = true;
                break;
            }
        }
        if ((!(overwrite | overwrite_all)) | skip_all)
        {
            return true;
        }
    }

    // Create symbolic link using the standard C function
    errno = 0;

    // Use the relative path for the symbolic link
    FXString relativepath;
    if (xf_existfile(target) && xf_isdirectory(target))
    {
        relativepath = FXPath::relative(target, source);
    }
    else
    {
        relativepath = FXPath::relative(FXPath::directory(target), source);
    }

    int ret = ::symlink(relativepath.text(), targetfile.text());

    int errcode = errno;
    if (ret == 0)
    {
        return true;
    }
    else
    {
        forceTimeout();
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't symlink %s: %s"), target.text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't symlink %s"), target.text());
        }
        return -1;
    }
}


// Chmod a file or directory, recursively or not
// We don't process symbolic links (since their permissions cannot be changed)
//
// Note : the variable file returns the last processed file
// It can be different from the initial path, if recursive chmod is used
// (Used to fill an error message, if needed)
int File::chmod(char* path, char* file, mode_t mode, FXbool rec, const FXbool dironly, const FXbool fileonly)
{
    struct stat linfo;

    // Initialize the file variable with the initial path
    xf_strlcpy(file, path, strlen(path) + 1);

    // If it doesn't exist
    if (xf_lstat(path, &linfo))
    {
        return -1;
    }

    // If it's a symbolic link
    if (S_ISLNK(linfo.st_mode))
    {
        return 0;
    }

    if (!S_ISDIR(linfo.st_mode)) // File
    {
        if (dironly)
        {
            return 0;
        }

        // Force timeout checking for progress dialog
        checkTimeout();

        // Give cancel button an opportunity to be clicked
        if (cancelButton)
        {
            getApp()->runModalWhileEvents(cancelButton);
        }

        // Set labels for progress dialog
        FXString label = _("Changing permissions...");
        uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        label = _("File: ") + FXString(path);
        downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        getApp()->repaint();

        // If cancel button was clicked, return
        if (cancelled)
        {
            return -1;
        }

        return ::chmod(path, mode);
    }
    else // Directory
    {
        if ((rec == false) && !fileonly)
        {
            // Force timeout checking for progress dialog
            checkTimeout();

            // Give cancel button an opportunity to be clicked
            if (cancelButton)
            {
                getApp()->runModalWhileEvents(cancelButton);
            }

            // Set labels for progress dialog
            FXString label = _("Changing permissions...");
            uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
            label = _("Folder: ") + FXString(path);
            downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
            getApp()->repaint();

            // If cancel button was clicked, return
            if (cancelled)
            {
                return -1;
            }

            if (::chmod(path, mode)) // Do not change recursively
            {
                return -1;
            }
        }
        else
        {
            return rchmod(path, file, mode, dironly, fileonly); // Recursive change
        }
    }
    return 0;
}


// Recursive chmod for a directory
// We don't process symbolic links (since their permissions cannot be changed)
int File::rchmod(char* path, char* file, mode_t mode, const FXbool dironly, const FXbool fileonly)
{
    struct stat linfo;

    // Initialize the file variable with the initial path
    xf_strlcpy(file, path, strlen(path) + 1);

    // If it doesn't exist
    if (xf_lstat(path, &linfo))
    {
        return -1;
    }

    // If it's a symbolic link
    if (S_ISLNK(linfo.st_mode))
    {
        return 0;
    }

    if (!S_ISDIR(linfo.st_mode)) // File
    {
        if (dironly)
        {
            return 0;
        }

        // Force timeout checking for progress dialog
        checkTimeout();

        // Give cancel button an opportunity to be clicked
        if (cancelButton)
        {
            getApp()->runModalWhileEvents(cancelButton);
        }

        // Set labels for progress dialog
        FXString label = _("Changing permissions...");
        uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        label = _("File: ") + FXString(path);
        downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        getApp()->repaint();

        // If cancel button was clicked, return
        if (cancelled)
        {
            return -1;
        }

        return ::chmod(path, mode);
    }

    DIR* dir;
    struct dirent* entry;
    int pl = strlen(path);

    if (!(dir = opendir(path)))
    {
        return -1;
    }

    for (; (entry = readdir(dir)); )
    {
        if ((entry->d_name[0] != '.') || ((entry->d_name[1] != '\0') &&
                                          ((entry->d_name[1] != '.') ||
                                           (entry->d_name[2] != '\0'))))
        {
            int pl1 = pl, l = strlen(entry->d_name);
            char* path1 = (char*)alloca(pl1 + l + 2);

            xf_strlcpy(path1, path, strlen(path) + 1);
            if (path1[pl1 - 1] != '/')
            {
                path1[pl1++] = '/';
            }
            xf_strlcpy(path1 + pl1, entry->d_name, strlen(entry->d_name) + 1);

            // Modify the file variable with the new path
            xf_strlcpy(file, path1, strlen(path1) + 1);
            if (rchmod(path1, file, mode, dironly, fileonly))
            {
                closedir(dir);
                return -1;
            }
        }
    }

    if (closedir(dir))
    {
        return -1;
    }

    if (fileonly)
    {
        return 0;
    }
    else
    {
        return ::chmod(path, mode);
    }
}


// Chown a file or directory, recursively or not
// We don't follow symbolic links
//
// Note : the variable file returns the last processed file
// It can be different from the initial path, if recursive chmod is used
// (Used to fill an error message, if needed)
int File::chown(char* path, char* file, uid_t uid, gid_t gid, const FXbool rec,
                const FXbool dironly, const FXbool fileonly)
{
    struct stat linfo;

    // Initialise the file variable with the initial path
    xf_strlcpy(file, path, strlen(path) + 1);

    // If it doesn't exist
    if (xf_lstat(path, &linfo))
    {
        return -1;
    }

    if (!S_ISDIR(linfo.st_mode)) // File
    {
        if (dironly)
        {
            return 0;
        }

        // Force timeout checking for progress dialog
        checkTimeout();

        // Give cancel button an opportunity to be clicked
        if (cancelButton)
        {
            getApp()->runModalWhileEvents(cancelButton);
        }

        // Set labels for progress dialog
        FXString label = _("Changing owner...");
        uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        label = _("File: ") + FXString(path);
        downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        getApp()->repaint();

        // If cancel button was clicked, return
        if (cancelled)
        {
            return -1;
        }

        if (::lchown(path, uid, gid))
        {
            return -1;
        }
    }
    else // Directory
    {
        if ((rec == false) && !fileonly)
        {
            // Force timeout checking for progress dialog
            checkTimeout();

            // Give cancel button an opportunity to be clicked
            if (cancelButton)
            {
                getApp()->runModalWhileEvents(cancelButton);
            }

            // Set labels for progress dialog
            FXString label = _("Changing owner...");
            uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
            label = _("Folder: ") + FXString(path);
            downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
            getApp()->repaint();

            // If cancel button was clicked, return
            if (cancelled)
            {
                return -1;
            }

            if (::lchown(path, uid, gid)) // Do not change recursively
            {
                return -1;
            }
        }
        else if (rchown(path, file, uid, gid, dironly, fileonly)) // Recursive change
        {
            return -1;
        }
    }
    return 0;
}


// Recursive chown for a directory
// We don't follow symbolic links
int File::rchown(char* path, char* file, uid_t uid, gid_t gid, const FXbool dironly, const FXbool fileonly)
{
    struct stat linfo;

    // Initialise the file variable with the initial path
    xf_strlcpy(file, path, strlen(path) + 1);

    // If it doesn't exist
    if (xf_lstat(path, &linfo))
    {
        return -1;
    }

    if (!S_ISDIR(linfo.st_mode)) // file
    {
        if (dironly)
        {
            return 0;
        }

        // Force timeout checking for progress dialog
        checkTimeout();

        // Give cancel button an opportunity to be clicked
        if (cancelButton)
        {
            getApp()->runModalWhileEvents(cancelButton);
        }

        // Set labels for progress dialog
        FXString label = _("Changing owner...");
        uplabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        label = _("File: ") + FXString(path);
        downlabel->setText(xf_truncline(label, MAX_MESSAGE_LENGTH));
        getApp()->repaint();

        // If cancel button was clicked, return
        if (cancelled)
        {
            return -1;
        }

        return lchown(path, uid, gid);
    }

    DIR* dir;
    struct dirent* entry;
    int pl = strlen(path);

    if (!(dir = opendir(path)))
    {
        return -1;
    }

    for ( ; (entry = readdir(dir)); )
    {
        if ((entry->d_name[0] != '.') || ((entry->d_name[1] != '\0') &&
                                          ((entry->d_name[1] != '.') ||
                                           (entry->d_name[2] != '\0'))))
        {
            int pl1 = pl, l = strlen(entry->d_name);
            char* path1 = (char*)alloca(pl1 + l + 2);

            xf_strlcpy(path1, path, strlen(path) + 1);
            if (path1[pl1 - 1] != '/')
            {
                path1[pl1++] = '/';
            }
            xf_strlcpy(path1 + pl1, entry->d_name, strlen(entry->d_name) + 1);
            xf_strlcpy(file, path1, strlen(path1) + 1);
            if (rchown(path1, file, uid, gid, dironly, fileonly))
            {
                closedir(dir);
                return -1;
            }
        }
    }

    if (closedir(dir))
    {
        return -1;
    }

    if (fileonly)
    {
        return 0;
    }
    else
    {
        return lchown(path, uid, gid);
    }
}


// Extract an archive in a specified directory
int File::extract(const FXString name, const FXString dir, const FXString cmd)
{
    int ret;

    // Change to the specified directory
    FXString currentdir = FXSystem::getCurrentDirectory();

    ret = chdir(dir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), dir.text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), dir.text());
        }

        return 0;
    }

    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Extract archive"), cmd, 30, 80);
    cmdwin->create();
    cmdwin->setIcon(miniarchexticon);

    // The command window object deletes itself after closing the window!

    // Return to initial directory
    ret = chdir(currentdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), currentdir.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), currentdir.text());
        }

        return 0;
    }

    return 1;
}


// Create an archive
int File::archive(const FXString name, const FXString cmd)
{
    // Target file already exists
    if (xf_existfile(FXPath::dequote(name)))
    {
        FXString msg;
        msg.format(_("File %s already exists.\nOverwrite?"), name.text());
        OverwriteBox* dlg = new OverwriteBox(ownerwin, _("Confirm Overwrite"), msg, OVWBOX_SINGLE_FILE);
        FXuint answer = dlg->execute(PLACEMENT_OWNER);
        delete dlg;
        if (answer == 0)
        {
            return false;
        }
    }

    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Add to Archive "), cmd, 30, 80);
    cmdwin->create();
    cmdwin->setIcon(miniarchaddicon);

    // The command window object deletes itself after closing the window!

    return 1;
}


// Compute data size and show progress dialog
FXString File::sourcesize(const FXString src, FXulong* totalsize, const FXbool url)
{
    // Set label
    FXString msg;

    if (op == COPY)
    {
        msg = _("Computing the size of files to copy: ");
    }
    else if (op == MOVE)
    {
        msg = _("Computing the size of files to move: ");
    }
    else // Should not happen
    {
        msg = "";
    }
    uplabel->setText(msg);

    // Compute data size
    *totalsize = 0;
    char buf[MAXPATHLEN];

    for (int i = 0; ; i++)
    {
        // Force timeout checking for progress dialog
        checkTimeout();

        FXString src_i;

        if (url)
        {
            char* token;
            (i == 0 ? token = strtok((char*)src.text(), "\r\n") : token = strtok(NULL, "\r\n"));

            if (token == NULL)
            {
                break;
            }

            src_i = FXURL::decode(FXURL::fileFromURL(token));
        }
        else
        {
            char* token;

            (i == 0 ? token = strtok((char*)src.text(), "\n") : token = strtok(NULL, "\n"));

            if (token == NULL)
            {
                break;
            }

            src_i = FXString(token);
        }

        FXuint nbfiles = 0, nbsubfolders = 0;
        FXulong size_i = 0;

        xf_strlcpy(buf, src_i.text(), src_i.length() + 1);
        computeSourceSize(this, getApp(), buf, &nbfiles, &nbsubfolders, &size_i, cancelButton, &cancelled);

        // Give cancel button an opportunity to be clicked
        if (cancelButton)
        {
            getApp()->runModalWhileEvents(cancelButton);
        }

        // If cancel button was clicked, return
        if (cancelled)
        {
            return "";
        }

        *totalsize += size_i;
    }

#if __WORDSIZE == 64
    snprintf(buf, sizeof(buf), "%lu", *totalsize);
#else
    snprintf(buf, sizeof(buf), "%llu", *totalsize);
#endif

    FXString hsourcesize = xf_humansize(buf);

    return hsourcesize;
}


#if defined(linux)
int File::mount(const FXString dir, const FXString msg, const FXString cmd)
{
    FXbool mount_messages = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_messages", true);

    // Set labels for progress dialog
    uplabel->setText(msg);
    downlabel->setText(dir.text());

    // Disable signal catch (required otherwise pclose() will fail)
    getApp()->removeSignal(SIGCHLD);

    // Perform mount/unmount command
    FILE* pcmd = popen(cmd.text(), "r");
    if (!pcmd)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Failed command: %s"), cmd.text());
        return -1;
    }

    // Get file descriptor and set it to non-blocking
    int fd = fileno(pcmd);
    if (fd < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), "%s", strerror(errno));
        pclose(pcmd);
        return -1;        
    }
    int ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), "%s", strerror(errno));
        pclose(pcmd);
        return -1;       
    }

    // Get error message if any
    char text[256] = { 0 };
    int process_finished = 0;
    FXString buf;
    while (!process_finished)
    {
        if (fgets(text, sizeof(text), pcmd) != NULL)
        {
            buf += text;
        }
        else
        {
            if (feof(pcmd))
            {
                process_finished = 1;  // Command has completed
            }
            else
            {
                checkTimeout();
                getApp()->repaint();
                sleep(SHOW_PROGRESSBAR_DELAY / 1000);  // Avoid busy-waiting
            }
        }
    }

    // Close process and get exit status
    int exit_status = pclose(pcmd);

    // Enable signal catch
    getApp()->addSignal(SIGINT, mainWindow, XFileExplorer::ID_QUIT);

    // Hide progress dialog
    forceTimeout();

    // An error has occurred
    if (exit_status == -1 || buf.contains("gio: not found"))
    {
        MessageBox::error(this, BOX_OK, _("Error"), "%s", buf.text());
        return -1;
    }

    // Success message
    if (mount_messages)
    {
        if (op == MOUNT)
        {
            MessageBox::information(this, BOX_OK, _("Success"), _("Folder %s was successfully mounted."), dir.text());
        }
        else
        {
            MessageBox::information(this, BOX_OK, _("Success"), _("Folder %s was successfully unmounted."), dir.text());
        }
    }
    return 1;
}


// Install / Upgrade package
int File::pkgInstall(const FXString name, const FXString cmd)
{
    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Install / Upgrade Package"), cmd, 10, 80);

    cmdwin->create();

    FXString msg;
    msg.format(_("Installing package: %s \n"), name.text());
    cmdwin->appendText(msg.text());

    // The command window object deletes itself after closing the window!

    return 1;
}


// Uninstall package
int File::pkgUninstall(const FXString name, const FXString cmd)
{
    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Uninstall Package"), cmd, 10, 80);

    cmdwin->create();

    FXString msg;
    msg.format(_("Uninstalling package: %s \n"), name.text());
    cmdwin->appendText(msg.text());

    // The command window object deletes itself after closing the window!

    return 1;
}

#endif


// Handle cancel button in progress bar dialog
long File::onCmdCancel(FXObject*, FXSelector, void*)
{
    cancelled = true;
    return 1;
}


// Handle timeout for progress bar
long File::onTimeout(FXObject*, FXSelector, void*)
{
    show(PLACEMENT_OWNER);
    getApp()->forceRefresh();
    getApp()->flush();
    return 1;
}


// Handle timeout for source size refresh
long File::onSourceSizeRefresh(FXObject*, FXSelector, void*)
{
    char buf[MAXPATHLEN];

#if __WORDSIZE == 64
    snprintf(buf, sizeof(buf), "%lu", totalsourcesize);
#else
    snprintf(buf, sizeof(buf), "%llu", totalsourcesize);
#endif

    FXString srcsize = xf_humansize(buf);

    FXString msg;
    if (op == COPY)
    {
        msg = _("Computing the size of files to copy: ");
    }
    else if (op == MOVE)
    {
        msg = _("Computing the size of files to move: ");
    }
    else // Should not happen
    {
        msg = "";
    }

    // Update label text
    uplabel->setText(msg + srcsize);

    // Restart timeout
    getApp()->addTimeout(this, File::ID_SOURCESIZE, SOURCESIZE_REFRESH_DELAY);

    return 1;
}
