#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#if defined(linux)
#include <mntent.h>
#endif

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "MessageBox.h"
#include "Places.h"


// mtab polling interval (ms)
#define MTAB_POLL_INTERVAL    1000


// Margin for places list height (in number of items)
#define PLACES_HEIGHT_MARGIN    2


// Obtain directory path from XDG directory name and user-dirs.dirs file contents
FXString getXDGDirPath(FXString xdgdirname, FXString contents)
{
    int pos1 = contents.find(xdgdirname) + xdgdirname.length() + 2;
    int pos2 = contents.find("\n", pos1) - 1;

    FXString xdgdirpath = contents.mid(pos1, pos2 - pos1);

    xdgdirpath.substitute("$HOME", FXSystem::getHomeDirectory());

    return xf_cleanpath(xdgdirpath);
}


// Map
FXDEFMAP(PlacesList) PlacesListMap[] =
{
    FXMAPFUNC(SEL_QUERY_TIP, 0, PlacesList::onQueryTip)
};


// Object implementation
FXIMPLEMENT(PlacesList, FXList, PlacesListMap, ARRAYNUMBER(PlacesListMap))


// Construct places list
PlacesList::PlacesList(FXWindow* owner, FXComposite* p, FXObject* tgt, FXSelector sel, FXColor listbackcolor,
                       FXColor listforecolor, FXuint opts, int x, int y, int w, int h) :
    FXList(p, tgt, sel, opts | LIST_SINGLESELECT, x, y, w, h)
{
    // Colors
    setTextColor(listforecolor);
    setBackColor(listbackcolor);

    // Dictionary of places
    places = new FXStringDict();

    FXString name, pathname;

    // Number of displayed items
    FXuint num = 0;

    // Home directory
    name = _("Home");
    pathname = FXSystem::getHomeDirectory();
    places->insert(name.text(), pathname.text());
    FXList::appendItem(name, minifolderhomeicon);
    num++;

    // Read user-dir.dirs file
    FXString userdirsfile = FXSystem::getHomeDirectory() + PATHSEPSTRING + ".config/user-dirs.dirs";
    FILE* file = fopen(userdirsfile.text(), "r");

    if (file)  // File exists
    {
        FXString str;

        char* line = NULL;
        size_t len = 0;
        ssize_t nread;

        while ((nread = getline(&line, &len, file)) != -1)
        {
            str += line;
        }

        fclose(file);
        free(line);

        // Desktop directory
        FXString xdgdirname = "XDG_DESKTOP_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Desktop");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minidesktopicon);
            num++;
        }

        // Downloads directory
        xdgdirname = "XDG_DOWNLOAD_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Downloads");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minifolderdownloadicon);
            num++;
        }

        // Documents directory
        xdgdirname = "XDG_DOCUMENTS_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Documents");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minifolderdocumentsicon);
            num++;
        }

        // Music directory
        xdgdirname = "XDG_MUSIC_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Music");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minifoldermusicicon);
            num++;
        }

        // Images directory
        xdgdirname = "XDG_PICTURES_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Images");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minifolderimagesicon);
            num++;
        }

        // Videos directory
        xdgdirname = "XDG_VIDEOS_DIR";
        pathname = getXDGDirPath(xdgdirname, str);

        if (pathname != FXSystem::getHomeDirectory())
        {
            name = _("Videos");
            places->insert(name.text(), pathname.text());
            FXList::appendItem(name, minifoldervideosicon);
            num++;
        }
    }

    // Trash
    name = _("Trash can");
    pathname = FXSystem::getHomeDirectory() + PATHSEPSTRING + ".local/share/Trash/files";
    places->insert(name.text(), pathname.text());
    FXList::appendItem(name, minideleteicon);
    num++;

    // Resize list to the number of items (including a margin)
    FXList::setNumVisible(num + PLACES_HEIGHT_MARGIN);
}


// Create X window
void PlacesList::create()
{
    FXList::create();
}


// Destructor
PlacesList::~PlacesList()
{
    delete places;
}


// Select item from key and set it current
void PlacesList::setPlace(FXString& key)
{
    for (int i = 0; i < getNumItems(); i++)
    {
        if (getItemText(i) == key)
        {
            selectItem(i);
            setCurrentItem(i);
            break;
        }
    }
}


// Display bookmark location tooltip
long PlacesList::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    getCursorPosition(x, y, state);
    int index = getItemAt(x, y);

    if (index != -1)
    {
        FXString key = getItemText(index);
        FXString tip = getItemPathname(key);

        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}



