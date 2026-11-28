// Helper functions for various purposes
// Some helper functions are also added to get large files support
// Also supports a timeout on the stat and lstat function (this is the reason
// why some standard FOX function cannot be used and are rewritten here)


#include "config.h"
#include "i18n.h"

#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <libgen.h>
#include <sys/statvfs.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "../st/x.h"


// FOX menu cascades do not own their popup panes. Clear nested panes explicitly
// before deleting their cascade entries, including dynamically built script menus.
void xf_clear_menu(FXComposite* menu)
{
    while (menu->getFirst())
    {
        FXWindow* item = menu->getFirst();
        FXMenuCascade* cascade = dynamic_cast<FXMenuCascade*>(item);
        if (cascade && cascade->getMenu())
        {
            FXPopup* submenu = cascade->getMenu();
            cascade->setMenu(NULL);
            xf_clear_menu(submenu);
            delete submenu;
        }
        delete item;
    }
}


#if defined(linux) && defined(XFE_AUTOMOUNTER)
FXbool xf_setautomountautostart(const FXString& configdir, FXbool enabled)
{
    FXString directory = configdir + PATHSEPSTRING + "autostart";
    FXString filename = directory + PATHSEPSTRING + "xfe-automount.desktop";

    if (!enabled)
    {
        return unlink(filename.text()) == 0 || errno == ENOENT;
    }

    if (xf_mkpath(directory.text(), 0700) != 0)
    {
        return false;
    }

    // Replace the entry atomically, so a failed write leaves the old file intact.
    FXString temporary = filename + ".XXXXXX";
    int fd = mkstemp(&temporary[0]);
    if (fd < 0)
    {
        return false;
    }
    FILE* file = fdopen(fd, "w");
    if (file == NULL)
    {
        close(fd);
        unlink(temporary.text());
        return false;
    }

    const char entry[] = "[Desktop Entry]\n"
                         "Type=Application\n"
                         "Name=Xfe Automounter\n"
                         "Comment=Automatically mount removable devices\n"
                         "Exec=xfe-automount\n"
                         "TryExec=xfe-automount\n"
                         "Terminal=false\n";
    FXbool success = fwrite(entry, 1, sizeof(entry) - 1, file) == sizeof(entry) - 1;
    if (fclose(file) != 0)
    {
        success = false;
    }
    if (success && rename(temporary.text(), filename.text()) == 0)
    {
        return true;
    }
    unlink(temporary.text());
    return false;
}
#endif


// Get available space on the file system where the file is located
FXlong xf_getavailablespace(const FXString& filepath)
{
    struct statvfs stat;

    if (statvfs(FXPath::directory(filepath).text(), &stat) != 0)
    {
        // An error has occurred
        return -1;
    }

    // The available size is f_bsize * f_bavail
    return stat.f_bsize * stat.f_bavail;
}


// Decode filename to get original again
FXString FXPath::dequote(const FXString& file)
{
    FXString result(file);

    if (0 < result.length())
    {
        int e = result.length();
        int b = 0;
        int r = 0;
        int q = 0;

        // Trim tail
        while (0 < e && Ascii::isSpace(result[e - 1]))
        {
            --e;
        }

        // Trim head
        while (b < e && Ascii::isSpace(result[b]))
        {
            ++b;
        }

        // Dequote the rest
        while (b < e)
        {
            if (result[b] == '\'')
            {
                q = !q;
                b++;
                continue;
            }
            if ((result[b] == '\\') && (result[b + 1] == '\'') && !q)
            {
                b++;
            }
            result[r++] = result[b++];
        }

        // Trunc to size
        result.trunc(r);
    }
    return result;
}


// Note : the original function from FXAccelTable is buggy!!
// Parse accelerator from string
FXHotKey xf_parseaccel(const FXString& string)
{
    FXuint code = 0, mods = 0;
    int pos = 0;

    // Parse leading space
    while (pos < string.length() && Ascii::isSpace(string[pos]))
    {
        pos++;
    }

    // Parse modifiers
    while (pos < string.length())
    {
        // Modifier
        if (comparecase(&string[pos], "ctl", 3) == 0)
        {
            mods |= CONTROLMASK;
            pos += 3;
        }
        else if (comparecase(&string[pos], "ctrl", 4) == 0)
        {
            mods |= CONTROLMASK;
            pos += 4;
        }
        else if (comparecase(&string[pos], "alt", 3) == 0)
        {
            mods |= ALTMASK;
            pos += 3;
        }
        else if (comparecase(&string[pos], "meta", 4) == 0)
        {
            mods |= METAMASK;
            pos += 4;
        }
        else if (comparecase(&string[pos], "shift", 5) == 0)
        {
            mods |= SHIFTMASK;
            pos += 5;
        }
        else
        {
            break;
        }

        // Separator
        if ((string[pos] == '+') || (string[pos] == '-') || Ascii::isSpace(string[pos]))
        {
            pos++;
        }
    }

    // Test for some special keys
    if (comparecase(&string[pos], "home", 4) == 0)
    {
        code = KEY_Home;
    }
    else if (comparecase(&string[pos], "end", 3) == 0)
    {
        code = KEY_End;
    }
    else if (comparecase(&string[pos], "pgup", 4) == 0)
    {
        code = KEY_Page_Up;
    }
    else if (comparecase(&string[pos], "pgdn", 4) == 0)
    {
        code = KEY_Page_Down;
    }
    else if (comparecase(&string[pos], "left", 4) == 0)
    {
        code = KEY_Left;
    }
    else if (comparecase(&string[pos], "right", 5) == 0)
    {
        code = KEY_Right;
    }
    else if (comparecase(&string[pos], "up", 2) == 0)
    {
        code = KEY_Up;
    }
    else if (comparecase(&string[pos], "down", 4) == 0)
    {
        code = KEY_Down;
    }
    else if (comparecase(&string[pos], "ins", 3) == 0)
    {
        code = KEY_Insert;
    }
    else if (comparecase(&string[pos], "del", 3) == 0)
    {
        code = KEY_Delete;
    }
    else if (comparecase(&string[pos], "esc", 3) == 0)
    {
        code = KEY_Escape;
    }
    else if (comparecase(&string[pos], "tab", 3) == 0)
    {
        code = KEY_Tab;
    }
    else if (comparecase(&string[pos], "return", 6) == 0)
    {
        code = KEY_Return;
    }
    else if (comparecase(&string[pos], "enter", 5) == 0)
    {
        code = KEY_Return;
    }
    else if (comparecase(&string[pos], "back", 4) == 0)
    {
        code = KEY_BackSpace;
    }
    else if (comparecase(&string[pos], "spc", 3) == 0)
    {
        code = KEY_space;
    }
    else if (comparecase(&string[pos], "space", 5) == 0)
    {
        code = KEY_space;
    }
    // Test for function keys
    else if ((Ascii::toLower(string[pos]) == 'f') && Ascii::isDigit(string[pos + 1]))
    {
        if (Ascii::isDigit(string[pos + 2]))
        {
            // !!!! Hack to fix a bug in FOX !!!!
            code = KEY_F1 + 10 * (string[pos + 1] - '0') + (string[pos + 2] - '0') - 1;
            // !!!! End of hack !!!!
        }
        else
        {
            code = KEY_F1 + string[pos + 1] - '1';
        }
    }
    // Test if hexadecimal code designator
    else if (string[pos] == '#')
    {
        code = strtoul(&string[pos + 1], NULL, 16);
    }
    // Test if its a single character accelerator
    else if (Ascii::isPrint(string[pos]))
    {
        if (mods & SHIFTMASK)
        {
            code = Ascii::toUpper(string[pos]) + KEY_space - ' ';
        }
        else
        {
            code = Ascii::toLower(string[pos]) + KEY_space - ' ';
        }
    }
    return MKUINT(code, mods);
}


