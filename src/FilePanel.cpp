#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "startupnotification.h"
#include "FileDialog.h"
#include "FileList.h"
#include "Places.h"
#include "Properties.h"
#include "XFileExplorer.h"
#include "InputDialog.h"
#include "BrowseInputDialog.h"
#include "ArchInputDialog.h"
#include "HistInputDialog.h"
#include "RenameDialog.h"
#include "File.h"
#include "MessageBox.h"
#include "OverwriteBox.h"
#include "CommandWindow.h"
#include "ExecuteBox.h"
#include "PathLinker.h"
#include "FilePanel.h"


// Duration (in ms) before we can stop refreshing the file list
// Used for file operations on a large list of files
#define STOP_LIST_REFRESH_INTERVAL    5000

// Number of files before stopping the file list refresh
#define STOP_LIST_REFRESH_NBMAX       100


// Clipboard notes :
// The uri-list type used for Xfe is the same as the Gnome uri-list type
// The standard uri-list type is used for KDE and non Gnome / XFCE file managers
// A special uri-list type that contains only "0" (for copy) or "1" (for cut) is used for KDE compatibility


// Global Variables
extern FXMainWindow* mainWindow;
extern FXString homedir;
extern FXString xdgdatahome;

// Clipboard
extern FXString clipboard;
FXuint clipboard_type = 0;

// Open history
extern char OpenHistory[OPEN_HIST_SIZE][MAX_COMMAND_SIZE];
extern int OpenNum;

// Filter history
extern char FilterHistory[FILTER_HIST_SIZE][MAX_PATTERN_SIZE];
extern int FilterNum;

#if defined(linux)
extern FXStringDict* fsdevices;
extern FXStringDict* mtdevices;
#endif


// Map
FXDEFMAP(FilePanel) FilePanelMap[] =
{
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, FilePanel::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, FilePanel::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, FilePanel::onClipboardRequest),
    FXMAPFUNC(SEL_TIMEOUT, FilePanel::ID_STOP_LIST_REFRESH_TIMER, FilePanel::onCmdStopListRefreshTimer),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_DIRECTORY_UP, FilePanel::onCmdDirectoryUp),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILTER, FilePanel::onCmdItemFilter),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILTER_CURRENT, FilePanel::onCmdItemFilter),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_GO_HOME, FilePanel::onCmdGoHome),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_GO_TRASH, FilePanel::onCmdGoTrash),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_VIEW, FilePanel::onCmdEdit),
    FXMAPFUNC(SEL_MIDDLEBUTTONPRESS, FilePanel::ID_FILELIST, FilePanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_EDIT, FilePanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_COMPARE, FilePanel::onCmdCompare),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_PROPERTIES, FilePanel::onCmdProperties),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_XTERM, FilePanel::onCmdXTerm),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_NEW_DIR, FilePanel::onCmdNewDir),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_NEW_FILE, FilePanel::onCmdNewFile),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_NEW_SYMLINK, FilePanel::onCmdNewSymlink),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_ADD_BOOKMARK, FilePanel::onCmdAddBookmark),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_COPY, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_CUT, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_COPYTO, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_MOVETO, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_RENAME, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_SYMLINK, FilePanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_COPY_CLIPBOARD, FilePanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_COPYNAME_CLIPBOARD, FilePanel::onCmdCopyName),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_CUT_CLIPBOARD, FilePanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_ADDCOPY_CLIPBOARD, FilePanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_ADDCUT_CLIPBOARD, FilePanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_PASTE_CLIPBOARD, FilePanel::onCmdPaste),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_TRASH, FilePanel::onCmdFileTrash),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_RESTORE, FilePanel::onCmdFileRestore),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_FILE_DELETE, FilePanel::onCmdFileDelete),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_OPEN_WITH, FilePanel::onCmdOpenWith),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_OPEN, FilePanel::onCmdOpen),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_REFRESH, FilePanel::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_SHOW_BIG_ICONS, FilePanel::onCmdShow),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_SHOW_MINI_ICONS, FilePanel::onCmdShow),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_SHOW_DETAILS, FilePanel::onCmdShow),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_TOGGLE_HIDDEN, FilePanel::onCmdToggleHidden),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_TOGGLE_THUMBNAILS, FilePanel::onCmdToggleThumbnails),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_SELECT_ALL, FilePanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_DESELECT_ALL, FilePanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_SELECT_INVERSE, FilePanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_ADD_TO_ARCH, FilePanel::onCmdAddToArch),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_EXTRACT, FilePanel::onCmdExtract),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_EXTRACT_TO_FOLDER, FilePanel::onCmdExtractToFolder),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_EXTRACT_HERE, FilePanel::onCmdExtractHere),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_RUN_SCRIPT, FilePanel::onCmdRunScript),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_RUN_SCRIPT, FilePanel::onUpdRunScript),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_GO_SCRIPTDIR, FilePanel::onCmdGoScriptDir),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_DIR_USAGE, FilePanel::onCmdDirUsage),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, FilePanel::ID_FILELIST, FilePanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_POPUP_MENU, FilePanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_DOUBLECLICKED, FilePanel::ID_FILELIST, FilePanel::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, FilePanel::ID_KEY_RETURN, FilePanel::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_KEYPRESS, 0, FilePanel::onKeyPress),
    FXMAPFUNC(SEL_CLICKED, FilePanel::ID_FILELIST, FilePanel::onCmdItemClicked),
    FXMAPFUNC(SEL_FOCUSIN, FilePanel::ID_FILELIST, FilePanel::onCmdFocus),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_STATUS, FilePanel::onUpdStatus),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_DIRECTORY_UP, FilePanel::onUpdUp),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_COPY_CLIPBOARD, FilePanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_CUT_CLIPBOARD, FilePanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_PASTE_CLIPBOARD, FilePanel::onUpdPaste),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_PROPERTIES, FilePanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_FILE_TRASH, FilePanel::onUpdFileTrash),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_FILE_RESTORE, FilePanel::onUpdFileRestore),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_GO_TRASH, FilePanel::onUpdGoTrash),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_FILE_DELETE, FilePanel::onUpdFileDelete),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_FILE_MOVETO, FilePanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_FILE_COPYTO, FilePanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_COMPARE, FilePanel::onUpdCompare),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_EDIT, FilePanel::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_VIEW, FilePanel::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_OPEN, FilePanel::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_ADD_TO_ARCH, FilePanel::onUpdAddToArch),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_SHOW_BIG_ICONS, FilePanel::onUpdShow),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_SHOW_MINI_ICONS, FilePanel::onUpdShow),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_SHOW_DETAILS, FilePanel::onUpdShow),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_TOGGLE_HIDDEN, FilePanel::onUpdToggleHidden),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_TOGGLE_THUMBNAILS, FilePanel::onUpdToggleThumbnails),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_DIR_USAGE, FilePanel::onUpdDirUsage),
#if defined(linux)
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_MOUNT, FilePanel::onCmdMount),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_UMOUNT, FilePanel::onCmdMount),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_MOUNT, FilePanel::onUpdMount),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_UMOUNT, FilePanel::onUpdUnmount),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_PKG_QUERY, FilePanel::onCmdPkgQuery),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_PKG_INSTALL, FilePanel::onCmdPkgInstall),
    FXMAPFUNC(SEL_COMMAND, FilePanel::ID_PKG_UNINSTALL, FilePanel::onCmdPkgUninstall),
    FXMAPFUNC(SEL_UPDATE, FilePanel::ID_PKG_QUERY, FilePanel::onUpdPkgQuery),
#endif
};


// Object implementation
FXIMPLEMENT(FilePanel, FXVerticalFrame, FilePanelMap, ARRAYNUMBER(FilePanelMap))


// Construct
FilePanel::FilePanel(FXWindow* owner, const char* nm, FXComposite* p, DirPanel* dp, FXuint* ic, FXuint nc,
                     FXuint name_size, FXuint size_size, FXuint type_size, FXuint ext_size, FXuint date_size,
                     FXuint user_size, FXuint group_size, FXuint perms_size, FXuint link_size, FXuint deldate_size,
                     FXuint origpath_size, FXbool showthumbs, FXColor listbackcolor, FXColor listforecolor,
                     FXColor attentioncolor, FXbool smoothscroll, FXuint opts, int x, int y, int w, int h) :
    FXVerticalFrame(p, opts, x, y, w, h, 0, 0, 0, 0)
{
    nbCols = nc;
    name = nm;
    dirpanel = dp;
    attenclr = attentioncolor;

    // Global container
    FXVerticalFrame* cont = new FXVerticalFrame(this, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 1);

    // Container for the path linker
    FXHorizontalFrame* pathframe = new FXHorizontalFrame(cont, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE,
                                                         0, 0, 0, 0, 0, 0, 2, 2);

    // File list

    // Smooth scrolling
    FXuint options;
    if (smoothscroll)
    {
        options = LAYOUT_FILL_X | LAYOUT_FILL_Y | ICONLIST_MINI_ICONS;
    }
    else
    {
        options = LAYOUT_FILL_X | LAYOUT_FILL_Y | ICONLIST_MINI_ICONS | SCROLLERS_DONT_TRACK;
    }

    FXVerticalFrame* cont2 = new FXVerticalFrame(cont, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE,
                                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    list = new FileList(owner, cont2, ic, nc, this, ID_FILELIST, showthumbs, deldate_size, origpath_size,
                        options);

    // Set list headers size
    for (FXuint i = 0; i < nbCols; i++)
    {
        FXuint size = 0;

        FXuint id = list->getHeaderId(i);

        switch (id)
        {
        case FileList::ID_COL_NAME:
            size = name_size;
            break;

        case FileList::ID_COL_SIZE:
            size = size_size;
            break;

        case FileList::ID_COL_TYPE:
            size = type_size;
            break;

        case FileList::ID_COL_EXT:
            size = ext_size;
            break;

        case FileList::ID_COL_DATE:
            size = date_size;
            break;

        case FileList::ID_COL_USER:
            size = user_size;
            break;

        case FileList::ID_COL_GROUP:
            size = group_size;
            break;

        case FileList::ID_COL_PERMS:
            size = perms_size;
            break;

        case FileList::ID_COL_LINK:
            size = link_size;
            break;
        }

        list->setHeaderSize(i, size);
    }

    list->setTextColor(listforecolor);
    list->setBackColor(listbackcolor);

    // Visually indicate if the panel is active
    activeicon = new FXButton(pathframe, "", minigraybuttonicon, this, FilePanel::ID_FILELIST, BUTTON_TOOLBAR |
                              JUSTIFY_LEFT | LAYOUT_LEFT);

    // Path text
    pathtext = new TextLabel(pathframe, 0, this, ID_FILELIST, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    pathtext->setBackColor(getApp()->getBaseColor());

    // Path linker
    pathlink = new PathLinker(pathframe, list, dirpanel->getList(), JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X);

    // Status bar
    statusbar = new FXHorizontalFrame(cont, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 3, 3,
                                      3, 3);

    statusbar->setTarget(this);
    statusbar->setSelector(FXSEL(SEL_UPDATE, FilePanel::ID_STATUS));

    FXString key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    new FXToggleButton(statusbar, TAB + _("Show Hidden Files") + PARS(key), TAB + _("Hide Hidden Files") + PARS(key),
                       minishowhiddenicon, minihidehiddenicon, this->list,
                       FileList::ID_TOGGLE_HIDDEN, TOGGLEBUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    new FXToggleButton(statusbar, TAB + _("Show Thumbnails") + PARS(key), TAB + _("Hide Thumbnails") + PARS(key),
                       minishowthumbicon, minihidethumbicon, this->list,
                       FileList::ID_TOGGLE_THUMBNAILS,
                       TOGGLEBUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "filter", "Ctrl-D");
    new FXButton(statusbar, TAB + _("Filter") + PARS(key), minifiltericon, this,
                 FilePanel::ID_FILTER, BUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);

    FXHorizontalFrame* hframe = new FXHorizontalFrame(statusbar, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE,
                                                      0, 0, 0, 0, 0, 0, 0, 0);
    filterlabel = new FXLabel(hframe, "", NULL, JUSTIFY_LEFT | LAYOUT_LEFT);
    closefilter = new FXButton(hframe, TAB + _("Close Filter"), miniclosefiltericon, this->list,
                               FileList::ID_CLOSE_FILTER,
                               JUSTIFY_LEFT | LAYOUT_LEFT | BUTTON_TOOLBAR | ICON_BEFORE_TEXT | FRAME_NONE);

    statuslabel = new FXLabel(hframe, _("Status"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT);

    corner = new FXDragCorner(statusbar);

    // Panel separator
    panelsep = new FXHorizontalSeparator(cont, SEPARATOR_GROOVE | LAYOUT_FILL_X);

    // Initializations
    selmult = false;
    current = NULL;

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }

    // Home and trahscan locations
    trashlocation = xdgdatahome + PATHSEPSTRING TRASHPATH;
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;

    // Start location (we return to the start location after each chdir)
    startlocation = FXSystem::getCurrentDirectory();

    // Initialize clipboard flags
    clipboard_locked = false;
    fromPaste = false;

    // Initialize control flag for right click popup menu
    ctrl = false;

    // Initialize the Shift-F10 flag
    shiftf10 = false;

    // Initialize the active panel flag
    isactive = false;

    // Default programs identifiers
    progs["<txtviewer>"] = TXTVIEWER;
    progs["<txteditor>"] = TXTEDITOR;
    progs["<imgviewer>"] = IMGVIEWER;
    progs["<imgeditor>"] = IMGEDITOR;
    progs["<pdfviewer>"] = PDFVIEWER;
    progs["<audioplayer>"] = AUDIOPLAYER;
    progs["<videoplayer>"] = VIDEOPLAYER;
    progs["<archiver>"] = ARCHIVER;
}


// Create X window
void FilePanel::create()
{
    // Register standard uri-list type
    urilistType = getApp()->registerDragType("text/uri-list");

    // Register special uri-list type used for Gnome, XFCE and Xfe
    xfelistType = getApp()->registerDragType("x-special/gnome-copied-files");

    // Register special uri-list type used for KDE
    kdelistType = getApp()->registerDragType("application/x-kde-cutselection");

    // Register standard UTF-8 text type used for file dialogs
    utf8Type = getApp()->registerDragType("UTF8_STRING");

    // Display or hide path linker
    FXbool show_pathlink = getApp()->reg().readUnsignedEntry("SETTINGS", "show_pathlinker", true);
    if (show_pathlink)
    {
        pathtext->hide();
        pathlink->show();
    }
    else
    {
        pathtext->show();
        pathlink->hide();
    }

    FXVerticalFrame::create();
}


// Destructor
FilePanel::~FilePanel()
{
    delete list;
    delete current;
    delete next;
    delete statuslabel;
    delete filterlabel;
    delete statusbar;
    delete panelsep;
    delete pathlink;
    delete newfiledialog;
    delete newlinkdialog;
    delete newdirdialog;
    delete opendialog;
    delete archdialog;
    delete filterdialog;
    delete comparedialog;
    delete addbookmarkdialog;
    delete operationdialogsingle;
    delete operationdialogrename;
    delete operationdialogmultiple;
    delete pathtext;
    delete closefilter;
}


// Make panel active
void FilePanel::setActive()
{
    // Set active icon
    activeicon->setIcon(minigreenbuttonicon);
    activeicon->setTipText(_("Panel is active"));

    pathlink->focus();
    current = this;

    // Make dirpanel point on the current directory,
    // but only if Filepanel and Dirpanel directories are different
    if (dirpanel->getDirectory() != current->list->getDirectory())
    {
        dirpanel->setDirectory(current->list->getDirectory(), true);
    }

    // Make dirpanel inactive
    dirpanel->setInactive();

    next->setInactive();
    list->setFocus();

    isactive = true;
}


// Make panel inactive
void FilePanel::setInactive(FXbool force)
{
    // Set active icon
    activeicon->setIcon(minigraybuttonicon);
    activeicon->setTipText(_("Activate panel"));

    // By default we set the panel inactive
    if (force)
    {
        current = next;
        list->handle(this, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), NULL);

        isactive = false;
    }
}


// Make panel focus (i.e. active) when clicked
long FilePanel::onCmdFocus(FXObject* sender, FXSelector sel, void* ptr)
{
    setActive();
    return 1;
}


// Set Pointer to Another FilePanel
void FilePanel::Next(FilePanel* nxt)
{
    next = nxt;
}


// Show or hide drag corner
void FilePanel::showCorner(FXbool show)
{
    if (show)
    {
        corner->show();
    }
    else
    {
        corner->hide();
    }
}


// Show or hide active icon
void FilePanel::showActiveIcon(FXbool show)
{
    if (show)
    {
        activeicon->show();
    }
    else
    {
        activeicon->hide();
    }
}


// Update location history when changing directory (home, up or double click)
void FilePanel::updateLocation()
{
    FXString item;
    int i = 0;
    ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();

    address->setNumVisible(5);
    int count = address->getNumItems();
    FXString p = list->getDirectory();

    // Remember latest directory in the location address
    if (!count)
    {
        count++;
        address->insertItem(0, address->getText());
    }
    while (i < count)
    {
        item = address->getItem(i++);
        if (xf_strequal((const char*)&p[0], (const char*)&item[0]))
        {
            i--;
            break;
        }
    }
    if (i == count)
    {
        address->insertItem(0, list->getDirectory());
    }

    // Make current directory visible to avoid scrolling again
    list->makeItemVisible(list->getCurrentItem());
}


// We now really do have the clipboard, keep clipboard content
long FilePanel::onClipboardGained(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardGained(sender, sel, ptr);
    return 1;
}


// We lost the clipboard
long FilePanel::onClipboardLost(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardLost(sender, sel, ptr);
    return 1;
}


// Somebody wants our clipboard content
long FilePanel::onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuchar* data;
    FXuint len;

    // Perhaps the target wants to supply its own data for the clipboard
    if (FXVerticalFrame::onClipboardRequest(sender, sel, ptr))
    {
        return 1;
    }

    // Clipboard target is xfelistType (Xfe, Gnome or XFCE)
    if (event->target == xfelistType)
    {
        // Don't modify the clipboard if we are called from updPaste()
        if (!clipboard_locked)
        {
            // Prepend "copy" or "cut" as in the Gnome way and avoid duplicating these strings
            if ((clipboard.find("copy\n") < 0) && (clipboard.find("cut\n") < 0))
            {
                if (clipboard_type == CUT_CLIPBOARD)
                {
                    clipboard = "cut\n" + clipboard;
                }
                else
                {
                    clipboard = "copy\n" + clipboard;
                }
            }
        }

        // Return clipboard content
        if (event->target == xfelistType)
        {
            if (!clipboard.empty())
            {
                len = clipboard.length();
                FXMEMDUP(&data, clipboard.text(), FXuchar, len);
                setDNDData(FROM_CLIPBOARD, event->target, data, len);

                // Return because xfelistType is not compatible with other types
                return 1;
            }
        }
    }

    // Clipboard target is kdelisType (KDE)
    if (event->target == kdelistType)
    {
        // The only data to be passed in this case is "0" for copy and "1" for cut
        // The uri data are passed using the standard uri-list type
        FXString flag;
        if (clipboard_type == CUT_CLIPBOARD)
        {
            flag = "1";
        }
        else
        {
            flag = "0";
        }

        // Return clipboard content
        if (event->target == kdelistType)
        {
            FXMEMDUP(&data, flag.text(), FXuchar, 1);
            setDNDData(FROM_CLIPBOARD, event->target, data, 1);
        }
    }

    // Clipboard target is urilistType (KDE apps ; non Gnome, non XFCE and non Xfe apps)
    if (event->target == urilistType)
    {
        if (!clipboard.empty())
        {
            len = clipboard.length();
            FXMEMDUP(&data, clipboard.text(), FXuchar, len);
            setDNDData(FROM_CLIPBOARD, event->target, data, len);

            return 1;
        }
    }

    // Clipboard target is utf8Type (to paste file pathes as text to other applications)
    if (event->target == utf8Type)
    {
        if (!clipboard.empty())
        {
            int beg = 0, end = 0;
            FXString str = "";
            FXString pathname, url;

            // Clipboard don't contain 'copy\n' or 'cut\n' as first line
            if ((clipboard.find("copy\n") < 0) && (clipboard.find("cut\n") < 0))
            {
                // Remove the 'file:' prefix for each file path
                while (1)
                {
                    end = clipboard.find('\n', end);
                    if (end < 0) // Last line
                    {
                        end = clipboard.length();
                        url = clipboard.mid(beg, end - beg + 1);
                        pathname = FXURL::decode(FXURL::fileFromURL(url));
                        str += pathname;
                        break;
                    }
                    url = clipboard.mid(beg, end - beg + 1);
                    pathname = FXURL::decode(FXURL::fileFromURL(url));
                    str += pathname;
                    end++;
                    beg = end;
                }
            }
            // Clipboard contains 'copy\n' or 'cut\n' as first line, thus skip it
            else
            {
                // Start after the 'copy\n' or 'cut\n' prefix
                end = clipboard.find('\n', 0);
                end++;
                beg = end;

                // Remove the 'file:' prefix for each file path
                while (1)
                {
                    end = clipboard.find('\n', end);
                    if (end < 0) // Last line
                    {
                        end = clipboard.length();
                        url = clipboard.mid(beg, end - beg + 1);
                        pathname = FXURL::decode(FXURL::fileFromURL(url));
                        str += pathname;
                        break;
                    }
                    url = clipboard.mid(beg, end - beg + 1);
                    pathname = FXURL::decode(FXURL::fileFromURL(url));
                    str += pathname;
                    end++;
                    beg = end;
                }
            }

            if (!str.empty())
            {
                len = str.length();
                FXMEMDUP(&data, str.text(), FXuchar, len);
                setDNDData(FROM_CLIPBOARD, event->target, data, len);

                return 1;
            }
        }
    }
    return 0;
}


// Copy or cut to clipboard (and add copy / add cut)
long FilePanel::onCmdCopyCut(FXObject*, FXSelector sel, void*)
{
    FXString name, curdir;

    // Clear clipboard if normal copy or cut
    if ((FXSELID(sel) == ID_COPY_CLIPBOARD) || (FXSELID(sel) == ID_CUT_CLIPBOARD))
    {
        clipboard.clear();
    }
    // Add an '\n' at the end if addcopy or addcut
    else
    {
        clipboard += '\n';
    }

    // Clipboard type
    if ((FXSELID(sel) == ID_CUT_CLIPBOARD) || (FXSELID(sel) == ID_ADDCUT_CLIPBOARD))
    {
        clipboard_type = CUT_CLIPBOARD;
    }
    else
    {
        clipboard_type = COPY_CLIPBOARD;
    }

    // Items number in the file list
    int num = current->list->getNumSelectedItems();

    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        // Construct the uri list of files and fill the clipboard with it
        curdir = current->list->getDirectory();

        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                name = current->list->getItemText(u).text();
                name = name.section('\t', 0);
                if (curdir == ROOTDIR)
                {
                    clipboard += FXURL::encode(::xf_filetouri(curdir + name)) + "\n";
                }
                else
                {
                    clipboard += FXURL::encode(::xf_filetouri(curdir + PATHSEPSTRING + name)) + "\n";
                }
            }
        }
    }

    // Remove the last \n of the list, for compatibility with some file managers (e.g. nautilus, nemo)
    clipboard.erase(clipboard.length() - 1);

    // Acquire the clipboard
    FXDragType types[4];
    types[0] = xfelistType;
    types[1] = kdelistType;
    types[2] = urilistType;
    types[3] = utf8Type;
    if (acquireClipboard(types, 4))
    {
        return 0;
    }

    return 1;
}