// Map
FXDEFMAP(MountsList) MountsListMap[] =
{
#if defined(linux)
    FXMAPFUNC(SEL_TIMEOUT, MountsList::ID_MOUNTS_REFRESH, MountsList::onMountsRefresh),
#endif
    FXMAPFUNC(SEL_QUERY_TIP, 0, MountsList::onQueryTip)
};


// Object implementation
FXIMPLEMENT(MountsList, FXList, MountsListMap, ARRAYNUMBER(MountsListMap))


// Construct mount points list
MountsList::MountsList(FXWindow* owner, FXComposite* p, FXObject* tgt, FXSelector sel, FXColor listbackcolor,
                       FXColor listforecolor, FXuint opts, int x, int y, int w, int h) :
    FXList(p, tgt, sel, opts | LIST_SINGLESELECT, x, y, w, h)
{
    // Colors
    setTextColor(listforecolor);
    setBackColor(listbackcolor);

    // Dictionary of mount points
    mounts = new FXStringDict();

#if defined(linux)

    FXString name, pathname;

    // Mount points from /etc/mtab
    struct mntent* mnt;
    FILE* mtab = setmntent(MTAB_PATH, "r");
    if (mtab)
    {
        while ((mnt = getmntent(mtab)))
        {
            // Filter out some file systems
            if (xf_keepmount(mnt->mnt_dir, mnt->mnt_fsname))
            {
                name = mnt->mnt_dir;
                pathname = mnt->mnt_dir;

                // Root file system
                if (name == ROOTDIR)
                {
                    name = _("File system");
                    mounts->insert(name.text(), pathname.text());
                    FXList::appendItem(name, miniharddriveicon);
                }
                else if (strncmp(mnt->mnt_type, "cifs", 4) == 0 || strncmp(mnt->mnt_type, "nfs", 3) == 0)
                {
                    mounts->insert(name.text(), pathname.text());
                    FXList::appendItem(name, mininetdriveicon);
                }
                else if (strncmp(mnt->mnt_dir, "/media", 6) == 0)
                {
                    if (strncmp(mnt->mnt_type, "iso", 3) == 0)
                    {
                        mounts->insert(name.text(), pathname.text());
                        FXList::appendItem(name, minicdromicon);
                    }
                    else
                    {
                        mounts->insert(name.text(), pathname.text());
                        FXList::appendItem(name, miniremovabledeviceicon);
                    }
                }
                else
                {
                    // Nothing to do
                }
            }
        }
        endmntent(mtab);
    }

    // Get mtab hash
    hash_prev = xf_hashfile(MTAB_PATH);

    // Path of GVFS mount points
    FXString gvfspath = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";

    // Get directory stream pointer
    DIR* dirp;
    struct dirent* dp;
    dirp = opendir(gvfspath.text());

    // Managed to open directory
    if (dirp)
    {
        // Process directory entries
        while ((dp = readdir(dirp)) != NULL)
        {
            // Get name of entry
            FXString gvfsname = dp->d_name;

            // List of GVFS mounts
            gvfsmounts_prev += gvfsname;

            // Get mount type
            FXString mounttype = gvfsname.before(':');

            // Samba share
            if (mounttype == "smb-share")
            {
                FXString tmp;
                int pos = 0;

                // Server name
                pos = gvfsname.find("server=") + 7;
                tmp = gvfsname.right(gvfsname.length() - pos);
                FXString server = tmp.before(',');

                // Share name
                pos = gvfsname.find("share=") + 6;
                tmp = gvfsname.right(gvfsname.length() - pos);
                FXString share = tmp.before(',');

                // Mount name and path
                FXString name = share.substitute("%20", " ") + "@" + server;
                FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                mounts->insert(name.text(), pathname.text());
                FXList::appendItem(name, mininetdriveicon);
            }

            // SSH mount
            if (mounttype == "sftp")
            {
                FXString tmp;
                int pos = 0;

                // Host name
                pos = gvfsname.find("host=") + 5;
                tmp = gvfsname.right(gvfsname.length() - pos);
                FXString host = tmp.before(',');

                // User name
                pos = gvfsname.find("user=") + 5;
                tmp = gvfsname.right(gvfsname.length() - pos);
                FXString user = tmp.before(',');

                // Mount name and path
                FXString name = user.substitute("%20", " ") + "@" + host;
                FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                mounts->insert(name.text(), pathname.text());
                FXList::appendItem(name, mininetdriveicon);
            }

            // MTP device
            if (mounttype == "mtp")
            {
                // Get mount name
                FXString name = gvfsname.after('_');
                name = name.before('_', 2);

                // Mount name and path
                FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                mounts->insert(name.text(), pathname.text());
                FXList::appendItem(name, miniremovabledeviceicon);
            }
        }

        // Close directory
        closedir(dirp);
    }

#endif
}