// Find if the specified command exists
FXbool xf_existcommand(const FXString cmd)
{
    struct stat linfo;

    // Command file path
    FXString cmdpath = cmd.before(' ');

    // If first character is '/' then cmdpath is an absolute path
    if (cmdpath[0] == PATHSEPCHAR)
    {
        // Check if command file name exists and is not a directory
        if (!cmdpath.empty() && (xf_lstat(cmdpath.text(), &linfo) == 0) && !S_ISDIR(linfo.st_mode))
        {
            return true;
        }
    }
    // If first character is '~' then cmdpath is a path relative to home directory
    else if (cmdpath[0] == '~')
    {
        // Form command absolute path
        cmdpath = FXSystem::getHomeDirectory() + cmdpath.after('~');

        // Check if command file name exists and is not a directory
        if (!cmdpath.empty() && (xf_lstat(cmdpath.text(), &linfo) == 0) && !S_ISDIR(linfo.st_mode))
        {
            return true;
        }
    }
    // Simple command name or path relative to the exec path
    else
    {
        // Get exec path
        FXString execpath = FXSystem::getExecPath();

        if (execpath != "")
        {
            // Number of delimiters
            int nbseps = execpath.contains(':');

            // Loop over path components
            for (int i = 0; i <= nbseps; i++)
            {
                // Obtain path component
                FXString path = execpath.section(':', i);

                // Form command absolute path
                path += PATHSEPSTRING + cmdpath;

                // Check if command file name exists and is not a directory
                if (!path.empty() && (xf_lstat(path.text(), &linfo) == 0) && !S_ISDIR(linfo.st_mode))
                {
                    return true;
                }
            }
        }
    }

    return false;
}


// Get key binding string from user input
// Code adapted from FXAccelTable::unparseAccel() and modified to get strings like 'Ctrl-A' instead of 'ctrl+a'
FXString xf_getkeybinding(FXEvent* event)
{
    // Get modifiers and key
    int mods = event->state;
    int code = event->code;

    char buffer[64];
    FXString s;

    // Handle modifier keys
    if (mods & CONTROLMASK)
    {
        s += "Ctrl-";
    }
    if (mods & ALTMASK)
    {
        s += "Alt-";
    }
    if (mods & SHIFTMASK)
    {
        s += "Shift-";
    }
    if (mods & METAMASK)
    {
        s += "Meta-";
    }

    // Handle some special keys
    switch (code)
    {
    case KEY_Home:
        s += "Home";
        break;

    case KEY_End:
        s += "End";
        break;

    case KEY_Page_Up:
        s += "PgUp";
        break;

    case KEY_Page_Down:
        s += "PgDn";
        break;

    case KEY_Left:
        s += "Left";
        break;

    case KEY_Right:
        s += "Right";
        break;

    case KEY_Up:
        s += "Up";
        break;

    case KEY_Down:
        s += "Down";
        break;

    case KEY_Insert:
        s += "Ins";
        break;

    case KEY_Delete:
        s += "Del";
        break;

    case KEY_Escape:
        s += "Esc";
        break;

    case KEY_Tab:
        s += "Tab";
        break;

    case KEY_Return:
        s += "Return";
        break;

    case KEY_BackSpace:
        s += "Back";
        break;

    case KEY_space:
        s += "Space";
        break;

    case KEY_F1:
    case KEY_F2:
    case KEY_F3:
    case KEY_F4:
    case KEY_F5:
    case KEY_F6:
    case KEY_F7:
    case KEY_F8:
    case KEY_F9:
    case KEY_F10:
    case KEY_F11:
    case KEY_F12:
    case KEY_F13:
    case KEY_F14:
    case KEY_F15:
    case KEY_F16:
    case KEY_F17:
    case KEY_F18:
    case KEY_F19:
    case KEY_F20:
    case KEY_F21:
    case KEY_F22:
    case KEY_F23:
    case KEY_F24:
    case KEY_F25:
    case KEY_F26:
    case KEY_F27:
    case KEY_F28:
    case KEY_F29:
    case KEY_F30:
    case KEY_F31:
    case KEY_F32:
    case KEY_F33:
    case KEY_F34:
    case KEY_F35:
        snprintf(buffer, sizeof(buffer), "F%d", code - KEY_F1 + 1);
        s += buffer;
        break;

    default:
        if (Ascii::isPrint(code))
        {
            s += Ascii::toUpper(code);
        }
        else
        {
            s = ""; // Invalid case
        }
        break;
    }

    return s;
}