// Copy file names (without path) to clipboard
long FilePanel::onCmdCopyName(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard and set clipboard type
    clipboard.clear();
    clipboard_type = COPYNAME_CLIPBOARD;

    // Items number in the file list
    int num = current->list->getNumSelectedItems();

    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        // Construct the uri list of files and fill the clipboard with it
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                FXString name = current->list->getItemText(u).text();
                name = name.section('\t', 0);
                clipboard += FXURL::encode(::xf_filetouri(name)) + "\n";
            }
        }
    }

    // Remove the last \n of the list
    clipboard.erase(clipboard.length() - 1);

    // Acquire the clipboard
    FXDragType types[4];
    types[0] = xfelistType;
    types[1] = kdelistType;
    types[2] = urilistType;
    types[3] = utf8Type;
    if (acquireClipboard(types, 4))
    {
        return 0;
    }

    return 1;
}


// Paste file(s) from clipboard
long FilePanel::onCmdPaste(FXObject*, FXSelector sel, void*)
{
    FXuchar* data;
    FXuint len;
    int beg, end, pos;
    FXString url, param;
    int num = 0;
    FXbool from_kde = false;

    // If source is xfelistType (Gnome, XFCE, or Xfe app)
    if (getDNDData(FROM_CLIPBOARD, xfelistType, data, len))
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';

        clipboard = (char*)data;

        // Loop over clipboard items
        for (beg = 0; beg < clipboard.length(); beg = end + 1)
        {
            if ((end = clipboard.find("\n", beg)) < 0)
            {
                end = clipboard.length();
            }

            // Obtain item url
            url = clipboard.mid(beg, end - beg);

            // Possibly remove the trailing '\r' if any
            if ((pos = url.rfind('\r')) > 0)
            {
                url.erase(pos);
            }

            // Process first item
            if (num == 0)
            {
                // First item should be "copy" or "cut"
                if (url == "copy")
                {
                    clipboard_type = COPY_CLIPBOARD;
                    num++;
                }
                else if (url == "cut")
                {
                    clipboard_type = CUT_CLIPBOARD;
                    num++;
                }
                // If first item is not "copy" nor "cut", process it as a normal url
                // and use default clipboard type
                else
                {
                    // Update the param string
                    param += FXURL::decode(FXURL::fileFromURL(url)) + "\n";

                    // Add one more because the first line "copy" or "cut" was not present
                    num += 2;
                }
            }
            // Process other items
            else
            {
                // Update the param string
                param += FXURL::decode(FXURL::fileFromURL(url)) + "\n";
                num++;
            }
        }

        // Construct the final param string passed to the file management routine
        param = current->list->getDirectory() + "\n" + FXStringVal(num - 1) + "\n" + param;

        // Copy or cut operation depending on the clipboard type
        switch (clipboard_type)
        {
        case COPY_CLIPBOARD:
            sel = FXSEL(SEL_COMMAND, FilePanel::ID_FILE_COPY);
            break;

        case CUT_CLIPBOARD:
            clipboard.clear();
            sel = FXSEL(SEL_COMMAND, FilePanel::ID_FILE_CUT);
            break;
        }
        fromPaste = true;
        handle(this, sel, (void*)param.text());

        // Free data pointer
        FXFREE(&data);

        // Return here because xfelistType is not compatible with other types
        return 1;
    }

    // If source type is kdelistType (KDE)
    if (getDNDData(FROM_CLIPBOARD, kdelistType, data, len))
    {
        from_kde = true;

        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        clipboard = (char*)data;

        // Obtain clipboard type (copy or cut)
        if (clipboard == "1")
        {
            clipboard_type = CUT_CLIPBOARD;
        }
        else
        {
            clipboard_type = COPY_CLIPBOARD;
        }

        FXFREE(&data);
    }

    // If source type is urilistType (KDE apps ; non Gnome, non XFCE and non Xfe apps)
    if (getDNDData(FROM_CLIPBOARD, urilistType, data, len))
    {
        // For non KDE apps, set action to copy
        if (!from_kde)
        {
            clipboard_type = COPY_CLIPBOARD;
        }

        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        clipboard = (char*)data;

        // Loop over clipboard items
        for (beg = 0; beg < clipboard.length(); beg = end + 1)
        {
            if ((end = clipboard.find("\n", beg)) < 0)
            {
                end = clipboard.length();
            }

            // Obtain item url
            url = clipboard.mid(beg, end - beg);

            // Possibly remove the trailing '\r' if any
            if ((pos = url.rfind('\r')) > 0)
            {
                url.erase(pos);
            }

            // Update the param string
            param += FXURL::decode(FXURL::fileFromURL(url)) + "\n";
            num++;
        }

        // Construct the final param string passed to the file management routine
        param = current->list->getDirectory() + "\n" + FXStringVal(num) + "\n" + param;

        // Copy or cut operation depending on the clipboard type
        switch (clipboard_type)
        {
        case COPY_CLIPBOARD:
            sel = FXSEL(SEL_COMMAND, FilePanel::ID_FILE_COPY);
            break;

        case CUT_CLIPBOARD:
            clipboard.clear();
            sel = FXSEL(SEL_COMMAND, FilePanel::ID_FILE_CUT);
            break;
        }
        fromPaste = true;
        handle(this, sel, (void*)param.text());

        FXFREE(&data);
        return 1;
    }

    // If source type is utf8Type (simple text)
    if (getDNDData(FROM_CLIPBOARD, utf8Type, data, len))
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        clipboard = (char*)data;

        // Loop over clipboard items
        FXString filepath;
        for (beg = 0; beg < clipboard.length(); beg = end + 1)
        {
            if ((end = clipboard.find("\n", beg)) < 0)
            {
                end = clipboard.length();
            }

            // Obtain item file path
            filepath = clipboard.mid(beg, end - beg);

            // Possibly remove the trailing '\r' if any
            if ((pos = filepath.rfind('\r')) > 0)
            {
                filepath.erase(pos);
            }

            // Update the param string
            param += filepath + "\n";
            num++;
        }

        // Construct the final param string passed to the file management routine
        param = current->list->getDirectory() + "\n" + FXStringVal(num) + "\n" + param;

        // Copy
        sel = FXSEL(SEL_COMMAND, FilePanel::ID_FILE_COPY);
        fromPaste = true;
        handle(this, sel, (void*)param.text());

        FXFREE(&data);
        return 1;
    }

    return 0;
}


// Execute file with an optional confirm dialog
void FilePanel::execFile(FXString filepath)
{
    FXString cmd, cmdname, str;

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Text file
    int ret = xf_istextfile(filepath);
    if (ret == 1)
    {
        // Execution forbidden, edit the file
        FXbool no_script = getApp()->reg().readUnsignedEntry("OPTIONS", "no_script", false);
        if (no_script)
        {
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            cmd = txteditor;
            cmdname = cmd;

            // If command exists, run it
            if (xf_existcommand(cmdname))
            {
                cmd = cmdname + " " + xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
            }
            // If command does not exist, call the "Open with..." dialog
            else
            {
                current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            }
        }
        // Execution allowed, execute file with optional confirmation dialog
        else
        {
            // With confirmation dialog
            FXbool confirm_execute = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_execute", true);
            if (confirm_execute)
            {
                FXString msg;
                msg.format(_("File %s is an executable text file, what do you want to do?"), filepath.text());
                ExecuteBox* dlg = new ExecuteBox(this, _("Confirm Execute"), msg);
                FXuint answer = dlg->execute(PLACEMENT_CURSOR);
                delete dlg;

                // Execute
                if (answer == EXECBOX_CLICKED_EXECUTE)
                {
                    cmdname = FXPath::name(filepath);
                    cmd = xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                    // No startup notification in this case
                    runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, false, "");
#else
                    runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                }

                // Execute in console mode
                if (answer == EXECBOX_CLICKED_CONSOLE)
                {
                    int ret = chdir(current->list->getDirectory().text());
                    if (ret < 0)
                    {
                        int errcode = errno;
                        if (errcode)
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                              current->list->getDirectory().text(), strerror(errcode));
                        }
                        else
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                                              current->list->getDirectory().text());
                        }
                    }

                    cmdname = FXPath::name(filepath);
                    cmd = xf_quote(filepath);

                    // Make and show command window
                    // The CommandWindow object will delete itself when closed!
                    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Command log"), cmd, 30, 80);
                    cmdwin->create();
                    cmdwin->setIcon(minirunicon);

                    ret = chdir(startlocation.text());
                    if (ret < 0)
                    {
                        int errcode = errno;
                        if (errcode)
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                              startlocation.text(), strerror(errcode));
                        }
                        else
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                                              startlocation.text());
                        }
                    }
                }

                // Edit
                if (answer == EXECBOX_CLICKED_EDIT)
                {
                    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
                    cmd = txteditor;
                    cmdname = cmd;

                    // If command exists, run it
                    if (xf_existcommand(cmdname))
                    {
                        cmd = cmdname + " " + xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                        runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                        runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                    }
                    // If command does not exist, call the "Open with..." dialog
                    else
                    {
                        current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }
            }
        }
    }
    // Binary file, execute it
    else if (ret == 0)
    {
        cmdname = FXPath::name(filepath);
        cmd = xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
        runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
    }
    // Broken link
    else  // ret = -1
    {
        return;
    }
}


// Keyboard press
long FilePanel::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    int currentitem = current->list->getCurrentItem();

    switch (event->code)
    {
    case KEY_Escape:

        this->handle(sender, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), ptr);
        return 1;

    case KEY_KP_Enter:
    case KEY_Return:

        // Call onCmdItemDoubleClicked()
        handle(this, FXSEL(SEL_DOUBLECLICKED, FilePanel::ID_KEY_RETURN), (void*)(FXival)(currentitem));
        return 1;

    default:

        // Pass key to IconList
        return current->list->onKeyPress(sender, sel, ptr);
    }

    return 0;
}


// Double Click on File Item
long FilePanel::onCmdItemDoubleClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    // In single click mode, return if not called from return key
    if ( (FXSELID(sel) != FilePanel::ID_KEY_RETURN) && (single_click == SINGLE_CLICK_DIR_FILE))
    {
        int x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        if ((!(list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
             ((x - list->getXPosition()) < list->getHeaderSize(0)))
            || (list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)))
        {
            return 1;
        }
    }

    // Make panel active
    setActive();

    // Wait cursor
    getApp()->beginWaitCursor();

    // At most one item selected
    if (current->list->getNumSelectedItems() <= 1)
    {
        FXString cmd, cmdname, filename, pathname;

        FXlong item = (FXlong)ptr;
        if (item > -1)
        {
#ifdef STARTUP_NOTIFICATION
            // Startup notification option and exceptions (if any)
            FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
            FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif
            // Default programs
            FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
            FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
            FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
            FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
            FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
            FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

            // File name and path
            filename = list->getItemFilename(item);
            pathname = list->getItemPathname(item);

            // If directory, open the directory
            if (list->isItemDirectory(item))
            {
                // Does not have access
                if (!xf_isreadexecutable(pathname))
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."),
                                      pathname.text());
                    getApp()->endWaitCursor();
                    return 0;
                }
                if (filename == "..")
                {
                    list->handle(this, FXSEL(SEL_COMMAND, FileList::ID_DIRECTORY_UP), NULL);
                }
                else
                {
                    list->setDirectory(pathname);
                }

                // Change directory in tree list
                dirpanel->setDirectory(pathname, true);
                current->updatePath();

                // Update location history
                updateLocation();
            }
            else if (list->isItemFile(item))
            {
                // Update associations dictionary
                FileDict* assocdict = new FileDict(getApp());
                FileAssoc* association = assocdict->findFileBinding(pathname.text());

                // If there is an association
                if (association)
                {
                    // Use it to open the file
                    if (association->command.section(',', 0) != "")
                    {
                        cmdname = association->command.section(',', 0);

                        // Use a default program if possible
                        switch (progs[cmdname])
                        {
                        case TXTVIEWER:
                            cmdname = txtviewer;
                            break;

                        case TXTEDITOR:
                            cmdname = txteditor;
                            break;

                        case IMGVIEWER:
                            cmdname = imgviewer;
                            break;

                        case IMGEDITOR:
                            cmdname = imgeditor;
                            break;

                        case PDFVIEWER:
                            cmdname = pdfviewer;
                            break;

                        case AUDIOPLAYER:
                            cmdname = audioplayer;
                            break;

                        case VIDEOPLAYER:
                            cmdname = videoplayer;
                            break;

                        case ARCHIVER:
                            cmdname = archiver;
                            break;

                        case NONE: // No program found
                            ;
                            break;
                        }

                        // If command exists, run it
                        if (xf_existcommand(cmdname))
                        {
                            cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                            runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                        }
                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                    // Or execute the file
                    else if (list->isItemExecutable(item))
                    {
                        execFile(pathname);
                    }
                    // Or call the "Open with..." dialog
                    else
                    {
                        getApp()->endWaitCursor();
                        current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }
                // If no association but executable
                else if (list->isItemExecutable(item))
                {
                    execFile(pathname);
                }
                // Other cases
                else
                {
                    getApp()->endWaitCursor();
                    current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }

                delete assocdict;
            }
        }
    }
    // More than one selected files
    else
    {
        current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN), NULL);
    }

    getApp()->endWaitCursor();

    return 1;
}


// Single click on File Item
long FilePanel::onCmdItemClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    // Make panel active
    setActive();

    if (single_click != SINGLE_CLICK_NONE)
    {
        // Single click with control or shift
        int x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        if (state & (CONTROLMASK | SHIFTMASK))
        {
            return 1;
        }

        // In detailed mode, avoid single click when mouse cursor is not over the first column
        FXbool allow = true;
        if (!(list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
            ((x - list->getXPosition()) > list->getHeaderSize(0)))
        {
            allow = false;
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // At most one item selected
        if (current->list->getNumSelectedItems() <= 1)
        {
            // Default programs
            FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
            FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
            FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
            FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
            FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
            FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

            FXString cmd, cmdname, filename, pathname;

#ifdef STARTUP_NOTIFICATION
            // Startup notification option and exceptions (if any)
            FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
            FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

            FXlong item = (FXlong)ptr;
            if (item > -1)
            {
                // File name and path
                filename = list->getItemFilename(item);
                pathname = list->getItemPathname(item);

                // If directory, open the directory
                if ((single_click != SINGLE_CLICK_NONE) && list->isItemDirectory(item) && allow)
                {
                    // Does not have access
                    if (!xf_isreadexecutable(pathname))
                    {
                        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."),
                                          pathname.text());
                        getApp()->endWaitCursor();
                        return 0;
                    }
                    if (filename == "..")
                    {
                        list->handle(this, FXSEL(SEL_COMMAND, FileList::ID_DIRECTORY_UP), NULL);
                    }
                    else
                    {
                        list->setDirectory(pathname);
                    }

                    // Change directory in tree list
                    dirpanel->setDirectory(pathname, true);
                    current->updatePath();

                    // Update location history
                    updateLocation();
                }
                // If file, use the association if any
                else if ((single_click == SINGLE_CLICK_DIR_FILE) && list->isItemFile(item) && allow)
                {
                    // Update associations dictionary
                    FileDict* assocdict = new FileDict(getApp());
                    FileAssoc* association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to open the file
                        if (association->command.section(',', 0) != "")
                        {
                            cmdname = association->command.section(',', 0);

                            // Use a default program if possible
                            switch (progs[cmdname])
                            {
                            case TXTVIEWER:
                                cmdname = txtviewer;
                                break;

                            case TXTEDITOR:
                                cmdname = txteditor;
                                break;

                            case IMGVIEWER:
                                cmdname = imgviewer;
                                break;

                            case IMGEDITOR:
                                cmdname = imgeditor;
                                break;

                            case PDFVIEWER:
                                cmdname = pdfviewer;
                                break;

                            case AUDIOPLAYER:
                                cmdname = audioplayer;
                                break;

                            case VIDEOPLAYER:
                                cmdname = videoplayer;
                                break;

                            case ARCHIVER:
                                cmdname = archiver;
                                break;

                            case NONE: // No program found
                                ;
                                break;
                            }

                            // If command exists, run it
                            if (xf_existcommand(cmdname))
                            {
                                cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                                runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                            }
                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
                                current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }
                        // Or execute the file
                        else if (list->isItemExecutable(item))
                        {
                            execFile(pathname);
                        }
                        // Or call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                    // If no association but executable
                    else if (list->isItemExecutable(item))
                    {
                        execFile(pathname);
                    }
                    // Other cases
                    else
                    {
                        getApp()->endWaitCursor();
                        current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }

                    delete assocdict;
                }
            }
        }
        // More than one selected files
        else if ((single_click == SINGLE_CLICK_DIR_FILE) && allow)
        {
            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN), NULL);
        }

        getApp()->endWaitCursor();
    }
    return 1;
}