// Create X window
void MountsList::create()
{
#if defined(linux)
    getApp()->addTimeout(this, ID_MOUNTS_REFRESH, MTAB_POLL_INTERVAL);
#endif
    FXList::create();
}


// Destructor
MountsList::~MountsList()
{
#if defined(linux)
    getApp()->removeTimeout(this, ID_MOUNTS_REFRESH);
#endif
    delete mounts;
}


// Select item from key and set it current
void MountsList::setMount(FXString& key)
{
    for (int i = 0; i < getNumItems(); i++)
    {
        if (getItemText(i) == key)
        {
            selectItem(i);
            setCurrentItem(i);
            break;
        }
    }
}


#if defined(linux)

// Refresh mount points
long MountsList::onMountsRefresh(FXObject* sender, FXSelector, void* ptr)
{
    // Get mtab hash
    FXulong hash = xf_hashfile(MTAB_PATH);

    // Check GVFS mounts
    DIR* dirp;
    struct dirent* dp;
    FXString gvfsmounts = "";

    // Path of gfvs mounts
    FXString gvfspath = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";

    // Obtain gvfs mounts list
    dirp = opendir(gvfspath.text());
    if (dirp)
    {
        while ((dp = readdir(dirp)) != NULL)
        {
            gvfsmounts += dp->d_name;
        }
        closedir(dirp);
    }

    // Mount points or GVFS mounts have changed
    if (hash != hash_prev || gvfsmounts != gvfsmounts_prev)
    {
        // Update hash
        hash_prev = hash;

        // Remove all mount points from list
        for (int i = mounts->first(); i < mounts->size(); i = mounts->next(i))
        {
            // Remove item from list
            FXList::removeItem(FXList::findItem(mounts->key(i)));
        }

        // Clear mount points dictionary
        mounts->clear();

        FXString name, pathname;

        struct mntent* mnt;
        FILE* mtab = setmntent(MTAB_PATH, "r");
        if (mtab)
        {
            while ((mnt = getmntent(mtab)))
            {
                // Filter out some file systems
                if (xf_keepmount(mnt->mnt_dir, mnt->mnt_fsname))
                {
                    name = mnt->mnt_dir;
                    pathname = mnt->mnt_dir;

                    // Root file system
                    if (name == ROOTDIR)
                    {
                        name = _("File system");
                        mounts->insert(name.text(), pathname.text());
                        FXList::appendItem(name, miniharddriveicon);
                    }
                    else if (strncmp(mnt->mnt_type, "cifs", 4) == 0 || strncmp(mnt->mnt_type, "nfs", 3) == 0)
                    {
                        mounts->insert(name.text(), pathname.text());
                        FXList::appendItem(name, mininetdriveicon);
                    }
                    else if (strncmp(mnt->mnt_dir, "/media", 6) == 0)
                    {
                        if (strncmp(mnt->mnt_type, "iso", 3) == 0)
                        {
                            mounts->insert(name.text(), pathname.text());
                            FXList::appendItem(name, minicdromicon);
                        }
                        else
                        {
                            mounts->insert(name.text(), pathname.text());
                            FXList::appendItem(name, miniremovabledeviceicon);
                        }
                    }
                    else
                    {
                        // Nothing to do
                    }
                }
            }
            endmntent(mtab);
        }

        // Update GVFS mounts list
        gvfsmounts_prev = gvfsmounts;

        // Get directory stream pointer
        dirp = opendir(gvfspath.text());

        // Managed to open directory
        if (dirp)
        {
            // Process directory entries
            while ((dp = readdir(dirp)) != NULL)
            {
                // Get name of entry
                FXString gvfsname = dp->d_name;

                // Get mount type
                FXString mounttype = gvfsname.before(':');

                // Samba share
                if (mounttype == "smb-share")
                {
                    FXString tmp;
                    int pos = 0;

                    // Server name
                    pos = gvfsname.find("server=") + 7;
                    tmp = gvfsname.right(gvfsname.length() - pos);
                    FXString server = tmp.before(',');

                    // Share name
                    pos = gvfsname.find("share=") + 6;
                    tmp = gvfsname.right(gvfsname.length() - pos);
                    FXString share = tmp.before(',');

                    // Mount name and path
                    FXString name = share + "@" + server;
                    FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                    mounts->insert(name.text(), pathname.text());
                    FXList::appendItem(name, mininetdriveicon);
                }

                // SSH mount
                if (mounttype == "sftp")
                {
                    FXString tmp;
                    int pos = 0;

                    // Host name
                    pos = gvfsname.find("host=") + 5;
                    tmp = gvfsname.right(gvfsname.length() - pos);
                    FXString host = tmp.before(',');

                    // User name
                    pos = gvfsname.find("user=") + 5;
                    tmp = gvfsname.right(gvfsname.length() - pos);
                    FXString user = tmp.before(',');

                    // Mount name and path
                    FXString name = user.substitute("%20", " ") + "@" + host;
                    FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                    mounts->insert(name.text(), pathname.text());
                    FXList::appendItem(name, mininetdriveicon);
                }

                // MTP device
                if (mounttype == "mtp")
                {
                    // Mount name and path
                    FXString name = gvfsname.after('_');
                    name = name.before('_', 2);
                    FXString pathname = gvfspath + PATHSEPSTRING + gvfsname;
                    mounts->insert(name.text(), pathname.text());
                    FXList::appendItem(name, miniremovabledeviceicon);
                }
            }

            // Close directory
            closedir(dirp);
        }
    }

    // Restart timer
    getApp()->addTimeout(this, ID_MOUNTS_REFRESH, MTAB_POLL_INTERVAL);

    return 1;
}