// Create a directory with its path, like 'mkdir -p'
// Return 0 if success or -1 if fail
// Original author : Niall O'Higgins */
// http://niallohiggins.com/2009/01/08/mkpath-mkdir-p-alike-in-c-for-unix
int xf_mkpath(const char* s, mode_t mode)
{
    char* q, * r = NULL, * path = NULL, * up = NULL;
    int rv = -1;

    if ((strcmp(s, ".") == 0) || (strcmp(s, "/") == 0))
    {
        return 0;
    }

    if ((path = strdup(s)) == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if ((q = strdup(s)) == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if ((r = (char*)dirname(q)) == NULL)
    {
        goto out;
    }

    if ((up = strdup(r)) == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if ((xf_mkpath(up, mode) == -1) && (errno != EEXIST))
    {
        goto out;
    }

    if ((mkdir(path, mode) == 0) || (errno == EEXIST))
    {
        rv = 0;
    }

out:
    if (up != NULL)
    {
        free(up);
    }
    free(q);
    free(path);
    return rv;
}


// Obtain a unique trash files path name based on the file path name
FXString xf_create_trashpathname(FXString pathname, FXString trashfileslocation)
{
    // Initial trash files path name
    FXString trashpathname = trashfileslocation + PATHSEPSTRING + FXPath::name(pathname);

    // Possibly modify the trash files path name by adding a suffix like '_1', '_2', etc.,
    // if the file already exists in the trash can files directory
    for (int i = 1; ; i++)
    {
        if (xf_existfile(trashpathname))
        {
            char suffix[32];
            snprintf(suffix, sizeof(suffix), "_%d", i);
            FXString prefix = trashpathname.rbefore('_');
            if (prefix == "")
            {
                prefix = trashpathname;
            }
            trashpathname = prefix + suffix;
        }
        else
        {
            break;
        }
    }

    return trashpathname;
}


// Create trashinfo file based on the pathname and the trashpathname
int xf_create_trashinfo(FXString pathname, FXString trashpathname,
                       FXString trashfileslocation, FXString trashinfolocation)
{
    // Create trash can files if it doesn't exist
    if (!xf_existfile(trashfileslocation))
    {
        int mask = umask(0);
        umask(mask);
        int ret = xf_mkpath(trashfileslocation.text(), 511 & ~mask);
        return ret;
    }

    // Create trash can info if it doesn't exist
    if (!xf_existfile(trashinfolocation))
    {
        int mask = umask(0);
        umask(mask);
        int ret = xf_mkpath(trashinfolocation.text(), 511 & ~mask);
        return ret;
    }

    // Deletion date
    struct timeval tv;
    gettimeofday(&tv, NULL);
    FXString deldate = FXSystem::time("%FT%T", tv.tv_sec);

    // Trash info path name
    FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(trashpathname) + ".trashinfo";

    // Create trash info file
    FILE* fp;
    int ret;
    if ((fp = fopen(trashinfopathname.text(), "w")) != NULL)
    {
        fprintf(fp, "[Trash Info]\n");
        fprintf(fp, "Path=%s\n", pathname.text());
        fprintf(fp, "DeletionDate=%s\n", deldate.text());
        fclose(fp);
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}


// Return mime type of a file
// Makes use of the Unix file command, thus this function may be slow
FXString xf_mimetype(FXString pathname)
{
    FXString cmd = "/usr/bin/file -b -i " + pathname;
    FILE* filecmd = popen(cmd.text(), "r");

    if (!filecmd)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    char text[128] = { 0 };
    FXString buf;
    while (fgets(text, sizeof(text), filecmd))
    {
        buf += text;
    }
    pclose(filecmd);

    return buf.rbefore('\n');
}


// Return 1 if the file given by its pathname is a text file, else return 0
// If the file is a valid link, then the source of the link is tested
// If the file is a broken link, return -1
int xf_istextfile(FXString pathname)
{
    FXString filepath = pathname;
    FXString str;

    // Is file a link ?
    if (xf_islink(pathname))
    {
        // Loop to find the file towards the link (or chain of links) points
        while (1)
        {
            FXString linkpath = FXPath::directory(filepath) + PATHSEPSTRING + FXFile::symlink(filepath.text());

            if (xf_existfile(linkpath))
            {
                if (xf_islink(linkpath))
                {
                    filepath = linkpath;
                }
                else
                {
                    // File pointed by the link
                    str = xf_mimetype(linkpath);
                    break;
                }
            }
            else
            {
                // Broken link
                return -1;
            }
        }
    }
    // Regular file
    else
    {
        str = xf_mimetype(filepath);
    }

    // Text file?
    if (strstr(str.text(), "charset=binary"))
    {
        return 0;
    }

    return 1;
}


// Quote a filename against shell substitutions
// Thanks to Glynn Clements <glynnc@users.sourceforge.net>
FXString xf_quote(FXString str)
{
    FXString result = "'";
    const char* p;

    for (p = str.text(); *p; p++)
    {
        if (*p == '\'')
        {
            result += "'\\''";
        }
        else
        {
            result += *p;
        }
    }

    result += '\'';
    return result;
}


// Test if a string is encoded in UTF-8
// "length" is the number of bytes of the string to consider
FXbool xf_isutf8(const char* string, FXuint length)
{
    const FXuchar* ptr = (const FXuchar*)string;
    const size_t highbits = (size_t)-1 / 0xFF * 0x80;

    while (length)
    {
        FXuchar ch = ptr[0];
        if (ch < 0x80)
        {
            // Skip whole ASCII words without alignment or aliasing assumptions.
            while (length >= sizeof(size_t))
            {
                size_t word;
                memcpy(&word, ptr, sizeof(word));
                if (word & highbits)
                {
                    break;
                }
                ptr += sizeof(word);
                length -= sizeof(word);
            }
            // Finish the word containing the first non-ASCII byte, or the tail.
            while (length && ptr[0] < 0x80)
            {
                ptr++;
                length--;
            }
            continue;
        }
        if (ch < 0xC2)
        {
            return false;
        }
        // Two-byte sequence: exclude continuation bytes and overlong encodings.
        if (ch < 0xE0)
        {
            if (length < 2 || (ptr[1] & 0xC0) != 0x80)
            {
                return false;
            }
            ptr += 2;
            length -= 2;
        }
        // Three-byte sequence: exclude overlong encodings and UTF-16 surrogates.
        else if (ch < 0xF0)
        {
            if (length < 3 || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80 ||
                (ch == 0xE0 && ptr[1] < 0xA0) || (ch == 0xED && ptr[1] >= 0xA0))
            {
                return false;
            }
            ptr += 3;
            length -= 3;
        }
        // Four-byte sequence: Unicode ends at U+10FFFF.
        else if (ch <= 0xF4)
        {
            if (length < 4 || (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80 ||
                (ptr[3] & 0xC0) != 0x80 || (ch == 0xF0 && ptr[1] < 0x90) ||
                (ch == 0xF4 && ptr[1] >= 0x90))
            {
                return false;
            }
            ptr += 4;
            length -= 4;
        }
        else
        {
            return false;
        }
    }
    return true;
}


// Safe strcpy function (Public domain, by C.B. Falconer)
// The destination string is always null terminated
// Size sz is the destination capacity, including the terminating null byte
size_t xf_strlcpy(char* dst, const char* src, size_t sz)
{
    const char* start = src;

    if (src && sz--)
    {
        while ((*dst++ = *src))
        {
            if (sz--)
            {
                src++;
            }
            else
            {
                *(--dst) = '\0';
                break;
            }
        }
    }
    if (src)
    {
        while (*src++)
        {
            continue;
        }
        return src - start - 1;
    }
    else if (sz)
    {
        *dst = '\0';
    }
    return 0;
}


// Safe strcat function (Public domain, by C.B. Falconer)
// The destination string is always null terminated
size_t xf_strlcat(char* dst, const char* src, size_t sz)
{
    char* start = dst;

    while (*dst++)  // assumes sz >= strlen(dst)
    {
        if (sz)
        {
            sz--; // i.e. well formed string
        }
    }
    dst--;
    return dst - start + xf_strlcpy(dst, src, sz);
}


// Obtain the non recursive size of a directory
FXulong xf_dirsize(const char* path)
{
    DIR* dp;
    struct dirent* dirp;
    struct stat statbuf;
    char buf[MAXPATHLEN];
    FXulong dsize = 0;
    int ret;

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

        if (xf_strequal(path, ROOTDIR))
        {
            snprintf(buf, sizeof(buf), "%s%s", path, dirp->d_name);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%s/%s", path, dirp->d_name);
        }

#if defined(linux)
        // Mount points are not processed to improve performances
        if (mtdevices->find(buf))
        {
            continue;
        }
#endif

        ret = xf_lstat(buf, &statbuf);
        if (ret == 0)
        {
            if (!S_ISDIR(statbuf.st_mode))
            {
                dsize += (FXulong)statbuf.st_size;
            }
        }
    }
    if (closedir(dp) < 0)
    {
        fprintf(stderr, _("Error: Can't close folder %s\n"), path);
    }
    return dsize;
}


// Obtain the recursive size of a directory
// The number of files and the number of sub directories is also stored in the nbfiles and nbsubdirs pointers
// Caution: this only works if nbfiles and nbsubdirs are initialized to 0 in the calling function
// After that, nbfiles contains the total number of files (including the count of sub directories),
// nbsubdirs contains the number of sub directories and totalsize the total directory size
// readsuccess is set to false if some file or folder was not read (due to insufficient permissions or other reason)
// The pipes are used to write partial results, for inter process communication
FXulong xf_pathsize(const char* path, FXuint* nbfiles, FXuint* nbsubdirs, FXulong* totalsize, FXbool* readsuccess, int pipes[2])
{
    struct stat statbuf;
    struct dirent* dirp;
    DIR* dp;
    FXulong dsize;
    int ret;

    ret = xf_lstat(path, &statbuf);
    if (ret < 0)
    {
        *readsuccess = false;
        return 0;
    }
    dsize = (FXulong)statbuf.st_size;
    (*totalsize) += dsize;
    (*nbfiles)++;

    // Write to pipe, if requested
    if (pipes != NULL)
    {
        char buf[256];

#if __WORDSIZE == 64
        snprintf(buf, sizeof(buf), "%lu %u %u %u/", *totalsize, *nbfiles, *nbsubdirs, *readsuccess);
#else
        snprintf(buf, sizeof(buf), "%llu %u %u %u/", *totalsize, *nbfiles, *nbsubdirs, *readsuccess);
#endif

        if (write(pipes[1], buf, strlen(buf)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        ;
    }

    // Not a directory
    if (!S_ISDIR(statbuf.st_mode))
    {
        return dsize;
    }

    // Directory
    (*nbsubdirs)++;

    if ((dp = opendir(path)) == NULL)
    {
        *readsuccess = false;
        return 0;
    }

    while ((dirp = readdir(dp)))
    {
        if (xf_strequal(dirp->d_name, ".") || xf_strequal(dirp->d_name, ".."))
        {
            continue;
        }
        FXString child = path;
        if (child.empty() || child[child.length() - 1] != '/')
        {
            child += '/';
        }
        child += dirp->d_name;

        // Recursive call
        dsize += xf_pathsize(child.text(), nbfiles, nbsubdirs, totalsize, readsuccess, pipes);
    }

    if (closedir(dp) < 0)
    {
        fprintf(stderr, _("Error: Can't close folder %s\n"), path);
    }

    return dsize;
}


// Write the file size in human readable form (bytes, kBytes, MBytes, GBytes)
// We use a decimal basis for kB, MB, GB count
FXString xf_humansize(char* size)
{
    int flag = 0;
    char suf[64];
    char buf[128];
    FXString hsize;

    FXulong lsize = strtoull(size, NULL, 10);
    float fsize = 0.0;

    if (lsize > 1e9)
    {
        fsize = lsize / 1e9;
        xf_strlcpy(suf, _("GB"), sizeof(suf));
        flag = 1;
    }
    else if (lsize > 1e6)
    {
        fsize = lsize / 1e6;
        xf_strlcpy(suf, _("MB"), sizeof(suf));
        flag = 1;
    }
    else if (lsize > 1e3)
    {
        fsize = lsize / 1e3;
        xf_strlcpy(suf, _("kB"), sizeof(suf));
        flag = 1;
    }
    else
    {
        xf_strlcpy(suf, _("bytes"), sizeof(suf));
    }

    if (flag)
    {
        if (fsize == (int)fsize)
        {
            snprintf(buf, sizeof(buf), "%.0f %s", fsize, suf);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%.1f %s", fsize, suf);
        }
    }
    else
    {
#if __WORDSIZE == 64
        snprintf(buf, sizeof(buf), "%lu %s", lsize, suf);
#else
        snprintf(buf, sizeof(buf), "%llu %s", lsize, suf);
#endif
    }

    hsize = buf;
    return hsize;
}


// Remove terminating '/' on a path string to simplify a file or directory path
// Thus '/bla/bla////' becomes '/bla/bla'
// Special case : '/' stays to '/'
FXString xf_cleanpath(const FXString path)
{
    FXString in = path, out = path;

    while (1)
    {
        if ((in.length() > 1) && (in[in.length() - 1] == '/'))
        {
            out = in.trunc(in.length() - 1);
            in = out;
        }
        else
        {
            break;
        }
    }
    return out;
}


// Return the absolute path, based on the current directory path
// Remove terminating '/' on a path string to simplify a file or directory path
// Thus '/bla/bla////' becomes '/bla/bla'
// Special case : '/' stays to '/'
FXString xf_filepath(const FXString path)
{
    FXString in = path, out = path;

    while (1)
    {
        if ((in.length() > 1) && (in[in.length() - 1] == '/'))
        {
            out = in.trunc(in.length() - 1);
            in = out;
        }
        else
        {
            break;
        }
    }
    FXString dir = FXSystem::getCurrentDirectory();

    // If absolute path
    if (ISPATHSEP(out[0]))
    {
        return out;
    }
    else
    {
        return dir + PATHSEPSTRING + out;
    }
}


// Return the absolute path, based on the specified directory path
// Remove terminating '/' on a path string to simplify a file or directory path
// Thus '/bla/bla////' becomes '/bla/bla'
// Special case : '/' stays to '/'
FXString xf_filepath(const FXString path, const FXString dir)
{
    FXString in = path, out = path;

    while (1)
    {
        if ((in.length() > 1) && (in[in.length() - 1] == '/'))
        {
            out = in.trunc(in.length() - 1);
            in = out;
        }
        else
        {
            break;
        }
    }
    // If absolute path
    if (ISPATHSEP(out[0]))
    {
        return out;
    }
    else
    {
        return dir + PATHSEPSTRING + out;
    }
}


// Obtain real pathname (remove ../ and ./ from absolute path)
FXString xf_realpath(const FXString path)
{
    char buf[MAXPATHLEN];
    FXString ret;
    
    // Get real pathname
    char* res = realpath(path.text(), buf);
    if (res != NULL)
    {
        ret = FXString(res);
    }
   
    // An error has occurred, return original pathname
    else
    {
        ret = path;
    }
    
    return ret;
}


// Return the absolute path of the application executable
// Exit application if error
// Source: https://www.linuxquestions.org/questions/programming-9/c-function-to-get-application-path-335442/
FXString xf_execpath(char* path)
{
    FXString execpath;

#if defined(linux)
    std::vector<char> buffer(MAXPATHLEN);
    for (;;)
    {
        ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (len < 0)
        {
            goto err;
        }
        if ((size_t)len < buffer.size())
        {
            // readlink does not append a terminator, even when it fills the buffer.
            return FXString(buffer.data(), len);
        }
        buffer.resize(buffer.size() * 2);
    }
#else
    char abs_exe_path[MAXPATHLEN] = "";
    int ret;
    char path_save[MAXPATHLEN];
    char *p1, *p2;

    if(!(p1 = strrchr(path, '/')))
    {
        p2 = getcwd(abs_exe_path, sizeof(abs_exe_path));
    }
    else
    {
        *p1 = '\0';
        p2 = getcwd(path_save, sizeof(path_save));
        if (p2 == NULL)
        {
            goto err;
        }

        ret = chdir(path);
        if (ret < 0)
        {
            goto err;
        }

        p2 = getcwd(abs_exe_path, sizeof(abs_exe_path));
        if (p2 == NULL)
        {
            goto err;
        }
    
        ret = chdir(path_save);
        if (ret < 0)
        {
            goto err;
        }
    }
    
    execpath = abs_exe_path;
    return execpath;
#endif

err:
        fprintf(stderr, _("Error: %s\n"), strerror(errno));
        exit(EXIT_FAILURE);
}


// Obtain file path from URI specified as file:///bla/bla/bla...
// If no 'file:' prefix is found, return the input string as is
FXString xf_filefromuri(FXString uri)
{
    if (comparecase("file:", uri, 5) == 0)
    {
        if ((uri[5] == PATHSEPCHAR) && (uri[6] == PATHSEPCHAR))
        {
            return uri.mid(7, uri.length() - 7);
        }
        return uri.mid(5, uri.length() - 5);
    }

    return uri;
}


// Return URI of filename
FXString xf_filetouri(const FXString& file)
{
    return "file://" + file;
}


// Construct a target name by adding a suffix that tells it's a copy of the original target file name
// This suffix can be added before the file extension (if any) and between parentheses (ex: test (copy).txt)
// or at the end of the file name (ex: test.txt.copy)
FXString xf_buildcopyname(const FXString& target, const FXbool isDir, FXString& suffix, const FXuint suffix_pos)
{
    FXString copytarget;
    FXString name = FXPath::name(target);
    FXString dir = FXPath::directory(target);

    // Copy suffix is added before file name extension (if any) and between parentheses
    if (suffix_pos == 0)
    {
        FXString copystr = " (" + suffix;

        // Get file extensions
        FXString ext1 = name.rafter('.', 1);
        FXString ext2 = name.rafter('.', 2);
        FXString ext3 = ext2.before('.');
        FXString ext4 = name.before('.');

        // Case of folder or dot file names (hidden files or folders)
        if (isDir || name.before('.') == "")
        {
            int pos = name.rfind(copystr);

            // First copy
            if (pos < 0)
            {
                copytarget = dir + PATHSEPSTRING + name + copystr + ")";
            }
            // Add a number to the suffix for next copies
            else
            {
                FXString strnum = name.mid(pos + copystr.length(), name.length() - pos - copystr.length());
                FXuint num = FXUIntVal(strnum);
                num = (num == 0 ? num + 2 : num + 1);
                copytarget = dir + PATHSEPSTRING + name.left(pos) + copystr + " " + FXStringVal(num) + ")";
            }
        }
        // Case of compressed tar archive names
        else if (ext3.lower() == "tar")
        {
            FXString basename = name.rbefore('.', 2);
            int pos = basename.rfind(copystr);

            if (pos < 0)
            {
                copytarget = dir + PATHSEPSTRING + basename + copystr + ")." + ext2;
            }
            else
            {
                // Add a number if it's not the first copy
                FXString strnum = name.mid(pos + copystr.length(),
                                           name.length() - pos - copystr.length() - ext2.length() - 1);
                FXuint num = FXUIntVal(strnum);
                num = (num == 0 ? num + 2 : num + 1);

                copytarget = dir + PATHSEPSTRING + name.left(pos) + copystr + " " + FXStringVal(num) + ")." + ext2;
            }
        }
        // Other cases
        else
        {
            // File name has no extension
            if (ext1 == name)
            {
                int pos = name.rfind(copystr);

                // First copy
                if (pos < 0)
                {
                    copytarget = dir + PATHSEPSTRING + name + copystr + ")";
                }
                // Add a number to the suffix for next copies
                else
                {
                    FXString strnum = name.mid(pos + copystr.length(), name.length() - pos - copystr.length());
                    FXuint num = FXUIntVal(strnum);
                    num = (num == 0 ? num + 2 : num + 1);
                    copytarget = dir + PATHSEPSTRING + name.left(pos) + copystr + " " + FXStringVal(num) + ")";
                }
            }
            // File name has an extension
            else
            {
                FXString basename = name.rbefore('.', 1);
                int pos = basename.rfind(copystr);

                // First copy
                if (pos < 0)
                {
                    copytarget = dir + PATHSEPSTRING + basename + copystr + ")." + ext1;
                }
                // Add a number to the suffix for next copies
                else
                {
                    FXString strnum = name.mid(pos + copystr.length(),
                                               name.length() - pos - copystr.length() - ext1.length() - 1);
                    FXuint num = FXUIntVal(strnum);
                    num = (num == 0 ? 2 : num + 1);
                    copytarget = dir + PATHSEPSTRING + name.left(pos) + copystr + " " + FXStringVal(num) + ")." + ext1;
                }
            }
        }
    }
    // Copy suffix is added at the end of the file name
    else
    {
        FXString copystr = "." + suffix;

        int pos = name.rfind(copystr);

        // First copy
        if (pos < 0)
        {
            copytarget = dir + PATHSEPSTRING + name + copystr;
        }
        // Add a number to the suffix for next copies
        else
        {
            FXString strnum = name.mid(pos + copystr.length(), name.length());
            FXuint num = FXUIntVal(strnum);
            num = (num == 0 ? 2 : num + 1);
            copytarget = dir + PATHSEPSTRING + name.left(pos) + copystr + FXStringVal(num);
        }
    }

    // Recursive call to avoid existing file names
    if (xf_existfile(copytarget))
    {
        copytarget = xf_buildcopyname(copytarget, isDir, suffix, suffix_pos);
    }

    return copytarget;
}


// Convert the deletion date to the number of seconds since the epoch
// The string representing the deletion date must be in the format YYYY-MM-DDThh:mm:ss
FXlong xf_deltime(FXString delstr)
{
    // Decompose the date into year, month, day, hour, minutes and seconds
    FXString year = delstr.mid(0, 4);
    FXString mon = delstr.mid(5, 2);
    FXString mday = delstr.mid(8, 2);
    FXString hour = delstr.mid(11, 2);
    FXString min = delstr.mid(14, 2);
    FXString sec = delstr.mid(17, 2);

    // Convert date using mktime()
    tm tmval;

    tmval.tm_sec = atoi(sec.text());
    tmval.tm_min = atoi(min.text());
    tmval.tm_hour = atoi(hour.text()) - 1;
    tmval.tm_mday = atoi(mday.text());
    tmval.tm_mon = atoi(mon.text()) - 1;
    tmval.tm_year = atoi(year.text()) - 1900;
    tmval.tm_isdst = 0;
    FXlong t = (FXlong)mktime(&tmval);

    // If conversion failed, return 0
    if (t < 0)
    {
        t = 0;
    }

    return t;
}


// Test if a directory is empty
// Return -1 if not a directory, 1 if empty and 0 if not empty
int xf_isemptydir(const FXString directory)
{
    int ret = -1;
    DIR* dir;
    struct dirent* entry = NULL;
    int n = 0;

    if ((dir = opendir(directory.text())) != NULL)
    {
        // Skip . and .. and read the third entry
        while (n < 3)
        {
            entry = readdir(dir);
            n++;
        }
        if (entry == NULL)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }
    }
    if (dir)
    {
        closedir(dir);
    }
    return ret;
}


// Test if a directory has sub-directories
// Return -1 if not a directory, 1 if has sub-directories, 0 if does not have
int xf_hassubdirs(const FXString directory)
{
    int ret = -1;
    DIR* dir;
    struct dirent* entry;

    if ((dir = opendir(directory.text())) != NULL)
    {
        ret = 0;

        // Process directory entries
        while (1)
        {
            entry = readdir(dir);

            // No more entries
            if (entry == NULL)
            {
                break;
            }
            // Entry is . or ..
            else if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
            {
                continue;
            }
            // Regular entry
            // We don't use dirent.d_type anymore because of portability issues
            // (e.g. reiserfs don't know dirent.d_type)
            else
            {
                // Stat entry
                struct stat entrystat;
                FXString entrypath = directory + PATHSEPSTRING + entry->d_name;
                if (xf_stat(entrypath.text(), &entrystat) != 0)
                {
                    continue;
                }

                // If directory
                if (S_ISDIR(entrystat.st_mode))
                {
                    ret = 1;
                    break;
                }
            }
        }
    }
    if (dir)
    {
        closedir(dir);
    }
    return ret;
}


// Check if file or directory exists
FXbool xf_existfile(const FXString& file)
{
    struct stat linfo;

    return !file.empty() && (xf_lstat(file.text(), &linfo) == 0);
}


// Check if the file represents a directory
FXbool xf_isdirectory(const FXString& file)
{
    struct stat info;

    return !file.empty() && (xf_stat(file.text(), &info) == 0) && S_ISDIR(info.st_mode);
}


// Check if file represents a file
FXbool xf_isfile(const FXString& file)
{
    struct stat info;

    return !file.empty() && (xf_stat(file.text(), &info) == 0) && S_ISREG(info.st_mode);
}


// Check if current user is member of gid
// (thanks to Armin Buehler <abuehler@users.sourceforge.net>)
FXbool xf_isgroupmember(gid_t gid)
{
    static std::vector<gid_t> gids;
    static FXbool initialized = false;

    // First call : initialization of the number of supplementary groups and the group list
    if (!initialized)
    {
        int ngroups = getgroups(0, NULL);

        if (ngroups < 0)
        {
            goto err;
        }
        gids.resize(ngroups);
        if (ngroups > 0)
        {
            int ret = getgroups(ngroups, gids.data());
            if (ret < 0)
            {
                gids.clear();
                goto err;
            }
            gids.resize(ret);
        }
        initialized = true;
    }

    // Check if the group id is contained within the group list
    for (size_t i = 0; i < gids.size(); i++)
    {
        if (gid == gids[i])
        {
            return true;
        }
    }
    return false;

err:
    int errcode = errno;
    if (errcode)
    {
        fprintf(stderr, _("Error: Can't read group list: %s"), strerror(errcode));
    }
    else
    {
        fprintf(stderr, _("Error: Can't read group list"));
    }

    return false;
}


// Check if the file or the link referred file is readable AND executable
// Function used to test if we can enter a directory
// Uses the access() system function
FXbool xf_isreadexecutable(const FXString& file)
{
    struct stat info;

    // File exists and can be stated
    if (!file.empty() && (xf_stat(file.text(), &info) == 0))
    {
        int ret = access(file.text(), R_OK | X_OK);
        if (ret == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    // File doesn't exist
    else
    {
        return false;
    }
}


// Check if file is readable
FXbool xf_isreadable(const FXString& file)
{
    struct stat info;

    // File exists and can be stated
    if (!file.empty() && (xf_stat(file.text(), &info) == 0))
    {
        int ret = access(file.text(), R_OK);
        if (ret == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    // File doesn't exist
    else
    {
        return false;
    }
}


// Check if file is writable
FXbool xf_iswritable(const FXString& file)
{
    struct stat info;

    // File exists and can be stated
    if (!file.empty() && (xf_stat(file.text(), &info) == 0))
    {
        int ret = access(file.text(), W_OK);
        if (ret == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    // File doesn't exist
    else
    {
        return false;
    }
}


// Check if file represents a link
FXbool xf_islink(const FXString& file)
{
    struct stat linfo;

    return !file.empty() && (xf_lstat(file.text(), &linfo) == 0) && S_ISLNK(linfo.st_mode);
}


// Get file info (file or link referred file)
FXbool xf_info(const FXString& file, struct stat& inf)
{
    return !file.empty() && (xf_stat(file.text(), &inf) == 0);
}


// Return permissions string
// (the FOX function FXSystem::modeString() seems to use another format for the mode field)
FXString xf_permissions(FXuint mode)
{
    char result[11];

    result[0] =
        S_ISLNK(mode) ? 'l' : S_ISREG(mode) ? '-' : S_ISDIR(mode) ? 'd' : S_ISCHR(mode) ? 'c' : S_ISBLK(mode) ? 'b' :
        S_ISFIFO(mode) ? 'p' : S_ISSOCK(mode) ? 's' : '?';
    result[1] = (mode & S_IRUSR) ? 'r' : '-';
    result[2] = (mode & S_IWUSR) ? 'w' : '-';
    result[3] = (mode & S_ISUID) ? 's' : (mode & S_IXUSR) ? 'x' : '-';
    result[4] = (mode & S_IRGRP) ? 'r' : '-';
    result[5] = (mode & S_IWGRP) ? 'w' : '-';
    result[6] = (mode & S_ISGID) ? 's' : (mode & S_IXGRP) ? 'x' : '-';
    result[7] = (mode & S_IROTH) ? 'r' : '-';
    result[8] = (mode & S_IWOTH) ? 'w' : '-';
    result[9] = (mode & S_ISVTX) ? 't' : (mode & S_IXOTH) ? 'x' : '-';
    result[10] = 0;
    return result;
}


// Read symbolic link
FXString xf_readlink(const FXString& file)
{
    char lnk[MAXPATHLEN + 1];
    int len = readlink(file.text(), lnk, MAXPATHLEN);

    if (0 <= len)
    {
        return FXString(lnk, len);
    }
    else
    {
        return FXString::null;
    }
}


// Return true if files are identical
// Compare file names and inodes for case insensitive filesystems
FXbool xf_isidentical(const FXString& file1, const FXString& file2)
{
    if (file1 != file2)
    {
        struct stat linfo1, linfo2;
        return !::xf_lstat(file1.text(), &linfo1) && !::xf_lstat(file2.text(),
                           &linfo2) && linfo1.st_ino == linfo2.st_ino &&
                           linfo1.st_dev == linfo2.st_dev;
    }
    return true;
}


// Start or stop wait cursor (start if type is BEGIN_CURSOR, stop if type is END_CURSOR)
// Do nothing if type is QUERY_CURSOR or anything different from BEGIN_CURSOR and END_CURSOR)
// Return wait cursor count (0 means wait cursor is not set)
int xf_setwaitcursor(FXApp* app, FXuint type)
{
    static int waitcount = 0;

    // Begin wait cursor
    if (type == BEGIN_CURSOR)
    {
        app->beginWaitCursor();
        waitcount++;
    }
    // End wait cursor
    else if (type == END_CURSOR)
    {
        app->endWaitCursor();
        if (waitcount >= 1)
        {
            waitcount--;
        }
        else
        {
            waitcount = 0;
        }
    }
    // Other cases : do nothing
    else
    {
    }

    return waitcount;
}


// Run a command in an internal st terminal
// Return 0 if success, -1 else
// N.B.: zombie process should be dealt with in the main application class
int xf_runst(FXString cmd)
{
    FXString str1, str2;
    std::vector<FXString> arguments;
    int i = 0, j = 1;

    // Parse once; the strings own their storage until the forked terminal exits.
    while (1)
    {
        str1 = cmd.section(' ', i);
        if (str1.empty())
        {
            break;
        }
        if (str1[0] == '\'')
        {
            str2 = cmd.section('\'', j);
            j += 2;
            i += str2.contains(' ');
            arguments.push_back(str2);
        }
        else
        {
            arguments.push_back(str1);
        }
        i++;
    }

    std::vector<char*> args;
    for (size_t k = 0; k < arguments.size(); k++)
    {
        args.push_back(&arguments[k][0]);
    }
    args.push_back(NULL);
    int nbargs = arguments.size();

    // Launch the command in an internal st terminal
    int res;
    static pid_t childpid = 0;
    childpid = fork();

    // Fork succeeded
    if (childpid >= 0)
    {
        if (childpid == 0) // Child
        {
            st(nbargs, &args[0]);
            exit(EXIT_SUCCESS);
        }
        else // Parent
        {
            // Non blocking wait for child
            if (waitpid(childpid, NULL, WNOHANG) < 0)
            {
                res = -1;
            }
            else
            {
                res = 0;
            }
        }
    }
    // Fork failed
    else
    {
        fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));
        res = -1;
    }

    return res;
}


// Get the output of a Unix command
// Return the command output or the error message if any
FXString xf_getcommandoutput(FXString cmd)
{
    FXString data;
    FILE* stream;

    const int max_buffer = 1024;
    char buffer[max_buffer];

    cmd += " 2>&1";

    stream = popen(cmd.text(), "r");
    if (stream)
    {
        while (!feof(stream))
        {
            if (fgets(buffer, max_buffer, stream) != NULL)
            {
                data += buffer;
            }
        }
        pclose(stream);
    }
    return data;
}


// Load a PNG icon from a file in the icon path
FXIcon* xf_loadiconfile(FXApp* app, const FXString iconpath, const FXString iconname,
                        const double scalefrac, const FXColor blendcolor)
{
    // Icon name is empty
    if (iconname.length() == 0)
    {
        return NULL;
    }

    // New PNG icon
    FXIcon* icon = NULL;
    icon = new FXPNGIcon(app);

    if (icon)
    {
        // Find icon in the icon directory
        FXString iconfile = FXPath::search(iconpath, iconname.text());

        if (!iconfile.empty())
        {
            FXFileStream str;

            // Try open the file
            if (str.open(iconfile, FXStreamLoad))
            {
                // Load it
                icon->loadPixels(str);

                // Use IMAGE_KEEP for blending
                icon->setOptions(IMAGE_KEEP);

                // Scale it
                icon->scale(scalefrac * icon->getWidth(), scalefrac * icon->getHeight());

                // Blend it
                icon->blend(blendcolor);

                // Create it
                icon->create();

                // Done
                str.close();

                return icon;
            }
        }

        // Failed, delete the icon
        delete icon;
    }
    return NULL;
}


// Truncate a string to the specified number of UTF-8 characters
// and adds "..." to the end
FXString xf_truncline(FXString str, FXuint maxlinesize)
{
    if (str.count() <= (int)maxlinesize)
    {
        return str;
    }

    return str.trunc(str.validate(maxlinesize)) + "...";
}


// Insert line breaks as needed to allow displaying string using lines
// of specified maximum number of UTF-8 characters
// Don't insert line breaks in a middle of a word
FXString xf_multilines(FXString str, FXuint maxlinesize)
{
    int pos1 = 0;
    int pos2;

    while (1)
    {
        // No more line breaks found
        pos2 = str.find('\n', pos1);
        if (pos2 < 0)
        {
            int nbc = str.count(pos1, str.length());
            if (nbc > (int)maxlinesize)
            {
                int nbl = nbc / maxlinesize;
                for (int n = 1; n <= nbl; n++)
                {
                    // Don't insert line break in a middle of a word
                    int pos3 = str.find(' ', pos1 + n * maxlinesize);
                    if (pos3 < 0)
                    {
                        str.insert(str.validate(pos1 + n * maxlinesize), '\n'); // Use a valid UTF-8 position
                    }
                    else
                    {
                        // Don't insert line break before '?' or '!' 
                        if ( (str.find('?', pos3 + 1) < 0) && (str.find('!', pos3 + 1) < 0) )
                        {
                            str.insert(str.validate(pos3 + 1), '\n'); // Use a valid UTF-8 position
                        } 
                    }
                }
            }

            break;
        }
        // Line break found
        else
        {
            int nbc = str.count(pos1, pos2);
            if (nbc > (int)maxlinesize)
            {
                int nbl = nbc / maxlinesize;
                for (int n = 1; n <= nbl; n++)
                {
                    // Don't insert line break in a middle of a word
                    int pos3 = str.find(' ', pos1 + n * maxlinesize);
                    if (pos3 < 0)
                    {
                        str.insert(str.validate(pos1 + n * maxlinesize), '\n'); // Use a valid UTF-8 position
                    }
                    else
                    {
                        // Don't insert line break before '?' or '!' 
                        if ( (str.find('?', pos3 + 1) < 0) && (str.find('!', pos3 + 1) < 0) )
                        {
                            str.insert(str.validate(pos3 + 1), '\n'); // Use a valid UTF-8 position
                        } 
                    }
  
                    pos2++;
                }
            }
            pos1 = pos2 + 1;
        }
    }

    return str;
}


// Helper function to filter out some file systems from the mounts check
FXbool xf_keepmount(FXString mntdir, FXString mntname)
{
    FXbool ret = false;

    if ( (mntdir.left(6) == "/media") ||
         (mntdir.left(10) == "/run/media") ||
         ( (mntdir.left(4) != "/sys") &&
           (mntdir.left(4) != "/dev") &&
           (mntdir.left(5) != "/proc") &&
           (mntname.left(5) != "gvfsd") &&    // gvfsd mounts
           (mntname.left(6) != "portal") &&   // Some fuse mounts
           (mntname.left(4) != "/run") ) )
    {
        ret = true;
    }

    return ret;
}


// Return current time in seconds
FXulong xf_getcurrenttime(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (FXulong)tv.tv_sec;
}


// Convert duration in seconds tu human readable format
FXString xf_secondstotimestring(FXuint seconds)
{
    FXuint hour = seconds / 3600;
    FXuint min = (seconds % 3600) / 60;
    FXuint sec = seconds % 60;

    char str[64];

    if (hour > 0)
    {
        snprintf(str, sizeof(str), _("%u h %u min %u s"), hour, min, sec);
    }
    else if (min > 0)
    {
        snprintf(str, sizeof(str), _("%u min %u s"), min, sec);
    }
    else
    {
        snprintf(str, sizeof(str), _("%u s"), sec);
    }

    return FXString(str);
}


// Substitute one string by another, case insensitive, without modifying the input.
FXString xf_substitutecase(const FXString& str, const FXString& org, const FXString& rep, bool all)
{
    if (org.empty())
    {
        return str;
    }

    FXString result;
    FXint pos = 0;
    FXint start = 0;
    while (pos <= str.length() - org.length())
    {
        if (::comparecase(str.text() + pos, org.text(), org.length()) == 0)
        {
            result.append(str.text() + start, pos - start);
            result.append(rep);
            pos += org.length();
            start = pos;
            if (!all)
            {
                break;
            }
        }
        else
        {
            pos++;
        }
    }
    result.append(str.text() + start, str.length() - start);
    return result;
}


// Function to calculate a simple hash of the file contents
// Can be used to check if a file has been changed since last check
// Return 0 if file can't be read
FXulong xf_hashfile(const FXchar* filename)
{
    FXlong hash = 7873;
    int c;
    FILE* file = fopen(filename, "r");

    if (!file)
    {
        perror("fopen");
        return 0;
    }

    while ((c = fgetc(file)) != EOF)
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    fclose(file);

    return (FXulong)FXMAX(hash, 0);
}