// Go to parent directory
long FilePanel::onCmdDirectoryUp(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->handle(this, FXSEL(SEL_COMMAND, FileList::ID_DIRECTORY_UP), NULL);
    current->list->setFocus();
    dirpanel->setDirectory(current->list->getDirectory(), true);
    current->updatePath();
    updateLocation();
    return 1;
}


// Go to home directory
long FilePanel::onCmdGoHome(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->setDirectory(homedir);
    current->list->setFocus();
    dirpanel->setDirectory(homedir, true);
    current->updatePath();
    updateLocation();
    return 1;
}


// Go to trash directory
long FilePanel::onCmdGoTrash(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->setDirectory(trashfileslocation);
    current->list->setFocus();
    dirpanel->setDirectory(trashfileslocation, true);
    current->updatePath();
    updateLocation();
    return 1;
}


// Set the flag that allows to stop the file list refresh
long FilePanel::onCmdStopListRefreshTimer(FXObject*, FXSelector, void*)
{
    stopListRefresh = true;

    return 0;
}


// Copy/Move/Rename/Symlink file(s)
long FilePanel::onCmdFileMan(FXObject* sender, FXSelector sel, void* ptr)
{
    int num;
    FXString src, targetdir, target, name, source;
    int firstitem = 0, lastitem = 0;

    // Focus on this panel list
    current->list->setFocus();

    // Confirmation dialog?
    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);

    // Copy suffix
    FXString copysuffix = getApp()->reg().readStringEntry("OPTIONS", "copysuffix", DEFAULT_COPY_SUFFIX);
    FXuint copysuffix_pos = getApp()->reg().readUnsignedEntry("OPTIONS", "copysuffix_pos", 0);

    // If we are we called from the paste command, get the parameters from the pointer
    if (fromPaste)
    {
        // Reset the flag
        fromPaste = false;

        // Get the parameters
        FXString str = (char*)ptr;
        targetdir = str.section('\n', 0);
        num = FXUIntVal(str.section('\n', 1));
        src = str.after('\n', 2);

        // If no item in clipboard, return
        if (num <= 0)
        {
            return 0;
        }

        // If there is a selected directory in file panel, use it as target directory
        if (current->list->getNumSelectedItems() == 1)
        {
            int item = current->list->getCurrentItem();
            if (current->list->isItemDirectory(item))
            {
                targetdir = list->getItemPathname(item);
            }
        }
    }
    // Obtain the parameters from the file panel
    else
    {
        // Current directory
        FXString curdir = current->list->getDirectory();

        // Number of selected items
        num = current->list->getNumSelectedItems();

        // If no item, return
        if (num <= 0)
        {
            return 0;
        }

        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        // Obtain the list of source files and the target directory
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                if (firstitem == 0)
                {
                    firstitem = u;
                }
                lastitem = u;
                name = current->list->getItemText(u).text();
                name = name.section('\t', 0);
                src += curdir + PATHSEPSTRING + name + "\n";
            }
        }
        targetdir = current->next->list->getDirectory();

        if (!current->next->shown() || (FXSELID(sel) == ID_FILE_RENAME))
        {
            targetdir = current->list->getDirectory();
        }
    }

    // Number of items in the FileList
    int numitems = current->list->getNumItems();

    // Name and directory of the first source file
    source = src.section('\n', 0);
    name = FXPath::name(source);
    FXString dir = FXPath::directory(source);

    // Initialize target name
    if (targetdir != ROOTDIR)
    {
        target = targetdir + PATHSEPSTRING;
    }
    else
    {
        target = targetdir;
    }

    // Configure the command, title, message, etc.
    FXIcon* icon = NULL;
    FXString command, title, message;
    if (FXSELID(sel) == ID_FILE_COPY)
    {
        command = "copy";
        title = _("Copy");
        icon = bigcopyicon;
        if (num == 1)
        {
            message = _("Copy ");
            message += source;
            if (xf_isfile(source))
            {
                target += name;
            }

            // Source and target are identical => add a suffix to the name
            FXString tgt = xf_cleanpath(target);                                                 // Remove trailing / if any
            if ((xf_isidentical(source, tgt) && (tgt != current->list->getDirectory())) ||         // Check we are not within target
                (xf_isdirectory(source) && (source == tgt + PATHSEPSTRING + FXPath::name(source))))
            {
                target = xf_buildcopyname(source, xf_isdirectory(source), copysuffix, copysuffix_pos);
            }
        }
        else
        {
            message.format(_("Copy %s items from: %s"), FXStringVal(num).text(), dir.text());
        }
    }
    if (FXSELID(sel) == ID_FILE_RENAME)
    {
        command = "rename";
        title = _("Rename");
        icon = bigrenameicon;
        if (num == 1)
        {
            message = _("Rename ");
            message += name;
            target = name;
        }
    }
    if (FXSELID(sel) == ID_FILE_COPYTO)
    {
        command = "copy";
        title = _("Copy");
        icon = bigcopyicon;
        if (num == 1)
        {
            message = _("Copy ");
            message += source;
        }
        else
        {
            message.format(_("Copy %s items from: %s"), FXStringVal(num).text(), dir.text());
        }
    }
    if (FXSELID(sel) == ID_FILE_MOVETO)
    {
        command = "move";
        title = _("Move");
        icon = bigmoveicon;
        if (num == 1)
        {
            message = _("Move ");
            message += source;
            title = _("Move");
        }
        else
        {
            message.format(_("Move %s items from: %s"), FXStringVal(num).text(), dir.text());
        }
    }
    if (FXSELID(sel) == ID_FILE_CUT)
    {
        command = "move";
        title = _("Move");
        icon = bigmoveicon;
        if (num == 1)
        {
            message = _("Move ");
            message += source;
            if (xf_isfile(source))
            {
                target += name;
            }
            title = _("Move");
        }
        else
        {
            message.format(_("Move %s items from: %s"), FXStringVal(num).text(), dir.text());
        }
    }
    if (FXSELID(sel) == ID_FILE_SYMLINK)
    {
        command = "symlink";
        title = _("Symlink");
        icon = biglinktoicon;
        if (num == 1)
        {
            message = _("Symlink ");
            message += source;
            target += name;
        }
        else
        {
            message.format(_("Symlink %s items from: %s"), FXStringVal(num).text(), dir.text());
        }
    }

    // File operation dialog, if needed
    if (ask_before_copy || (source == target) || (FXSELID(sel) == ID_FILE_COPYTO) || (FXSELID(sel) == ID_FILE_MOVETO) ||
        (FXSELID(sel) == ID_FILE_RENAME) || (FXSELID(sel) == ID_FILE_SYMLINK))
    {
        if (num == 1) // One source
        {
            if (FXSELID(sel) == ID_FILE_RENAME)
            {
                if (operationdialogrename == NULL)
                {
                    operationdialogrename = new InputDialog(this, "", "", title, _("To:"), icon);
                }
                operationdialogrename->setTitle(title);
                operationdialogrename->setIcon(icon);
                operationdialogrename->setMessage(message);
                operationdialogrename->setText(target);

                if (xf_isdirectory(source)) // directory
                {
                    operationdialogrename->selectAll();
                }
                else
                {
                    int pos = target.rfind('.');
                    if (pos <= 0)
                    {
                        operationdialogrename->selectAll(); // no extension or dot file
                    }
                    else
                    {
                        operationdialogrename->setSelection(0, pos);
                    }
                }

                int rc = operationdialogrename->execute(PLACEMENT_CURSOR);
                target = operationdialogrename->getText();

                // Target name contains '/'
                if (target.contains(PATHSEPCHAR))
                {
                    MessageBox::warning(this, BOX_OK, _("Warning"),
                    _("The / character is not allowed in file or folder names, operation cancelled"));
                    return 0;
                }

                if (!rc)
                {
                    return 0;
                }
            }
            else
            {
                if (operationdialogsingle == NULL)
                {
                    operationdialogsingle = new BrowseInputDialog(this, "", "", title, _("To:"), icon,
                                                                  BROWSE_INPUT_MIXED);
                }
                operationdialogsingle->setTitle(title);
                operationdialogsingle->setIcon(icon);
                operationdialogsingle->setMessage(message);
                operationdialogsingle->setText(target);

                // Select file name without path
                if (FXSELID(sel) == ID_FILE_SYMLINK)
                {
                    int pos = target.rfind(PATHSEPSTRING);
                    if (pos >= 0)
                    {
                        operationdialogsingle->setSelection(pos + 1, target.length());
                    }
                }

                operationdialogsingle->setDirectory(targetdir);
                int rc = operationdialogsingle->execute(PLACEMENT_CURSOR);
                target = operationdialogsingle->getText();
                if (!rc)
                {
                    return 0;
                }
            }
        }
        else // Multiple sources
        {
            if (FXSELID(sel) != ID_FILE_RENAME) // No such dialog for multiple rename
            {
                if (operationdialogmultiple == NULL)
                {
                    operationdialogmultiple = new BrowseInputDialog(this, "", "", title, _("To folder:"), icon,
                                                                    BROWSE_INPUT_FOLDER);
                }
                operationdialogmultiple->setTitle(title);
                operationdialogmultiple->setIcon(icon);
                operationdialogmultiple->setMessage(message);
                operationdialogmultiple->setText(target);
                operationdialogmultiple->CursorEnd();
                operationdialogmultiple->setDirectory(targetdir);
                int rc = operationdialogmultiple->execute(PLACEMENT_CURSOR);
                target = operationdialogmultiple->getText();
                if (!rc)
                {
                    return 0;
                }
            }
        }
    }

    // Nothing entered
    if (target == "")
    {
        MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
        return 0;
    }

    // Update target and target parent directory
    target = xf_filepath(target, current->list->getDirectory());
    if (xf_isdirectory(target))
    {
        targetdir = target;
    }
    else
    {
        targetdir = FXPath::directory(target);
    }

    // Target parent directory doesn't exist
    if (!xf_existfile(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), targetdir.text());
        return 0;
    }

    // Target parent directory not writable
    if (!xf_iswritable(targetdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), targetdir.text());
        return 0;
    }

    // Target parent directory is not a directory
    if (!xf_isdirectory(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("%s is not a folder"), targetdir.text());
        return 0;
    }

    // Multiple sources and non existent destination
    if ((num > 1) && !xf_existfile(target))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), target.text());
        return 0;
    }

    // Multiple sources and target is a file
    if ((num > 1) && xf_isfile(target))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("%s is not a folder"), target.text());
        return 0;
    }

    // Target is a directory and is not writable
    if (xf_isdirectory(target) && !xf_iswritable(target))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), target.text());
        return 0;
    }

    // Target is a file and its parent directory is not writable
    if (xf_isfile(target) && !xf_iswritable(targetdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), targetdir.text());
        return 0;
    }

    FXString hsourcesize;
    FXulong sourcesize = 0, tstart = 0;

    // One source
    File* f = NULL;
    int ret;
    if (num == 1)
    {
        // An empty source file name corresponds to the ".." file
        // Don't perform any file operation on it!
        if (source == "")
        {
            return 0;
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        if (command == "copy")
        {
            f = new File(this, _("Copy files"), COPY, num);
            f->create();

            // Get total source size and start time
            hsourcesize = f->sourcesize(src, &sourcesize);
            tstart = xf_getcurrenttime();

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                goto out;
            }

            // If target file is located at trash location, also create the corresponding trashinfo file
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && (target == trashfileslocation))
            {
                // Trash files path name
                FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                // Adjust target name to get the _N suffix if any
                FXString trashtarget = target + PATHSEPSTRING + FXPath::name(trashpathname);

                // Create trashinfo file
                xf_create_trashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                // Copy source to trash target
                ret = f->copy(source, trashtarget, hsourcesize, sourcesize, tstart);
            }
            // Copy source to target
            else
            {
                ret = f->copy(source, target, hsourcesize, sourcesize, tstart);
            }

            // An unknown error has occurred
            if ((ret == 0) && !f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the copy file operation!"));
                goto out;
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                goto out;
            }
        }
        else if (command == "rename")
        {
            f = new File(this, _("Rename Files"), RENAME, num);
            f->create();
            ret = f->rename(source, target);

            // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
            {
                FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) + ".trashinfo";
                unlink(trashinfopathname.text());
            }
        }
        else if (command == "move")
        {
            f = new File(this, _("Move Files"), MOVE, num);
            f->create();

            // Get total source size and start time
            hsourcesize = f->sourcesize(src, &sourcesize);
            tstart = xf_getcurrenttime();

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                goto out;
            }

            // If target file is located at trash location, also create the corresponding trashinfo file
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && (target == trashfileslocation))
            {
                // Trash files path name
                FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                // Adjust target name to get the _N suffix if any
                FXString trashtarget = target + PATHSEPSTRING + FXPath::name(trashpathname);

                // Create trashinfo file
                xf_create_trashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                // Move source to trash target
                ret = f->fmove(source, trashtarget, hsourcesize, sourcesize, tstart);
            }
            // Move source to target
            else
            {
                ret = f->fmove(source, target, hsourcesize, sourcesize, tstart);
            }

            // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
            // Do it silently and don't report any error if it fails
            if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
            {
                FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) + ".trashinfo";
                unlink(trashinfopathname.text());
            }

            // An unknown error has occurred
            if ((ret == 0) && !f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move file operation!"));
                goto out;
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                goto out;
            }
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK, num);
            f->create();
            f->symlink(source, target);
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }

        getApp()->endWaitCursor();
        delete f;
    }
    // Multiple sources
    else if (num > 1)
    {
        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        if (command == "copy")
        {
            f = new File(this, _("Copy Files"), COPY, num);
            f->create();

            // Get total source size and start time
            hsourcesize = f->sourcesize(src, &sourcesize);
            tstart = xf_getcurrenttime();

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                goto out;
            }
        }
        else if (command == "move")
        {
            f = new File(this, _("Move Files"), MOVE, num);
            f->create();

            // Get total source size
            hsourcesize = f->sourcesize(src, &sourcesize);
            tstart = xf_getcurrenttime();

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                goto out;
            }
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK, num);
            f->create();
        }
        else if (command == "rename")
        {
            getApp()->endWaitCursor();

            // Read Rename Dialog geometry
            FXuint width = getApp()->reg().readUnsignedEntry("OPTIONS", "rename_width", 800);
            FXuint height = getApp()->reg().readUnsignedEntry("OPTIONS", "rename_height", 600);

            // Rename dialog
            RenameDialog* renamedialog = new RenameDialog(this, title, src, num, false, width, height);  // Hide folder names
            renamedialog->execute(PLACEMENT_OWNER);

            // Rename files
            FXString tgt = renamedialog->getTarget();
            if (tgt != "")
            {
                getApp()->beginWaitCursor();

                f = new File(this, _("Rename Files"), RENAME, num);
                f->create();

                // Loop on the multiple files
                for (int i = 0; i < num; i++)
                {
                    // Individual source and target files
                    source = src.section('\n', i);
                    target = tgt.section('\n', i);

                    if (target != "" && target != source)
                    {
                        ret = f->rename(source, target);

                        // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
                        // Do it silently and don't report any error if it fails
                        FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                        if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
                        {
                            FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) +
                                                         ".trashinfo";
                            unlink(trashinfopathname.text());
                        }

                        // If action is cancelled in progress dialog
                        if (f->isCancelled())
                        {
                            MessageBox::error(this, BOX_OK, _("Warning"), _("Rename file operation cancelled!"));
                            break;
                        }
                        // An known error has occurred
                        if (ret == -1)
                        {
                            break;
                        }

                        // An unknown error has occurred
                        if (ret == 0)
                        {
                            MessageBox::error(this, BOX_OK, _("Error"),
                                              _("An error has occurred during the rename file operation!"));
                            break;
                        }
                    }
                }

                // Deselect all items
                list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), NULL);

                getApp()->endWaitCursor();
                delete f;
            }

            // Write Rename Dialog geometry
            getApp()->reg().writeUnsignedEntry("OPTIONS", "rename_width", (FXuint)renamedialog->getWidth());
            getApp()->reg().writeUnsignedEntry("OPTIONS", "rename_height", (FXuint)renamedialog->getHeight());
            getApp()->reg().write();

            delete renamedialog;
            return 1;
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }

        // Initialize file list stop refresh timer and flag
        stopListRefresh = false;
        getApp()->addTimeout(this, ID_STOP_LIST_REFRESH_TIMER, STOP_LIST_REFRESH_INTERVAL);

        // Loop on the multiple files
        for (int i = 0; i < num; i++)
        {
            // Stop refreshing the file list if file operation is long and has many files
            // This avoids flickering and speeds up things a bit
            if (stopListRefresh && (i > STOP_LIST_REFRESH_NBMAX))
            {
                // Force a last refresh if current panel is destination
                if (current->getDirectory() == targetdir)
                {
                    current->list->onCmdRefresh(0, 0, 0);
                }

                // Force a last refresh if next panel is destination
                if (next->getDirectory() == targetdir)
                {
                    next->list->onCmdRefresh(0, 0, 0);
                }

                // Tell the dir and file list to not refresh anymore
                setAllowRefresh(false);
                next->setAllowRefresh(false);
                dirpanel->setAllowDirsizeRefresh(false);

                // Don't need to stop again
                stopListRefresh = false;
            }

            // Individual source file
            //~ source = src.section('\n', i);

            // NB: src string is altered by strtok()
            char* token;
            (i == 0 ? token = strtok((char*)src.text(), "\n") : token = strtok(NULL, "\n"));
            source = FXString(token);

            // File could have already been moved above in the tree
            if (!xf_existfile(source))
            {
                continue;
            }

            // An empty file name corresponds to the ".." file (why?)
            // Don't perform any file operation on it!
            if (source != "")
            {
                if (command == "copy")
                {
                    // If target file is located at trash location, also create the corresponding trashinfo file
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (target == trashfileslocation))
                    {
                        // Trash files path name
                        FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                        // Adjust target name to get the _N suffix if any
                        FXString trashtarget = target + PATHSEPSTRING + FXPath::name(trashpathname);

                        // Create trashinfo file
                        xf_create_trashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                        // Copy source to trash target
                        ret = f->copy(source, trashtarget, hsourcesize, sourcesize, tstart);
                    }
                    // Copy source to target
                    else
                    {
                        ret = f->copy(source, target, hsourcesize, sourcesize, tstart);
                    }

                    // An known error has occurred
                    if (ret == -1)
                    {
                        f->hideProgressDialog();
                        break;
                    }

                    // An unknown error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"),
                                          _("An error has occurred during the copy file operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                        break;
                    }
                }
                else if (command == "move")
                {
                    // If target file is located at trash location, also create the corresponding trashinfo file
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (target == trashfileslocation))
                    {
                        // Trash files path name
                        FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                        // Adjust target name to get the _N suffix if any
                        FXString trashtarget = target + PATHSEPSTRING + FXPath::name(trashpathname);

                        // Create trashinfo file
                        xf_create_trashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                        // Move source to trash target
                        ret = f->fmove(source, trashtarget, hsourcesize, sourcesize, tstart);
                    }
                    // Move source to target
                    else
                    {
                        ret = f->fmove(source, target, hsourcesize, sourcesize, tstart);
                    }

                    // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
                    // Do it silently and don't report any error if it fails
                    if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
                    {
                        FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) +
                                                     ".trashinfo";
                        unlink(trashinfopathname.text());
                    }

                    // An known error has occurred
                    if (ret == -1)
                    {
                        f->hideProgressDialog();
                        break;
                    }

                    // An unknown error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"),
                                          _("An error has occurred during the move file operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                        break;
                    }
                }
                else if (command == "symlink")
                {
                    ret = f->symlink(source, target);

                    // An known error has occurred
                    if (ret == -1)
                    {
                        f->hideProgressDialog();
                        break;
                    }

                    // An unknown error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"),
                                          _("An error has occurred during the symlink operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Symlink operation cancelled!"));
                        break;
                    }
                }
                // Shouldn't happen
                else
                {
                    exit(EXIT_FAILURE);
                }
            }
        }

        // Reinit timer and refresh flags