#endif


// Display bookmark location tooltip
long MountsList::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    getCursorPosition(x, y, state);
    int index = getItemAt(x, y);

    if (index != -1)
    {
        FXString key = getItemText(index);
        FXString tip = getItemPathname(key);

        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}


// Map
FXDEFMAP(BookmarksList) BookmarksListMap[] =
{
    FXMAPFUNC(SEL_QUERY_TIP, 0, BookmarksList::onQueryTip),
};


// Object implementation
FXIMPLEMENT(BookmarksList, FXList, BookmarksListMap, ARRAYNUMBER(BookmarksListMap))


// Construct bookmarks list
BookmarksList::BookmarksList(FXWindow* owner, FXComposite* p, FXObject* tgt, FXSelector sel, FXColor listbackcolor,
                             FXColor listforecolor, FXuint opts, int x, int y, int w, int h) :
    FXList(p, tgt, sel, opts | LIST_SINGLESELECT, x, y, w, h)
{
    // Fractional scaling factor
    FXint res = getApp()->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    scalefrac = FXMAX(1.0, res / 100.0);

    // Colors
    setTextColor(listforecolor);
    setBackColor(listbackcolor);

    // Read bookmark keys from registry
    if (getApp()->reg().readStringEntry("BOOKMARKS", "bookmark0", NULL))
    {
        associations = new FileDict(getApp());

        FXuint i = 0;
        while (1)
        {
            FXString regkey = "bookmark" + FXStringVal(i);
            FXString data = getApp()->reg().readStringEntry("BOOKMARKS", regkey.text(), NULL);

            if (data != "")
            {
                FXString name = getBookmarkName(data);
                FXString pathname = getBookmarkPathname(data);
                FXString iconpathname = getBookmarkIconPathname(data);

                // Append to vector
                bookmarks.push_back(data);

                // Append to list
                if (iconpathname == "" || !xf_existfile(iconpathname))
                {
                    FXList::appendItem(name, minibookmarkicon);   // Default icon
                }
                else
                {
                    FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(iconpathname),
                                                FXPath::name(iconpathname), scalefrac, this->getBackColor());
                    if (icon)
                    {
                        icon->create();
                    }

                    FXList::appendItem(name, icon);  // Dedicated icon
                }

                i++;
            }
            else
            {
                break;
            }
        }

        nbBookmarks = bookmarks.size();
    }

    // Check if there are legacy bookmarks
    FXbool legacy_bookmarks = getApp()->reg().readUnsignedEntry("OPTIONS", "legacy_bookmarks", true);
    if (legacy_bookmarks && getApp()->reg().readStringEntry("bookmarks", "BOOKMARK1", NULL))
    {
        FXuint i = 1;
        FXuint j = bookmarks.size();
        while (1)
        {
            FXString regkey = "BOOKMARK" + FXStringVal(i);
            FXString pathname = getApp()->reg().readStringEntry("bookmarks", regkey.text(), NULL);

            if (pathname != "")
            {
                // Append bookmark to vector if directory exists
                if (xf_existfile(pathname))
                {
                    // Append to vector
                    FXString data = pathname + ";" + pathname + ";";
                    bookmarks.push_back(data);

                    // Append to list with default icon
                    FXList::appendItem(pathname, minibookmarkicon);

                    // Add registry key
                    regkey = "bookmark" + FXStringVal(j);
                    getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), data.text());

                    j++;
                }

                i++;
            }
            else
            {
                break;
            }
        }

        nbBookmarks = bookmarks.size();

        // Remove legacy bookmarks section
        getApp()->reg().deleteSection("bookmarks");
    }

    // Write legacy bookmarks flag
    getApp()->reg().writeUnsignedEntry("OPTIONS", "legacy_bookmarks", false);

    // Update registry
    getApp()->reg().write();
}


// Create X window
void BookmarksList::create()
{
    FXList::create();
}


// Destructor
BookmarksList::~BookmarksList()
{
    if (associations)
    {
        delete associations;
    }
}


// Select item from key and set it current
void BookmarksList::setBookmark(FXString& key)
{
    for (int i = 0; i < getNumItems(); i++)
    {
        if (getItemText(i) == key)
        {
            selectItem(i);
            setCurrentItem(i);
            break;
        }
    }
}


// Modify bookmark (name or icon)
// Return 1 if success else return 0
int BookmarksList::modifyBookmark(FXString& oldname, FXString& newname, FXString& oldiconpathname,
                                  FXString& newiconpathname, FXString& pathname)
{
    // Check new name
    if (newname == "" || (newname == oldname && newiconpathname == oldiconpathname))
    {
        return 0;
    }

    if (newname.contains(';'))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Bookmark name must not contain the character ';'"));
        return 0;
    }

    if (newiconpathname.contains(';'))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Bookmark icon path name must not contain the character ';'"));
        return 0;
    }

    // Bookmark position
    FXuint index = FXList::getCurrentItem();

    // Check if an item with the same name already exists
    for (FXuint i = 0; i < nbBookmarks; i++)
    {
        if ((getBookmarkName(i) == newname) && (i != index))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Bookmark %s already exists"), newname.text());
            return 0;
        }
    }

    // Modify bookmark
    if (newiconpathname == "" || !xf_existfile(newiconpathname))
    {
        FXList::setItem(index, newname, minibookmarkicon);   // Default icon
        bookmarks[index] = newname + ";" + getBookmarkPathname(index) + ";" + "";
    }
    else
    {
        FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(newiconpathname),
                                    FXPath::name(newiconpathname), scalefrac, this->getBackColor());
        if (icon)
        {
            icon->create();
        }

        FXList::setItem(index, newname, icon);   // Dedicated icon

        bookmarks[index] = newname + ";" + getBookmarkPathname(index) + ";" + newiconpathname;
    }

    // Write registry key
    FXString regkey = "bookmark" + FXStringVal(index);
    getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), bookmarks[index].text());

    // Update registry
    getApp()->reg().write();

    return 1;
}


// Append bookmark
// Return 1 if success else return 0
int BookmarksList::appendBookmark(FXString& name, FXString& pathname, FXString& iconpathname)
{
    // Check name
    if (name == "")
    {
        return 0;
    }
    else
    {
        for (FXuint i = 0; i < nbBookmarks; i++)
        {
            if (getBookmarkName(i) == name)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Bookmark %s already exists"), name.text());
                return 0;
            }

            if (getBookmarkPathname(i) == pathname)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Bookmark location %s already exists"), pathname.text());
                return 0;
            }
        }
    }

    // Append bookmark
    appendItem(name, pathname, iconpathname);

    return 1;
}