out:
        getApp()->removeTimeout(this, ID_STOP_LIST_REFRESH_TIMER);
        current->setAllowRefresh(true);
        next->setAllowRefresh(true);
        dirpanel->setAllowDirsizeRefresh(true);

        getApp()->endWaitCursor();
        delete f;
    }

    // Force panels refresh
    next->onCmdRefresh(0, 0, 0);
    current->onCmdRefresh(0, 0, 0);

    // Enable previous or last selected item for keyboard navigation
    if (((FXSELID(sel) == ID_FILE_MOVETO) || (FXSELID(sel) == ID_FILE_RENAME)) &&
        (current->list->getNumItems() < numitems))
    {
        firstitem = (firstitem < 1) ? 0 : firstitem - 1;
        current->list->enableItem(firstitem);
        current->list->setCurrentItem(firstitem);
    }
    else
    {
        current->list->enableItem(lastitem);
        current->list->setCurrentItem(lastitem);
    }

    return 1;
}


// Trash files from the file list or the tree list
long FilePanel::onCmdFileTrash(FXObject*, FXSelector, void*)
{
    int firstitem = 0;
    File* f = NULL;

    current->list->setFocus();
    FXString dir = current->list->getDirectory();

    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    // If we don't have permission to write to the parent directory
    if (!xf_iswritable(dir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), dir.text());
        return 0;
    }

    // If we don't have permission to write to the trash directory
    if (!xf_iswritable(trashfileslocation))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to trash location %s: Permission denied"),
                          trashfileslocation.text());
        return 0;
    }

    // Items number in the file list
    int num = current->list->getNumSelectedItems();

    // If nothing selected, return
    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        if (confirm_trash)
        {
            FXString message;
            if (num == 1)
            {
                FXString pathname;
                for (int u = 0; u < current->list->getNumItems(); u++)
                {
                    if (current->list->isItemSelected(u))
                    {
                        pathname = current->list->getItemPathname(u);
                    }
                }
                if (xf_isdirectory(pathname))
                {
                    message.format(_("Move folder %s to trash can?"), pathname.text());
                }
                else
                {
                    message.format(_("Move file %s to trash can?"), pathname.text());
                }
            }
            else
            {
                message.format(_("Move %s selected items to trash can?"), FXStringVal(num).text());
            }

            MessageBox box(this, _("Confirm Trash"), message, bigdeleteicon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return 0;
            }
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Move to Trash"), DELETE, num);
        f->create();
        list->setAllowRefresh(false);

        // Overwrite initialisations
        FXbool overwrite = false;
        FXbool overwrite_all = false;
        FXbool skip_all = false;

        // Delete selected files
        FXString filename, pathname;
        int i = 0;
        stopListRefresh = false;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                // Get index of first selected item
                if (firstitem == 0)
                {
                    firstitem = u;
                }

                // Stop refreshing the dirsize in dirpanel
                // when there are many files to delete
                i++;
                if (!stopListRefresh && (i > STOP_LIST_REFRESH_NBMAX))
                {
                    dirpanel->setAllowDirsizeRefresh(false);
                    stopListRefresh = true;
                }

                // Get file name and path
                filename = current->list->getItemFilename(u);
                pathname = current->list->getItemPathname(u);

                // If we don't have permission to write to the file
                if (!xf_iswritable(pathname))
                {
                    // Overwrite dialog if necessary
                    if (!(overwrite_all | skip_all))
                    {
                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("File %s is write-protected, move it anyway to trash can?"), pathname.text());

                        if (num == 1)
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg, OVWBOX_SINGLE_FILE);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            if (answer == 1)
                            {
                                overwrite = true;
                            }
                            else
                            {
                                goto end;
                            }
                        }
                        else
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            switch (answer)
                            {
                            // Cancel
                            case 0:
                                goto end;
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
                    }
                    if ((overwrite | overwrite_all) && !skip_all)
                    {
                        // Caution!! Don't delete parent directory!!
                        if (filename != "..")
                        {
                            // Trash files path name
                            FXString trashpathname = xf_create_trashpathname(pathname, trashfileslocation);

                            // Create trashinfo file
                            xf_create_trashinfo(pathname, trashpathname, trashfileslocation, trashinfolocation);

                            // Move file to trash files location
                            int ret = f->fmove(pathname, trashpathname);

                            // An error has occurred
                            if ((ret == 0) && !f->isCancelled())
                            {
                                f->hideProgressDialog();
                                MessageBox::error(this, BOX_OK, _("Error"),
                                                  _("An error has occurred during the move to trash operation!"));
                                break;
                            }
                        }
                    }
                    f->showProgressDialog();
                }
                // If we have permission to write
                else
                {
                    // Caution!! Don't delete parent directory!!
                    if (filename != "..")
                    {
                        // Trash files path name
                        FXString trashpathname = xf_create_trashpathname(pathname, trashfileslocation);

                        // Create trashinfo file
                        xf_create_trashinfo(pathname, trashpathname, trashfileslocation, trashinfolocation);

                        // Move file to trash files location
                        int ret = f->fmove(pathname, trashpathname);

                        // An error has occurred
                        if ((ret == 0) && !f->isCancelled())
                        {
                            f->hideProgressDialog();
                            MessageBox::error(this, BOX_OK, _("Error"),
                                              _("An error has occurred during the move to trash operation!"));
                            break;
                        }
                    }
                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Move to trash file operation cancelled!"));
                        break;
                    }
                }
            }
        }
end:
        getApp()->endWaitCursor();
        delete f;
    }
    // Force FilePanel and DirPanel refresh
    list->setAllowRefresh(true);
    stopListRefresh = false;
    dirpanel->setAllowDirsizeRefresh(true);
    onCmdRefresh(0, 0, 0);

    // Enable last item before the first selected item (for keyboard navigation)
    firstitem = (firstitem < 1) ? 0 : firstitem - 1;
    current->list->enableItem(firstitem);
    current->list->setCurrentItem(firstitem);

    return 1;
}


// Restore files from trash can
long FilePanel::onCmdFileRestore(FXObject*, FXSelector, void*)
{
    int firstitem = 0;
    File* f = NULL;

    current->list->setFocus();
    FXString dir = current->list->getDirectory();
    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    // Items number in the file list
    int num = current->list->getNumSelectedItems();

    // If nothing selected, return
    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Restore From Trash"), DELETE, num);
        f->create();
        list->setAllowRefresh(false);

        // Restore (i.e. move to their original location) selected files
        FXString filename, pathname;
        int i = 0;
        stopListRefresh = false;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                // Get index of first selected item
                if (firstitem == 0)
                {
                    firstitem = u;
                }

                // Stop refreshing the dirsize in dirpanel
                // when there are many files to delete
                i++;
                if (!stopListRefresh && (i > STOP_LIST_REFRESH_NBMAX))
                {
                    dirpanel->setAllowDirsizeRefresh(false);
                    stopListRefresh = true;
                }

                // Get file name and path
                filename = current->list->getItemFilename(u);
                pathname = current->list->getItemPathname(u);

                // Don't restore '..' directory
                if (filename != "..")
                {
                    // Obtain trash base name and sub path
                    FXString subpath = pathname;
                    subpath.erase(0, trashfileslocation.length() + 1);
                    FXString trashbasename = subpath.before('/');
                    if (trashbasename == "")
                    {
                        trashbasename = name;
                    }
                    subpath.erase(0, trashbasename.length());

                    // Read the .trashinfo file
                    FILE* fp;
                    char line[1024];
                    FXbool success = true;
                    FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + trashbasename + ".trashinfo";
                    FXString origpathname = "";

                    if ((fp = fopen(trashinfopathname.text(), "r")) != NULL)
                    {
                        // Read the first two lines and get the strings
                        if (fgets(line, sizeof(line), fp) == NULL)
                        {
                            success = false;
                        }
                        if (fgets(line, sizeof(line), fp) == NULL)
                        {
                            success = false;
                        }
                        if (success)
                        {
                            origpathname = line;
                            origpathname = origpathname.after('=');
                            origpathname = origpathname.before('\n');
                        }
                        fclose(fp);
                        origpathname = origpathname + subpath;
                    }

                    // Confirm restore dialog
                    if (confirm_trash && (u == firstitem))
                    {
                        FXString message;
                        if (num == 1)
                        {
                            if (xf_isdirectory(pathname))
                            {
                                message.format(_("Restore folder %s to its original location %s ?"), filename.text(),
                                               origpathname.text());
                            }
                            else
                            {
                                message.format(_("Restore file %s to its original location %s ?"), filename.text(),
                                               origpathname.text());
                            }
                        }
                        else
                        {
                            message.format(_("Restore %s selected items to their original locations?"),
                                           FXStringVal(num).text());
                        }
                        f->hideProgressDialog();
                        MessageBox box(this, _("Confirm Restore"), message, bigrestoreicon,
                                       BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
                        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
                        {
                            getApp()->endWaitCursor();
                            delete f;
                            return 0;
                        }
                        f->showProgressDialog();
                    }

                    if (origpathname == "")
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"), _("Restore information not available for %s"),
                                          pathname.text());
                        goto end;
                    }

                    // If parent dir of the original location does not exist
                    FXString origparentdir = FXPath::directory(origpathname);
                    if (!xf_existfile(origparentdir))
                    {
                        // Ask the user if he wants to create it
                        f->hideProgressDialog();
                        FXString message;
                        message.format(_("Parent folder %s does not exist, do you want to create it?"),
                                       origparentdir.text());
                        MessageBox box(this, _("Confirm Restore"), message, bigrestoreicon,
                                       BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
                        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
                        {
                            goto end;
                        }
                        else
                        {
                            errno = 0;
                            int ret = xf_mkpath(origparentdir.text(), 0755);
                            int errcode = errno;
                            if (ret == -1)
                            {
                                f->hideProgressDialog();
                                if (errcode)
                                {
                                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s: %s"),
                                                      origparentdir.text(), strerror(errcode));
                                }
                                else
                                {
                                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s"),
                                                      origparentdir.text());
                                }
                                goto end;
                            }
                            f->showProgressDialog();
                        }
                    }

                    // Move file to original location (with restore option)
                    int ret = f->fmove(pathname, origpathname, "", true);

                    // An error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"),
                                          _("An error has occurred during the restore from trash operation!"));
                        goto end;
                    }

                    // Silently remove trashinfo file
                    FXString trashfilespathname = trashfileslocation + PATHSEPSTRING + trashbasename;
                    if ((pathname == trashfilespathname) && !xf_existfile(trashfilespathname))
                    {
                        unlink(trashinfopathname.text());
                    }
                }
                // If action is cancelled in progress dialog
                if (f->isCancelled())
                {
                    f->hideProgressDialog();
                    MessageBox::error(this, BOX_OK, _("Warning"), _("Restore from trash file operation cancelled!"));
                    goto end;
                }
            }
        }
end:
        getApp()->endWaitCursor();
        delete f;
    }
    // Force FilePanel and DirPanel refresh
    list->setAllowRefresh(true);
    stopListRefresh = false;
    dirpanel->setAllowDirsizeRefresh(true);
    onCmdRefresh(0, 0, 0);

    // Enable last item before the first selected item (for keyboard navigation)
    firstitem = (firstitem < 1) ? 0 : firstitem - 1;
    current->list->enableItem(firstitem);
    current->list->setCurrentItem(firstitem);

    return 1;
}


// Definitively delete files from the file list or the tree list (no trash can)
long FilePanel::onCmdFileDelete(FXObject*, FXSelector, void*)
{
    int firstitem = 0;
    File* f = NULL;

    current->list->setFocus();
    FXString dir = current->list->getDirectory();

    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);
    FXbool confirm_del_emptydir = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete_emptydir", true);

    // If we don't have permission to write to the parent directory
    if (!xf_iswritable(dir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), dir.text());
        return 0;
    }

    // Items number in the file list
    int num = current->list->getNumSelectedItems();

    // If nothing selected, return
    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        if (confirm_del)
        {
            FXString message;
            if (num == 1)
            {
                FXString pathname;
                for (int u = 0; u < current->list->getNumItems(); u++)
                {
                    if (current->list->isItemSelected(u))
                    {
                        pathname = current->list->getItemPathname(u);
                    }
                }
                if (xf_isdirectory(pathname))
                {
                    message.format(_("Definitively delete folder %s ?"), pathname.text());
                }
                else
                {
                    message.format(_("Definitively delete file %s ?"), pathname.text());
                }
            }
            else
            {
                message.format(_("Definitively delete %s selected items?"), FXStringVal(num).text());
            }
            MessageBox box(this, _("Confirm Delete"), message, bigdeletepermicon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return 0;
            }
        }
        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Delete Files"), DELETE, num);
        f->create();
        list->setAllowRefresh(false);

        // Overwrite initialisations
        FXbool overwrite = false;
        FXbool overwrite_all = false;
        FXbool skip_all = false;
        FXbool ask_del_empty = true;
        FXbool skip_all_del_emptydir = false;

        // Delete selected files
        FXString filename, pathname;
        int i = 0;
        stopListRefresh = false;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                // Get index of first selected item
                if (firstitem == 0)
                {
                    firstitem = u;
                }

                // Stop refreshing the dirsize in dirpanel
                // when there are many files to delete
                i++;
                if (!stopListRefresh && (i > STOP_LIST_REFRESH_NBMAX))
                {
                    dirpanel->setAllowDirsizeRefresh(false);
                    stopListRefresh = true;
                }

                // Get file name and path
                filename = current->list->getItemFilename(u);
                pathname = current->list->getItemPathname(u);

                // Confirm empty directory deletion
                if (confirm_del & confirm_del_emptydir & ask_del_empty)
                {
                    if ((xf_isemptydir(pathname) == 0) && !xf_islink(pathname))
                    {
                        if (skip_all_del_emptydir)
                        {
                            continue;
                        }

                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("Folder %s is not empty, delete it anyway?"), pathname.text());

                        if (num == 1)
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg, OVWBOX_SINGLE_FILE);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;

                            if (answer == 0)
                            {
                                goto end;
                            }
                        }
                        else
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            switch (answer)
                            {
                            // Cancel
                            case 0:
                                goto end;
                                break;

                            // Yes
                            case 1:
                                break;

                            // Yes for all
                            case 2:
                                ask_del_empty = false;
                                break;

                            // Skip
                            case 3:
                                continue;
                                break;

                            // Skip all
                            case 4:
                                skip_all_del_emptydir = true;
                                continue;
                                break;
                            }
                        }
                        f->showProgressDialog();
                    }
                }

                // If we don't have permission to write to the file
                if (!xf_iswritable(pathname))
                {
                    // Overwrite dialog if necessary
                    if (!(overwrite_all | skip_all))
                    {
                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("File %s is write-protected, delete it anyway?"), pathname.text());

                        if (num == 1)
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg, OVWBOX_SINGLE_FILE);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            if (answer == 1)
                            {
                                overwrite = true;
                            }
                            else
                            {
                                goto end;
                            }
                        }
                        else
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            switch (answer)
                            {
                            // Cancel
                            case 0:
                                goto end;
                                break;

                            // Yes
                            case 1:
                                overwrite = true;
                                break;

                            // Yes for all
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
                    }
                    if ((overwrite | overwrite_all) && !skip_all)
                    {
                        // Caution!! Don't delete parent directory!!
                        if (filename != "..")
                        {
                            // Definitively remove file or folder
                            f->remove(pathname);
                        }
                    }
                    f->showProgressDialog();
                }
                // If we have permission to write
                else
                {
                    // Caution!! Don't delete parent directory!!
                    if (filename != "..")
                    {
                        // Definitively remove file or folder
                        f->remove(pathname);

                        // If is located at trash location, try to also remove the corresponding trashinfo file if it exists
                        // Do it silently and don't report any error if it fails
                        FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                        if (use_trash_can && (pathname.left(trashfileslocation.length()) == trashfileslocation))
                        {
                            FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + filename + ".trashinfo";
                            unlink(trashinfopathname.text());
                        }
                    }
                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Delete file operation cancelled!"));
                        break;
                    }
                }
            }
        }
end:
        getApp()->endWaitCursor();
        delete f;
    }
    // Force FilePanel and DirPanel refresh
    list->setAllowRefresh(true);
    stopListRefresh = false;
    dirpanel->setAllowDirsizeRefresh(true);
    onCmdRefresh(0, 0, 0);

    // Enable last item before the first selected item (for keyboard navigation)
    firstitem = (firstitem < 1) ? 0 : firstitem - 1;
    current->list->enableItem(firstitem);
    current->list->setCurrentItem(firstitem);

    return 1;
}