// Append an item
void BookmarksList::appendItem(FXString& name, FXString& pathname, FXString& iconpathname)
{
    // Append item to vector
    FXString data;

    if (xf_existfile(iconpathname))
    {
        data = name + ";" + pathname + ";" + iconpathname;
    }
    else
    {
        data = name + ";" + pathname + ";";
    }
    bookmarks.push_back(data);
    nbBookmarks = bookmarks.size();

    // Write registry key
    FXString regkey = "bookmark" + FXStringVal(nbBookmarks - 1);
    getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), data.text());


    // Append item to list
    if (xf_existfile(iconpathname))
    {
        FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(iconpathname),
                                    FXPath::name(iconpathname), scalefrac, this->getBackColor());
        if (icon)
        {
            icon->create();
        }

        FXList::appendItem(name, icon);  // Dedicated icon
    }
    else
    {
        FXList::appendItem(name, minibookmarkicon);   // Default icon
    }

    // Update registry
    getApp()->reg().write();
}


// Remove an item
void BookmarksList::removeItem(const FXString& name)
{
    // Remove item from list
    FXList::removeItem(FXList::findItem(name));

    // Remove item from vector
    for (FXuint i = 0; i < nbBookmarks; i++)
    {
        if (getBookmarkName(i) == name)
        {
            bookmarks.erase(bookmarks.begin() + i);
            break;
        }
    }
    nbBookmarks = bookmarks.size();

    // Delete bookmarks section
    getApp()->reg().read();
    getApp()->reg().deleteSection("BOOKMARKS");

    // Add bookmarks to registry
    for (FXuint i = 0; i < nbBookmarks; i++)
    {
        FXString regkey = "bookmark" + FXStringVal(i);
        getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), bookmarks[i].text());
    }

    // Update registry
    getApp()->reg().write();
}


// Remove all items
void BookmarksList::removeAllItems(void)
{
    // Remove all bookmarks from list
    FXList::clearItems();

    // Clear bookmarks vector
    bookmarks.clear();
    nbBookmarks = 0;

    // Remove registry section
    getApp()->reg().read();
    getApp()->reg().deleteSection("BOOKMARKS");

    // Update registry
    getApp()->reg().write();
}


// Move bookmark up
void BookmarksList::moveUp(FXString& name)
{
    FXuint index = FXList::getCurrentItem();

    if (index > 0)
    {
        FXString tmp = bookmarks[index - 1];
        bookmarks[index - 1] = bookmarks[index];
        bookmarks[index] = tmp;

        FXList::clearItems();

        getApp()->reg().deleteSection("BOOKMARKS");

        for (FXuint i = 0; i < nbBookmarks; i++)
        {
            // Append item to list
            FXString iconpathname = getItemIconPathname(getBookmarkName(i));
            if (xf_existfile(iconpathname))
            {
                FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(iconpathname),
                                            FXPath::name(iconpathname), scalefrac, this->getBackColor());
                if (icon)
                {
                    icon->create();
                }

                FXList::appendItem(getBookmarkName(i), icon);  // Dedicated icon
            }
            else
            {
                FXList::appendItem(getBookmarkName(i), minibookmarkicon);   // Default icon
            }

            FXString regkey = "bookmark" + FXStringVal(i);
            getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), bookmarks[i].text());
        }
        getApp()->reg().write();
    }
}

// Move bookmark down
void BookmarksList::moveDown(FXString& name)
{
    FXuint index = FXList::getCurrentItem();

    if (index < nbBookmarks - 1)
    {
        FXString tmp = bookmarks[index + 1];
        bookmarks[index + 1] = bookmarks[index];
        bookmarks[index] = tmp;

        FXList::clearItems();

        getApp()->reg().deleteSection("BOOKMARKS");

        for (FXuint i = 0; i < nbBookmarks; i++)
        {
            // Append item to list
            FXString iconpathname = getItemIconPathname(getBookmarkName(i));
            if (xf_existfile(iconpathname))
            {
                FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(iconpathname),
                                            FXPath::name(iconpathname), scalefrac, this->getBackColor());
                if (icon)
                {
                    icon->create();
                }

                FXList::appendItem(getBookmarkName(i), icon);  // Dedicated icon
            }
            else
            {
                FXList::appendItem(getBookmarkName(i), minibookmarkicon);       // Default icon
            }

            FXString regkey = "bookmark" + FXStringVal(i);
            getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), bookmarks[i].text());
        }
        getApp()->reg().write();
    }
}


// Display bookmark location tooltip
long BookmarksList::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    getCursorPosition(x, y, state);
    int index = getItemAt(x, y);

    if (index != -1)
    {
        FXString name = getItemText(index);

        FXString tip;
        for (FXuint i = 0; i < nbBookmarks; i++)
        {
            if (getBookmarkName(i) == name)
            {
                tip = getBookmarkPathname(i);
                break;
            }
        }

        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}