// View/Edit files
long FilePanel::onCmdEdit(FXObject*, FXSelector sel, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

    FXString pathname, samecmd, cmd, cmdname, itemslist = " ";
    FileAssoc* association;
    FXbool same = true;
    FXbool first = true;

    current->list->setFocus();

    FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
    FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
    FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
    FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
    FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
    FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
    FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

    // Update associations dictionary
    FileDict* assocdict = new FileDict(getApp());

    // Check if all files have the same association
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        if (current->list->isItemSelected(u))
        {
            // Increment number of selected items
            pathname = current->list->getItemPathname(u);
            association = assocdict->findFileBinding(pathname.text());

            // If there is an association
            if (association)
            {
                // Use it to edit/view the files
                if (FXSELID(sel) == ID_EDIT) // Edit
                {
                    cmd = association->command.section(',', 2);

                    // Use a default editor if possible
                    switch (progs[cmd])
                    {
                    case TXTEDITOR:
                        cmd = txteditor;
                        break;

                    case IMGEDITOR:
                        cmd = imgeditor;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No default editor found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txteditor;
                    }
                }
                else // Any other is View
                {
                    cmd = association->command.section(',', 1);

                    // Use a default viewer if possible
                    switch (progs[cmd])
                    {
                    case TXTVIEWER:
                        cmd = txtviewer;
                        break;

                    case IMGVIEWER:
                        cmd = imgviewer;
                        break;

                    case PDFVIEWER:
                        cmd = pdfviewer;
                        break;

                    case AUDIOPLAYER:
                        cmd = audioplayer;
                        break;

                    case VIDEOPLAYER:
                        cmd = videoplayer;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No default viewer found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txtviewer;
                    }
                }
                if (cmd.text() != NULL)
                {
                    // First selected item
                    if (first)
                    {
                        samecmd = cmd;
                        first = false;
                    }

                    if (samecmd != cmd)
                    {
                        same = false;
                        break;
                    }

                    // List of selected items
                    itemslist += xf_quote(pathname) + " ";
                }
                else
                {
                    same = false;
                    break;
                }
            }
            // Text file
            else if (xf_istextfile(pathname) == 1)
            {
                if (FXSELID(sel) == ID_EDIT) // Edit
                {
                    cmd = txteditor;
                }
                else
                {
                    cmd = txtviewer;
                }

                // First selected item
                if (first)
                {
                    samecmd = cmd;
                    first = false;
                }

                if (samecmd != cmd)
                {
                    same = false;
                    break;
                }

                // List of selected items
                itemslist += xf_quote(pathname) + " ";
            }
            // Broken link
            else if (xf_istextfile(pathname) == -1)
            {
                return 1;
            }
            // No association
            else
            {
                same = false;
                break;
            }
        }
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Same association for all files : execute the associated or default editor or viewer
    if (same)
    {
        cmdname = samecmd;

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            cmd = cmdname + itemslist;
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
            runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
        }
        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
        }
    }
    // Files have different associations : handle them separately
    else
    {
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                pathname = current->list->getItemPathname(u);

                // Only View / Edit regular files (not directories)
                if (xf_isfile(pathname))
                {
                    association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to edit/view the file
                        if (FXSELID(sel) == ID_EDIT) // Edit
                        {
                            cmd = association->command.section(',', 2);

                            // Use a default editor if possible
                            switch (progs[cmd])
                            {
                            case TXTEDITOR:
                                cmd = txteditor;
                                break;

                            case IMGEDITOR:
                                cmd = imgeditor;
                                break;

                            case ARCHIVER:
                                cmd = archiver;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txteditor;
                            }
                        }
                        else // Any other is View
                        {
                            cmd = association->command.section(',', 1);

                            // Use a default viewer if possible
                            switch (progs[cmd])
                            {
                            case TXTVIEWER:
                                cmd = txtviewer;
                                break;

                            case IMGVIEWER:
                                cmd = imgviewer;
                                break;

                            case PDFVIEWER:
                                cmd = pdfviewer;
                                break;

                            case AUDIOPLAYER:
                                cmd = audioplayer;
                                break;

                            case VIDEOPLAYER:
                                cmd = videoplayer;
                                break;

                            case ARCHIVER:
                                cmd = archiver;
                                break;

                            case NONE: // No default viewer found
                                ;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txtviewer;
                            }
                        }

                        if (cmd.text() != NULL)
                        {
                            cmdname = cmd;

                            // If command exists, run it
                            if (xf_existcommand(cmdname))
                            {
                                cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                                runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                            }
                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
                                current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }
                    }
                    // No association
                    else
                    {
                        if (FXSELID(sel) == ID_EDIT)
                        {
                            cmd = txteditor;
                        }
                        else
                        {
                            cmd = txtviewer;
                        }

                        cmdname = cmd;

                        // If command exists, run it
                        if (xf_existcommand(cmdname))
                        {
                            cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
                            runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
                        }
                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                }
            }
        }
    }

    delete assocdict;
    getApp()->endWaitCursor();

    return 1;
}


// Compare two files
long FilePanel::onCmdCompare(FXObject*, FXSelector sel, void*)
{
    current->list->setFocus();
    int num = current->list->getNumSelectedItems();

    // Only one or two selected items can be handled
    if ((num != 1) && (num != 2))
    {
        getApp()->endWaitCursor();
        return 0;
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    FXString filecomparator = getApp()->reg().readStringEntry("PROGS", "filecomparator", DEFAULT_FILECOMPARATOR);
    FXString pathname, cmd, cmdname, itemslist = " ";

    // One selected item
    if (num == 1)
    {
        // Get the selected item
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                pathname = current->list->getItemPathname(u);
                itemslist += xf_quote(pathname) + " ";
            }
        }

        // Open a dialog to select the other item to be compared
        if (comparedialog == NULL)
        {
            comparedialog = new BrowseInputDialog(this, "", "", _("Compare"), _("With:"), bigcompareicon,
                                                  BROWSE_INPUT_FILE);
        }
        comparedialog->setIcon(bigcompareicon);
        comparedialog->setMessage(pathname);
        comparedialog->setText("");

        // Select browse input type
        if (xf_isdirectory(pathname))
        {
            comparedialog->setBrowseType(BROWSE_INPUT_FOLDER);
        }
        else
        {
            comparedialog->setBrowseType(BROWSE_INPUT_FILE);            
        }
        
        int rc = comparedialog->execute(PLACEMENT_CURSOR);

        // Get item path and add it to the list
        FXString str = comparedialog->getText();
        itemslist += xf_quote(str);
        if (!rc || (str == ""))
        {
            return 0;
        }
    }
    // Two selected items
    else if (num == 2)
    {
        // Get the two selected items
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                pathname = current->list->getItemPathname(u);
                itemslist += xf_quote(pathname) + " ";
            }
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // If command exists, run it
    cmdname = filecomparator;
    if (xf_existcommand(cmdname))
    {
        cmd = cmdname + itemslist;
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
        runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
    }
    // If command does not exist, issue an error message
    else
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"),
                          _("Program %s not found. Please define a file / folder comparator program in the Preferences dialog!"),
                          cmdname.text());
    }

    getApp()->endWaitCursor();

    return 1;
}


// File or directory properties
long FilePanel::onCmdProperties(FXObject* sender, FXSelector, void*)
{
    int ret;
    int num, itm;

    current->list->setFocus();

    // If no selected files in the file list, use the selected folder from the tree list (if any)
    num = current->list->getNumSelectedItems(&itm);
    if (num == 0)
    {
        return 0;
    }
    // There is one selected file in the file list
    else if (num == 1)
    {
        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        FXString path = current->list->getDirectory();
        FXString filename = current->list->getItemText(itm);
        filename = filename.section('\t', 0);
        PropertiesBox* attrdlg = new PropertiesBox(this, filename, path);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);
    }
    // There are multiple selected files in the file list
    else if (num > 1)
    {
        ret = chdir(current->list->getDirectory().text());
        if (ret < 0)
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                  current->list->getDirectory().text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                                  current->list->getDirectory().text());
            }

            return 0;
        }

        FXString path = current->list->getDirectory();
        FXString* files = new FXString[num];
        FXString* paths = new FXString[num];

        // Possibly deselect the '..' directory
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        int i = 0;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                files[i] = current->list->getItemText(u).section('\t', 0);
                paths[i] = path;
                i++;
            }
        }

        PropertiesBox* attrdlg = new PropertiesBox(this, files, num, paths);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);

        ret = chdir(startlocation.text());
        if (ret < 0)
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
            }

            return 0;
        }
    }

    // Force panel refresh
    return 1;
}


// Create new directory
long FilePanel::onCmdNewDir(FXObject*, FXSelector, void*)
{
    FXString dirname = "";

    // Focus on current panel list
    current->list->setFocus();

    FXString dirpath = current->list->getDirectory();
    if (dirpath != ROOTDIR)
    {
        dirpath += PATHSEPSTRING;
    }

    if (newdirdialog == NULL)
    {
        newdirdialog = new InputDialog(this, "", _("Create new folder:"), _("New Folder"), "", bigfoldernewicon);
    }
    newdirdialog->setText("");

    // Accept was pressed
    if (newdirdialog->execute(PLACEMENT_CURSOR))
    {
        if (newdirdialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Folder name is empty, operation cancelled"));
            return 0;
        }

        // Directory name contains '/'
        if (newdirdialog->getText().contains(PATHSEPCHAR))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"),
                                _("The / character is not allowed in folder names, operation cancelled"));
            return 0;
        }

        dirname = dirpath + newdirdialog->getText();
        if (dirname != dirpath)
        {
            // Check if file or folder already exists
            if (xf_existfile(dirname))
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("File or folder %s already exists"), dirname.text());
                return 0;
            }

            // Create the new dir according to the current umask
            int mask;
            mask = umask(0);
            umask(mask);

            // Note that the umask value is in decimal (511 means octal 0777)
            errno = 0;
            int ret = ::mkdir(dirname.text(), 511 & ~mask);
            int errcode = errno;
            if (ret == -1)
            {
                if (errcode)
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s: %s"), dirname.text(),
                                      strerror(errcode));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s"), dirname.text());
                }
                return 0;
            }
        }
    }
    // Cancel was pressed
    else
    {
        return 0;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    // Enable created item, if any (for keyboard navigation)
    FXString name;
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        name = current->list->getItemPathname(u);
        if (name == dirname)
        {
            current->list->enableItem(u);
            current->list->setCurrentItem(u);
            break;
        }
    }

    return 1;
}


// Create new file
long FilePanel::onCmdNewFile(FXObject*, FXSelector, void*)
{
    FXString filename = "";

    // Focus on current panel list
    current->list->setFocus();

    FXString pathname = current->list->getDirectory();
    if (pathname != ROOTDIR)
    {
        pathname += PATHSEPSTRING;
    }

    if (newfiledialog == NULL)
    {
        newfiledialog = new InputDialog(this, "", _("Create new file:"), _("New File"), "", bignewfileicon, false);
    }
    newfiledialog->setText("");

    // Accept was pressed
    if (newfiledialog->execute(PLACEMENT_CURSOR))
    {
        if (newfiledialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }

        // File name contains '/'
        if (newfiledialog->getText().contains(PATHSEPCHAR))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"),
                                _("The / character is not allowed in file names, operation cancelled"));
            return 0;
        }

        filename = pathname + newfiledialog->getText();
        FILE* file;
        if (filename != pathname)
        {
            // Check if file or folder already exists
            if (xf_existfile(filename))
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("File or folder %s already exists"), filename.text());
                return 0;
            }

            // Create the new file
            errno = 0;
            if (!(file = fopen(filename.text(), "w+")) || fclose(file))
            {
                if (errno)
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create file %s: %s"), filename.text(),
                                      strerror(errno));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create file %s"), filename.text());
                }
                return 0;
            }

            // Change the file permissions according to the current umask
            int mask;
            mask = umask(0);
            umask(mask);
            errno = 0;
            int rc = chmod(filename.text(), 438 & ~mask);
            int errcode = errno;
            if (rc)
            {
                if (errcode)
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't set permissions in %s: %s"),
                                      filename.text(), strerror(errcode));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't set permissions in %s"),
                                      filename.text());
                }
            }
        }
    }
    // Cancel was pressed
    else
    {
        return 0;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    // Enable created item, if any (for keyboard navigation)
    FXString name;
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        name = current->list->getItemPathname(u);
        if (name == filename)
        {
            current->list->enableItem(u);
            current->list->setCurrentItem(u);
            break;
        }
    }

    return 1;
}


// Create new symbolic link
long FilePanel::onCmdNewSymlink(FXObject*, FXSelector, void*)
{
    FXString linkname = "";

    // Focus on current panel list
    current->list->setFocus();

    FXString linkpath = current->list->getDirectory();
    if (linkpath != ROOTDIR)
    {
        linkpath += PATHSEPSTRING;
    }

    if (newlinkdialog == NULL)
    {
        newlinkdialog = new InputDialog(this, "", _("Create new symbolic link:"), _("New Symlink"), "", bignewlinkicon,
                                        false);
    }
    newlinkdialog->setText("");

    // Accept was pressed
    if (newlinkdialog->execute(PLACEMENT_CURSOR))
    {
        if (newlinkdialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }
        linkname = linkpath + newlinkdialog->getText();
        File* f;
        if (linkname != linkpath)
        {
            // Test some error conditions
            if (xf_existfile(linkname))
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("File or folder %s already exists"), linkname.text());
                return 0;
            }

            // Select target
            FileDialog browse(this, _("Select the symlink refered file or folder"));
            browse.setDirectory(linkpath);
            browse.setSelectMode(SELECT_FILE_MIXED);
            if (browse.execute())
            {
                FXString linksource = browse.getFilename();

                // Source does not exist
                if (!xf_existfile(linksource))
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Symlink source %s does not exist"),
                                      linksource.text());
                    return 0;
                }

                f = new File(this, _("Symlink"), SYMLINK);
                f->create();
                f->symlink(linksource, linkname);
                delete f;
            }
            //else
            //return 0;
        }
    }
    // Cancel was pressed
    else
    {
        return 0;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    // Enable created item, if any (for keyboard navigation)
    FXString name;
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        name = current->list->getItemPathname(u);
        if (name == linkname)
        {
            current->list->enableItem(u);
            current->list->setCurrentItem(u);
            break;
        }
    }

    return 1;
}


// Add bookmark
long FilePanel::onCmdAddBookmark(FXObject*, FXSelector, void*)
{
    FXString dirpathname = current->list->getDirectory();

    // Bookmark dialog
    if (addbookmarkdialog == NULL)
    {
        addbookmarkdialog = new BookmarkDialog(this, "", "", _("Add Bookmark"), _("Name:"),
                                               _("Icon:"), bigaddbookicon);
    }
    addbookmarkdialog->setName(FXPath::name(dirpathname));
    addbookmarkdialog->selectAll();

    FXString msg = FXString(_("Location:")) + " " + dirpathname;
    addbookmarkdialog->setMessage(msg);

    // Accept was pressed
    if (addbookmarkdialog->execute(PLACEMENT_CURSOR))
    {
        // Append bookmark
        FXString bookname = addbookmarkdialog->getName();
        FXString iconpathname = addbookmarkdialog->getIconPathname();
        int ret = dirpanel->getBookmarksList()->appendBookmark(bookname, dirpathname, iconpathname);
        if (ret == 0)
        {
            return 0;
        }
    }
    // Cancel was pressed
    else
    {
        return 0;
    }

    return 1;
}


// Open single or multiple files
long FilePanel::onCmdOpen(FXObject*, FXSelector, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

    FileAssoc* association;

    current->list->setFocus();
    if (current->list->getNumSelectedItems() == 0)
    {
        getApp()->endWaitCursor();
        return 0;
    }

    // Default programs
    FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
    FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
    FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
    FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
    FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
    FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
    FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

    // Update associations dictionary
    FileDict* assocdict = new FileDict(getApp());

    // Lists of command and items
    vector_FXString cmdlist;
    vector_FXString itemslist;

    // Ignored items
    FXString ignoreditems = "";

    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        if (current->list->isItemSelected(u))
        {
            FXString pathname = current->list->getItemPathname(u);

            // If directory, skip it
            if (xf_isdirectory(pathname))
            {
                continue;
            }

            // If association found
            association = assocdict->findFileBinding(pathname.text());
            if (association)
            {
                FXString cmd = association->command.section(',', 0);

                // Use a default program if possible
                switch (progs[cmd])
                {
                case TXTVIEWER:
                    cmd = txtviewer;
                    break;

                case TXTEDITOR:
                    cmd = txteditor;
                    break;

                case IMGVIEWER:
                    cmd = imgviewer;
                    break;

                case IMGEDITOR:
                    cmd = imgeditor;
                    break;

                case PDFVIEWER:
                    cmd = pdfviewer;
                    break;

                case AUDIOPLAYER:
                    cmd = audioplayer;
                    break;

                case VIDEOPLAYER:
                    cmd = videoplayer;
                    break;

                case ARCHIVER:
                    cmd = archiver;
                    break;

                case NONE: // No program found
                    ;
                    break;
                }

                // A command exists
                if (cmd != "")
                {
                    // Check if the command is already in the list
                    FXuint j;
                    FXbool found = false;
                    for (FXuint i = 0; i < cmdlist.size(); i++)
                    {
                        if (cmdlist[i] == cmd)
                        {
                            found = true;
                            j = i;
                            break;
                        }
                    }

                    // Command not in list, then add the command to the list and add the path name to the items list
                    if (!found)
                    {
                        cmdlist.push_back(cmd);
                        itemslist.push_back(xf_quote(pathname));
                    }
                    // Command already in list, then update the items list for that command
                    else
                    {
                        itemslist[j] = itemslist[j] + " " + xf_quote(pathname);
                    }
                }
            }
            // Or execute the file
            else if (current->list->isItemExecutable(u))
            {
                execFile(pathname);
            }
            // Or call the "Open with..." dialog
            else
            {
                // One item, call the "Open with..." dialog
                if (current->list->getNumSelectedItems() == 1)
                {
                    getApp()->endWaitCursor();
                    current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }
                // More than one item, add items to the ignored list
                else
                {
                    ignoreditems += " " + FXPath::name(pathname) + "\n";
                }
            }
        }
    }


#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Open files using the associated commands
    for (FXuint i = 0; i < cmdlist.size(); i++)
    {
        FXString cmdname = cmdlist[i];

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            FXString cmd = cmdname + " " + itemslist[i];
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
            runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
        }
    }

    // If there are ignored items, display a warning message
    if (ignoreditems.length() != 0)
    {
        FXString message;
        message.format(_("The following files have no association and were ignored:\n\n%s"), ignoreditems.text());
        MessageBox::warning(this, BOX_OK, _("Warning"), "%s", message.text());
    }

    delete assocdict;
    getApp()->endWaitCursor();

    return 1;
}


// Open with
long FilePanel::onCmdOpenWith(FXObject*, FXSelector, void*)
{
    char** str = NULL;

    current->list->setFocus();

    if (current->list->getNumSelectedItems() == 0)
    {
        return 0;
    }

    FXString cmd = "", cmdname;
    if (opendialog == NULL)
    {
        opendialog = new HistInputDialog(this, "", _("Open selected file(s) with:"), _("Open With"), "",
                                         bigfileopenicon, false, true, HIST_INPUT_EXECUTABLE_FILE, true, _("A&ssociate"));
    }
    opendialog->setText(cmd);

    // Dialog with history list and associate checkbox
    opendialog->CursorEnd();
    opendialog->selectAll();
    opendialog->clearItems();
    for (int i = 0; i < OpenNum; i++)
    {
        opendialog->appendItem(OpenHistory[i]);
    }
    opendialog->setDirectory(ROOTDIR);
    opendialog->sortItems();
    if (opendialog->execute())
    {
        cmd = opendialog->getText();
        if (cmd == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }

        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                // Handles "associate" checkbox for "open with..." dialog
                if (opendialog->getOption())
                {
                    FXString filename = current->list->getItemFilename(u);
                    FXString ext = filename.rafter('.', 2).lower();

                    if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") || (ext == "tar.zst") ||
                        (ext == "tar.z")) // Special cases
                    {
                    }
                    else
                    {
                        ext = FXPath::extension(filename).lower();
                    }

                    if (ext == "")
                    {
                        ext = FXPath::name(filename);
                    }

                    FileAssoc* association = current->list->getItemAssoc(u);

                    if (association)
                    {
                        // Update existing association
                        FXString oldfileassoc = getApp()->reg().readStringEntry("FILETYPES", ext.text(), "");
                        oldfileassoc.erase(0, oldfileassoc.section(';', 0).section(',', 0).length());
                        oldfileassoc.prepend(opendialog->getText());
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), oldfileassoc.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text()) + 1];
                        str[1] = new char[strlen(oldfileassoc.text()) + 1];
                        xf_strlcpy(str[0], ext.text(), ext.length() + 1);
                        xf_strlcpy(str[1], oldfileassoc.text(), oldfileassoc.length() + 1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                    else
                    {
                        // New association
                        FXString newcmd = opendialog->getText().append(";Document;;;;");
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), newcmd.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text()) + 1];
                        str[1] = new char[strlen(newcmd.text()) + 1];
                        xf_strlcpy(str[0], ext.text(), ext.length() + 1);
                        xf_strlcpy(str[1], newcmd.text(), newcmd.length() + 1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                }
                // End

                FXString pathname = current->list->getItemPathname(u);
                cmdname = cmd;
                cmd += " ";
                cmd = cmd + xf_quote(pathname);
            }
        }

        // Run command if it exists
        getApp()->beginWaitCursor();

#ifdef STARTUP_NOTIFICATION
        // Startup notification option and exceptions (if any)
        FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
        FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, current->list->getDirectory(), startlocation, usesn, snexcepts);
#else
            runcmd(cmd, current->list->getDirectory(), startlocation);
#endif
        }
        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            current->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            return 1;
        }

        // Update history list
        OpenNum = opendialog->getHistorySize();
        cmd = opendialog->getText();

        // Check if cmd is a new string, i.e. is not already in history
        FXbool newstr = true;
        for (int i = 0; i < OpenNum - 1; i++)
        {
            if (xf_strequal(OpenHistory[i], cmd.text()))
            {
                newstr = false;
                break;
            }
        }

        // History limit reached
        if (OpenNum > OPEN_HIST_SIZE)
        {
            OpenNum--;
        }

        // Restore original history order
        opendialog->clearItems();
        for (int i = 0; i < OpenNum; i++)
        {
            opendialog->appendItem(OpenHistory[i]);
        }

        // New string
        if (newstr)
        {
            // FIFO
            xf_strlcpy(OpenHistory[0], cmd.text(), cmd.length() + 1);
            for (int i = 1; i < OpenNum; i++)
            {
                xf_strlcpy(OpenHistory[i], opendialog->getHistoryItem(i - 1).text(),
                         opendialog->getHistoryItem(i - 1).length() + 1);
            }
        }

        getApp()->endWaitCursor();
    }

    // If list has been cleared, set history size to zero
    if (opendialog->getHistorySize() == 0)
    {
        OpenNum = 0;
    }

    return 1;
}


long FilePanel::onCmdItemFilter(FXObject* sender, FXSelector sel, void*)
{
    if (FilterNum == 0)
    {
        xf_strlcpy(FilterHistory[FilterNum], "*", 2);
        FilterNum++;
    }

    int i;
    FXString pat = list->getPattern();

    if (filterdialog == NULL)
    {
        filterdialog = new HistInputDialog(this, pat, _("Show:"), _("Filter"), "", bigfiltericon, false, true,
                                           HIST_INPUT_FILE, true, _("Also apply to folders"));
    }

    filterdialog->clearItems();
    filterdialog->CursorEnd();

    for (int i = 0; i < FilterNum; i++)
    {
        filterdialog->appendItem(FilterHistory[i]);
    }

    filterdialog->sortItems();
    filterdialog->setText(pat);

    // Set filter type
    if (filterdialog->execute() && ((pat = filterdialog->getText()) != ""))
    {
        // Set filter folders option
        list->setFilterFolders(filterdialog->getOption());

        // Change file list pattern
        if (FXSELID(sel) == ID_FILTER_CURRENT)
        {
            current->list->setPattern(pat);
        }
        else
        {
            list->setPattern(pat);
        }

        FXbool newstr = true;
        for (i = 0; i < FilterNum; i++)
        {
            if (xf_strequal(FilterHistory[i], pat.text()))
            {
                newstr = false;
                break;
            }
        }
        // Append new string to the list bottom
        if (newstr && (FilterNum < FILTER_HIST_SIZE))
        {
            xf_strlcpy(FilterHistory[FilterNum], pat.text(), pat.length() + 1);
            FilterNum++;
        }
    }

    list->onCmdRefresh(0, 0, 0);
    list->setFocus();

    // If list has been cleared, set history size to zero
    if (filterdialog->getHistorySize() == 0)
    {
        FilterNum = 0;
    }

    return 1;
}


// Panel context menu
long FilePanel::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // Make panel active
    setActive();

    // Check if control key or Shift-F10 or menu was pressed
    if (ptr != NULL)
    {
        FXEvent* event = (FXEvent*)ptr;
        if (event->state & CONTROLMASK)
        {
            ctrl = true;
        }
        if ((event->state & SHIFTMASK && event->code == KEY_F10) || event->code == KEY_Menu)
        {
            shiftf10 = true;
        }
    }

    // Use to select the item under cursor when right clicking
    // Only when Shift-F10 was not pressed
    if (!shiftf10 && (list->getNumSelectedItems() <= 1))
    {
        int x, y;
        FXuint state;
        list->getCursorPosition(x, y, state);

        int item = list->getItemAt(x, y);

        if (list->getCurrentItem() >= 0)
        {
            list->deselectItem(list->getCurrentItem());
        }
        if (item >= 0)
        {
            list->setCurrentItem(item);
            list->selectItem(item);
        }
    }

    // If first item (i.e. the '..' item)
    if ((list->getNumSelectedItems() == 1) && list->isItemSelected(0))
    {
        ctrl = true;
    }

    // If control flag is set, deselect all items
    if (ctrl)
    {
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), ptr);
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);
    int x, y;
    FXuint state;
    getRoot()->getCursorPosition(x, y, state);

    int num, itm;
    num = current->list->getNumSelectedItems(&itm);

    // Find if both selected items have the same file type
    // This is used to enable / disable compare menu item
    if (num == 2)
    {
        int u1 = 0;
        FXbool isfile1 = true, isfile2 = true;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                FXString pathname = current->list->getItemPathname(u);
               
                if (u1 == 0)
                {
                    u1 = u;
                    isfile1 = xf_isfile(pathname);
                }
                else
                {
                    isfile2 = xf_isfile(pathname);                    
                }
            }
        }
        // Indicates that both selected items have the same file type (files or directories)
        sameFileType = (isfile1 == isfile2 ? true : false);
    }

    // No selection or control flag set
    if ((num == 0) || current->ctrl)
    {
        // Menu items
        new FXMenuCommand(menu, _("New& File..."), mininewfileicon, current, FilePanel::ID_NEW_FILE);
        new FXMenuCommand(menu, _("New F&older..."), minifoldernewicon, current, FilePanel::ID_NEW_DIR);
        new FXMenuCommand(menu, _("New S&ymlink..."), mininewlinkicon, current, FilePanel::ID_NEW_SYMLINK);
        new FXMenuCommand(menu, _("&Add Bookmark..."), miniaddbookicon, this, FilePanel::ID_ADD_BOOKMARK);
        new FXMenuCommand(menu, _("Fi&lter..."), minifiltericon, current, FilePanel::ID_FILTER);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Paste"), minipasteicon, current, FilePanel::ID_PASTE_CLIPBOARD);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("&Hidden Files"), current->list, FileList::ID_TOGGLE_HIDDEN);
        new FXMenuCheck(menu, _("Thum&bnails"), current->list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("B&ig Icons"), current->list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(menu, _("&Small Icons"), current->list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(menu, _("&Detailed File List"), current->list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("Autos&ize"), current->list, FileList::ID_AUTOSIZE);
        new FXMenuRadio(menu, _("&Rows"), current->list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(menu, _("&Columns"), current->list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("&Name"), current->list, FileList::ID_COL_NAME);
        new FXMenuRadio(menu, _("Si&ze"), current->list, FileList::ID_COL_SIZE);
        new FXMenuRadio(menu, _("&Type"), current->list, FileList::ID_COL_TYPE);
        new FXMenuRadio(menu, _("E&xtension"), current->list, FileList::ID_COL_EXT);
        new FXMenuRadio(menu, _("&Date"), current->list, FileList::ID_COL_DATE);
        new FXMenuRadio(menu, _("&User"), current->list, FileList::ID_COL_USER);
        new FXMenuRadio(menu, _("&Group"), current->list, FileList::ID_COL_GROUP);
        new FXMenuRadio(menu, _("Per&missions"), current->list, FileList::ID_COL_PERMS);
        new FXMenuRadio(menu, _("&Link"), current->list, FileList::ID_COL_LINK);
        new FXMenuRadio(menu, _("&Deletion Date"), current->list, FileList::ID_COL_DELDATE);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("Ignore C&ase"), current->list, FileList::ID_SORT_CASE);
        new FXMenuCheck(menu, _("Fold&ers First"), current->list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(menu, _("Re&verse Order"), current->list, FileList::ID_SORT_REVERSE);
    }
    // Non empty selection
    else
    {
        // Deselect the '..' item
        if (current->list->isItemSelected(0))
        {
            current->list->deselectItem(0);
        }

        // Panel submenu items
        FXMenuPane* submenu = new FXMenuPane(this);
        new FXMenuCommand(submenu, _("Ne&w File..."), mininewfileicon, current, FilePanel::ID_NEW_FILE);
        new FXMenuCommand(submenu, _("New F&older..."), minifoldernewicon, current, FilePanel::ID_NEW_DIR);
        new FXMenuCommand(submenu, _("New S&ymlink..."), mininewlinkicon, current, FilePanel::ID_NEW_SYMLINK);
        new FXMenuCommand(submenu, _("&Add Bookmark..."), miniaddbookicon, this, FilePanel::ID_ADD_BOOKMARK);
        new FXMenuCommand(submenu, _("Fi&lter..."), minifiltericon, current, FilePanel::ID_FILTER);
        new FXMenuSeparator(submenu);
        new FXMenuCommand(submenu, _("&Paste"), minipasteicon, current, FilePanel::ID_PASTE_CLIPBOARD);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("&Hidden Files"), current->list, FileList::ID_TOGGLE_HIDDEN);
        new FXMenuCheck(submenu, _("Thum&bnails"), current->list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("B&ig Icons"), current->list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(submenu, _("&Small Icons"), current->list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(submenu, _("&Detailed File List"), current->list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("Autos&ize"), current->list, FileList::ID_AUTOSIZE);
        new FXMenuRadio(submenu, _("&Rows"), current->list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(submenu, _("&Columns"), current->list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("&Name"), current->list, FileList::ID_COL_NAME);
        new FXMenuRadio(submenu, _("Si&ze"), current->list, FileList::ID_COL_SIZE);
        new FXMenuRadio(submenu, _("&Type"), current->list, FileList::ID_COL_TYPE);
        new FXMenuRadio(submenu, _("E&xtension"), current->list, FileList::ID_COL_EXT);
        new FXMenuRadio(submenu, _("&Date"), current->list, FileList::ID_COL_DATE);
        new FXMenuRadio(submenu, _("&User"), current->list, FileList::ID_COL_USER);
        new FXMenuRadio(submenu, _("&Group"), current->list, FileList::ID_COL_GROUP);
        new FXMenuRadio(submenu, _("Per&missions"), current->list, FileList::ID_COL_PERMS);
        new FXMenuRadio(submenu, _("&Link"), current->list, FileList::ID_COL_LINK);
        new FXMenuRadio(submenu, _("&Deletion Date"), current->list, FileList::ID_COL_DELDATE);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("Ignore C&ase"), current->list, FileList::ID_SORT_CASE);
        new FXMenuCheck(submenu, _("Fold&ers First"), current->list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(submenu, _("Re&verse Order"), current->list, FileList::ID_SORT_REVERSE);
        new FXMenuCascade(menu, _("Pane&l"), NULL, submenu);
        new FXMenuSeparator(menu);

#if defined(linux)
        FXString name = current->list->getItemPathname(itm);
        if ((num == 1) && (fsdevices->find(name.text()) || mtdevices->find(name.text())))
        {
            new FXMenuCommand(menu, _("&Mount"), minimounticon, current, FilePanel::ID_MOUNT);
            new FXMenuCommand(menu, _("Unmount"), miniunmounticon, current, FilePanel::ID_UMOUNT);
            new FXMenuSeparator(menu);
        }
#endif

        FXbool ar = false;
        if (current->list->getItem(itm) && current->list->isItemFile(itm))
        {
            new FXMenuCommand(menu, _("Open &With..."), minifileopenicon, current, FilePanel::ID_OPEN_WITH);
            new FXMenuCommand(menu, _("&Open"), minifileopenicon, current, FilePanel::ID_OPEN);
            FXString name = current->list->getItemText(itm).section('\t', 0);

            // Last and before last file extensions
            FXString ext1 = name.rafter('.', 1).lower();
            FXString ext2 = name.rafter('.', 2).lower();

            // Destination folder name
            FXString extract_to_folder;
            if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") ||
                (ext2 == "tar.z"))
            {
                extract_to_folder = _("Extr&act to Folder ") + name.section('\t', 0).rbefore('.', 2);
            }
            else
            {
                extract_to_folder = _("Extr&act to Folder ") + name.section('\t', 0).rbefore('.', 1);
            }

            // Display the extract and package menus according to the archive extensions
            if ((num == 1) &&
                ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") ||
                 (ext2 == "tar.z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&Extract Here"), miniarchexticon, current, FilePanel::ID_EXTRACT_HERE);
                new FXMenuCommand(menu, extract_to_folder, miniarchexticon, current, FilePanel::ID_EXTRACT_TO_FOLDER);
                new FXMenuCommand(menu, _("E&xtract to..."), miniarchexticon, current, FilePanel::ID_EXTRACT);
            }
            else if ((num == 1) &&
                     ((ext1 == "gz") || (ext1 == "bz2") || (ext1 == "zst") || (ext1 == "xz") || (ext1 == "z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&Extract here"), miniarchexticon, current, FilePanel::ID_EXTRACT_HERE);
            }
            else if ((num == 1) && ((ext1 == "tar") || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz")
                                    || (ext1 == "tzst") || (ext1 == "taz") || (ext1 == "txz") || (ext1 == "zip") ||
                                    (ext1 == "7z")
                                    || (ext1 == "lzh") || (ext1 == "rar") || (ext1 == "ace") || (ext1 == "arj")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&Extract Here"), miniarchexticon, current, FilePanel::ID_EXTRACT_HERE);
                new FXMenuCommand(menu, extract_to_folder, miniarchexticon, current, FilePanel::ID_EXTRACT_TO_FOLDER);
                new FXMenuCommand(menu, _("E&xtract to..."), miniarchexticon, current, FilePanel::ID_EXTRACT);
            }
#if defined(linux)
            else if ((num == 1) && ((ext1 == "rpm") || (ext1 == "deb")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&View"), minipackageicon, current, FilePanel::ID_VIEW);
                new FXMenuCommand(menu, _("Install / Up&grade"), minipackageicon, current, ID_PKG_INSTALL);
                new FXMenuCommand(menu, _("Un&install"), minipackageicon, current, ID_PKG_UNINSTALL);
            }
#endif
            // Not archive nor package
            if (!ar)
            {
                new FXMenuCommand(menu, _("&View"), miniviewicon, current, FilePanel::ID_VIEW);
                new FXMenuCommand(menu, _("&Edit"), miniediticon, current, FilePanel::ID_EDIT);
            }
        }
        
        // Not a file
        if (!ar)
        {
            // Not archive nor package
            if (num == 1)
            {
                new FXMenuCommand(menu, _("Com&pare..."), minicompareicon, current, FilePanel::ID_COMPARE);
            }
            else
            {
                new FXMenuCommand(menu, _("Com&pare"), minicompareicon, current, FilePanel::ID_COMPARE);
            }
            new FXMenuCommand(menu, _("&Add to Archive..."), miniarchaddicon, current, FilePanel::ID_ADD_TO_ARCH);
        }
#if defined(linux)
        if ((num == 1) && !ar && current->list->isItemFile(itm))
        {
            new FXMenuCommand(menu, _("Packages &Query "), minipackageicon, current, FilePanel::ID_PKG_QUERY);
        }
#endif

        // Build scripts menu
        new FXMenuSeparator(menu);
        FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;
        FXMenuPane* scriptsmenu = new FXMenuPane(this);
        new FXMenuCascade(menu, _("Scripts"), minirunicon, scriptsmenu);
        readScriptDir(this, scriptsmenu, scriptpath);
        new FXMenuSeparator(scriptsmenu);
        new FXMenuCommand(scriptsmenu, _("&Go to Script Folder"), minigotodiricon, this, FilePanel::ID_GO_SCRIPTDIR);

        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Copy"), minicopyicon, current, FilePanel::ID_COPY_CLIPBOARD);
        new FXMenuCommand(menu, _("C&ut"), minicuticon, current, FilePanel::ID_CUT_CLIPBOARD);
        new FXMenuCommand(menu, _("&Paste"), minipasteicon, current, FilePanel::ID_PASTE_CLIPBOARD);
        if (num == 1)
        {
            new FXMenuCommand(menu, _("Cop&y Name"), minicopyicon, current, FilePanel::ID_COPYNAME_CLIPBOARD);
        }
        else
        {
            new FXMenuCommand(menu, _("Cop&y Names"), minicopyicon, current, FilePanel::ID_COPYNAME_CLIPBOARD);
        }
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Re&name..."), minirenameicon, current, FilePanel::ID_FILE_RENAME);
        new FXMenuCommand(menu, _("Copy &to..."), minicopyicon, current, FilePanel::ID_FILE_COPYTO);
        new FXMenuCommand(menu, _("&Move to..."), minimoveicon, current, FilePanel::ID_FILE_MOVETO);
        new FXMenuCommand(menu, _("Symlin&k to..."), minilinktoicon, current, FilePanel::ID_FILE_SYMLINK);
        new FXMenuCommand(menu, _("M&ove to Trash"), minideleteicon, current, FilePanel::ID_FILE_TRASH);
        new FXMenuCommand(menu, _("Restore &From Trash"), minirestoreicon, current, FilePanel::ID_FILE_RESTORE);
        new FXMenuCommand(menu, _("&Delete"), minideletepermicon, current, FilePanel::ID_FILE_DELETE);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Compare &Sizes"), minicharticon, current, FilePanel::ID_DIR_USAGE);
        new FXMenuCommand(menu, _("P&roperties"), miniattribicon, current, FilePanel::ID_PROPERTIES);
    }
    menu->create();

    // Reset flags
    ctrl = false;
    shiftf10 = false;

    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);

    return 1;
}


// Read all executable file names that are located into the script directory
// Sort entries alphabetically, directories first
int FilePanel::readScriptDir(FXWindow* owner, FXMenuPane* scriptsmenu, FXString dir)
{
    DIR* dp;
    struct dirent** namelist;

    // Open directory
    if ((dp = opendir(dir.text())) == NULL)
    {
        return 0;
    }

    // Possibly add a / at the end of the directory name
    if (dir[dir.length() - 1] != '/')
    {
        dir = dir + "/";
    }

    // First, read only directory entries and sort them alphabetically
    int n;
    n = scandir(dir.text(), &namelist, NULL, alphasort);
    if (n < 0)
    {
        perror("scandir");
    }
    else
    {
        for (int k = 0; k < n; k++)
        {
            // Avoid hidden directories and '.' and '..'
            if (namelist[k]->d_name[0] != '.')
            {
                FXString pathname = dir + namelist[k]->d_name;

                // Recurse if non empty directory
                if (xf_isdirectory(pathname))
                {
                    if (!xf_isemptydir(pathname))
                    {
                        FXMenuPane* submenu = new FXMenuPane(owner);
                        new FXMenuCascade(scriptsmenu, namelist[k]->d_name, NULL, submenu);
                        readScriptDir(this, submenu, pathname);
                    }
                }
            }
            free(namelist[k]);
        }
        free(namelist);
    }

    // Then, read only executable files and sort them alphabetically
    n = scandir(dir.text(), &namelist, NULL, alphasort);
    if (n < 0)
    {
        perror("scandir");
    }
    else
    {
        for (int k = 0; k < n; k++)
        {
            // Add only executable files to the list
            FXString pathname = dir + namelist[k]->d_name;
            if (!xf_isdirectory(pathname) && xf_isreadexecutable(pathname))
            {
                new FXMenuCommand(scriptsmenu, namelist[k]->d_name + FXString("\t\t") + pathname, miniexecicon, this,
                                  FilePanel::ID_RUN_SCRIPT);
            }
            free(namelist[k]);
        }
        free(namelist);
    }

    // Close directory
    (void)closedir(dp);

    return 1;
}


// Run Terminal in the selected directory
long FilePanel::onCmdXTerm(FXObject*, FXSelector, void*)
{
    int ret;

    getApp()->beginWaitCursor();
    ret = chdir(current->list->getDirectory().text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                              current->list->getDirectory().text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                              current->list->getDirectory().text());
        }

        return 0;
    }

    FXString cmd = getApp()->reg().readStringEntry("PROGS", "xterm", "xterm -sb");
    cmd += " &";
    ret = system(cmd.text());
    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
    }

    current->list->setFocus();
    ret = chdir(startlocation.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
        }

        return 0;
    }

    getApp()->endWaitCursor();
    return 1;
}


// Add files or directory to an archive
long FilePanel::onCmdAddToArch(FXObject* sender, FXSelector, void*)
{
    int ret;
    FXString name, ext1, ext2, cmd, archive = "";
    File* f;

    ret = chdir(current->list->getDirectory().text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), list->getDirectory().text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), list->getDirectory().text());
        }

        return 0;
    }

    // Possibly deselect the '..' directory
    if (current->list->isItemSelected(0))
    {
        current->list->deselectItem(0);
    }

    // Return if nothing is selected
    if (current->list->getNumSelectedItems() == 0)
    {
        return 0;
    }

    // If only one item is selected, use its name as a starting guess for the archive name
    if (current->list->getNumSelectedItems() == 1)
    {
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                name = current->list->getItemFilename(u);
                break;
            }
        }
        archive = name;
    }

    // Initial archive name with full path and default extension
    FXString archpath = current->list->getDirectory();
    if (archpath == PATHSEPSTRING)
    {
        archive = archpath + archive + ".tar.gz";
    }
    else
    {
        archive = archpath + PATHSEPSTRING + archive + ".tar.gz";
    }

    // Archive dialog
    if (archdialog == NULL)
    {
        archdialog = new ArchInputDialog(this, "");
    }
    archdialog->setText(archive);
    archdialog->CursorEnd();

    if (archdialog->execute())
    {
        if (archdialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }

        // Get string and preserve escape characters
        archive = xf_quote(archdialog->getText());

        // Get extensions of the archive name
        ext1 = archdialog->getText().rafter('.', 1).lower();
        ext2 = archdialog->getText().rafter('.', 2).lower();

        // Handle different archive formats
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst"))
        {
            cmd = "tar -acvf " + archive + " ";
        }
        else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "tzst"))
        {
            cmd = "tar -acvf " + archive + " ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -cvf " + archive + " ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gzip -v ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bzip2 -v ";
        }
        else if (ext1 == "zst")
        {
            cmd = "zstd ";
        }
        else if (ext1 == "xz")
        {
            cmd = "xz -v ";
        }
        else if (ext1 == "zip")
        {
            cmd = "zip -r " + archive + " ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z a " + archive + " ";
        }
        else if (ext1 == "rar")
        {
            cmd = "rar a " + archive + " ";
        }
        // Default archive format
        else
        {
            archive += ".tar.gz";
            cmd = "tar -acvf " + archive + " ";
        }

        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                // Don't include '..' in the list
                name = current->list->getItemFilename(u);
                if (name != "..")
                {
                    cmd += " ";
                    cmd = cmd + xf_quote(name);
                    cmd += " ";
                }
            }
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Create archive"), ARCHIVE);
        f->create();

        // Create archive
        f->archive(archive, cmd);
        ret = chdir(startlocation.text());
        if (ret < 0)
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
            }

            delete f;
            return 0;
        }

        delete f;
        getApp()->endWaitCursor();

        // Force panel refresh
        onCmdRefresh(0, 0, 0);
    }
    return 1;
}


// Extract archive to selected folder
long FilePanel::onCmdExtract(FXObject*, FXSelector, void*)
{
    FXString name, ext1, ext2, cmd, dir, cdir;
    File* f;

    // Current directory
    cdir = current->list->getDirectory();

    // File selection dialog
    FileDialog browse(this, _("Select a Destination Folder"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };
    browse.setDirectory(homedir);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_DIRECTORY);

    int item;
    current->list->getNumSelectedItems(&item);
    if (current->list->getItem(item))
    {
        // Archive name and extensions
        name = current->list->getItemText(item).text();

        ext1 = name.section('\t', 0).rafter('.', 1).lower();
        ext2 = name.section('\t', 0).rafter('.', 2).lower();
        name = xf_quote(cdir + PATHSEPSTRING + name.section('\t', 0));

        // Handle different archive formats
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z"))
        {
            cmd = "tar -axvf ";
        }
        else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                 (ext1 == "tzst"))
        {
            cmd = "tar -axvf ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -xvf ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gunzip -v ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bunzip2 -v ";
        }
        else if (ext1 == "zst")
        {
            cmd = "zstd -df ";
        }
        else if (ext1 == "xz")
        {
            cmd = "unxz -v ";
        }
        else if (ext1 == "z")
        {
            cmd = "uncompress -v ";
        }
        else if (ext1 == "zip")
        {
            cmd = "unzip -o ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z x -y ";
        }
        else if (ext1 == "rar")
        {
            cmd = "unrar x -o+ ";
        }
        else if (ext1 == "lzh")
        {
            cmd = "lha -xf ";
        }
        else if (ext1 == "ace")
        {
            cmd = "unace x ";
        }
        else if (ext1 == "arj")
        {
            cmd = "arj x -y ";
        }
        else
        {
            cmd = "tar -axvf ";
        }

        // Final extract command
        cmd += name + " ";


        // Extract archive
        if (browse.execute())
        {
            dir = browse.getFilename();

            if (xf_iswritable(dir))
            {
                // Wait cursor
                getApp()->beginWaitCursor();

                // File object
                f = new File(this, _("Extract Archive"), EXTRACT);
                f->create();

                // Extract archive
                f->extract(name, dir, cmd);

                getApp()->endWaitCursor();
                delete f;
            }
            else
            {
                MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"),
                                  dir.text());
            }
        }
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


// Extract archive to a folder name based on the archive name
long FilePanel::onCmdExtractToFolder(FXObject*, FXSelector, void*)
{
    FXString name, pathname, ext1, ext2, cmd, dirname, dirpath, cdir;
    File* f;

    // Current directory
    cdir = current->list->getDirectory();

    int item;
    current->list->getNumSelectedItems(&item);
    if (current->list->getItem(item))
    {
        // Archive name and extensions
        name = current->list->getItemText(item).text();
        ext1 = name.section('\t', 0).rafter('.', 1).lower();
        ext2 = name.section('\t', 0).rafter('.', 2).lower();

        // Destination folder name
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z"))
        {
            dirname = name.section('\t', 0).rbefore('.', 2);
        }
        else
        {
            dirname = name.section('\t', 0).rbefore('.', 1);
        }

        // Create the new dir according to the current umask
        // Don't complain if directory already exists
        int mask = umask(0);
        umask(mask);
        dirpath = cdir + PATHSEPSTRING + dirname;
        errno = 0;
        int ret = ::mkdir(dirpath.text(), 511 & ~mask);
        int errcode = errno;
        if ((ret == -1) && (errcode != EEXIST))
        {
            if (errcode)
            {
                MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s: %s"), dirpath.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s"), dirpath.text());
            }
            return 0;
        }

        // Archive pathname
        pathname = xf_quote(cdir + PATHSEPSTRING + name.section('\t', 0));

        // Handle different archive formats
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z"))
        {
            cmd = "tar -axvf ";
        }
        else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                 (ext1 == "tzst"))
        {
            cmd = "tar -axvf ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -xvf ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gunzip -v ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bunzip2 -v ";
        }
        else if (ext1 == "zst")
        {
            cmd = "zstd -df ";
        }
        else if (ext1 == "xz")
        {
            cmd = "unxz -v ";
        }
        else if (ext1 == "z")
        {
            cmd = "uncompress -v ";
        }
        else if (ext1 == "zip")
        {
            cmd = "unzip -o ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z x -y ";
        }
        else if (ext1 == "rar")
        {
            cmd = "unrar x -o+ ";
        }
        else if (ext1 == "lzh")
        {
            cmd = "lha -xf ";
        }
        else if (ext1 == "ace")
        {
            cmd = "unace x ";
        }
        else if (ext1 == "arj")
        {
            cmd = "arj x -y ";
        }
        else
        {
            cmd = "tar -axvf ";
        }

        // Final extract command
        cmd += pathname + " ";

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Extract Archive"), EXTRACT);
        f->create();

        // Extract archive
        f->extract(pathname, dirpath, cmd);

        getApp()->endWaitCursor();
        delete f;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


// Extract archive in the current directory
long FilePanel::onCmdExtractHere(FXObject*, FXSelector, void*)
{
    FXString name, ext1, ext2, cmd, cdir;
    File* f;

    // Current directory
    cdir = current->list->getDirectory();

    int item;
    current->list->getNumSelectedItems(&item);
    if (current->list->getItem(item))
    {
        if (xf_iswritable(cdir))
        {
            // Archive name and extensions
            name = current->list->getItemText(item).text();
            ext1 = name.section('\t', 0).rafter('.', 1);
            lower();
            ext2 = name.section('\t', 0).rafter('.', 2).lower();
            name = xf_quote(cdir + PATHSEPSTRING + name.section('\t', 0));

            // Handle different archive formats
            if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") ||
                (ext2 == "tar.z"))
            {
                cmd = "tar -axvf ";
            }
            else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                     (ext1 == "tzst"))
            {
                cmd = "tar -axvf ";
            }
            else if (ext1 == "tar")
            {
                cmd = "tar -xvf ";
            }
            else if (ext1 == "gz")
            {
                cmd = "gunzip -v ";
            }
            else if (ext1 == "bz2")
            {
                cmd = "bunzip2 -v ";
            }
            else if (ext1 == "zst")
            {
                cmd = "zstd -df ";
            }
            else if (ext1 == "xz")
            {
                cmd = "unxz -v ";
            }
            else if (ext1 == "z")
            {
                cmd = "uncompress -v ";
            }
            else if (ext1 == "zip")
            {
                cmd = "unzip -o ";
            }
            else if (ext1 == "7z")
            {
                cmd = "7z x -y ";
            }
            else if (ext1 == "rar")
            {
                cmd = "unrar x -o+ ";
            }
            else if (ext1 == "lzh")
            {
                cmd = "lha -xf ";
            }
            else if (ext1 == "ace")
            {
                cmd = "unace x ";
            }
            else if (ext1 == "arj")
            {
                cmd = "arj x -y ";
            }
            else
            {
                cmd = "tar -axvf ";
            }

            // Final extract command
            cmd += name + " ";

            // Wait cursor
            getApp()->beginWaitCursor();

            // File object
            f = new File(this, _("Extract Archive"), EXTRACT);
            f->create();

            // Extract archive
            f->extract(name, cdir, cmd);

            getApp()->endWaitCursor();
            delete f;
        }
        else
        {
            MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), cdir.text());
        }
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


#if defined(linux)
// Install/Upgrade package
long FilePanel::onCmdPkgInstall(FXObject*, FXSelector, void*)
{
    FXString name, path, cmd, dir, cdir;
    File* f;

    cdir = current->list->getDirectory();

    int itm;
    current->list->getNumSelectedItems(&itm);
    if (current->list->getItem(itm))
    {
        name = current->list->getItemText(itm).text();
        name = name.section('\t', 0);
        path = xf_quote(cdir + PATHSEPSTRING + name);

        // Command to perform
        FXString ext = FXPath::extension(name);
        if (comparecase(ext, "rpm") == 0)
        {
            cmd = "rpm -Uvh " + path;
        }
        else if (comparecase(ext, "deb") == 0)
        {
            cmd = "dpkg -i " + path;
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Package Install / Upgrade"), PKG_INSTALL);
        f->create();

        // Install/Upgrade package
        f->pkgInstall(name, cmd);

        getApp()->endWaitCursor();
        delete f;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


// Uninstall package based on its name (package version is ignored)
long FilePanel::onCmdPkgUninstall(FXObject*, FXSelector, void*)
{
    FXString name, cmd, dir, cdir;
    File* f;

    cdir = current->list->getDirectory();

    int itm;
    current->list->getNumSelectedItems(&itm);
    if (current->list->getItem(itm))
    {
        name = current->list->getItemText(itm).text();
        name = name.section('\t', 0);

        // Command to perform
        FXString ext = FXPath::extension(name);
        if (comparecase(ext, "rpm") == 0)
        {
            name = name.section('-', 0);
            cmd = "rpm -e " + name;
        }
        else if (comparecase(ext, "deb") == 0)
        {
            name = name.section('_', 0);
            cmd = "dpkg -r " + name;
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Package Uninstall"), PKG_UNINSTALL);
        f->create();

        // Uninstall package
        f->pkgUninstall(name, cmd);

        getApp()->endWaitCursor();
        delete f;
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


#endif


// Force FilePanel and DirPanel refresh
long FilePanel::onCmdRefresh(FXObject*, FXSelector, void*)
{
    // Refresh panel
    FXString dir = list->getDirectory();

    list->setDirectory(ROOTDIR, false);
    list->setDirectory(dir, false);
    updatePath();

    // Focus on current panel
    current->list->setFocus();

    return 1;
}


// Handle item selection
long FilePanel::onCmdSelect(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->setFocus();
    switch (FXSELID(sel))
    {
    case ID_SELECT_ALL:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_ALL), ptr);
        return 1;

    case ID_DESELECT_ALL:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), ptr);
        return 1;

    case ID_SELECT_INVERSE:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_INVERSE), ptr);
        return 1;
    }
    return 1;
}


// Handle show commands
long FilePanel::onCmdShow(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_SHOW_BIG_ICONS:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SHOW_BIG_ICONS), ptr);
        break;

    case ID_SHOW_MINI_ICONS:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SHOW_MINI_ICONS), ptr);
        break;

    case ID_SHOW_DETAILS:
        current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SHOW_DETAILS), ptr);
        break;
    }

    // Set focus on current panel list
    current->list->setFocus();

    return 1;
}


// Update show commands
long FilePanel::onUpdShow(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuint msg = FXWindow::ID_UNCHECK;
    FXuint style = current->list->getListStyle();

    switch (FXSELID(sel))
    {
    case ID_SHOW_BIG_ICONS:
        if (style & ICONLIST_BIG_ICONS)
        {
            msg = FXWindow::ID_CHECK;
        }
        break;

    case ID_SHOW_MINI_ICONS:
        if (style & ICONLIST_MINI_ICONS)
        {
            msg = FXWindow::ID_CHECK;
        }
        break;

    case ID_SHOW_DETAILS:
        if (!(style & (ICONLIST_MINI_ICONS | ICONLIST_BIG_ICONS)))
        {
            msg = FXWindow::ID_CHECK;
        }
        break;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);

    return 1;
}


// Handle toggle hidden command
long FilePanel::onCmdToggleHidden(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_TOGGLE_HIDDEN), ptr);
    return 1;
}


// Update toggle hidden command
long FilePanel::onUpdToggleHidden(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuint msg = FXWindow::ID_UNCHECK;
    FXbool hidden = current->list->shownHiddenFiles();

    if (hidden == false)
    {
        msg = FXWindow::ID_CHECK;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);
    return 1;
}


// Handle toggle thumbnails command
long FilePanel::onCmdToggleThumbnails(FXObject* sender, FXSelector sel, void* ptr)
{
    current->list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_TOGGLE_THUMBNAILS), ptr);
    return 1;
}


// Update toggle hidden command
long FilePanel::onUpdToggleThumbnails(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuint msg = FXWindow::ID_UNCHECK;
    FXbool showthumb = current->list->shownThumbnails();

    if (showthumb == false)
    {
        msg = FXWindow::ID_CHECK;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);
    return 1;
}


// Run script
long FilePanel::onCmdRunScript(FXObject* sender, FXSelector sel, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

    FXString pathname, cmd, itemslist = " ";
    FXString scriptpath = dynamic_cast<FXMenuCommand*>(sender)->getHelpText();

    // Construct selected files list
    current->list->setFocus();
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        if (current->list->isItemSelected(u))
        {
            pathname = current->list->getItemPathname(u);

            // List of selected items
            itemslist += xf_quote(pathname) + " ";
        }
    }

    // Construct command line
    cmd = xf_quote(scriptpath) + itemslist + " &";

    // Go to the current directory
    int ret = chdir(current->list->getDirectory().text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                              current->list->getDirectory().text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                              current->list->getDirectory().text());
        }
    }

    // Execute command
    static pid_t child_pid = 0;
    switch ((child_pid = fork()))
    {
    case -1:
        fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));
        break;

    case 0:
        execl("/bin/sh", "sh", "-c", cmd.text(), (char*)NULL);
        _exit(EXIT_SUCCESS);
        break;
    }

    // Return to the starting directory
    ret = chdir(startlocation.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
        }
    }

    getApp()->endWaitCursor();

    return 1;
}


// Go to scripts directory
long FilePanel::onCmdGoScriptDir(FXObject* sender, FXSelector sel, void*)
{
    FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;

    if (!xf_existfile(scriptpath))
    {
        // Create the script directory according to the umask
        int mask = umask(0);
        umask(mask);
        errno = 0;
        int ret = xf_mkpath(scriptpath.text(), 511 & ~mask);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s: %s"), scriptpath.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s"), scriptpath.text());
            }

            return 0;
        }
    }

    // Go to scripts directory
    current->list->setDirectory(scriptpath);
    current->list->setFocus();
    dirpanel->setDirectory(scriptpath, true);
    current->updatePath();
    updateLocation();

    return 1;
}


#if defined(linux)
// Mount/Unmount file systems
long FilePanel::onCmdMount(FXObject*, FXSelector sel, void*)
{
    int ret;
    FXString cmd, msg, msgwait;
    FXuint op;
    File* f;
    FXString dir;

    current->list->setFocus();

    // Use the selected directory in FilePanel if any or use the selected directory in DirPanel
    if (current->list->getNumSelectedItems() == 0)
    {
        dir = current->list->getDirectory();
    }
    else
    {
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                dir = current->list->getItemPathname(u);
            }
        }
    }

    // If symbolic link, read the linked directory
    if (xf_islink(dir))
    {
        dir = xf_readlink(dir);
    }

    if (FXSELID(sel) == ID_MOUNT)
    {
        op = MOUNT;
        msg = _("Mount file system...");
        msgwait = _("Volume busy, please wait while mounting folder:");
        cmd = getApp()->reg().readStringEntry("PROGS", "mount", DEFAULT_MOUNTCMD) + FXString(" ");
    }
    else
    {
        op = UNMOUNT;
        msg = _("Unmount file system...");
        msgwait = _("Volume busy, please wait while unmounting folder:");
        cmd = getApp()->reg().readStringEntry("PROGS", "unmount", DEFAULT_UMOUNTCMD) + FXString(" ");
    }
    cmd += xf_quote(dir);
    cmd += " 2>&1";
    ret = chdir(ROOTDIR);
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), ROOTDIR, strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), ROOTDIR);
        }

        return 0;
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // File object
    f = new File(this, msg.text(), op);
    f->create();

    // Mount/unmount file system
    f->mount(dir, msgwait, cmd);

    ret = chdir(startlocation.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
        }

        delete f;
        return 0;
    }

    delete f;
    getApp()->endWaitCursor();

    // If unmount, return to start location
    if (FXSELID(sel) == ID_UMOUNT)
    {
        dirpanel->setDirectory(startlocation, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(startlocation);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();        
    }

    // Force panel refresh
    onCmdRefresh(0, 0, 0);

    return 1;
}


// Update the Mount button
long FilePanel::onUpdMount(FXObject* sender, FXSelector sel, void*)
{
    FXString dir;

    int num = current->list->getNumSelectedItems();

    // Use the selected directory in FilePanel if any
    // or use the selected directory in DirPanel
    if (num == 0)
    {
        dir = current->list->getDirectory();
    }
    else
    {
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                dir = current->list->getItemPathname(u);
            }
        }
    }

    if (fsdevices->find(dir.text()) && !mtdevices->find(dir.text()) && current->list->getNumItems() &&
        !current->list->isItemSelected(0))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update the Unmount button
long FilePanel::onUpdUnmount(FXObject* sender, FXSelector sel, void*)
{
    FXString dir;

    int num = current->list->getNumSelectedItems();

    // Use the selected directory in FilePanel if any
    // or use the selected directory in DirPanel
    if (num == 0)
    {
        dir = current->list->getDirectory();
    }
    else
    {
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                dir = current->list->getItemPathname(u);
            }
        }
    }

    if ((fsdevices->find(dir.text()) || mtdevices->find(dir.text())) && current->list->getNumItems() &&
        !current->list->isItemSelected(0))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Query packages data base
long FilePanel::onCmdPkgQuery(FXObject* sender, FXSelector sel, void*)
{
    FXString cmd;

    // Name of the current selected file
    FXString file = current->list->getCurrentFile();

    // Read package format
    FXuint pkg_format = getApp()->reg().readUnsignedEntry("SETTINGS", "package_format", DEB_PKG);

    // Command to perform
    if (pkg_format == DEB_PKG)
    {
        cmd = "dpkg -S " + xf_quote(file);
    }
    else if (pkg_format == RPM_PKG)
    {
        cmd = "rpm -qf " + xf_quote(file);
    }
    else
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("No compatible package manager (rpm or dpkg) found!"));
        return 0;
    }

    // Query command
    cmd += " 2>&1";

    // Wait cursor
    getApp()->beginWaitCursor();

    // Perform the command
    FILE* pcmd = popen(cmd.text(), "r");
    if (!pcmd)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Failed command: %s"), cmd.text());
        return 0;
    }

    // Get command output
    char text[10000] = { 0 };
    FXString buf;
    while (fgets(text, sizeof(text), pcmd))
    {
        buf += text;
    }
    snprintf(text, sizeof(text), "%s", buf.text());

    // Close the stream and display error message if any
    if ((pclose(pcmd) == -1) && (errno != ECHILD)) // ECHILD can be set if the child was caught by sigHarvest
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), "%s", text);
        return 0;
    }
    getApp()->endWaitCursor();

    // Get package name, or detect when the file isn't in a package
    FXString str = text;
    if (pkg_format == DEB_PKG)                                  // DEB based distribution
    {
        int idx = str.find(" ");                                // Split output at first whitespace
        FXString pkgname = str.left(idx - 1);                   // Remove trailing colon
        FXString fname = str.right(str.length() - idx);
        fname.trim();                                           // Remove leading space and trailing newline
        if (xf_strequal(fname.text(), file.text()))             // No other word than the file name
        {
            str = pkgname.text();
        }
        else
        {
            str = "";
        }
    }
    if (pkg_format == RPM_PKG)                  // RPM based distribution
    {
        if (str.find(' ') != -1)                // Space character exists in the string
        {
            str = "";
        }
    }

    // Display the related output message
    FXString message;
    if (str == "")
    {
        message.format(_("File %s does not belong to any package."), file.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }
    else
    {
        message.format(_("File %s belongs to the package: %s"), file.text(), str.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }

    return 1;
}


// Update the package query menu
long FilePanel::onUpdPkgQuery(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is disabled when nothing is selected or multiple selection
    // or when unique selection and the selected item is a directory

    int num;

    num = current->list->getNumSelectedItems();

    if ((num == 0) || (num > 1))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else // num=1
    {
        int item = current->list->getCurrentItem();
        if ((item >= 0) && current->list->isItemDirectory(item))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }

    return 1;
}


#endif // End #if defined(linux)


// Directory usage on file selection
long FilePanel::onCmdDirUsage(FXObject* sender, FXSelector, void*)
{
    FXString name, command, itemslist = " ";
    FXString cmd1 = "/usr/bin/du --apparent-size -k -s ";
    FXString cmd2 =
        " 2> /dev/null | /usr/bin/sort -rn | /usr/bin/cut -f2 | /usr/bin/xargs -d '\n' /usr/bin/du --apparent-size --total --si -s 2> /dev/null";

    // Enter current directory
    int ret = chdir(current->getDirectory().text());

    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), current->getDirectory().text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), current->getDirectory().text());
        }

        return 0;
    }

    // Possibly deselect the '..' directory
    if (current->list->isItemSelected(0))
    {
        current->list->deselectItem(0);
    }

    // Return if nothing is selected
    if (current->list->getNumSelectedItems() == 0)
    {
        return 0;
    }

    // Construct selected files list
    current->list->setFocus();
    for (int u = 0; u < current->list->getNumItems(); u++)
    {
        if (current->list->isItemSelected(u))
        {
            name = current->list->getItemFilename(u);

            // List of selected items
            itemslist += xf_quote(name) + " ";
        }
    }

    // Command to be executed
    command = cmd1 + itemslist + cmd2;

    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Sizes of Selected Items"), command, 25, 50);
    cmdwin->create();
    cmdwin->setIcon(minicharticon);

    // The CommandWindow object will delete itself when closed!

    // Return to start location
    ret = chdir(startlocation.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), startlocation.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
        }
    }

    return 1;
}


// Update the status bar and the path linker
long FilePanel::onUpdStatus(FXObject* sender, FXSelector, void*)
{
    // Update the status bar
    int item = -1;
    FXString status, linkto;
    char usize[64];
    FXulong size = 0;
    FXString hsize = _("0 bytes");

    int num = list->getNumSelectedItems();

    // To handle the update rename (ugly, I know)
    if (current == this)
    {
        if (num <= 1)
        {
            selmult = false;
        }
        else if (num > 1)
        {
            selmult = true;
        }
    }

    item = list->getCurrentItem();

    // Several items selected
    if (num > 1)
    {
        int nbdirs = 0;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u) && !list->isItemDirectory(u))
            {
                size += list->getItemFileSize(u);
#if __WORDSIZE == 64
                snprintf(usize, sizeof(usize), "%lu", size);
#else
                snprintf(usize, sizeof(usize), "%llu", size);
#endif
                hsize = xf_humansize(usize);
            }

            if (list->isItemSelected(u) && list->isItemDirectory(u) && (list->getItemText(u) != ".."))
            {
                nbdirs++;
            }
        }

        int nbfiles = num - nbdirs;
        if (nbdirs <= 1 && nbfiles <= 1)
        {
            status.format(_("%s in %s selected items (%s folder, %s file)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else if (nbdirs <= 1 && nbfiles > 1)
        {
            status.format(_("%s in %s selected items (%s folder, %s files)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else if (nbdirs > 1 && nbfiles <= 1)
        {
            status.format(_("%s in %s selected items (%s folders, %s file)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else
        {
            status.format(_("%s in %s selected items (%s folders, %s files)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
    }
    else
    {
        // Nothing selected
        if ((num == 0) || (item < 0))
        {
            num = list->getNumItems();
            if (num == 1)
            {
                status = _("1 item (1 folder)");
            }
            else
            {
                int nbdirs = 0;
                for (int u = 0; u < num; u++)
                {
                    if (list->isItemDirectory(u))
                    {
                        nbdirs++;
                    }
                    else
                    {
                        size += list->getItemFileSize(u);
#if __WORDSIZE == 64
                        snprintf(usize, sizeof(usize), "%lu", size);
#else
                        snprintf(usize, sizeof(usize), "%llu", size);
#endif
                        hsize = xf_humansize(usize);
                    }
                }

                int nbfiles = num - nbdirs;
                status.format(_("%s items (%s folders, %s files, %s)"), FXStringVal(num).text(),
                              FXStringVal(nbdirs).text(),
                              FXStringVal(nbfiles).text(), hsize.text());
                if (nbdirs <= 1 && nbfiles <= 1)
                {
                    status.format(_("%s items (%s folder, %s file, %s)"), FXStringVal(num).text(),
                                  FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text(), hsize.text());
                }
                else if (nbdirs <= 1 && nbfiles > 1)
                {
                    status.format(_("%s items (%s folder, %s files, %s)"), FXStringVal(num).text(),
                                  FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text(), hsize.text());
                }
                else if (nbdirs > 1 && nbfiles <= 1)
                {
                    status.format(_("%s items (%s folders, %s file, %s)"), FXStringVal(num).text(),
                                  FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text(), hsize.text());
                }
                else
                {
                    status.format(_("%s items (%s folders, %s files, %s)"), FXStringVal(num).text(),
                                  FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text(), hsize.text());
                }
            }
        }

        // One item selected
        else
        {
            // Item text
            FXString str = list->getItemText(item);

            // Are we in trash?
            FXString dirpath = list->getDirectory();
            FXbool inTrash = false;
            if (dirpath.left(trashfileslocation.length()) == trashfileslocation)
            {
                inTrash = true;
            }

            // Actual number of columns
            FXuint nbHeaders;
            FXuint beg;

            if (inTrash)
            {
                nbHeaders = nbCols + 2;
                beg = 3; // Skip name, original path and deletion date
            }
            else
            {
                nbHeaders = nbCols;
                beg = 1; // Skip name
            }

            // Status text
            for (FXuint i = beg; i < nbHeaders - 1; i++)
            {
                FXString txt = str.section('\t', i);
                if (txt.length() != 0)
                {
                    status += txt + " | ";
                }
            }
            status += str.section('\t', nbHeaders - 1);
        }
    }

    // Set status label
    statuslabel->setText(status);

    // Prepend the filter pattern if any
    if ((list->getPattern() != "*") && (list->getPattern() != "*.*"))
    {
        closefilter->show();

        if (list->getFilterFolders())
        {
            status.format(_("Filter: %s (files and folders)"), list->getPattern().text());
        }
        else
        {
            status.format(_("Filter: %s (files)"), list->getPattern().text());
        }
        filterlabel->setText(status);
        filterlabel->setTextColor(attenclr);
    }
    else
    {
        closefilter->hide();
        filterlabel->setText("");
    }

    return 1;
}


// Update the path text and the path link
void FilePanel::updatePath()
{
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());
}


// Update the go to parent directory command
long FilePanel::onUpdUp(FXObject* sender, FXSelector, void*)
{
    FXButton* button = (FXButton*)sender;
    int style = button->getButtonStyle();

    if (style & TOGGLEBUTTON_TOOLBAR)
    {
        if (current->list->getDirectory() != ROOTDIR)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    return 1;
}


// Update the paste button
long FilePanel::onUpdPaste(FXObject* sender, FXSelector, void*)
{
    FXuchar* data;
    FXuint len;
    FXString buf;
    FXbool clipboard_empty = true;

    // Lock clipboard to prevent changes in method onCmdRequestClipboard()
    clipboard_locked = true;

    // If source is xfelistType (Gnome, XFCE, or Xfe app)
    if (getDNDData(FROM_CLIPBOARD, xfelistType, data, len))
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        buf = (char*)data;

        // Check if valid clipboard
        if (buf.find("file:/") >= 0)
        {
            clipboard_empty = false;
        }

        // Free data pointer
        FXFREE(&data);
    }
    // If source type is urilistType (KDE apps ; non Gnome, non XFCE and non Xfe apps)
    else if (getDNDData(FROM_CLIPBOARD, urilistType, data, len))
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        buf = (char*)data;

        // Check if valid clipboard
        if (buf.find("file:/") >= 0)
        {
            clipboard_empty = false;
        }

        // Free data pointer
        FXFREE(&data);
    }
    // If source is utf8Type (simple text)
    else if (getDNDData(FROM_CLIPBOARD, utf8Type, data, len))
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        buf = (char*)data;

        // Check if valid clipboard
        int beg, end;
        FXString filepath;
        FXbool clipboard_valid = true;
        for (beg = 0; beg < buf.length(); beg = end + 1)
        {
            if ((end = buf.find("\n", beg)) < 0)
            {
                end = buf.length();
            }

            // Obtain item file path
            filepath = buf.mid(beg, end - beg);

            // File path does not begin with '/'
            if (filepath[0] != PATHSEPCHAR)
            {
                clipboard_valid = false;
                break;
            }
            // File path is not an existing file or directory
            else
            {
                if (!xf_existfile(filepath))
                {
                    clipboard_valid = false;
                    break;
                }
            }
        }

        // Clipboard not empty
        if (clipboard_valid)
        {
            clipboard_empty = false;
        }

        // Free data pointer
        FXFREE(&data);
    }

    // Gray out the paste button, if necessary
    if (clipboard_empty || clipboard_type == COPYNAME_CLIPBOARD)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    // Unlock clipboard
    clipboard_locked = false;

    return 1;
}


// Update menu items and toolbar buttons that are related to file operations
long FilePanel::onUpdMenu(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is disabled when nothing or only ".." is selected
    int num;

    num = current->list->getNumSelectedItems();
    DirItem* item = (DirItem*)dirpanel->getCurrentItem();

    if ((dirpanel->shown() && item))
    {
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
        else if ((num == 1) && current->list->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else if ((num == 1) && current->list->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }

    return 1;
}


// Update file delete menu item and toolbar button
long FilePanel::onUpdFileDelete(FXObject* sender, FXSelector sel, void*)
{
    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    FXbool use_trash_bypass = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_bypass", false);

    if ((!use_trash_can) | use_trash_bypass)
    {
        int num = current->list->getNumSelectedItems();
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else if ((num == 1) && current->list->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update move to trash menu item and toolbar button
long FilePanel::onUpdFileTrash(FXObject* sender, FXSelector sel, void*)
{
    // Disable move to trash menu if we are in trash can
    // or if the trash can directory is selected

    FXbool trashenable = true;
    FXString trashparentdir = trashlocation.rbefore('/');
    FXString curdir = current->list->getDirectory();

    if (curdir.left(trashlocation.length()) == trashlocation)
    {
        trashenable = false;
    }

    if (curdir == trashparentdir)
    {
        FXString pathname;
        for (int u = 0; u < current->list->getNumItems(); u++)
        {
            if (current->list->isItemSelected(u))
            {
                pathname = current->list->getItemPathname(u);
                if (pathname == trashlocation)
                {
                    trashenable = false;
                }
            }
        }
    }

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    if (use_trash_can && trashenable)
    {
        int num = current->list->getNumSelectedItems();
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else if ((num == 1) && current->list->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update restore from trash menu item and toolbar button
long FilePanel::onUpdFileRestore(FXObject* sender, FXSelector sel, void*)
{
    // Enable restore from trash menu if we are in trash can

    FXbool restoreenable = false;
    FXString curdir = current->list->getDirectory();

    if (curdir.left(trashfileslocation.length()) == trashfileslocation)
    {
        restoreenable = true;
    }

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    if (use_trash_can && restoreenable)
    {
        int num = current->list->getNumSelectedItems();
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else if ((num == 1) && current->list->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update go trash menu item and toolbar button
long FilePanel::onUpdGoTrash(FXObject* sender, FXSelector sel, void*)
{
    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);

    if (use_trash_can)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update file open menu
long FilePanel::onUpdOpen(FXObject* sender, FXSelector, void*)
{
    // Menu item is disabled when nothing or a directory (including "..") is selected
    int num, item;

    num = current->list->getNumSelectedItems(&item);

    if (num == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        if (current->list->getItem(item) && current->list->isItemFile(item))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
    }
    return 1;
}


// Update the status of the menu items that should be disabled when selecting multiple files
long FilePanel::onUpdSelMult(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is disabled when nothing is selected or multiple selection or ".." is only selected
    int num;

    num = current->list->getNumSelectedItems();
    DirItem* item = (DirItem*)dirpanel->getCurrentItem();

    if (num == 0)
    {
        if (!item || !dirpanel->shown())
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else if (current->selmult || ((num == 1) && current->list->isItemSelected(0)))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    return 1;
}


// Update the file compare menu item
long FilePanel::onUpdCompare(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is enabled only when one file or folder is selected
    // or when two files or two folders are selected
    int num;

    num = current->list->getNumSelectedItems();

    if (num == 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else if (num == 2)
    {
        if (sameFileType)  // Selected items are both files or directories
        {
             sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);                       
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update Add to archive menu
long FilePanel::onUpdAddToArch(FXObject* sender, FXSelector, void*)
{
    // Menu item is disabled when nothing or ".." is selected
    int num, item;

    num = current->list->getNumSelectedItems(&item);
    if (num == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else if ((num == 1) && current->list->isItemSelected(0))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    return 1;
}


// Update scripts menu item
long FilePanel::onUpdRunScript(FXObject* sender, FXSelector, void*)
{
    // Menu item is disabled when nothing or ".." is selected
    int num, item;

    num = current->list->getNumSelectedItems(&item);
    if (num == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    return 1;
}


// Update directory usage menu item
long FilePanel::onUpdDirUsage(FXObject* sender, FXSelector, void*)
{
    // Menu item is enabled only when at least two items are selected
    int num, item;

    num = current->list->getNumSelectedItems(&item);
    if (num > 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}
