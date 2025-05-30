// File list. Taken from the FOX library and heavily modified.
// The compare(), compare_nolocale() and compare_locale() functions are adapted from a patch
// submitted by Vladimir Támara Patiño


#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>
#include <FXJPGIcon.h>
#include <FXTIFIcon.h>
#if defined(linux)
#include <mntent.h>
#endif

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "File.h"
#include "FileDict.h"
#include "IconList.h"
#include "MessageBox.h"
#include "InputDialog.h"
#include "StringList.h"
#include "FileList.h"


// Time interval before refreshing the view
#define REFRESH_INTERVAL     1000
#define REFRESH_FREQUENCY    30

// Time interval before opening a folder
#define OPEN_INTERVAL        1000

// Counter limit for image refreshing
#define REFRESH_COUNT        100

#define HASH1(x, n)    (((FXuint)(x) * 13) % (n))               // Probe Position [0..n-1]
#define HASH2(x, n)    (1 | (((FXuint)(x) * 17) % ((n) - 1)))   // Probe Distance [1..n-1]


// Global variables
extern FXString xdgdatahome;

#if defined(linux)
FXStringDict* fsdevices = NULL; // Devices from fstab
FXStringDict* mtdevices = NULL; // Mounted devices
FXStringDict* updevices = NULL; // Responding devices
#endif


// Initialize static variables
int FileList::name_index = -1;
int FileList::size_index = -1;
int FileList::type_index = -1;
int FileList::ext_index = -1;
int FileList::date_index = -1;
int FileList::user_index = -1;
int FileList::group_index = -1;
int FileList::perms_index = -1;
int FileList::link_index = -1;
int FileList::origpath_index = -1;
int FileList::deldate_index = -1;
int FileList::dirname_index = -1;


// Object implementation
FXIMPLEMENT(FileItem, IconItem, NULL, 0)


// Map
FXDEFMAP(FileList) FileListMap[] =
{
    FXMAPFUNC(SEL_DRAGGED, 0, FileList::onDragged),
    FXMAPFUNC(SEL_TIMEOUT, FileList::ID_REFRESH_TIMER, FileList::onCmdRefreshTimer),
    FXMAPFUNC(SEL_TIMEOUT, FileList::ID_OPEN_TIMER, FileList::onOpenTimer),
    FXMAPFUNC(SEL_DND_ENTER, 0, FileList::onDNDEnter),
    FXMAPFUNC(SEL_DND_LEAVE, 0, FileList::onDNDLeave),
    FXMAPFUNC(SEL_DND_DROP, 0, FileList::onDNDDrop),
    FXMAPFUNC(SEL_DND_MOTION, 0, FileList::onDNDMotion),
    FXMAPFUNC(SEL_DND_REQUEST, 0, FileList::onDNDRequest),
    FXMAPFUNC(SEL_BEGINDRAG, 0, FileList::onBeginDrag),
    FXMAPFUNC(SEL_ENDDRAG, 0, FileList::onEndDrag),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DRAG_COPY, FileList::onCmdDragCopy),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DRAG_MOVE, FileList::onCmdDragMove),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DRAG_LINK, FileList::onCmdDragLink),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DRAG_REJECT, FileList::onCmdDragReject),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DIRECTORY_UP, FileList::onCmdDirectoryUp),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_NAME, FileList::onCmdSortByName),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_DIRNAME, FileList::onCmdSortByDirName),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_TYPE, FileList::onCmdSortByType),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_SIZE, FileList::onCmdSortBySize),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_EXT, FileList::onCmdSortByExt),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_DATE, FileList::onCmdSortByDate),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_USER, FileList::onCmdSortByUser),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_GROUP, FileList::onCmdSortByGroup),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_PERMS, FileList::onCmdSortByPerms),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_LINK, FileList::onCmdSortByLink),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_DELDATE, FileList::onCmdSortByDeldate),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_COL_ORIGPATH, FileList::onCmdSortByOrigpath),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_SORT_REVERSE, FileList::onCmdSortReverse),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_SORT_CASE, FileList::onCmdSortCase),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_DIRS_FIRST, FileList::onCmdDirsFirst),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_SET_PATTERN, FileList::onCmdSetPattern),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_SHOW_HIDDEN, FileList::onCmdShowHidden),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_HIDE_HIDDEN, FileList::onCmdHideHidden),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_TOGGLE_HIDDEN, FileList::onCmdToggleHidden),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_TOGGLE_THUMBNAILS, FileList::onCmdToggleThumbnails),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_HEADER_CHANGE, FileList::onCmdHeader),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_REFRESH, FileList::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, FileList::ID_CLOSE_FILTER, FileList::onCmdCloseFilter),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_HEADER_CHANGE, FileList::onUpdHeader),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_DIRECTORY_UP, FileList::onUpdDirectoryUp),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_NAME, FileList::onUpdSortByName),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_DIRNAME, FileList::onUpdSortByDirName),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_TYPE, FileList::onUpdSortByType),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_SIZE, FileList::onUpdSortBySize),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_EXT, FileList::onUpdSortByExt),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_DATE, FileList::onUpdSortByDate),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_USER, FileList::onUpdSortByUser),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_GROUP, FileList::onUpdSortByGroup),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_PERMS, FileList::onUpdSortByPerms),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_LINK, FileList::onUpdSortByLink),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_DELDATE, FileList::onUpdSortByDeldate),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_COL_ORIGPATH, FileList::onUpdSortByOrigpath),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_SORT_REVERSE, FileList::onUpdSortReverse),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_SORT_CASE, FileList::onUpdSortCase),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_DIRS_FIRST, FileList::onUpdDirsFirst),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_SET_PATTERN, FileList::onUpdSetPattern),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_SHOW_HIDDEN, FileList::onUpdShowHidden),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_HIDE_HIDDEN, FileList::onUpdHideHidden),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_TOGGLE_HIDDEN, FileList::onUpdToggleHidden),
    FXMAPFUNC(SEL_UPDATE, FileList::ID_TOGGLE_THUMBNAILS, FileList::onUpdToggleThumbnails),
    FXMAPFUNC(SEL_UPDATE, 0, FileList::onUpdRefreshTimer),
};


// Object implementation
FXIMPLEMENT(FileList, IconList, FileListMap, ARRAYNUMBER(FileListMap))



// // Construct
FileList::FileList(FXWindow* focuswin, FXComposite* p, FXuint* ic, FXuint nc,
                   FXObject* tgt, FXSelector sel, FXbool showthumbs,
                   FXuint ds, FXuint os, FXuint opts, int x, int y, int w, int h) :
    IconList(p, ic, nc, tgt, sel, opts | ICONLIST_FILELIST, x, y, w, h),
    directory(ROOTDIR), orgdirectory(ROOTDIR), pattern("*")
{
    flags |= FLAG_ENABLED | FLAG_DROPTARGET;
    associations = NULL;
    deldate_size = ds;
    origpath_size = os;

    // Columns id
    nbCols = nc;
    for (FXuint i = 0; i < nbCols; i++)
    {
        idCol[i] = ic[i];
    }
    idCol[0] = ID_COL_NAME; // Force first column to name

    // Append headers
    for (FXuint i = 0; i < nbCols; i++)
    {
        appendHeader(getHeaderName(idCol[i]));
    }

    // Not in a search list
    if (!(options & FILELIST_SEARCH))
    {
        // Column ids when in Trash
        idColTrash[0] = ID_COL_NAME; // Force to name
        idColTrash[1] = ID_COL_ORIGPATH;
        idColTrash[2] = ID_COL_DELDATE;
        for (FXuint i = 1; i < nbCols; i++)
        {
            idColTrash[i + 2] = idCol[i];
        }
        nbColsTrash = nbCols + 2;
    }

    // Header indices by name
    name_index = 0; // Always 0
    size_index = getHeaderIndex(ID_COL_SIZE);
    type_index = getHeaderIndex(ID_COL_TYPE);
    ext_index = getHeaderIndex(ID_COL_EXT);
    date_index = getHeaderIndex(ID_COL_DATE);
    user_index = getHeaderIndex(ID_COL_USER);
    group_index = getHeaderIndex(ID_COL_GROUP);
    perms_index = getHeaderIndex(ID_COL_PERMS);
    link_index = getHeaderIndex(ID_COL_LINK);
    origpath_index = getHeaderIndex(ID_COL_ORIGPATH);
    deldate_index = getHeaderIndex(ID_COL_DELDATE);
    dirname_index = getHeaderIndex(ID_COL_DIRNAME);

    // Fractional scaling factor
    FXint res = getApp()->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    scalefrac = FXMAX(1.0, res / 100.0);

    // Initializations
    matchmode = FILEMATCH_FILE_NAME | FILEMATCH_NOESCAPE;
    associations = new FileDict(getApp());
    dropaction = DRAG_MOVE;
    sortfunc = ascendingCase;
    dirsfirst = true;
    filter_folders = false;
    allowrefresh = true;
    timestamp = 0;
    counter = 1;
    prevIndex = -1;
    list = NULL;
    displaythumbnails = showthumbs;
    backhist = NULL;
    forwardhist = NULL;
    focuswindow = focuswin;

#if defined(linux)
    // Initialize the fsdevices, mtdevices and updevices lists
    // if it was not done in DirList (useful for XFileWrite, XFilePackage and XFileImage)
    struct mntent* mnt;
    if (fsdevices == NULL)
    {
        // To list file system devices
        fsdevices = new FXStringDict();
        FILE* fstab = setmntent(FSTAB_PATH, "r");
        if (fstab)
        {
            while ((mnt = getmntent(fstab)))
            {
                if (!xf_strequal(mnt->mnt_type, MNTTYPE_IGNORE) && !xf_strequal(mnt->mnt_type, MNTTYPE_SWAP) &&
                    !xf_strequal(mnt->mnt_dir, "/"))
                {
                    if (!strncmp(mnt->mnt_fsname, "/dev/fd", 7))
                    {
                        fsdevices->insert(mnt->mnt_dir, "floppy");
                    }
                    else if (!strncmp(mnt->mnt_type, "iso", 3))
                    {
                        fsdevices->insert(mnt->mnt_dir, "cdrom");
                    }
                    else if (!strncmp(mnt->mnt_fsname, "/dev/zip", 8))
                    {
                        fsdevices->insert(mnt->mnt_dir, "zip");
                    }
                    else if (!strncmp(mnt->mnt_type, "nfs", 3))
                    {
                        fsdevices->insert(mnt->mnt_dir, "nfsdisk");
                    }
                    else if (xf_strequal(mnt->mnt_type, "smbfs") || xf_strequal(mnt->mnt_type, "cifs"))
                    {
                        fsdevices->insert(mnt->mnt_dir, "smbdisk");
                    }
                    else
                    {
                        fsdevices->insert(mnt->mnt_dir, "harddrive");
                    }
                }
            }
            endmntent(fstab);
        }
    }
    if (mtdevices == NULL)
    {
        // To list mounted devices
        mtdevices = new FXStringDict();
        FILE* mtab = setmntent(MTAB_PATH, "r");
        if (mtab)
        {
            while ((mnt = getmntent(mtab)))
            {
                // Filter out some file systems
                FXString mntdir = mnt->mnt_dir;
                if ((mntdir != "/dev/.static/dev") && (mntdir.rfind("gvfs", 4, mntdir.length()) == -1) &&
                    (mntdir.left(4) != "/sys") && (mntdir.left(5) != "/proc"))
                {
                    mtdevices->insert(mnt->mnt_dir, mnt->mnt_type);
                }
            }
            endmntent(mtab);
        }
    }
    if (updevices == NULL)
    {
        // To mark mount points that are up or down
        updevices = new FXStringDict();
        FXString mtstate;
        FILE* mtab = setmntent(MTAB_PATH, "r");

        if (mtab)
        {
            vector_FXString mntlist;
            while ((mnt = getmntent(mtab)))
            {
                // Filter out some file systems
                if (xf_keepmount(mnt->mnt_dir, mnt->mnt_fsname))
                {
                    mntlist.push_back(mnt->mnt_dir);
                }
            }
            endmntent(mtab);

            FXuint timeout = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_timeout", MOUNT_TIMEOUT);

            for (FXuint i = 0; i < mntlist.size(); i++)
            {
                if (xf_statvfs_timeout(mntlist[i].text(), timeout) == 1)
                {
                    mtstate = "down";
                }
                else
                {
                    mtstate = "up";
                }
                updevices->insert(mntlist[i].text(), mtstate.text());
            }
        }
    }
#endif

    // Trashcan location
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;
}


// Get header position from column id
// Return -1 if column id was not found
int FileList::getHeaderIndex(FXuint id)
{
    int index = -1;

    // In a search list
    if (options & FILELIST_SEARCH)
    {
        for (FXuint i = 0; i < nbCols; i++)
        {
            if (idCol[i] == id)
            {
                index = i;
                break;
            }
        }
    }
    // In a normal list
    else
    {
        // In trash
        if (getNumHeaders() == (int)nbColsTrash)
        {
            for (FXuint i = 0; i < nbColsTrash; i++)
            {
                if (idColTrash[i] == id)
                {
                    index = i;
                    break;
                }
            }
        }
        // Not in trash
        else
        {
            for (FXuint i = 0; i < nbCols; i++)
            {
                if (idCol[i] == id)
                {
                    index = i;
                    break;
                }
            }
        }
    }

    return index;
}


#if defined(linux)

// Force mtdevices list refresh
void FileList::refreshMtdevices(void)
{
    struct mntent* mnt;
    FXStringDict* tmpdict = new FXStringDict();
    FILE* mtab = setmntent(MTAB_PATH, "r");

    if (mtab)
    {
        while ((mnt = getmntent(mtab)))
        {
            // Filter out some file systems
            if (xf_keepmount(mnt->mnt_dir, mnt->mnt_fsname))
            {
                tmpdict->insert(mnt->mnt_dir, "");
                if (mtdevices->find(mnt->mnt_dir))
                {
                    mtdevices->remove(mnt->mnt_dir);
                }
                mtdevices->insert(mnt->mnt_dir, mnt->mnt_type);
            }
        }
        endmntent(mtab);
    }

    // Remove mount points that don't exist anymore
    int s;
    for (s = mtdevices->first(); s < mtdevices->size(); s = mtdevices->next(s))
    {
        if (!tmpdict->find(mtdevices->key(s)))
        {
            mtdevices->remove(mtdevices->key(s));
        }
    }

    delete tmpdict;
}

#endif


// Create file list
void FileList::create()
{
    IconList::create();
    if (!deleteType)
    {
        deleteType = getApp()->registerDragType(deleteTypeName);
    }
    if (!urilistType)
    {
        urilistType = getApp()->registerDragType(urilistTypeName);
    }
    getApp()->addTimeout(this, ID_REFRESH_TIMER, REFRESH_INTERVAL);
}


// Cleanup
FileList::~FileList()
{
    getApp()->removeTimeout(this, ID_REFRESH_TIMER);
    getApp()->removeTimeout(this, ID_OPEN_TIMER);
    delete associations;
    delete forwardhist;
    delete backhist;

    associations = (FileDict*)-1;
    list = (FileItem*)-1L;
}


// Open up folder when hovering long over a folder
long FileList::onOpenTimer(FXObject*, FXSelector, void*)
{
    int xx, yy, index;
    FXuint state;

    getCursorPosition(xx, yy, state);
    index = getItemAt(xx, yy);
    if ((0 <= index) && isItemDirectory(index))
    {
        dropdirectory = getItemPathname(index);
        setDirectory(dropdirectory);
        getApp()->addTimeout(this, ID_OPEN_TIMER, OPEN_INTERVAL);
        prevIndex = -1;
    }
    return 1;
}


// Handle drag-and-drop enter
long FileList::onDNDEnter(FXObject* sender, FXSelector sel, void* ptr)
{
    IconList::onDNDEnter(sender, sel, ptr);

    // Keep original directory
    orgdirectory = getDirectory();

    return 1;
}


// Handle drag-and-drop leave
long FileList::onDNDLeave(FXObject* sender, FXSelector sel, void* ptr)
{
    IconList::onDNDLeave(sender, sel, ptr);

    if (prevIndex != -1)
    {
        setItemMiniIcon(prevIndex, minifoldericon);
        setItemBigIcon(prevIndex, bigfoldericon);
        prevIndex = -1;
    }

    // Cancel open up timer
    getApp()->removeTimeout(this, ID_OPEN_TIMER);

    // Stop scrolling
    stopAutoScroll();

    // Restore original directory
    setDirectory(orgdirectory);
    return 1;
}


// Handle drag-and-drop motion
long FileList::onDNDMotion(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    int index = -1;

    // Cancel open up timer
    getApp()->removeTimeout(this, ID_OPEN_TIMER);

    // Start autoscrolling
    if (startAutoScroll(event, false))
    {
        return 1;
    }

    // Give base class a shot
    if (IconList::onDNDMotion(sender, sel, ptr))
    {
        return 1;
    }

    // Dropping list of filenames
    if (offeredDNDType(FROM_DRAGNDROP, urilistType))
    {
        index = getItemAt(event->win_x, event->win_y);
        if ((prevIndex != -1) && (prevIndex != index))
        {
            // Symlink folders have a different icon
            if (isItemLink(prevIndex))
            {
                setItemMiniIcon(prevIndex, minifolderlinkicon);
                setItemBigIcon(prevIndex, bigfolderlinkicon);
                deselectItem(prevIndex);
            }
            else
            {
                setItemMiniIcon(prevIndex, minifoldericon);
                setItemBigIcon(prevIndex, bigfoldericon);
                deselectItem(prevIndex);
            }
            prevIndex = -1;
        }

        // Drop in the background
        dropdirectory = getDirectory();

        // What is being done (move,copy,link)
        dropaction = inquireDNDAction();

        // We will open up a folder if we can hover over it for a while
        if ((0 <= index) && isItemDirectory(index))
        {
            // Set open up timer
            getApp()->addTimeout(this, ID_OPEN_TIMER, OPEN_INTERVAL);
            prevIndex = index;
            setItemMiniIcon(index, minifolderopenicon);
            setItemBigIcon(index, bigfolderopenicon);
            selectItem(index);

            // Directory where to drop, or directory to open up
            dropdirectory = getItemPathname(index);
        }

        // See if dropdirectory is writable
        if (xf_iswritable(dropdirectory))
        {
            acceptDrop(DRAG_ACCEPT);
        }
        return 1;
    }
    return 0;
}


// Set drag type to copy
long FileList::onCmdDragCopy(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_COPY;
    return 1;
}


// Set drag type to move
long FileList::onCmdDragMove(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_MOVE;
    return 1;
}


// Set drag type to symlink
long FileList::onCmdDragLink(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_LINK;
    return 1;
}


// Cancel drag action
long FileList::onCmdDragReject(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_REJECT;
    return 1;
}


// Handle drag-and-drop drop
long FileList::onDNDDrop(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuchar* data;
    FXuint len;
    FXbool showdialog = true;
    File* f = NULL;
    int ret;

    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);
    FXbool confirm_dnd = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_drag_and_drop", true);

    // Cancel open up timer
    getApp()->removeTimeout(this, ID_OPEN_TIMER);

    // Stop scrolling
    stopAutoScroll();

    // Perhaps target wants to deal with it
    if (IconList::onDNDDrop(sender, sel, ptr))
    {
        return 1;
    }

    // Check if control key or shift key were pressed
    FXbool ctrlshiftkey = false;

    if (ptr != NULL)
    {
        FXEvent* event = (FXEvent*)ptr;
        if (event->state & CONTROLMASK)
        {
            ctrlshiftkey = true;
        }
        if (event->state & SHIFTMASK)
        {
            ctrlshiftkey = true;
        }
    }

    // Get DND data
    // This is done before displaying the popup menu to fix a drag and drop problem
    // with Konqueror and Dolphin file managers
    FXbool dnd = getDNDData(FROM_DRAGNDROP, urilistType, data, len);

    int xx, yy, index = -1;

    // Display the dnd dialog if the control or shift key were not pressed
    if (confirm_dnd && !ctrlshiftkey)
    {
        // Get item index
        FXuint state;
        getCursorPosition(xx, yy, state);
        index = getItemAt(xx, yy);

        // Display a popup to select the drag type
        dropaction = DRAG_REJECT;
        FXMenuPane menu(this);
        int x, y;
        getRoot()->getCursorPosition(x, y, state);
        new FXMenuCommand(&menu, _("Copy Here"), minicopyicon, this, FileList::ID_DRAG_COPY);
        new FXMenuCommand(&menu, _("Move Here"), minimoveicon, this, FileList::ID_DRAG_MOVE);
        new FXMenuCommand(&menu, _("Link Here"), minilinktoicon, this, FileList::ID_DRAG_LINK);
        new FXMenuSeparator(&menu);
        new FXMenuCommand(&menu, _("Cancel"), NULL, this, FileList::ID_DRAG_REJECT);

        menu.create();
        menu.popup(NULL, x, y);
        getApp()->runModalWhileShown(&menu);
    }

    if (prevIndex != -1)
    {
        setItemMiniIcon(prevIndex, minifoldericon);
        setItemBigIcon(prevIndex, bigfoldericon);
        deselectItem(prevIndex);
        prevIndex = -1;
    }
    // Restore original directory
    setDirectory(orgdirectory);

    // Get uri-list of files being dropped
    if (dnd) // See comment upper
    {
        FXRESIZE(&data, FXuchar, len + 1);
        data[len] = '\0';
        char* p, * q;
        p = q = (char*)data;

        // Number of selected items
        FXString buf = p;
        int num = buf.contains('\n') + 1;

        // Possibly correct the number of selected items
        // because sometimes there is another '\n' at the end of the string
        int pos = buf.rfind('\n');
        if (pos == buf.length() - 1)
        {
            num = num - 1;
        }

        // File object
        if (dropaction == DRAG_COPY)
        {
            f = new File(this, _("Copy Files"), COPY, num);
            f->create();
        }
        else if (dropaction == DRAG_MOVE)
        {
            f = new File(this, _("Move Files"), MOVE, num);
            f->create();
        }
        else if (dropaction == DRAG_LINK)
        {
            f = new File(this, _("Symlink Files"), SYMLINK, num);
            f->create();
        }
        else
        {
            // Deselect item
            if (index >= 0)
            {
                deselectItem(index);
            }
            FXFREE(&data);
            return 0;
        }

        // Target directory
        FXString targetdir = dropdirectory;

        // Get total source size
        FXString hsourcesize;
        FXulong sourcesize = 0, tstart = 0;

        if (dropaction == DRAG_COPY || dropaction == DRAG_MOVE)
        {
            hsourcesize = f->sourcesize(buf, &sourcesize, true);
            tstart = xf_getcurrenttime();

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                if (dropaction == DRAG_COPY)
                {
                    MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                }
                else
                {
                    MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                }
                goto out;
            }
        }

        // Process sources
        while (*p)
        {
            while (*q && *q != '\r')
            {
                q++;
            }
            FXString url(p, q - p);
            FXString source(FXURL::decode(FXURL::fileFromURL(url)));
            FXString target(targetdir);
            FXString sourcedir = FXPath::directory(source);

            // File operation dialog, if needed
            if (((!confirm_dnd) | ctrlshiftkey) & ask_before_copy & showdialog)
            {
                f->hideProgressDialog();
                
                FXIcon* icon = NULL;
                FXString title, message;
                if (dropaction == DRAG_COPY)
                {
                    title = _("Copy");
                    icon = bigcopyicon;
                    if (num == 1)
                    {
                        message = title + source;
                    }
                    else
                    {
                        message.format(_("Copy %s files/folders from: %s"), FXStringVal(num).text(),
                                       sourcedir.text());
                    }
                }
                else if (dropaction == DRAG_MOVE)
                {
                    title = _("Move");
                    icon = bigmoveicon;
                    if (num == 1)
                    {
                        message = title + source;
                    }
                    else
                    {
                        message.format(_("Move %s files/folders from: %s"), FXStringVal(num).text(),
                                       sourcedir.text());
                    }
                }
                else if ((dropaction == DRAG_LINK) && (num == 1))
                {
                    title = _("Symlink");
                    icon = biglinktoicon;
                    message = title + source;
                }

                InputDialog* dialog = new InputDialog(this, targetdir, message, title, _("To:"), icon);
                dialog->CursorEnd();
                int rc = dialog->execute();
                target = dialog->getText();
                target = xf_filepath(target);

                if (num > 1)
                {
                    showdialog = false;
                }
                delete dialog;
                if (!rc)
                {
                    delete f;
                    return 0;
                }
            }

            f->showProgressDialog();
            getApp()->beginWaitCursor();

            // Move the source file
            if (dropaction == DRAG_MOVE)
            {

                // If target file is located at trash location, also create the corresponding trashinfo file
                // Do it silently and don't report any error if it fails
                FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                if (use_trash_can && (FXPath::directory(target) == trashfileslocation))
                {
                    // Trash files path name
                    FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                    // Adjust target name to get the _N suffix if any
                    FXString trashtarget = FXPath::directory(target) + PATHSEPSTRING + FXPath::name(trashpathname);

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

                // If source file is located at trash location, try to also remove the corresponding
                // trashinfo file if it exists
                // Do it silently and don't report any error if it fails
                if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
                {
                    FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) +
                                                 ".trashinfo";
                    unlink(trashinfopathname.text());
                }

                // An error has occurred
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
            // Copy the source file
            else if (dropaction == DRAG_COPY)
            {
                // If target file is located at trash location, also create the corresponding trashinfo file
                // Do it silently and don't report any error if it fails
                FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                if (use_trash_can && (FXPath::directory(target) == trashfileslocation))
                {
                    // Trash files path name
                    FXString trashpathname = xf_create_trashpathname(source, trashfileslocation);

                    // Adjust target name to get the _N suffix if any
                    FXString trashtarget = FXPath::directory(target) + PATHSEPSTRING + FXPath::name(trashpathname);

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

                // An error has occurred
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
            // Link the source file (no progress dialog in this case)
            else if (dropaction == DRAG_LINK)
            {
                // Link file
                f->symlink(source, target);
            }
            if (*q == '\r')
            {
                q += 2;
            }
            p = q;

            getApp()->endWaitCursor();
        }
out:
        delete f;
        FXFREE(&data);
        return 1;
    }

    return 0;
}


// Somebody wants our dragged data
long FileList::onDNDRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuchar* data;
    FXuint len;

    // Perhaps the target wants to supply its own data
    if (IconList::onDNDRequest(sender, sel, ptr))
    {
        return 1;
    }

    // Return list of filenames as a uri-list
    if (event->target == urilistType)
    {
        if (!dragfiles.empty())
        {
            len = dragfiles.length();
            FXMEMDUP(&data, dragfiles.text(), FXuchar, len);
            setDNDData(FROM_DRAGNDROP, event->target, data, len);
        }
        return 1;
    }

    // Delete selected files
    if (event->target == deleteType)
    {
        return 1;
    }

    return 0;
}


// Start a drag operation
long FileList::onBeginDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    int i;

    if (IconList::onBeginDrag(sender, sel, ptr))
    {
        return 1;
    }
    if (beginDrag(&urilistType, 1))
    {
        dragfiles = FXString::null;
        for (i = 0; i < getNumItems(); i++)
        {
            if (isItemSelected(i))
            {
                if (getItemFilename(i) == "..")
                {
                    deselectItem(i);
                }
                else
                {
                    dragfiles += FXURL::encode(::xf_filetouri(getItemFullPathname(i)));
                    dragfiles += "\r\n";
                }
            }
        }
        return 1;
    }
    return 0;
}


// End drag operation
long FileList::onEndDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    if (IconList::onEndDrag(sender, sel, ptr))
    {
        return 1;
    }
    endDrag((didAccept() != DRAG_REJECT));
    setDragCursor(getDefaultCursor());
    dragfiles = FXString::null;
    return 1;
}


// Dragged stuff around
long FileList::onDragged(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXDragAction action;

    if (IconList::onDragged(sender, sel, ptr))
    {
        return 1;
    }
    action = DRAG_MOVE;

    if (event->state & CONTROLMASK)
    {
        action = DRAG_COPY;
    }
    if (event->state & SHIFTMASK)
    {
        action = DRAG_MOVE;
    }
    if ((event->state & CONTROLMASK) && (event->state & SHIFTMASK))
    {
        action = DRAG_LINK;
    }

    // If source directory is read-only, convert move action to copy
    if (!xf_iswritable(orgdirectory) && (action == DRAG_MOVE))
    {
        action = DRAG_COPY;
    }

    handleDrag(event->root_x, event->root_y, action);
    if (didAccept() != DRAG_REJECT)
    {
        if (action == DRAG_MOVE)
        {
            setDragCursor(getApp()->getDefaultCursor(DEF_DNDMOVE_CURSOR));
        }
        else if (action == DRAG_LINK)
        {
            setDragCursor(getApp()->getDefaultCursor(DEF_DNDLINK_CURSOR));
        }
        else
        {
            setDragCursor(getApp()->getDefaultCursor(DEF_DNDCOPY_CURSOR));
        }
    }
    else
    {
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDSTOP_CURSOR));
    }
    return 1;
}


// Toggle hidden files display
long FileList::onCmdToggleHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(!shownHiddenFiles());
    return 1;
}


// Close filter
long FileList::onCmdCloseFilter(FXObject*, FXSelector, void*)
{
    setPattern("*");
    onCmdRefresh(0, 0, 0);

    return 1;
}


// Toggle thumbnails display
long FileList::onCmdToggleThumbnails(FXObject*, FXSelector, void*)
{
    showThumbnails(!displaythumbnails);
    return 1;
}


// Update toggle thumbnails button
long FileList::onUpdToggleThumbnails(FXObject* sender, FXSelector, void*)
{
    if (shownThumbnails())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Update toggle hidden files button
long FileList::onUpdToggleHidden(FXObject* sender, FXSelector, void*)
{
    if (shownHiddenFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Show hidden files
long FileList::onCmdShowHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(true);
    return 1;
}


// Update show hidden files widget
long FileList::onUpdShowHidden(FXObject* sender, FXSelector, void*)
{
    if (shownHiddenFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Hide hidden files
long FileList::onCmdHideHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(false);
    return 1;
}


// Update hide hidden files widget
long FileList::onUpdHideHidden(FXObject* sender, FXSelector, void*)
{
    if (!shownHiddenFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Move up one level
long FileList::onCmdDirectoryUp(FXObject*, FXSelector, void*)
{
    setFocus();
    setDirectory(FXPath::upLevel(directory), true, directory);
    return 1;
}


// Determine if we can still go up more
long FileList::onUpdDirectoryUp(FXObject* sender, FXSelector, void* ptr)
{
    FXuint msg = FXPath::isTopDirectory(directory) ? ID_DISABLE : ID_ENABLE;

    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);
    return 1;
}


// Change pattern
long FileList::onCmdSetPattern(FXObject*, FXSelector, void* ptr)
{
    if (!ptr)
    {
        return 0;
    }
    setPattern((const char*)ptr);
    return 1;
}


// Update pattern
long FileList::onUpdSetPattern(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SETVALUE), (void*)pattern.text());
    return 1;
}


// Sort by name
long FileList::onCmdSortByName(FXObject*, FXSelector, void*)
{
    name_index = getHeaderIndex(ID_COL_NAME);

    if (dirsfirst == false)
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingCaseMix) ? descendingCaseMix : ascendingCaseMix;
        }
        else
        {
            sortfunc = (sortfunc == ascendingMix) ? descendingMix : ascendingMix;
        }
    }
    else
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingCase) ? descendingCase : ascendingCase;
        }
        else
        {
            sortfunc = (sortfunc == ascending) ? descending : ascending;
        }
    }
    setSortHeader(name_index);
    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByName(FXObject* sender, FXSelector, void*)
{
    sender->handle(this,
                   (sortfunc == ascending || sortfunc == descending || sortfunc == ascendingCase ||
                    sortfunc == descendingCase || sortfunc == ascendingMix || sortfunc == descendingMix ||
                    sortfunc == ascendingCaseMix || sortfunc == descendingCaseMix) ?
                    FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    return 1;
}


// Sort by directory name (for search list)
long FileList::onCmdSortByDirName(FXObject*, FXSelector, void*)
{
    dirname_index = getHeaderIndex(ID_COL_DIRNAME);

    if (dirsfirst == false)
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingDirCaseMix) ? descendingDirCaseMix : ascendingDirCaseMix;
        }
        else
        {
            sortfunc = (sortfunc == ascendingDirMix) ? descendingDirMix : ascendingDirMix;
        }
    }
    else
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingDirCase) ? descendingDirCase : ascendingDirCase;
        }
        else
        {
            sortfunc = (sortfunc == ascendingDir) ? descendingDir : ascendingDir;
        }
    }

    setSortHeader(dirname_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByDirName(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingDir || sortfunc == descendingDir || sortfunc == ascendingDirCase ||
                          sortfunc == descendingDirCase || sortfunc == ascendingDirMix ||
                          sortfunc == descendingDirMix || sortfunc == ascendingDirCaseMix ||
                          sortfunc == descendingDirCaseMix) ?
                          FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    return 1;
}


// Sort by type
long FileList::onCmdSortByType(FXObject*, FXSelector, void*)
{
    type_index = getHeaderIndex(ID_COL_TYPE);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingTypeMix) ? descendingTypeMix : ascendingTypeMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingType) ? descendingType : ascendingType;
    }

    setSortHeader(type_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByType(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingType || sortfunc == descendingType || sortfunc == ascendingTypeMix ||
                          sortfunc == descendingTypeMix) ?
                          FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by size
long FileList::onCmdSortBySize(FXObject*, FXSelector, void*)
{
    size_index = getHeaderIndex(ID_COL_SIZE);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingSizeMix) ? descendingSizeMix : ascendingSizeMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingSize) ? descendingSize : ascendingSize;
    }

    setSortHeader(size_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortBySize(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingSize || sortfunc == descendingSize || sortfunc == ascendingSizeMix ||
                          sortfunc == descendingSizeMix) ?
                          FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by ext
long FileList::onCmdSortByExt(FXObject*, FXSelector, void*)
{
    ext_index = getHeaderIndex(ID_COL_EXT);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingExtMix) ? descendingExtMix : ascendingExtMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingExt) ? descendingExt : ascendingExt;
    }

    setSortHeader(ext_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByExt(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingExt || sortfunc == descendingExt || sortfunc == ascendingExtMix ||
                          sortfunc == descendingExtMix) ?
                          FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by original path
long FileList::onCmdSortByOrigpath(FXObject*, FXSelector, void*)
{
    origpath_index = getHeaderIndex(ID_COL_ORIGPATH);

    if (dirsfirst == false)
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingOrigpathCaseMix) ? descendingOrigpathCaseMix : ascendingOrigpathCaseMix;
        }
        else
        {
            sortfunc = (sortfunc == ascendingOrigpathMix) ? descendingOrigpathMix : ascendingOrigpathMix;
        }
    }
    else
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingOrigpathCase) ? descendingOrigpathCase : ascendingOrigpathCase;
        }
        else
        {
            sortfunc = (sortfunc == ascendingOrigpath) ? descendingOrigpath : ascendingOrigpath;
        }
    }

    setSortHeader(origpath_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByOrigpath(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingOrigpath || sortfunc == descendingOrigpath ||
                   sortfunc == ascendingOrigpathCase || sortfunc == descendingOrigpathCase ||
                   sortfunc == ascendingOrigpathMix || sortfunc == descendingOrigpathMix ||
                   sortfunc == ascendingOrigpathCaseMix || sortfunc == descendingOrigpathCaseMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    if (!(options & FILELIST_SEARCH) && (getNumHeaders() == (int)nbColsTrash))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by deletion date
long FileList::onCmdSortByDeldate(FXObject*, FXSelector, void*)
{
    deldate_index = getHeaderIndex(ID_COL_DELDATE);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingDeldateMix) ? descendingDeldateMix : ascendingDeldateMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingDeldate) ? descendingDeldate : ascendingDeldate;
    }

    setSortHeader(deldate_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByDeldate(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingDeldate || sortfunc == descendingDeldate ||
                   sortfunc == ascendingDeldateMix || sortfunc == descendingDeldateMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    if (!(options & FILELIST_SEARCH) && (getNumHeaders() == (int)nbColsTrash))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by date
long FileList::onCmdSortByDate(FXObject*, FXSelector, void*)
{
    date_index = getHeaderIndex(ID_COL_DATE);
    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingDateMix) ? descendingDateMix : ascendingDateMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingDate) ? descendingDate : ascendingDate;
    }

    setSortHeader(date_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByDate(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingDate || sortfunc == descendingDate ||
                   sortfunc == ascendingDateMix || sortfunc == descendingDateMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by user
long FileList::onCmdSortByUser(FXObject*, FXSelector, void*)
{
    user_index = getHeaderIndex(ID_COL_USER);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingUserMix) ? descendingUserMix : ascendingUserMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingUser) ? descendingUser : ascendingUser;
    }

    setSortHeader(user_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByUser(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingUser || sortfunc == descendingUser ||
                   sortfunc == ascendingUserMix || sortfunc == descendingUserMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by group
long FileList::onCmdSortByGroup(FXObject*, FXSelector, void*)
{
    group_index = getHeaderIndex(ID_COL_GROUP);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingGroupMix) ? descendingGroupMix : ascendingGroupMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingGroup) ? descendingGroup : ascendingGroup;
    }

    setSortHeader(group_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByGroup(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingGroup || sortfunc == descendingGroup ||
                   sortfunc == ascendingGroupMix || sortfunc == descendingGroupMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Sort by permissions
long FileList::onCmdSortByPerms(FXObject*, FXSelector, void*)
{
    perms_index = getHeaderIndex(ID_COL_PERMS);

    if (dirsfirst == false)
    {
        sortfunc = (sortfunc == ascendingPermsMix) ? descendingPermsMix : ascendingPermsMix;
    }
    else
    {
        sortfunc = (sortfunc == ascendingPerms) ? descendingPerms : ascendingPerms;
    }

    setSortHeader(perms_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByPerms(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingPerms || sortfunc == descendingPerms ||
                   sortfunc == ascendingPermsMix || sortfunc == descendingPermsMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Sort by link
long FileList::onCmdSortByLink(FXObject*, FXSelector, void*)
{
    link_index = getHeaderIndex(ID_COL_LINK);

    if (dirsfirst == false)
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingLinkCaseMix) ? descendingLinkCaseMix : ascendingLinkCaseMix;
        }
        else
        {
            sortfunc = (sortfunc == ascendingLinkMix) ? descendingLinkMix : ascendingLinkMix;
        }
    }
    else
    {
        if (getIgnoreCase() == true)
        {
            sortfunc = (sortfunc == ascendingLinkCase) ? descendingLinkCase : ascendingLinkCase;
        }
        else
        {
            sortfunc = (sortfunc == ascendingLink) ? descendingLink : ascendingLink;
        }
    }

    setSortHeader(link_index);

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortByLink(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, (sortfunc == ascendingLink || sortfunc == descendingLink ||
                   sortfunc == ascendingLinkCase || sortfunc == descendingLinkCase ||
                   sortfunc == ascendingLinkMix || sortfunc == descendingLinkMix ||
                   sortfunc == ascendingLinkCaseMix || sortfunc == descendingLinkCaseMix) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);

    FXbool isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);

    if (isShown)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Reverse sort order
long FileList::onCmdSortReverse(FXObject*, FXSelector, void*)
{
    if (sortfunc == ascending)
    {
        sortfunc = descending;
    }
    else if (sortfunc == ascendingMix)
    {
        sortfunc = descendingMix;
    }
    else if (sortfunc == descending)
    {
        sortfunc = ascending;
    }
    else if (sortfunc == descendingMix)
    {
        sortfunc = ascendingMix;
    }
    else if (sortfunc == ascendingCase)
    {
        sortfunc = descendingCase;
    }
    else if (sortfunc == ascendingCaseMix)
    {
        sortfunc = descendingCaseMix;
    }
    else if (sortfunc == descendingCase)
    {
        sortfunc = ascendingCase;
    }
    else if (sortfunc == descendingCaseMix)
    {
        sortfunc = ascendingCaseMix;
    }
    else if (sortfunc == ascendingType)
    {
        sortfunc = descendingType;
    }
    else if (sortfunc == ascendingTypeMix)
    {
        sortfunc = descendingTypeMix;
    }
    else if (sortfunc == descendingType)
    {
        sortfunc = ascendingType;
    }
    else if (sortfunc == descendingTypeMix)
    {
        sortfunc = ascendingTypeMix;
    }
    else if (sortfunc == ascendingExt)
    {
        sortfunc = descendingExt;
    }
    else if (sortfunc == ascendingExtMix)
    {
        sortfunc = descendingExtMix;
    }
    else if (sortfunc == descendingExt)
    {
        sortfunc = ascendingExt;
    }
    else if (sortfunc == descendingExtMix)
    {
        sortfunc = ascendingExtMix;
    }
    else if (sortfunc == ascendingSize)
    {
        sortfunc = descendingSize;
    }
    else if (sortfunc == ascendingSizeMix)
    {
        sortfunc = descendingSizeMix;
    }
    else if (sortfunc == descendingSize)
    {
        sortfunc = ascendingSize;
    }
    else if (sortfunc == descendingSizeMix)
    {
        sortfunc = ascendingSizeMix;
    }
    else if (sortfunc == ascendingDate)
    {
        sortfunc = descendingDate;
    }
    else if (sortfunc == ascendingDateMix)
    {
        sortfunc = descendingDateMix;
    }
    else if (sortfunc == descendingDate)
    {
        sortfunc = ascendingDate;
    }
    else if (sortfunc == descendingDateMix)
    {
        sortfunc = ascendingDateMix;
    }
    else if (sortfunc == ascendingUser)
    {
        sortfunc = descendingUser;
    }
    else if (sortfunc == ascendingUserMix)
    {
        sortfunc = descendingUserMix;
    }
    else if (sortfunc == descendingUser)
    {
        sortfunc = ascendingUser;
    }
    else if (sortfunc == descendingUserMix)
    {
        sortfunc = ascendingUserMix;
    }
    else if (sortfunc == ascendingGroup)
    {
        sortfunc = descendingGroup;
    }
    else if (sortfunc == ascendingGroupMix)
    {
        sortfunc = descendingGroupMix;
    }
    else if (sortfunc == descendingGroup)
    {
        sortfunc = ascendingGroup;
    }
    else if (sortfunc == descendingGroupMix)
    {
        sortfunc = ascendingGroupMix;
    }
    else if (sortfunc == ascendingPerms)
    {
        sortfunc = descendingPerms;
    }
    else if (sortfunc == ascendingPermsMix)
    {
        sortfunc = descendingPermsMix;
    }
    else if (sortfunc == descendingPerms)
    {
        sortfunc = ascendingPerms;
    }
    else if (sortfunc == descendingPermsMix)
    {
        sortfunc = ascendingPermsMix;
    }
    else if (sortfunc == ascendingLink)
    {
        sortfunc = descendingLink;
    }
    else if (sortfunc == ascendingLinkMix)
    {
        sortfunc = descendingLinkMix;
    }
    else if (sortfunc == descendingLink)
    {
        sortfunc = ascendingLink;
    }
    else if (sortfunc == descendingLinkMix)
    {
        sortfunc = ascendingLinkMix;
    }
    else if (sortfunc == ascendingLinkCase)
    {
        sortfunc = descendingLinkCase;
    }
    else if (sortfunc == ascendingLinkCaseMix)
    {
        sortfunc = descendingLinkCaseMix;
    }
    else if (sortfunc == descendingLinkCase)
    {
        sortfunc = ascendingLinkCase;
    }
    else if (sortfunc == descendingLinkCaseMix)
    {
        sortfunc = ascendingLinkCaseMix;
    }
    else if (sortfunc == ascendingDir)
    {
        sortfunc = descendingDir;
    }
    else if (sortfunc == ascendingDirMix)
    {
        sortfunc = descendingDirMix;
    }
    else if (sortfunc == descendingDir)
    {
        sortfunc = ascendingDir;
    }
    else if (sortfunc == descendingDirMix)
    {
        sortfunc = ascendingDirMix;
    }
    else if (sortfunc == ascendingDirCase)
    {
        sortfunc = descendingDirCase;
    }
    else if (sortfunc == ascendingDirCaseMix)
    {
        sortfunc = descendingDirCaseMix;
    }
    else if (sortfunc == descendingDirCase)
    {
        sortfunc = ascendingDirCase;
    }
    else if (sortfunc == descendingDirCaseMix)
    {
        sortfunc = ascendingDirCaseMix;
    }
    else if (sortfunc == ascendingOrigpathCase)
    {
        sortfunc = descendingOrigpathCase;
    }
    else if (sortfunc == ascendingOrigpathCaseMix)
    {
        sortfunc = descendingOrigpathCaseMix;
    }
    else if (sortfunc == descendingOrigpathCase)
    {
        sortfunc = ascendingOrigpathCase;
    }
    else if (sortfunc == descendingOrigpathCaseMix)
    {
        sortfunc = ascendingOrigpathCaseMix;
    }
    else if (sortfunc == ascendingDeldate)
    {
        sortfunc = descendingDeldate;
    }
    else if (sortfunc == ascendingDeldateMix)
    {
        sortfunc = descendingDeldateMix;
    }
    else if (sortfunc == descendingDeldate)
    {
        sortfunc = ascendingDeldate;
    }
    else if (sortfunc == descendingDeldateMix)
    {
        sortfunc = ascendingDeldateMix;
    }

    scan(true);
    return 1;
}


// Update sender
long FileList::onUpdSortReverse(FXObject* sender, FXSelector, void*)
{
    FXSelector selector = FXSEL(SEL_COMMAND, ID_UNCHECK);

    if (sortfunc == descending)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingCase)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingCaseMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingType)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingTypeMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingExt)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingExtMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingSize)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingSizeMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDate)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDateMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingUser)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingUserMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingGroup)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingGroupMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingPerms)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingPermsMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingLink)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingLinkMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingLinkCase)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingLinkCaseMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDir)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDirMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDirCase)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDirCaseMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingOrigpath)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingOrigpathMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingOrigpathCase)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingOrigpathCaseMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDeldate)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }
    else if (sortfunc == descendingDeldateMix)
    {
        selector = FXSEL(SEL_COMMAND, ID_CHECK);
    }

    sender->handle(this, selector, NULL);

    return 1;
}


// Toggle case sensitivity
long FileList::onCmdSortCase(FXObject*, FXSelector, void*)
{
    setIgnoreCase(!getIgnoreCase());
    if (dirsfirst == false)
    {
        if (getIgnoreCase() == true)
        {
            if ((sortfunc == ascending) || (sortfunc == ascendingMix) ||
                (sortfunc == ascendingCase))
            {
                sortfunc = ascendingCaseMix;
            }
            else if ((sortfunc == descending) || (sortfunc == descendingMix) ||
                     (sortfunc == descendingCase))
            {
                sortfunc = descendingCaseMix;
            }

            else if ((sortfunc == ascendingDir) || (sortfunc == ascendingDirMix) ||
                     (sortfunc == ascendingDirCase))
            {
                sortfunc = ascendingDirCaseMix;
            }
            else if ((sortfunc == descendingDir) || (sortfunc == descendingDirMix) ||
                     (sortfunc == descendingDirCase))
            {
                sortfunc = descendingDirCaseMix;
            }
            else if ((sortfunc == ascendingLink) || (sortfunc == ascendingLinkMix) ||
                     (sortfunc == ascendingLinkCase))
            {
                sortfunc = ascendingLinkCaseMix;
            }
            else if ((sortfunc == descendingLink) || (sortfunc == descendingLinkMix) ||
                     (sortfunc == descendingLinkCase))
            {
                sortfunc = descendingLinkCaseMix;
            }
            else if ((sortfunc == ascendingOrigpath) || (sortfunc == ascendingOrigpathMix) ||
                     (sortfunc == ascendingOrigpathCase))
            {
                sortfunc = ascendingOrigpathCaseMix;
            }
            else if ((sortfunc == descendingOrigpath) || (sortfunc == descendingOrigpathMix) ||
                     (sortfunc == descendingOrigpathCase))
            {
                sortfunc = descendingOrigpathCaseMix;
            }
        }
        else
        {
            if ((sortfunc == ascending) || (sortfunc == ascendingCase) ||
                (sortfunc == ascendingCaseMix))
            {
                sortfunc = ascendingMix;
            }
            else if ((sortfunc == descending) || (sortfunc == descendingCase) ||
                     (sortfunc == descendingCaseMix))
            {
                sortfunc = descendingMix;
            }
            else if ((sortfunc == ascendingDir) || (sortfunc == ascendingDirCase) ||
                     (sortfunc == ascendingDirCaseMix))
            {
                sortfunc = ascendingDirMix;
            }
            else if ((sortfunc == descendingDir) || (sortfunc == descendingDirCase) ||
                     (sortfunc == descendingDirCaseMix))
            {
                sortfunc = descendingDirMix;
            }
            else if ((sortfunc == ascendingLink) || (sortfunc == ascendingLinkCase) ||
                     (sortfunc == ascendingLinkCaseMix))
            {
                sortfunc = ascendingLinkMix;
            }
            else if ((sortfunc == descendingLink) || (sortfunc == descendingLinkCase) ||
                     (sortfunc == descendingLinkCaseMix))
            {
                sortfunc = descendingLinkMix;
            }
            else if ((sortfunc == ascendingOrigpath) || (sortfunc == ascendingOrigpathCase) ||
                     (sortfunc == ascendingOrigpathCaseMix))
            {
                sortfunc = ascendingOrigpathMix;
            }
            else if ((sortfunc == descendingOrigpath) || (sortfunc == descendingOrigpathCase) ||
                     (sortfunc == descendingOrigpathCaseMix))
            {
                sortfunc = descendingOrigpathMix;
            }
        }
    }
    else
    {
        if (getIgnoreCase() == true)
        {
            if ((sortfunc == ascending) || (sortfunc == ascendingMix) ||
                (sortfunc == ascendingCaseMix))
            {
                sortfunc = ascendingCase;
            }
            else if ((sortfunc == descending) || (sortfunc == descendingMix) ||
                     (sortfunc == descendingCaseMix))
            {
                sortfunc = descendingCase;
            }

            else if ((sortfunc == ascendingDir) || (sortfunc == ascendingDirMix) ||
                     (sortfunc == ascendingDirCaseMix))
            {
                sortfunc = ascendingDirCase;
            }
            else if ((sortfunc == descendingDir) || (sortfunc == descendingDirMix) ||
                     (sortfunc == descendingDirCaseMix))
            {
                sortfunc = descendingDirCase;
            }
            else if ((sortfunc == ascendingLink) || (sortfunc == ascendingLinkMix) ||
                     (sortfunc == ascendingLinkCaseMix))
            {
                sortfunc = ascendingLinkCase;
            }
            else if ((sortfunc == descendingLink) || (sortfunc == descendingLinkMix) ||
                     (sortfunc == descendingLinkCaseMix))
            {
                sortfunc = descendingLinkCase;
            }
            else if ((sortfunc == ascendingOrigpath) || (sortfunc == ascendingOrigpathMix) ||
                     (sortfunc == ascendingOrigpathCaseMix))
            {
                sortfunc = ascendingOrigpathCase;
            }
            else if ((sortfunc == descendingOrigpath) || (sortfunc == descendingOrigpathMix) ||
                     (sortfunc == descendingOrigpathCaseMix))
            {
                sortfunc = descendingOrigpathCase;
            }
        }
        else
        {
            if ((sortfunc == ascendingMix) || (sortfunc == ascendingCase) ||
                (sortfunc == ascendingCaseMix))
            {
                sortfunc = ascending;
            }
            else if ((sortfunc == descendingMix) || (sortfunc == descendingCase) ||
                     (sortfunc == descendingCaseMix))
            {
                sortfunc = descending;
            }
            else if ((sortfunc == ascendingDirMix) || (sortfunc == ascendingDirCase) ||
                     (sortfunc == ascendingDirCaseMix))
            {
                sortfunc = ascendingDir;
            }
            else if ((sortfunc == descendingDirMix) || (sortfunc == descendingDirCase) ||
                     (sortfunc == descendingDirCaseMix))
            {
                sortfunc = descendingDir;
            }
            else if ((sortfunc == ascendingLinkMix) || (sortfunc == ascendingLinkCase) ||
                     (sortfunc == ascendingLinkCaseMix))
            {
                sortfunc = ascendingLink;
            }
            else if ((sortfunc == descendingLinkMix) || (sortfunc == descendingLinkCase) ||
                     (sortfunc == descendingLinkCaseMix))
            {
                sortfunc = descendingLink;
            }
            else if ((sortfunc == ascendingOrigpathMix) || (sortfunc == ascendingOrigpathCase) ||
                     (sortfunc == ascendingOrigpathCaseMix))
            {
                sortfunc = ascendingOrigpath;
            }
            else if ((sortfunc == descendingOrigpathMix) || (sortfunc == descendingOrigpathCase) ||
                     (sortfunc == descendingOrigpathCaseMix))
            {
                sortfunc = descendingOrigpath;
            }
        }
    }
    scan(true);
    return 1;
}


// Update case sensitivity
long FileList::onUpdSortCase(FXObject* sender, FXSelector, void* ptr)
{
    if ((sortfunc == ascending) || (sortfunc == descending) || (sortfunc == ascendingMix) ||
        (sortfunc == descendingMix) || (sortfunc == ascendingCase) || (sortfunc == descendingCase) ||
        (sortfunc == ascendingCaseMix) || (sortfunc == descendingCaseMix))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

        if (getIgnoreCase() == true)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
        }
    }
    else if ((sortfunc == ascendingDir) || (sortfunc == descendingDir) || (sortfunc == ascendingDirMix) ||
             (sortfunc == descendingDirMix) || (sortfunc == ascendingDirCase) || (sortfunc == descendingDirCase) ||
             (sortfunc == ascendingDirCaseMix) || (sortfunc == descendingDirCaseMix))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

        if (getIgnoreCase() == true)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
        }
    }
    else if ((sortfunc == ascendingLink) || (sortfunc == descendingLink) || (sortfunc == ascendingLinkMix) ||
             (sortfunc == descendingLinkMix) || (sortfunc == ascendingLinkCase) || (sortfunc == descendingLinkCase) ||
             (sortfunc == ascendingLinkCaseMix) || (sortfunc == descendingLinkCaseMix))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

        if (getIgnoreCase() == true)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
        }
    }
    else if ((sortfunc == ascendingOrigpath) || (sortfunc == descendingOrigpath) ||
             (sortfunc == ascendingOrigpathMix) || (sortfunc == descendingOrigpathMix) ||
             (sortfunc == ascendingOrigpathCase) || (sortfunc == descendingOrigpathCase) ||
             (sortfunc == ascendingOrigpathCaseMix) || (sortfunc == descendingOrigpathCaseMix))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

        if (getIgnoreCase() == true)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), ptr);
    }

    return 1;
}


// Toggle directories first
long FileList::onCmdDirsFirst(FXObject*, FXSelector, void*)
{
    dirsfirst = !dirsfirst;
    if (dirsfirst == false)
    {
        if (sortfunc == ascending)
        {
            sortfunc = ascendingMix;
        }
        else if (sortfunc == descending)
        {
            sortfunc = descendingMix;
        }
        else if (sortfunc == ascendingCase)
        {
            sortfunc = ascendingCaseMix;
        }
        else if (sortfunc == descendingCase)
        {
            sortfunc = descendingCaseMix;
        }
        else if (sortfunc == ascendingType)
        {
            sortfunc = ascendingTypeMix;
        }
        else if (sortfunc == descendingType)
        {
            sortfunc = descendingTypeMix;
        }
        else if (sortfunc == ascendingExt)
        {
            sortfunc = ascendingExtMix;
        }
        else if (sortfunc == descendingExt)
        {
            sortfunc = descendingExtMix;
        }
        else if (sortfunc == ascendingSize)
        {
            sortfunc = ascendingSizeMix;
        }
        else if (sortfunc == descendingSize)
        {
            sortfunc = descendingSizeMix;
        }
        else if (sortfunc == ascendingDate)
        {
            sortfunc = ascendingDateMix;
        }
        else if (sortfunc == descendingDate)
        {
            sortfunc = descendingDateMix;
        }
        else if (sortfunc == ascendingUser)
        {
            sortfunc = ascendingUserMix;
        }
        else if (sortfunc == descendingUser)
        {
            sortfunc = descendingUserMix;
        }
        else if (sortfunc == ascendingGroup)
        {
            sortfunc = ascendingGroupMix;
        }
        else if (sortfunc == descendingGroup)
        {
            sortfunc = descendingGroupMix;
        }
        else if (sortfunc == ascendingPerms)
        {
            sortfunc = ascendingPermsMix;
        }
        else if (sortfunc == descendingPerms)
        {
            sortfunc = descendingPermsMix;
        }
        else if (sortfunc == ascendingLink)
        {
            sortfunc = ascendingLinkMix;
        }
        else if (sortfunc == descendingLink)
        {
            sortfunc = descendingLinkMix;
        }
        else if (sortfunc == ascendingLinkCase)
        {
            sortfunc = ascendingLinkCaseMix;
        }
        else if (sortfunc == descendingLinkCase)
        {
            sortfunc = descendingLinkCaseMix;
        }
        else if (sortfunc == ascendingDir)
        {
            sortfunc = ascendingDirMix;
        }
        else if (sortfunc == descendingDir)
        {
            sortfunc = descendingDirMix;
        }
        else if (sortfunc == ascendingDirCase)
        {
            sortfunc = ascendingDirCaseMix;
        }
        else if (sortfunc == descendingDirCase)
        {
            sortfunc = descendingDirCaseMix;
        }
        else if (sortfunc == ascendingOrigpath)
        {
            sortfunc = ascendingOrigpathMix;
        }
        else if (sortfunc == descendingOrigpath)
        {
            sortfunc = descendingOrigpathMix;
        }
        else if (sortfunc == ascendingOrigpathCase)
        {
            sortfunc = ascendingOrigpathCaseMix;
        }
        else if (sortfunc == descendingOrigpathCase)
        {
            sortfunc = descendingOrigpathCaseMix;
        }
        else if (sortfunc == ascendingDeldate)
        {
            sortfunc = ascendingDeldateMix;
        }
        else if (sortfunc == descendingDeldate)
        {
            sortfunc = descendingDeldateMix;
        }
    }
    else
    {
        if (sortfunc == ascendingMix)
        {
            sortfunc = ascending;
        }
        else if (sortfunc == descendingMix)
        {
            sortfunc = descending;
        }
        else if (sortfunc == ascendingCaseMix)
        {
            sortfunc = ascendingCase;
        }
        else if (sortfunc == descendingCaseMix)
        {
            sortfunc = descendingCase;
        }
        else if (sortfunc == ascendingTypeMix)
        {
            sortfunc = ascendingType;
        }
        else if (sortfunc == descendingTypeMix)
        {
            sortfunc = descendingType;
        }
        else if (sortfunc == ascendingExtMix)
        {
            sortfunc = ascendingExt;
        }
        else if (sortfunc == descendingExtMix)
        {
            sortfunc = descendingExt;
        }
        else if (sortfunc == ascendingSizeMix)
        {
            sortfunc = ascendingSize;
        }
        else if (sortfunc == descendingSizeMix)
        {
            sortfunc = descendingSize;
        }
        else if (sortfunc == ascendingDateMix)
        {
            sortfunc = ascendingDate;
        }
        else if (sortfunc == descendingDateMix)
        {
            sortfunc = descendingDate;
        }
        else if (sortfunc == ascendingUserMix)
        {
            sortfunc = ascendingUser;
        }
        else if (sortfunc == descendingUserMix)
        {
            sortfunc = descendingUser;
        }
        else if (sortfunc == ascendingGroupMix)
        {
            sortfunc = ascendingGroup;
        }
        else if (sortfunc == descendingGroupMix)
        {
            sortfunc = descendingGroup;
        }
        else if (sortfunc == ascendingPermsMix)
        {
            sortfunc = ascendingPerms;
        }
        else if (sortfunc == descendingPermsMix)
        {
            sortfunc = descendingPerms;
        }
        else if (sortfunc == ascendingLinkMix)
        {
            sortfunc = ascendingLink;
        }
        else if (sortfunc == descendingLinkMix)
        {
            sortfunc = descendingLink;
        }
        else if (sortfunc == ascendingLinkCaseMix)
        {
            sortfunc = ascendingLinkCase;
        }
        else if (sortfunc == descendingLinkCaseMix)
        {
            sortfunc = descendingLinkCase;
        }
        else if (sortfunc == ascendingDirMix)
        {
            sortfunc = ascendingDir;
        }
        else if (sortfunc == descendingDirMix)
        {
            sortfunc = descendingDir;
        }
        else if (sortfunc == ascendingDirCaseMix)
        {
            sortfunc = ascendingDirCase;
        }
        else if (sortfunc == descendingDirCaseMix)
        {
            sortfunc = descendingDirCase;
        }
        else if (sortfunc == ascendingOrigpathMix)
        {
            sortfunc = ascendingOrigpath;
        }
        else if (sortfunc == descendingOrigpathMix)
        {
            sortfunc = descendingOrigpath;
        }
        else if (sortfunc == ascendingOrigpathCaseMix)
        {
            sortfunc = ascendingOrigpathCase;
        }
        else if (sortfunc == descendingOrigpathCaseMix)
        {
            sortfunc = descendingOrigpathCase;
        }
        else if (sortfunc == ascendingDeldateMix)
        {
            sortfunc = ascendingDeldate;
        }
        else if (sortfunc == descendingDeldateMix)
        {
            sortfunc = descendingDeldate;
        }
    }
    scan(true);
    return 1;
}


// Update directories first
long FileList::onUpdDirsFirst(FXObject* sender, FXSelector, void* ptr)
{
    if (dirsfirst == true)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
    }

    return 1;
}


// Clicked header button
long FileList::onCmdHeader(FXObject*, FXSelector, void* ptr)
{
    FXuint index = (FXuint)(FXuval)ptr;

    handle(this, FXSEL(SEL_COMMAND, getHeaderId(index)), NULL);

    setSortHeader(index);

    // Limit header size
    if ((FXuint)getHeaderSize(getHeaderIndex(ID_COL_NAME)) < MIN_NAME_SIZE)
    {
        setHeaderSize((FXuint)getHeaderIndex(ID_COL_NAME), MIN_NAME_SIZE);
    }
    if ((FXuint)getHeaderSize(getHeaderIndex(ID_COL_NAME)) > MAX_NAME_SIZE)
    {
        setHeaderSize((FXuint)getHeaderIndex(ID_COL_NAME), MAX_NAME_SIZE);
    }

    return 1;
}


// Update header button
long FileList::onUpdHeader(FXObject*, FXSelector, void* ptr)
{
    // Set arrow on header, depending on the sort function

    // Column always displayed
    header->setArrowDir(getHeaderIndex(ID_COL_NAME), (sortfunc == ascending || sortfunc == ascendingCase ||
                        sortfunc == ascendingMix || sortfunc == ascendingCaseMix)  ?
                        false : (sortfunc == descending || sortfunc == descendingCase ||
                        sortfunc == descendingMix || sortfunc == descendingCaseMix) ? true : MAYBE);                       // Name

    if (getHeaderIndex(ID_COL_SIZE) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_SIZE), (sortfunc == ascendingSize || sortfunc == ascendingSizeMix) ?
                            false : (sortfunc == descendingSize || sortfunc == descendingSizeMix) ? true : MAYBE);         // Size
    }

    if (getHeaderIndex(ID_COL_TYPE) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_TYPE), (sortfunc == ascendingType || sortfunc == ascendingTypeMix) ?
                            false : (sortfunc == descendingType || sortfunc == descendingTypeMix) ? true : MAYBE);         // Type
    }

    if (getHeaderIndex(ID_COL_EXT) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_EXT), (sortfunc == ascendingExt || sortfunc == ascendingExtMix) ?
                            false : (sortfunc == descendingExt || sortfunc == descendingExtMix)  ? true : MAYBE);          // Extension
    }

    if (getHeaderIndex(ID_COL_DATE) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_DATE), (sortfunc == ascendingDate || sortfunc == ascendingDateMix) ?
                            false : (sortfunc == descendingDate || sortfunc == descendingDateMix) ? true : MAYBE);         // Date
    }

    if (getHeaderIndex(ID_COL_USER) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_USER), (sortfunc == ascendingUser || sortfunc == ascendingUserMix) ?
                            false : (sortfunc == descendingUser || sortfunc == descendingUserMix) ? true : MAYBE);         // User
    }

    if (getHeaderIndex(ID_COL_GROUP) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_GROUP), (sortfunc == ascendingGroup || sortfunc == ascendingGroupMix) ?
                            false : (sortfunc == descendingGroup || sortfunc == descendingGroupMix) ? true : MAYBE);       // Group
    }

    if (getHeaderIndex(ID_COL_PERMS) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_PERMS), (sortfunc == ascendingPerms || sortfunc == ascendingPermsMix)  ?
                            false : (sortfunc == descendingPerms || sortfunc == descendingPermsMix) ? true : MAYBE);       // Permissions
    }

    if (getHeaderIndex(ID_COL_LINK) >= 0)
    {
        header->setArrowDir(getHeaderIndex(ID_COL_LINK), (sortfunc == ascendingLink || sortfunc == ascendingLinkCase ||
                            sortfunc == ascendingLinkMix || sortfunc == ascendingLinkCaseMix)  ? false :
                            (sortfunc == descendingLink || sortfunc == descendingLinkCase ||
                            sortfunc == descendingLinkMix || sortfunc == descendingLinkCaseMix) ? true : MAYBE);           // Link
    }

    if (options & FILELIST_SEARCH)
    {
        // Column always displayed in a search list
        header->setArrowDir(getHeaderIndex(ID_COL_DIRNAME), (sortfunc == ascendingDir || sortfunc == ascendingDirCase ||
                            sortfunc == ascendingDirMix || sortfunc == ascendingDirCaseMix)  ? false :
                            (sortfunc == descendingDir || sortfunc == descendingDirCase ||
                            sortfunc == descendingDirMix || sortfunc == descendingDirCaseMix) ? true : MAYBE);            // Dir Name
    }
    else
    {
        // Columns always displayed in a file list, when in Trash
        if (!(options & FILELIST_SEARCH) && (getNumHeaders() == (int)nbColsTrash))
        {
            header->setArrowDir(getHeaderIndex(ID_COL_ORIGPATH), (sortfunc == ascendingOrigpath ||
                                sortfunc == ascendingOrigpathCase || sortfunc == ascendingOrigpathMix ||
                                sortfunc == ascendingOrigpathCaseMix)  ? false :
                                (sortfunc == descendingOrigpath || sortfunc == descendingOrigpathCase ||
                                sortfunc == descendingOrigpathMix || sortfunc == descendingOrigpathCaseMix) ?
                                true : MAYBE);                                                                            // Original path
            origpath_size = header->getItemSize(getHeaderIndex(ID_COL_ORIGPATH));

            header->setArrowDir(getHeaderIndex(ID_COL_DELDATE), (sortfunc == ascendingDeldate ||
                                sortfunc == ascendingDeldateMix) ? false : (sortfunc == descendingDeldate ||
                                sortfunc == descendingDeldateMix) ? true : MAYBE);                                        // Deletion date
            deldate_size = header->getItemSize(getHeaderIndex(ID_COL_DELDATE));
        }
    }

    // Set sort header, depending on the sort function
    if ((sortfunc == ascending) || (sortfunc == ascendingCase) || (sortfunc == ascendingMix) ||
        (sortfunc == ascendingCaseMix) ||
        (sortfunc == descending) || (sortfunc == descendingCase) || (sortfunc == descendingMix) ||
        (sortfunc == descendingCaseMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_NAME));
    }
    else if ((sortfunc == ascendingDir) || (sortfunc == ascendingDirCase) || (sortfunc == ascendingDirMix) ||
             (sortfunc == ascendingDirCaseMix) ||
             (sortfunc == descendingDir) || (sortfunc == descendingDirCase) || (sortfunc == descendingDirMix) ||
             (sortfunc == descendingDirCaseMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_DIRNAME));
    }
    else if ((sortfunc == ascendingSize) || (sortfunc == ascendingSizeMix) || (sortfunc == descendingSize) ||
             (sortfunc == descendingSizeMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_SIZE));
    }
    else if ((sortfunc == ascendingType) || (sortfunc == ascendingTypeMix) || (sortfunc == descendingType) ||
             (sortfunc == descendingTypeMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_TYPE));
    }
    else if ((sortfunc == ascendingExt) || (sortfunc == ascendingExtMix) || (sortfunc == descendingExt) ||
             (sortfunc == descendingExtMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_EXT));
    }
    else if ((sortfunc == ascendingDate) || (sortfunc == ascendingDateMix) || (sortfunc == descendingDate) ||
             (sortfunc == descendingDateMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_DATE));
    }
    else if ((sortfunc == ascendingUser) || (sortfunc == ascendingUserMix) || (sortfunc == descendingUser) ||
             (sortfunc == descendingUserMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_USER));
    }
    else if ((sortfunc == ascendingGroup) || (sortfunc == ascendingGroupMix) || (sortfunc == descendingGroup) ||
             (sortfunc == descendingGroupMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_GROUP));
    }
    else if ((sortfunc == ascendingPerms) || (sortfunc == ascendingPermsMix) || (sortfunc == descendingPerms) ||
             (sortfunc == descendingPermsMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_PERMS));
    }
    else if ((sortfunc == ascendingLink) || (sortfunc == ascendingLinkCase) || (sortfunc == ascendingLinkMix) ||
             (sortfunc == ascendingLinkCaseMix) || (sortfunc == descendingLink) || (sortfunc == descendingLinkCase) ||
             (sortfunc == descendingLinkMix) || (sortfunc == descendingLinkCaseMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_LINK));
    }
    else if ((sortfunc == ascendingOrigpath) || (sortfunc == ascendingOrigpathCase) ||
             (sortfunc == ascendingOrigpathMix) || (sortfunc == ascendingOrigpathCaseMix) ||
             (sortfunc == descendingOrigpath) || (sortfunc == descendingOrigpathCase) ||
             (sortfunc == descendingOrigpathMix) || (sortfunc == descendingOrigpathCaseMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_ORIGPATH));
    }
    else if ((sortfunc == ascendingDeldate) || (sortfunc == ascendingDeldateMix) ||
             (sortfunc == descendingDeldate) || (sortfunc == descendingDeldateMix))
    {
        setSortHeader(getHeaderIndex(ID_COL_DELDATE));
    }

    return 1;
}


/**
 * Compares fields of p and q, supposing they are single byte strings
 * without using the current locale.
 * @param  igncase    Ignore upper/lower-case?
 * @param  asc        Ascending?  If false is descending order
 * @param  jmp        Field to compare (separated with \t)
 *
 * @return 0 if equal, negative if p<q, positive if p>q
 * If jmp has an invalid value returns 0 and errno will be EINVAL
 */
static inline int compare_nolocale(char* p, char* q, FXbool igncase, FXbool asc, FXuint jmp)
{
    int retnames, ret = 0, i;

    // Compare names

    char* pp = p;
    char* qq = q;

    // Go to next '\t' or '\0'
    while (*pp != '\0' && *pp > '\t')
    {
        pp++;
    }

    while (*qq != '\0' && *qq > '\t')
    {
        qq++;
    }

    // Save characters at current position
    char pw = *pp;
    char qw = *qq;

    // Set characters to null, to stop comparison
    *pp = '\0';
    *qq = '\0';

    // Compare strings
    retnames = xf_comparenat(p, q, igncase);

    // Restore saved characters
    *pp = pw;
    *qq = qw;

    // Name column
    if (jmp == 0)
    {
        ret = retnames;
        goto end;
    }

    // Other columns
    else if (jmp < NMAX_COLS + 2)
    {
        for (i = jmp; *pp && i; i -= (*pp++ == '\t'))
        {
        }
        for (i = jmp; *qq && i; i -= (*qq++ == '\t'))
        {
        }
    }

    // Error
    else
    {
        errno = EINVAL;
        return 0;
    }

    // This part between brackets to make the compiler happy!
    {
        char* sp = pp;
        char* sq = qq;

        // Find where to stop comparison
        while (*pp != '\0' && *pp > '\t')
        {
            pp++;
        }

        while (*qq != '\0' && *qq > '\t')
        {
            qq++;
        }

        // Save characters at current position
        pw = *pp;
        qw = *qq;

        // Set characters to null, to stop comparison
        *pp = '\0';
        *qq = '\0';

        // Compare wide strings
        ret = xf_comparenat(sp, sq, igncase);

        // Restore saved characters
        *pp = pw;
        *qq = qw;

        if (ret == 0)
        {
            ret = retnames;
        }
    }

end:

    // If descending flip
    if (!asc)
    {
        ret = ret * -1;
    }

    return ret;
}


/**
 * Compares fields of p and q, supposing they are wide strings
 * and using the current locale.
 * @param  igncase    Ignore upper/lower-case?
 * @param  asc        Ascending?  If false is descending order
 * @param  jmp        Field to compare (separated with \t)
 *
 * @return 0 if equal, negative if p<q, positive if p>q
 * If jmp has an invalid value returns 0 and errno will be EINVAL
 */
static inline int compare_locale(wchar_t* p, wchar_t* q, FXbool igncase, FXbool asc, int jmp)
{
    int retnames, ret = 0, i;

    // Compare names

    wchar_t* pp = p;
    wchar_t* qq = q;

    // Go to next '\t' or '\0'
    while (*pp != '\0' && *pp > '\t')
    {
        pp++;
    }

    while (*qq != '\0' && *qq > '\t')
    {
        qq++;
    }

    // Save characters at current position
    wchar_t pw = *pp;
    wchar_t qw = *qq;

    // Set characters to null, to stop comparison
    *pp = '\0';
    *qq = '\0';

    // Compare wide strings
    retnames = xf_comparewnat(p, q, igncase);

    // Restore saved characters
    *pp = pw;
    *qq = qw;


    // Name column
    if (jmp == 0)
    {
        ret = retnames;
        goto end;
    }

    // Other columns
    else if (jmp < NMAX_COLS + 2)
    {
        for (i = jmp; *pp && i; i -= (*pp++ == '\t'))
        {
        }
        for (i = jmp; *qq && i; i -= (*qq++ == '\t'))
        {
        }
    }

    // Error
    else
    {
        errno = EINVAL;
        return 0;
    }

    // This part between brackets to make the compiler happy!
    {
        wchar_t* sp = pp;
        wchar_t* sq = qq;

        // Find where to stop comparison
        while (*pp != '\0' && *pp > '\t')
        {
            pp++;
        }

        while (*qq != '\0' && *qq > '\t')
        {
            qq++;
        }

        // Save characters at current position
        pw = *pp;
        qw = *qq;

        // Set characters to null, to stop comparison
        *pp = '\0';
        *qq = '\0';

        // Compare wide strings
        ret = xf_comparewnat(sp, sq, igncase);

        // Restore saved characters
        *pp = pw;
        *qq = qw;

        if (ret == 0)
        {
            ret = retnames;
        }
    }

end:

    // If descending flip
    if (!asc)
    {
        ret = ret * -1;
    }

    return ret;
}


/**
 * Compares a field of pa with the same field of pb, if the fields are
 * equal compare by name
 * @param  igncase    Ignore upper/lower-case?
 * @param  asc        Ascending?  If false is descending order
 * @param  mixdir    Mix directories with files?
 * @param  jmp        Field to compare (separated with \t)
 *
 * @return 0 if equal, negative if pa<pb, positive if pa>pb
 * Requires to allocate some space, if there is no memory this
 * function returns 0 and errno will be ENOMEM
 * If jmp has an invalid value returns 0 and errno will be EINVAL
 */
int FileList::compare(const IconItem* pa, const IconItem* pb,
                      FXbool igncase, FXbool asc, FXbool mixdir, FXuint jmp)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;
    char* p = (char*)a->label.text();
    char* q = (char*)b->label.text();

    // Common cases
    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }

    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    if (!mixdir)
    {
        int diff = (int)b->isDirectory() - (int)a->isDirectory();
        if (diff)
        {
            return diff;
        }
    }

    // Prepare wide char strings
    wchar_t* wa = NULL;
    wchar_t* wb = NULL;
    size_t an, bn;
    an = mbstowcs(NULL, (const char*)p, 0);
    if (an == (size_t)-1)
    {
        return compare_nolocale(p, q, igncase, asc, jmp); // If error, fall back to no locale comparison
    }
    wa = (wchar_t*)calloc(an + 1, sizeof(wchar_t));
    if (wa == NULL)
    {
        errno = ENOMEM;
        return 0;
    }
    mbstowcs(wa, p, an + 1);
    bn = mbstowcs(NULL, (const char*)q, 0);
    if (bn == (size_t)-1)
    {
        free(wa);
        return compare_nolocale(p, q, igncase, asc, jmp); // If error, fall back to no locale comparison
    }
    wb = (wchar_t*)calloc(bn + 1, sizeof(wchar_t));
    if (wb == NULL)
    {
        errno = ENOMEM;
        free(wa);
        return 0;
    }
    mbstowcs(wb, q, bn + 1);

    // Perform comparison based on the current locale
    int ret = compare_locale(wa, wb, igncase, asc, jmp);

    // Free memory
    if (wa != NULL)
    {
        free(wa);
    }
    if (wb != NULL)
    {
        free(wb);
    }

    return ret;
}


// Compare file names
int FileList::ascending(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, name_index);
}


// Compare file names, mixing files and directories
int FileList::ascendingMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, name_index);
}


// Compare file names, case insensitive
int FileList::ascendingCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, false, name_index);
}


// Compare file names, case insensitive, mixing files and directories
int FileList::ascendingCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, true, name_index);
}


// Compare directory names
int FileList::ascendingDir(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, dirname_index);
}


// Compare directory names, mixing files and directories
int FileList::ascendingDirMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, dirname_index);
}


// Compare directory names, case insensitive
int FileList::ascendingDirCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, false, dirname_index);
}


// Compare directory names, case insensitive, mixing files and directories
int FileList::ascendingDirCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, true, dirname_index);
}


// Compare file types
int FileList::ascendingType(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, type_index);
}


// Compare file types, mixing files and directories
int FileList::ascendingTypeMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, type_index);
}


// Compare file extension
int FileList::ascendingExt(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, ext_index);
}


// Compare file extension, mixing files and directories
int FileList::ascendingExtMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, ext_index);
}


// Compare file size - Warning: only returns the sign of the comparison!!!
int FileList::ascendingSize(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    int sum = (int)b->isDirectory() + (int)a->isDirectory();

    if (diff)
    {
        return diff;
    }
    if (sum == 2)
    {
        return ascendingCase(pa, pb);
    }

    FXlong l = a->size - b->size;
    if (l)
    {
        if (l >= 0)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    return ascendingCase(pa, pb);
}


// Compare file size - Warning: only returns the sign of the comparison!!!
// Mixing files and directories
int FileList::ascendingSizeMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();
    int adir = (int)a->isDirectory();
    int bdir = (int)b->isDirectory();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    if (adir && bdir)
    {
        return ascendingCaseMix(pa, pb);
    }
    if (adir && !bdir)
    {
        return -1;
    }
    if (!adir && bdir)
    {
        return 1;
    }

    FXlong l = a->size - b->size;
    if (l)
    {
        if (l >= 0)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    return ascendingCaseMix(pa, pb);
}


// Compare file time
int FileList::ascendingDate(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    if (diff)
    {
        return diff;
    }

    FXlong l = (FXlong)a->date - (FXlong)b->date;
    if (l)
    {
        return l;
    }
    return ascendingCase(pa, pb);
}


// Compare file time, mixing files and directories
int FileList::ascendingDateMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    FXlong l = (FXlong)a->date - (FXlong)b->date;
    if (l)
    {
        return l;
    }
    return ascendingCaseMix(pa, pb);
}


// Compare file user
int FileList::ascendingUser(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, user_index);
}


// Compare file user, mixing files and directories
int FileList::ascendingUserMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, user_index);
}


// Compare file group
int FileList::ascendingGroup(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, group_index);
}


// Compare file group, mixing files and directories
int FileList::ascendingGroupMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, group_index);
}


// Compare file permissions
int FileList::ascendingPerms(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, perms_index);
}


// Compare file permissions, mixing files and directories
int FileList::ascendingPermsMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, perms_index);
}


// Compare link
int FileList::ascendingLink(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, link_index);
}


// Compare link, case insensitive
int FileList::ascendingLinkCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, false, link_index);
}


// Compare link, mixing files and directories
int FileList::ascendingLinkMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, link_index);
}


// Compare link, mixing files and directories, case insensitive
int FileList::ascendingLinkCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, true, link_index);
}


// Compare file deletion time
int FileList::ascendingDeldate(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    if (diff)
    {
        return diff;
    }
    FXlong l = (FXlong)a->deldate - (FXlong)b->deldate;
    if (l)
    {
        return l;
    }
    return ascendingCase(pa, pb);
}


// Compare file deletion time, mixing files and directories
int FileList::ascendingDeldateMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    FXlong l = (FXlong)a->deldate - (FXlong)b->deldate;
    if (l)
    {
        return l;
    }
    return ascendingCaseMix(pa, pb);
}


// Compare original path
int FileList::ascendingOrigpath(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, false, origpath_index);
}


// Compare original path, case insensitive
int FileList::ascendingOrigpathCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, false, origpath_index);
}


// Compare original path, mixing files and directories
int FileList::ascendingOrigpathMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, true, true, origpath_index);
}


// Compare original path, mixing files and directories, case insensitive
int FileList::ascendingOrigpathCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, true, true, origpath_index);
}


// Reversed compare file name, case insensitive
int FileList::descendingCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, false, name_index);
}


// Reversed compare file name, case insensitive, mixing files and directories
int FileList::descendingCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, true, name_index);
}


// Reversed compare file name
int FileList::descending(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, name_index);
}


// Reversed compare file name, mixing files and directories
int FileList::descendingMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, name_index);
}


// Reversed compare directory names, case insensitive
int FileList::descendingDirCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, false, dirname_index);
}


// Reversed compare directory names, case insensitive, mixing files and directories
int FileList::descendingDirCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, true, dirname_index);
}


// Reversed compare directory names
int FileList::descendingDir(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, dirname_index);
}


// Reversed compare directory names, mixing files and directories
int FileList::descendingDirMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, dirname_index);
}


// Reversed compare file type
int FileList::descendingType(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, type_index);
}


// Reversed compare file type, mixing files and directories
int FileList::descendingTypeMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, type_index);
}


// Reversed compare file extension
int FileList::descendingExt(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, ext_index);
}


// Reversed compare file extension, mixing files and directories
int FileList::descendingExtMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, ext_index);
}


// Reversed compare file size
int FileList::descendingSize(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    int sum = (int)b->isDirectory() + (int)a->isDirectory();
    if (diff)
    {
        return diff;
    }
    if (sum == 2)
    {
        return -ascendingCase(pa, pb);
    }
    FXlong l = a->size - b->size;
    if (l)
    {
        if (l >= 0)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    return -ascendingCase(pa, pb);
}


// Reversed compare file size, mixing files and directories
int FileList::descendingSizeMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();
    int adir = (int)a->isDirectory();
    int bdir = (int)b->isDirectory();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    if (adir && bdir)
    {
        return -ascendingCaseMix(pa, pb);
    }
    if (adir && !bdir)
    {
        return 1;
    }
    if (!adir && bdir)
    {
        return -1;
    }

    FXlong l = a->size - b->size;
    if (l)
    {
        if (l >= 0)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    return -ascendingCaseMix(pa, pb);
}


// Reversed compare file time
int FileList::descendingDate(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    if (diff)
    {
        return diff;
    }
    FXlong l = (FXlong)a->date - (FXlong)b->date;
    if (l)
    {
        return -l;
    }
    return -ascendingCase(pa, pb);
}


// Reversed compare file time, mixing files and directories
int FileList::descendingDateMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    FXlong l = (FXlong)a->date - (FXlong)b->date;
    if (l)
    {
        return -l;
    }
    return -ascendingCaseMix(pa, pb);
}


// Reversed compare file user
int FileList::descendingUser(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, user_index);
}


// Reversed compare file user, mixing files and directories
int FileList::descendingUserMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, user_index);
}


// Reversed compare file group
int FileList::descendingGroup(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, group_index);
}


// Reversed compare file group, mixing files and directories
int FileList::descendingGroupMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, group_index);
}


// Reversed compare file permission
int FileList::descendingPerms(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, perms_index);
}

// Reversed compare file permission, mixing files and directories
int FileList::descendingPermsMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, perms_index);
}


// Reversed compare link
int FileList::descendingLink(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, link_index);
}


// Reversed compare link, case insensitive
int FileList::descendingLinkCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, false, link_index);
}


// Reversed compare link, mixing files and directories
int FileList::descendingLinkMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, link_index);
}


// Reversed compare link, mixing files and directories, case insensitive
int FileList::descendingLinkCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, true, link_index);
}


// Reversed compare file deletion time
int FileList::descendingDeldate(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    int diff = (int)b->isDirectory() - (int)a->isDirectory();
    if (diff)
    {
        return diff;
    }
    FXlong l = (FXlong)a->deldate - (FXlong)b->deldate;
    if (l)
    {
        return -l;
    }
    return -ascendingCase(pa, pb);
}


// Reversed compare file deletion time, mixing files and directories
int FileList::descendingDeldateMix(const IconItem* pa, const IconItem* pb)
{
    const FileItem* a = (FileItem*)pa;
    const FileItem* b = (FileItem*)pb;

    const FXuchar* p = (const FXuchar*)a->label.text();
    const FXuchar* q = (const FXuchar*)b->label.text();

    // Directory '..' should always be on top
    if ((p[0] == '.') && (p[1] == '.') && (p[2] == '\t'))
    {
        return -1;
    }
    if ((q[0] == '.') && (q[1] == '.') && (q[2] == '\t'))
    {
        return 1;
    }

    FXlong l = (FXlong)a->deldate - (FXlong)b->deldate;
    if (l)
    {
        return -l;
    }
    return -ascendingCaseMix(pa, pb);
}


// Reversed compare original path
int FileList::descendingOrigpath(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, false, origpath_index);
}


// Reversed original path, mixing files and directories
int FileList::descendingOrigpathMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, false, false, true, origpath_index);
}


// Reversed compare original path, case insensitive
int FileList::descendingOrigpathCase(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, false, origpath_index);
}


// Reversed compare original path, mixing files and directories, case insensitive
int FileList::descendingOrigpathCaseMix(const IconItem* pa, const IconItem* pb)
{
    return compare(pa, pb, true, false, true, origpath_index);
}


// Force an immediate update of the list
long FileList::onCmdRefresh(FXObject*, FXSelector, void*)
{
    // Force a refresh of the file association table
    if (associations)
    {
        delete associations;
        associations = new FileDict(getApp());
    }
    allowrefresh = true;
    scan(true);
    return 1;
}


// Allow or forbid file list refresh
void FileList::setAllowRefresh(const FXbool allow)
{
    if (allow == false)
    {
        allowrefresh = false;
    }
    else
    {
        allowrefresh = true;
    }
}


// Refresh; don't update if user is interacting with the list
long FileList::onCmdRefreshTimer(FXObject*, FXSelector, void*)
{
    // Don't refresh if window is minimized
    if (((FXTopWindow*)focuswindow)->isMinimized())
    {
        return 0;
    }

    // Don't refresh if not allowed
    if (flags & FLAG_UPDATE && allowrefresh)
    {
        scan(false);
        counter = (counter + 1) % REFRESH_FREQUENCY;
    }

    // Restart timer
    getApp()->addTimeout(this, ID_REFRESH_TIMER, REFRESH_INTERVAL);
    return 0;
}


// Set current filename
void FileList::setCurrentFile(const FXString& pathname)
{
    // FIXME notify argument?
    if (!pathname.empty())
    {
        setDirectory(FXPath::directory(pathname));
        setCurrentItem(findItem(FXPath::name(pathname)));
    }
}


// Get pathname to current file, if any
FXString FileList::getCurrentFile() const
{
    if (current < 0)
    {
        return FXString::null;
    }
    return getItemPathname(current);
}


// Set directory being displayed and update the history if necessary
void FileList::setDirectory(const FXString& pathname, const FXbool histupdate, FXString prevpath)
{
    // Only update the history if it was requested
    if (histupdate)
    {
        // At first call, allocate the history
        if (forwardhist == NULL)
        {
            forwardhist = new StringList();
        }
        if (backhist == NULL)
        {
            backhist = new StringList();
        }
        // Update the history
        else
        {
            backhist->insertFirstItem(getDirectory());
            forwardhist->removeAllItems();
        }
    }

    FXString path("");

    // FIXME notify argument?
    if (!pathname.empty())
    {
        path = FXPath::absolute(directory, pathname);

        while (!FXPath::isTopDirectory(path) && !xf_isdirectory(path))
        {
            path = FXPath::upLevel(path);
        }
        if (directory != path)
        {
            directory = xf_cleanpath(path);
            clearItems();
            list = NULL;
            scan(true);
        }
    }
    // If possible, select directory we came from, otherwise select first item
    if (hasFocus())
    {
        int sel_index = 0;

        if (!prevpath.empty())
        {
            if (path == FXPath::upLevel(prevpath)) // Did we come from subdirectory?
            {
                // Find dir in list
                sel_index = findItem(FXPath::name(prevpath));
                if ((sel_index == -1) && getNumItems())
                {
                    // Not found, select first item
                    sel_index = 0;
                }
            }
        }

        if ((sel_index != -1) && getNumItems())
        {
            enableItem(sel_index);
            setCurrentItem(sel_index);
        }
    }

    // Autosize name column in detailed mode
    IconList::autosizeNameHeader();
}


// Set the pattern to filter
void FileList::setPattern(const FXString& ptrn)
{
    if (ptrn.empty())
    {
        return;
    }
    if (pattern != ptrn)
    {
        pattern = ptrn;
        scan(true);
    }
    setFocus();
}


// Change file match mode
void FileList::setMatchMode(FXuint mode)
{
    if (matchmode != mode)
    {
        matchmode = mode;
        scan(true);
    }
    setFocus();
}


// Return true if showing hidden files
FXbool FileList::shownHiddenFiles() const
{
    return (options & FILELIST_SHOWHIDDEN) != 0;
}


// Change show hidden files mode
void FileList::showHiddenFiles(FXbool shown)
{
    FXuint opts = shown ? (options | FILELIST_SHOWHIDDEN) : (options & ~FILELIST_SHOWHIDDEN);

    if (opts != options)
    {
        options = opts;
        scan(true);
    }
    setFocus();
}


// Return true if showing thumbnails
FXbool FileList::shownThumbnails() const
{
    return displaythumbnails;
}


// Change show thumbnails mode
void FileList::showThumbnails(FXbool display)
{
    displaythumbnails = display;

    // Refresh to display or hide thumbnails
    scan(true);
    setFocus();
}


// Return true if showing directories only
FXbool FileList::showOnlyDirectories() const
{
    return (options & FILELIST_SHOWDIRS) != 0;
}


// Change show directories only mode
void FileList::showOnlyDirectories(FXbool shown)
{
    FXuint opts = shown ? (options | FILELIST_SHOWDIRS) : (options & ~FILELIST_SHOWDIRS);

    if (opts != options)
    {
        options = opts;
        scan(true);
    }
    setFocus();
}


// Compare till '\t' or '\0'
static FXbool fileequal(const FXString& a, const FXString& b)
{
    const FXuchar* p1 = (const FXuchar*)a.text();
    const FXuchar* p2 = (const FXuchar*)b.text();
    int c1, c2;

    do
    {
        c1 = *p1++;
        c2 = *p2++;
    }
    while (c1 != '\0' && c1 != '\t' && c1 == c2);
    return (c1 == '\0' || c1 == '\t') && (c2 == '\0' || c2 == '\t');
}


// Create custom item
IconItem* FileList::createItem(const FXString& text, FXIcon* big, FXIcon* mini, void* ptr)
{
    return new FileItem(getApp(), text, big, mini, ptr);
}


// Is directory
FXbool FileList::isItemDirectory(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::isItemDirectory: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (((FileItem*)items[index])->state & FileItem::FOLDER) != 0;
}


// Is file
FXbool FileList::isItemFile(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::isItemFile: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (((FileItem*)items[index])->state &
            (FileItem::FOLDER | FileItem::CHARDEV | FileItem::BLOCKDEV | FileItem::FIFO | FileItem::SOCK)) == 0;
}


// Is executable
FXbool FileList::isItemExecutable(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::isItemExecutable: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (((FileItem*)items[index])->state & FileItem::EXECUTABLE) != 0;
}


// Is link
FXbool FileList::isItemLink(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::isItemLink: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (((FileItem*)items[index])->state & FileItem::SYMLINK) != 0;
}


// Get file name from item at index
FXString FileList::getItemFilename(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::getItemFilename: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    FXString label = items[index]->getText();
    return label.section('\t', 0);
}


// Get pathname from item at index, relatively to the current directory
FXString FileList::getItemPathname(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::getItemPathname: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    FXString label = items[index]->getText();
    return FXPath::absolute(directory, label.section('\t', 0));
}


// Get full pathname from item at index, as obtained from the label string
FXString FileList::getItemFullPathname(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::getItemFullPathname: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    FXString label = items[index]->getText();
    return label.rafter('\t');
}


// Get associations (if any) from the file
FileAssoc* FileList::getItemAssoc(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::getItemAssoc: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return ((FileItem*)items[index])->assoc;
}


// Return file size of the item
FXulong FileList::getItemFileSize(int index) const
{
    if ((FXuint)index >= (FXuint)items.no())
    {
        fprintf(stderr, "%s::getItemFileSize: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return ((FileItem*)items[index])->size;
}


// Change associations table; force a rescan so as to
// update the bindings in each item to the new associations
void FileList::setAssociations(FileDict* assocs)
{
    if (associations != assocs)
    {
        associations = assocs;
        scan(true);
    }
    setFocus();
}


// Change header size
void FileList::setHeaderSize(FXuint index, FXuint size)
{
    if (getHeaderId(index) == ID_COL_ORIGPATH)
    {
        origpath_size = size;
    }
    else if (getHeaderId(index) == ID_COL_DELDATE)
    {
        deldate_size = size;
    }
    else
    {
        if ((FXuint)index >= (FXuint)header->getNumItems())
        {
            fprintf(stderr, "%s::setHeaderSize: index out of range.\n", getClassName());
            exit(EXIT_FAILURE);
        }
        else
        {
            header->setItemSize(index, size);
        }
    }
}


// Get header size
int FileList::getHeaderSize(FXuint index)
{
    if (getHeaderId(index) == ID_COL_ORIGPATH)
    {
        return origpath_size;
    }

    if (getHeaderId(index) == ID_COL_DELDATE)
    {
        return deldate_size;
    }

    if ((FXuint)index >= (FXuint)header->getNumItems())
    {
        fprintf(stderr, "%s::getHeaderSize: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }

    return header->getItemSize(index);
}


// Update refresh timer if the window is unminimized
long FileList::onUpdRefreshTimer(FXObject*, FXSelector, void*)
{
    static FXbool prevMinimized = true;
    static FXbool minimized = true;

    prevMinimized = minimized;
    if (((FXTopWindow*)focuswindow)->isMinimized())
    {
        minimized = false;
    }
    else
    {
        minimized = true;
    }

    // Update refresh timer
    if ((prevMinimized == false) && (minimized == true))
    {
        onCmdRefreshTimer(0, 0, 0);
    }

    return 1;
}


// Scan items to see if listing is necessary
void FileList::scan(FXbool force)
{
    // Start wait cursor if refresh forced (use custom function)
    if (force)
    {
        xf_setwaitcursor(getApp(), BEGIN_CURSOR);
    }

    // Special case where the file list is a search list
    if (options & FILELIST_SEARCH)
    {
        FXbool updated = updateItems(force);
        if (updated)
        {
            sortItems(); // Only sort if search list was updated
        }
    }
    // Normal case
    else
    {
        struct stat info;

        // Stat the current directory
        if (xf_info(directory, info))
        {
            // New date of directory
            FXTime newdate = (FXTime)FXMAX(info.st_mtime, info.st_ctime);

            // Forced, directory date was changed, or failed to get proper date or counter expired
            if (force || (timestamp != newdate) || (counter == 0))
            {
                // And do the refresh
#if defined(linux)
                refreshMtdevices();
#endif
                listItems(force);
                sortItems();

                // Remember when we did this
                timestamp = newdate;
            }
        }
        // Move to higher directory
        else
        {
            setDirectory(FXPath::upLevel(directory));
        }
    }

    // Stop wait cursor if refresh forced (use custom function)
    if (force)
    {
        xf_setwaitcursor(getApp(), END_CURSOR);
    }
}


// Set labels for file and search lists
void FileList::setItemLabels(FXuint* idCol, FXuint* idColTrash, FXbool inTrash, FXString filename, FXString dirname,
                             FXString hsize, FXString filetype, FXString ext, FXString mod, FXString usrid,
                             FXString grpid, FXString perms, FXString linkpath, FXString origpath, FXString del,
                             FXString pathname, FXString* labels)
{
    // In Trash
    if (!(options & FILELIST_SEARCH) && inTrash)
    {
        for (FXuint i = 0; i < nbColsTrash; i++)
        {
            switch (idColTrash[i])
            {
            case ID_COL_NAME:
                labels[i] = filename;
                break;

            case ID_COL_ORIGPATH:
                labels[i] = origpath;
                break;

            case ID_COL_SIZE:
                labels[i] = hsize;
                break;

            case ID_COL_TYPE:
                labels[i] = filetype;
                break;

            case ID_COL_EXT:
                labels[i] = ext;
                break;

            case ID_COL_DATE:
                labels[i] = mod;
                break;

            case ID_COL_DELDATE:
                labels[i] = del;
                break;

            case ID_COL_USER:
                labels[i] = usrid;
                break;

            case ID_COL_GROUP:
                labels[i] = grpid;
                break;

            case ID_COL_PERMS:
                labels[i] = perms;
                break;

            case ID_COL_LINK:
                if (linkpath.length() != 0)
                {
                    labels[i] = "-> " + linkpath;
                }
                else
                {
                    labels[i] = linkpath;                    
                }
                break;
            }
        }

        // Pathname is always at last position
        labels[NMAX_COLS + 2] = pathname;
    }
    // Not in Trash
    else
    {
        for (FXuint i = 0; i < nbCols; i++)
        {
            switch (idCol[i])
            {
            case ID_COL_NAME:
                labels[i] = filename;
                break;

            case ID_COL_DIRNAME:
                labels[i] = dirname;
                break;

            case ID_COL_SIZE:
                labels[i] = hsize;
                break;

            case ID_COL_TYPE:
                labels[i] = filetype;
                break;

            case ID_COL_EXT:
                labels[i] = ext;
                break;

            case ID_COL_DATE:
                labels[i] = mod;
                break;

            case ID_COL_USER:
                labels[i] = usrid;
                break;

            case ID_COL_GROUP:
                labels[i] = grpid;
                break;

            case ID_COL_PERMS:
                labels[i] = perms;
                break;

            case ID_COL_LINK:
                if (linkpath.length() != 0)
                {
                    labels[i] = "-> " + linkpath;
                }
                else
                {
                    labels[i] = linkpath;                    
                }
                break;
            }
        }

        // Pathname is always at last position
        labels[NMAX_COLS + 2] = pathname;
    }
}


// Update the list (used in a search list)
FXbool FileList::updateItems(FXbool force)
{
    FXString grpid, usrid, perms, linkpath, mod, ext, del;
    FXString filename, dirname, pathname;
    FileItem* item;
    FileAssoc* fileassoc;
    FXString filetype, lowext;
    FXIcon* big, * mini;
    FXIcon* bigthumb = NULL, * minithumb = NULL;
    FXIconSource* source;
    time_t filemtime, filectime;
    struct stat info, linfo;
    FXbool isLink, isBrokenLink, isLinkToDir;
    FXbool updated = false;

    // Loop over the item list
    for (int u = 0; u < getNumItems(); u++)
    {
        // Current item
        item = (FileItem*)getItem(u);
        pathname = getItemFullPathname(u);
        dirname = FXPath::directory(pathname);
        filename = FXPath::name(pathname);

        // Get file/link info and indicate if it is a link
        if (xf_lstat(pathname.text(), &linfo) != 0)
        {
            removeItem(u, true);
            u--;
            continue;
        }
        isLink = S_ISLNK(linfo.st_mode);

        // Find if it is a broken link or a link to a directory
        isBrokenLink = false;
        isLinkToDir = false;
        if (isLink)
        {
            if (xf_stat(pathname.text(), &info) != 0)
            {
                isBrokenLink = true;
            }
            else if (S_ISDIR(info.st_mode))
            {
                isLinkToDir = true;
            }

            linkpath = xf_cleanpath(xf_readlink(pathname));
        }
        else
        {
            linkpath = "";
        }

        // If not a directory, nor a link to a directory and we want only directories, skip it
        if (!isLinkToDir && !S_ISDIR(linfo.st_mode) && (options & FILELIST_SHOWDIRS))
        {
            continue;
        }

        // File times
        filemtime = linfo.st_mtime;
        filectime = linfo.st_ctime;

        // Update only if the item has changed (mtime or ctime)
        if (force || (item->date != filemtime) || (item->cdate != filectime))
        {
            // Indicate that the list was updated
            updated = true;

            // Obtain user name
            usrid = FXSystem::userName(linfo.st_uid);

            // Obtain group name
            grpid = FXSystem::groupName(linfo.st_gid);

            // Permissions (caution : we don't use the FXSystem::modeString() function because
            // it seems to be incompatible with the info.st_mode format)
            perms = xf_permissions(linfo.st_mode);

            // Mod time
            mod = FXSystem::time("%x %X", filemtime);

            del = "";
            ext = "";
            // Obtain the extension for files only
            if (!S_ISDIR(linfo.st_mode))
            {
                ext = FXPath::name(pathname).rafter('.', 2).lower();
                if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") || (ext == "tar.zst") ||
                    (ext == "tar.z")) // Special cases
                {
                    // Do nothing
                }
                else
                {
                    ext = FXPath::extension(pathname).lower();
                }
            }

            // Obtain the stat info on the file itself
            if (xf_stat(pathname.text(), &info) != 0)
            {
                // Except in the case of a broken link
                if (isBrokenLink)
                {
                    int ret = xf_lstat(pathname.text(), &info);
                    if (ret < 0)  // Should not happen
                    {
                        fprintf(stderr, "%s", strerror(errno));
                    }
                }
                else
                {
                    removeItem(u, true);
                    u--;
                    continue;
                }
            }

            // Set item flags from the obtained info
            if (S_ISDIR(info.st_mode))
            {
                item->state |= FileItem::FOLDER;
            }
            else
            {
                item->state &= ~FileItem::FOLDER;
            }
            if (S_ISLNK(info.st_mode))
            {
                item->state |= FileItem::SYMLINK;
            }
            else
            {
                item->state &= ~FileItem::SYMLINK;
            }
            if (S_ISCHR(info.st_mode))
            {
                item->state |= FileItem::CHARDEV;
            }
            else
            {
                item->state &= ~FileItem::CHARDEV;
            }
            if (S_ISBLK(info.st_mode))
            {
                item->state |= FileItem::BLOCKDEV;
            }
            else
            {
                item->state &= ~FileItem::BLOCKDEV;
            }
            if (S_ISFIFO(info.st_mode))
            {
                item->state |= FileItem::FIFO;
            }
            else
            {
                item->state &= ~FileItem::FIFO;
            }
            if (S_ISSOCK(info.st_mode))
            {
                item->state |= FileItem::SOCK;
            }
            else
            {
                item->state &= ~FileItem::SOCK;
            }
            if ((info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) && !(S_ISDIR(info.st_mode) || S_ISCHR(info.st_mode) ||
                                                                    S_ISBLK(info.st_mode) || S_ISFIFO(info.st_mode) ||
                                                                    S_ISSOCK(info.st_mode)))
            {
                item->state |= FileItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FileItem::EXECUTABLE;
            }

            // We can drag items
            item->state |= FileItem::DRAGGABLE;

            // Assume no associations
            fileassoc = NULL;

            // Determine icons and type
            if (item->state & FileItem::FOLDER)
            {
                if (!xf_isreadexecutable(pathname))
                {
                    big = bigfolderlockedicon;
                    mini = minifolderlockedicon;
                    filetype = _("Folder");
                }
                else
                {
                    big = bigfoldericon;
                    mini = minifoldericon;
                    filetype = _("Folder");
                }
            }
            else if (item->state & FileItem::CHARDEV)
            {
                big = bigchardevicon;
                mini = minichardevicon;
                filetype = _("Character Device");
            }
            else if (item->state & FileItem::BLOCKDEV)
            {
                big = bigblockdevicon;
                mini = miniblockdevicon;
                filetype = _("Block Device");
            }
            else if (item->state & FileItem::FIFO)
            {
                big = bigpipeicon;
                mini = minipipeicon;
                filetype = _("Named Pipe");
            }
            else if (item->state & FileItem::SOCK)
            {
                big = bigsocketicon;
                mini = minisocketicon;
                filetype = _("Socket");
            }
            else if (item->state & FileItem::EXECUTABLE)
            {
                big = bigexecicon;
                mini = miniexecicon;
                filetype = _("Executable");
                if (associations)
                {
                    fileassoc = associations->findFileBinding(pathname.text());
                }
            }
            else
            {
                big = bigdocicon;
                mini = minidocicon;
                filetype = _("Document");
                if (associations)
                {
                    fileassoc = associations->findFileBinding(pathname.text());
                }
            }

            // If association is found, use it
            if (fileassoc)
            {
                // Don't use associations when the file name is also an extension name (ex: zip, rar, tar, etc.)
                // and is not executable
                if (!(FXPath::name(pathname) == fileassoc->key) || !(item->state & FileItem::EXECUTABLE))
                {
                    filetype = fileassoc->extension.text();

                    if (fileassoc->bigicon)
                    {
                        big = fileassoc->bigicon;
                    }
                    if (fileassoc->miniicon)
                    {
                        mini = fileassoc->miniicon;
                    }
                }
            }

            // Symbolic links have a specific type
            if (isBrokenLink)
            {
                filetype = _("Broken Link");
            }
            else if (isLink)
            {
                // Referred file is executable
                if (item->state & FileItem::EXECUTABLE)
                {
                    filetype = FXString(_("Link to ")) + _("Executable");
                }
                else
                {
                    if (associations)
                    {
                        // Don't forget to remove trailing '/' here!
                        fileassoc = associations->findFileBinding(linkpath.text());
                        if (fileassoc && (fileassoc->extension != ""))
                        {
                            filetype = _("Link to ") + fileassoc->extension;

                            if (fileassoc->bigicon)
                            {
                                big = fileassoc->bigicon;
                            }
                            if (fileassoc->miniicon)
                            {
                                mini = fileassoc->miniicon;
                            }
                        }
                        // If no association found, get the link file type from the referred file type
                        else
                        {
                            if (item->state & FileItem::FOLDER)
                            {
                                filetype = _("Folder");
                            }
                            else if (item->state & FileItem::CHARDEV)
                            {
                                filetype = _("Character Device");
                            }
                            else if (item->state & FileItem::BLOCKDEV)
                            {
                                filetype = _("Block Device");
                            }
                            else if (item->state & FileItem::FIFO)
                            {
                                filetype = _("Named Pipe");
                            }
                            else if (item->state & FileItem::SOCK)
                            {
                                filetype = _("Socket");
                            }
                            else if (item->state & FileItem::EXECUTABLE)
                            {
                                filetype = _("Executable");
                            }
                            else
                            {
                                filetype = _("Document");
                            }

                            filetype = _("Link to ") + filetype;
                        }
                    }
                }
            }

            // Don't display the file size for directories
            FXString hsize;
            if (S_ISDIR(linfo.st_mode))
            {
                hsize = "";
            }
            else
            {
                char size[64];
#if __WORDSIZE == 64
                snprintf(size, sizeof(size), "%lu", (FXulong)linfo.st_size);
#else
                snprintf(size, sizeof(size), "%llu", (FXulong)linfo.st_size);
#endif
                hsize = xf_humansize(size);
            }

            // Set item icons
            if (isLink)
            {
                if (isBrokenLink)
                {
                    item->setBigIcon(bigbrokenlinkicon);
                    item->setMiniIcon(minibrokenlinkicon);
                }
                else
                {
                    item->setBigIcon(big, false, biglinkbadgeicon);
                    item->setMiniIcon(mini, false, minilinkbadgeicon);
                }
            }
            else
            {
                item->setBigIcon(big);
                item->setMiniIcon(mini);
            }

            // Attempt to load thumbnails for image files
            if (displaythumbnails)
            {
                // Scaled max sizes
                FXuint bigthumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "bigthumb_size", 32);
                FXuint minithumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "minithumb_size", 16);
                FXuint max_bigthumb_size = scalefrac * bigthumb_size;
                FXuint max_minithumb_size = scalefrac * minithumb_size;

                // Load big icon from file
                bigthumb = NULL;
                minithumb = NULL;
                if (associations)
                {
                    source = associations->getIconDict()->getIconSource();
                    if (!(item->state & FileItem::FIFO)) // Avoid pipes
                    {
                        bigthumb = source->loadIconFile(pathname);
                    }
                }

                if (bigthumb)
                {
                    FXuint w = bigthumb->getWidth();
                    FXuint h = bigthumb->getHeight();

                    // Possibly scale the big icon (best quality)
                    if ((w > max_bigthumb_size) || (h > max_bigthumb_size))
                    {
                        if (w > h)
                        {
                            bigthumb->scale(max_bigthumb_size, (max_bigthumb_size * h) / w, 1);
                        }
                        else
                        {
                            bigthumb->scale((max_bigthumb_size * w) / h, max_bigthumb_size, 1);
                        }

                        // Size has changed
                        w = bigthumb->getWidth();
                        h = bigthumb->getHeight();
                    }

                    // Copy the big icon to the mini icon (faster than direct rescaling)
                    minithumb = new FXIcon(getApp());
                    FXColor* tmpdata;
                    if (!FXMEMDUP(&tmpdata, bigthumb->getData(), FXColor, w * h))
                    {
                        throw FXMemoryException(_("Unable to load image"));
                    }
                    minithumb->setData(tmpdata, IMAGE_OWNED, w, h);

                    // Possibly scale the mini icon (best quality)
                    w = minithumb->getWidth();
                    h = minithumb->getHeight();
                    if ((w > max_minithumb_size) || (h > max_minithumb_size))
                    {
                        if (w > h)
                        {
                            minithumb->scale(max_minithumb_size, (max_minithumb_size * h) / w, 1);
                        }
                        else
                        {
                            minithumb->scale((max_minithumb_size * w) / h, max_minithumb_size, 1);
                        }
                    }

                    // Set thumbnail icons as owned
                    if (!isBrokenLink)
                    {
                        if (isLink)
                        {
                            item->setBigIcon(bigthumb, true, biglinkbadgeicon);
                            item->setMiniIcon(minithumb, true, minilinkbadgeicon);
                        }
                        else
                        {
                            item->setBigIcon(bigthumb, true);
                            item->setMiniIcon(minithumb, true);
                        }
                    }
                }
            }

            // Set other item attributes
            item->size = (FXulong)linfo.st_size;
            item->assoc = fileassoc;
            item->date = filemtime;
            item->cdate = filectime;

#if defined(linux)
            // Devices have a specific icon
            if (fsdevices->find(pathname.text()))
            {
                filetype = _("Mount Point");

                if (xf_strequal(fsdevices->find(pathname.text()), "harddrive"))
                {
                    item->setBigIcon(bigharddriveicon);
                    item->setMiniIcon(miniharddriveicon);
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "nfsdisk"))
                {
                    item->setBigIcon(bignetdriveicon);
                    item->setMiniIcon(mininetdriveicon);
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "smbdisk"))
                {
                    item->setBigIcon(bignetdriveicon);
                    item->setMiniIcon(mininetdriveicon);
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "floppy"))
                {
                    item->setBigIcon(bigfloppyicon);
                    item->setMiniIcon(minifloppyicon);
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "cdrom"))
                {
                    item->setBigIcon(bigcdromicon);
                    item->setMiniIcon(minicdromicon);
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "zip"))
                {
                    item->setBigIcon(bigzipdiskicon);
                    item->setMiniIcon(minizipdiskicon);
                }
            }
#endif

            // Update item label
            // NB : Item del is empty if we are not in trash can
            //      Item pathname is not displayed but is used in the tooltip
            if (dirname.length() > 256)
            {
                dirname = dirname.trunc(256) + "[...]"; // Truncate directory path name to 256 characters
                                                        // to prevent overflow
            }

            // Order labels
            setItemLabels(idCol, idColTrash, false, filename, dirname, hsize, filetype, ext, mod, usrid, grpid, perms,
                          linkpath, "", del, pathname, labels);

            // Set labels
            item->label.format("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s",
                               labels[0].text(), labels[1].text(), labels[2].text(), labels[3].text(),
                               labels[4].text(), labels[5].text(), labels[6].text(), labels[7].text(),
                               labels[8].text(), labels[9].text(), labels[10].text(), labels[11].text());
        }

        // Finally don't forget to create the item!
        item->create();

        // Refresh the GUI if an image has to be drawn and recompute the icon height
        /*
         * if (displaythumbnails)
         * {
         *  if (updated && bigthumb && minithumb && (u < REFRESH_COUNT))
         *  {
         *      update();
         *      recompute();
         *      makeItemVisible(0); // Fix some refresh problems
         *      repaint();
         *  }
         * }
         */
    }

    // Gotta recalc size of content
    if (updated)
    {
        recalc();
    }

    return updated;
}


// List directory (used in a standard file list)
void FileList::listItems(FXbool force)
{
    FileItem* oldlist, * newlist;
    FileItem** po, ** pn, ** pp;
    FXString grpid, usrid, perms, linkpath, mod, ext, del, origpath;
    FXString name, dirname, pathname;
    FileItem* curitem = NULL;
    FileItem* item, * link;
    FileAssoc* fileassoc;
    FXString filetype, lowext, timeformat;
    FXIconSource* source;
    FXIcon* big, * mini;
    FXIcon* bigthumb = NULL, * minithumb = NULL;
    time_t filemtime, filectime;
    struct stat info, linfo;
    struct dirent* dp;
    DIR* dirp;
    FXbool isLink, isBrokenLink, isLinkToDir;
    FXlong deldate;

    // Read time format
    timeformat = getApp()->reg().readStringEntry("SETTINGS", "time_format", DEFAULT_TIME_FORMAT);

    // Build old and new insert-order lists
    oldlist = list;
    newlist = NULL;

    // Head of old and new lists
    po = &oldlist;
    pn = &newlist;

    // Remember current item
    if (0 <= current)
    {
        curitem = (FileItem*)items[current];
    }

    // Start inserting
    items.clear();

    // Get info about directory
    if (xf_stat(directory.text(), &info) == 0)
    {
        // Need latest change no matter what actually changed!
        timestamp = FXMAX(info.st_mtime, info.st_ctime);

        // Set path to stat with
        dirname = directory.text();

        if (dirname != ROOTDIR)
        {
            dirname += PATHSEPSTRING;
        }

        // Add the deletion time and original path headers if we are in trash can
        if (!(options & FILELIST_SEARCH) && (dirname.left(trashfileslocation.length()) == trashfileslocation))
        {
            if (getNumHeaders() == (int)nbCols)
            {
                // Remove all headers and get their size
                FXuint size[NMAX_COLS + 2];
                for (FXuint i = 0; i < nbCols; i++)
                {
                    size[i] = header->getItemSize(0);
                    removeHeader(0);
                }

                // Append headers and set their size
                FXuint j = 0;
                for (FXuint i = 0; i < nbColsTrash; i++)
                {
                    if (idColTrash[i] == ID_COL_ORIGPATH)
                    {
                        appendHeader(getHeaderName(idColTrash[i]), NULL, origpath_size);
                    }
                    else if (idColTrash[i] == ID_COL_DELDATE)
                    {
                        appendHeader(getHeaderName(idColTrash[i]), NULL, deldate_size);
                    }
                    else
                    {
                        appendHeader(getHeaderName(idColTrash[i]), NULL, size[j]);
                        j++;
                    }
                }
            }
            inTrash = true;
            onUpdHeader(0, 0, 0); // Force update header
        }
        // If needed remove the deletion and original path headers if we are not in trash can
        else
        {
            // Not in a search list
            if (!(options & FILELIST_SEARCH) && (getNumHeaders() == (int)nbColsTrash))
            {
                origpath_size = header->getItemSize(1);
                deldate_size = header->getItemSize(2);
                removeHeader(1);
                removeHeader(1);

                // Change back the sort function to default if necessary
                if ((sortfunc == ascendingOrigpath) || (sortfunc == ascendingOrigpathMix) ||
                    (sortfunc == descendingOrigpath) || (sortfunc == descendingOrigpathMix))
                {
                    sortfunc = ascendingCase;
                    setSortHeader(getHeaderIndex(ID_COL_NAME));
                }
                if ((sortfunc == ascendingDeldate) || (sortfunc == ascendingDeldateMix) ||
                    (sortfunc == descendingDeldate) || (sortfunc == descendingDeldateMix))
                {
                    sortfunc = ascendingCase;
                    setSortHeader(getHeaderIndex(ID_COL_NAME));
                }
            }
            inTrash = false;
            onUpdHeader(0, 0, 0); // Force update header
        }

        // Get directory stream pointer
        dirp = opendir(directory.text());

        // Managed to open directory
        if (dirp)
        {
            // Loop over directory entries
            while ((dp = readdir(dirp)) != NULL)
            {
                // Directory entry name
                name = dp->d_name;

                // Hidden file (.xxx) or directory (. or .yyy) normally not shown,
                // but directory .. is always shown so we can navigate up or down
                // Hidden files in the trash can base directory are always shown
                if ((name[0] == '.') && ((name[1] == 0) || (!((name[1] == '.') && (name[2] == 0)) &&
                                                            !(options & FILELIST_SHOWHIDDEN) &&
                                                            (dirname != trashfileslocation + PATHSEPSTRING))))
                {
                    continue;
                }

                // Build full pathname
                pathname = dirname + name;

                // Get file/link info and indicate if it is a link
                if (xf_lstat(pathname.text(), &linfo) != 0)
                {
                    continue;
                }
                isLink = S_ISLNK(linfo.st_mode);

                // Find if it is a broken link or a link to a directory
                isBrokenLink = false;
                isLinkToDir = false;
                if (isLink)
                {
                    if (xf_stat(pathname.text(), &info) != 0)
                    {
                        isBrokenLink = true;
                    }
                    else if (S_ISDIR(info.st_mode))
                    {
                        isLinkToDir = true;
                    }

                    linkpath = xf_cleanpath(xf_readlink(pathname));
                }
                else
                {
                    linkpath = "";
                }

                // If not a directory, nor a link to a directory and we want only directories, skip it
                if (!isLinkToDir && !S_ISDIR(linfo.st_mode) && (options & FILELIST_SHOWDIRS))
                {
                    continue;
                }

                // If directory or link to directory and filter folders option, skip it
                if ((name != "..") && (isLinkToDir || S_ISDIR(linfo.st_mode)) && filter_folders &&
                    !FXPath::match(pattern, name, matchmode))
                {
                    continue;
                }

                // Does it match the filter pattern?
                if ((!isLinkToDir && !S_ISDIR(linfo.st_mode)) && (name != ".." &&
                    !FXPath::match(pattern, name, matchmode)))
                {
                    continue;
                }

                // File times
                filemtime = linfo.st_mtime;
                filectime = linfo.st_ctime;

                // Find it, and take it out from the old list if found
                for (pp = po; (item = *pp) != NULL; pp = &item->link)
                {
                    if (fileequal(item->label, name))
                    {
                        *pp = item->link;
                        item->link = NULL;
                        po = pp;
                        goto fnd;
                    }
                }

                // Make new item if we have to
                item = (FileItem*)createItem(FXString::null, NULL, NULL, NULL);

                // Append item in list
fnd:
                *pn = item;
                pn = &item->link;

                // Append
                if (item == curitem)
                {
                    current = items.no();
                }
                items.append(item);

                // Update only if forced, or if the item has changed (mtime or ctime)
                if (force || (item->date != filemtime) || (item->cdate != filectime))
                {
                    // Obtain user name
                    usrid = FXSystem::userName(linfo.st_uid);

                    // Obtain group name
                    grpid = FXSystem::groupName(linfo.st_gid);

                    // Permissions (caution : we don't use the FXSystem::modeString() function because
                    // it seems to be incompatible with the info.st_mode format)
                    perms = xf_permissions(linfo.st_mode);

                    // Mod time
                    mod = FXSystem::time(timeformat.text(), filemtime);

                    // If we are in trash can, obtain the deletion time and the original path
                    deldate = 0;
                    del = "";
                    ext = "";
                    origpath = "";
                    FXString delstr = "";
                    if (inTrash)
                    {
                        // Obtain trash base name and sub path
                        FXString subpath = dirname;
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
                        if ((fp = fopen(trashinfopathname.text(), "r")) != NULL)
                        {
                            // Read the first three lines and get the strings
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
                                origpath = line;
                                origpath = origpath.after('=');
                                origpath = origpath.before('\n');
                            }
                            if (fgets(line, sizeof(line), fp) == NULL)
                            {
                                success = false;
                            }
                            if (success)
                            {
                                delstr = line;
                                delstr = delstr.after('=');
                                delstr = delstr.before('\n');
                            }
                            fclose(fp);
                        }

                        // Possibly include sub path in the original path
                        if (subpath == "")
                        {
                            origpath = origpath + subpath;
                        }
                        else
                        {
                            origpath = origpath + subpath + name;
                        }
                        if (delstr == "")
                        {
                            origpath = "";
                        }

                        // Special case
                        if (name == "..")
                        {
                            origpath = "";
                            delstr = "";
                        }

                        // Convert date
                        deldate = xf_deltime(delstr);
                        if (deldate != 0)
                        {
                            del = FXSystem::time(timeformat.text(), deldate);
                        }

                        // Obtain the extension for files only
                        if (!S_ISDIR(linfo.st_mode))
                        {
                            ext = "";
                            if (dirname == trashfileslocation)
                            {
                                ext = FXPath::extension(pathname.rbefore('_'));
                            }
                            if (ext == "")
                            {
                                ext = FXPath::name(pathname).rafter('.', 2).lower();
                                if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") ||
                                    (ext == "tar.zst") || (ext == "tar.z")) // Special cases
                                {
                                    // Do nothing
                                }
                                else
                                {
                                    ext = FXPath::extension(pathname).lower();
                                }
                            }
                        }
                    }
                    else
                    {
                        // Obtain the extension for files only
                        if (!S_ISDIR(linfo.st_mode))
                        {
                            ext = FXPath::name(pathname).rafter('.', 2).lower();
                            if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") ||
                                (ext == "tar.zst") || (ext == "tar.z")) // Special cases
                            {
                                // Do nothing
                            }
                            else
                            {
                                ext = FXPath::extension(pathname).lower();
                            }
                        }
                    }

                    // Obtain the stat info on the file or the referred file if it is a link
                    if (xf_stat(pathname.text(), &info) != 0)
                    {
                        // Except in the case of a broken link
                        if (isBrokenLink)
                        {
                            xf_lstat(pathname.text(), &info);
                        }
                        else
                        {
                            continue;
                        }
                    }

                    // Set item flags from the obtained info or linfo
                    if (S_ISDIR(info.st_mode))
                    {
                        item->state |= FileItem::FOLDER;
                    }
                    else
                    {
                        item->state &= ~FileItem::FOLDER;
                    }
                    if (S_ISLNK(linfo.st_mode))
                    {
                        item->state |= FileItem::SYMLINK;
                    }
                    else
                    {
                        item->state &= ~FileItem::SYMLINK;
                    }
                    if (S_ISCHR(info.st_mode))
                    {
                        item->state |= FileItem::CHARDEV;
                    }
                    else
                    {
                        item->state &= ~FileItem::CHARDEV;
                    }
                    if (S_ISBLK(info.st_mode))
                    {
                        item->state |= FileItem::BLOCKDEV;
                    }
                    else
                    {
                        item->state &= ~FileItem::BLOCKDEV;
                    }
                    if (S_ISFIFO(info.st_mode))
                    {
                        item->state |= FileItem::FIFO;
                    }
                    else
                    {
                        item->state &= ~FileItem::FIFO;
                    }
                    if (S_ISSOCK(info.st_mode))
                    {
                        item->state |= FileItem::SOCK;
                    }
                    else
                    {
                        item->state &= ~FileItem::SOCK;
                    }
                    if ((info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) &&
                        !(S_ISDIR(info.st_mode) || S_ISCHR(info.st_mode) ||
                          S_ISBLK(info.st_mode) ||
                          S_ISFIFO(info.st_mode) || S_ISSOCK(info.st_mode)))
                    {
                        item->state |= FileItem::EXECUTABLE;
                    }
                    else
                    {
                        item->state &= ~FileItem::EXECUTABLE;
                    }

                    // We can drag items
                    item->state |= FileItem::DRAGGABLE;

                    // Assume no associations
                    fileassoc = NULL;

                    // Determine icons and type
                    if (item->state & FileItem::FOLDER)
                    {
                        if (!xf_isreadexecutable(pathname))
                        {
                            big = bigfolderlockedicon;
                            mini = minifolderlockedicon;
                            filetype = _("Folder");
                        }
                        else
                        {
                            big = bigfoldericon;
                            mini = minifoldericon;
                            filetype = _("Folder");
                        }
                    }
                    else if (item->state & FileItem::CHARDEV)
                    {
                        big = bigchardevicon;
                        mini = minichardevicon;
                        filetype = _("Character Device");
                    }
                    else if (item->state & FileItem::BLOCKDEV)
                    {
                        big = bigblockdevicon;
                        mini = miniblockdevicon;
                        filetype = _("Block Device");
                    }
                    else if (item->state & FileItem::FIFO)
                    {
                        big = bigpipeicon;
                        mini = minipipeicon;
                        filetype = _("Named Pipe");
                    }
                    else if (item->state & FileItem::SOCK)
                    {
                        big = bigsocketicon;
                        mini = minisocketicon;
                        filetype = _("Socket");
                    }
                    else if (item->state & FileItem::EXECUTABLE)
                    {
                        big = bigexecicon;
                        mini = miniexecicon;
                        filetype = _("Executable");
                        if (associations)
                        {
                            fileassoc = associations->findFileBinding(pathname.text());
                        }
                    }
                    else
                    {
                        big = bigdocicon;
                        mini = minidocicon;
                        filetype = _("Document");
                        if (associations)
                        {
                            // Possibly strip the '_' suffix when we are in trash root
                            if (dirname == trashfileslocation)
                            {
                                FXString stripname = pathname.rbefore('_');
                                if (stripname == "")
                                {
                                    fileassoc = associations->findFileBinding(pathname.text());
                                }
                                else
                                {
                                    fileassoc = associations->findFileBinding(stripname.text());
                                }
                            }
                            else
                            {
                                fileassoc = associations->findFileBinding(pathname.text());
                            }
                        }
                    }

                    // If association is found, use it
                    if (fileassoc)
                    {
                        // Don't use associations when the file name is also an extension name (ex: zip, rar, tar, etc.)
                        // and is not executable
                        if (!(FXPath::name(pathname) == fileassoc->key) || !(item->state & FileItem::EXECUTABLE))
                        {
                            if ((fileassoc->extension != "") || !(item->state & FileItem::EXECUTABLE))
                            {
                                filetype = fileassoc->extension.text();
                            }
                            if (fileassoc->bigicon)
                            {
                                big = fileassoc->bigicon;
                            }
                            if (fileassoc->miniicon)
                            {
                                mini = fileassoc->miniicon;
                            }
                        }
                    }

                    // Symbolic links have a specific type
                    if (isBrokenLink)
                    {
                        filetype = _("Broken Link");
                    }
                    else if (isLink)
                    {
                        // Referred file is executable
                        if (item->state & FileItem::EXECUTABLE)
                        {
                            filetype = FXString(_("Link to ")) + _("Executable");
                        }
                        else
                        {
                            if (associations)
                            {
                                // Don't forget to remove trailing '/' here!
                                fileassoc = associations->findFileBinding(linkpath.text());
                                if (fileassoc && (fileassoc->extension != ""))
                                {
                                    filetype = _("Link to ") + fileassoc->extension;

                                    if (fileassoc->bigicon)
                                    {
                                        big = fileassoc->bigicon;
                                    }
                                    if (fileassoc->miniicon)
                                    {
                                        mini = fileassoc->miniicon;
                                    }
                                }
                                // If no association found, get the link file type from the referred file type
                                else
                                {
                                    if (item->state & FileItem::FOLDER)
                                    {
                                        filetype = _("Folder");
                                    }
                                    else if (item->state & FileItem::CHARDEV)
                                    {
                                        filetype = _("Character Device");
                                    }
                                    else if (item->state & FileItem::BLOCKDEV)
                                    {
                                        filetype = _("Block Device");
                                    }
                                    else if (item->state & FileItem::FIFO)
                                    {
                                        filetype = _("Named Pipe");
                                    }
                                    else if (item->state & FileItem::SOCK)
                                    {
                                        filetype = _("Socket");
                                    }
                                    else if (item->state & FileItem::EXECUTABLE)
                                    {
                                        filetype = _("Executable");
                                    }
                                    else
                                    {
                                        filetype = _("Document");
                                    }

                                    filetype = _("Link to ") + filetype;
                                }
                            }
                        }
                    }

                    // Don't display the file size for directories
                    FXString hsize;
                    if (S_ISDIR(linfo.st_mode))
                    {
                        hsize = "";
                    }
                    else
                    {
                        char size[64];
#if __WORDSIZE == 64
                        snprintf(size, sizeof(size), "%lu", (FXulong)linfo.st_size);
#else
                        snprintf(size, sizeof(size), "%llu", (FXulong)linfo.st_size);
#endif
                        hsize = xf_humansize(size);
                    }

                    // Set item icons
                    if (isLink)
                    {
                        if (isBrokenLink)
                        {
                            item->setBigIcon(bigbrokenlinkicon);
                            item->setMiniIcon(minibrokenlinkicon);
                        }
                        else
                        {
                            item->setBigIcon(big, false, biglinkbadgeicon);
                            item->setMiniIcon(mini, false, minilinkbadgeicon);
                        }
                    }
                    else
                    {
                        item->setBigIcon(big);
                        item->setMiniIcon(mini);
                    }

                    // Attempt to load thumbnails for image files
                    if (displaythumbnails)
                    {
                        // Scaled max sizes
                        FXuint bigthumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "bigthumb_size", 32);
                        FXuint minithumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "minithumb_size", 16);
                        FXuint max_bigthumb_size = scalefrac * bigthumb_size;
                        FXuint max_minithumb_size = scalefrac * minithumb_size;

                        // Load big icon from file
                        bigthumb = NULL;
                        minithumb = NULL;
                        if (associations)
                        {
                            source = associations->getIconDict()->getIconSource();
                            if (!(item->state & FileItem::FIFO)) // Avoid pipes
                            {
                                bigthumb = source->loadIconFile(pathname);
                            }
                        }

                        if (bigthumb)
                        {
                            FXuint w = bigthumb->getWidth();
                            FXuint h = bigthumb->getHeight();

                            // Possibly scale the big icon (best quality)
                            if ((w > max_bigthumb_size) || (h > max_bigthumb_size))
                            {
                                if (w > h)
                                {
                                    bigthumb->scale(max_bigthumb_size, (max_bigthumb_size * h) / w, 1);
                                }
                                else
                                {
                                    bigthumb->scale((max_bigthumb_size * w) / h, max_bigthumb_size, 1);
                                }

                                // Size has changed
                                w = bigthumb->getWidth();
                                h = bigthumb->getHeight();
                            }

                            // Copy the big icon to the mini icon (faster than direct rescaling)
                            minithumb = new FXIcon(getApp());
                            FXColor* tmpdata;
                            if (!FXMEMDUP(&tmpdata, bigthumb->getData(), FXColor, w * h))
                            {
                                throw FXMemoryException(_("Unable to load image"));
                            }
                            minithumb->setData(tmpdata, IMAGE_OWNED, w, h);

                            // Possibly scale the mini icon (best quality)
                            w = minithumb->getWidth();
                            h = minithumb->getHeight();
                            if ((w > max_minithumb_size) || (h > max_minithumb_size))
                            {
                                if (w > h)
                                {
                                    minithumb->scale(max_minithumb_size, (max_minithumb_size * h) / w, 1);
                                }
                                else
                                {
                                    minithumb->scale((max_minithumb_size * w) / h, max_minithumb_size, 1);
                                }
                            }

                            // Set thumbnail icons as owned
                            if (!isBrokenLink)
                            {
                                if (isLink)
                                {
                                    item->setBigIcon(bigthumb, true, biglinkbadgeicon);
                                    item->setMiniIcon(minithumb, true, minilinkbadgeicon);
                                }
                                else
                                {
                                    item->setBigIcon(bigthumb, true);
                                    item->setMiniIcon(minithumb, true);
                                }
                            }
                        }
                    }

                    // Set other item attributes
                    item->size = (FXulong)linfo.st_size;
                    item->assoc = fileassoc;
                    item->date = filemtime;
                    item->cdate = filectime;
                    item->deldate = deldate;

#if defined(linux)
                    // Mounted devices may have a specific icon
                    if (mtdevices->find(pathname.text()))
                    {
                        filetype = _("Mount Point");

                        if (xf_strequal(mtdevices->find(pathname.text()), "cifs"))
                        {
                            item->setBigIcon(bignetdriveicon);
                            item->setMiniIcon(mininetdriveicon);
                        }
                        else
                        {
                            item->setBigIcon(bigharddriveicon);
                            item->setMiniIcon(miniharddriveicon);
                        }
                    }

                    // Devices found in fstab may have a specific icon
                    if (fsdevices->find(pathname.text()))
                    {
                        filetype = _("Mount Point");

                        if (xf_strequal(fsdevices->find(pathname.text()), "harddrive"))
                        {
                            item->setBigIcon(bigharddriveicon);
                            item->setMiniIcon(miniharddriveicon);
                        }
                        else if (xf_strequal(fsdevices->find(pathname.text()), "nfsdisk"))
                        {
                            item->setBigIcon(bignetdriveicon);
                            item->setMiniIcon(mininetdriveicon);
                        }
                        else if (xf_strequal(fsdevices->find(pathname.text()), "smbdisk"))
                        {
                            item->setBigIcon(bignetdriveicon);
                            item->setMiniIcon(mininetdriveicon);
                        }
                        else if (xf_strequal(fsdevices->find(pathname.text()), "floppy"))
                        {
                            item->setBigIcon(bigfloppyicon);
                            item->setMiniIcon(minifloppyicon);
                        }
                        else if (xf_strequal(fsdevices->find(pathname.text()), "cdrom"))
                        {
                            item->setBigIcon(bigcdromicon);
                            item->setMiniIcon(minicdromicon);
                        }
                        else if (xf_strequal(fsdevices->find(pathname.text()), "zip"))
                        {
                            item->setBigIcon(bigzipdiskicon);
                            item->setMiniIcon(minizipdiskicon);
                        }
                    }
#endif

                    // Order labels
                    setItemLabels(idCol, idColTrash, inTrash, name, "", hsize, filetype, ext, mod, usrid, grpid,
                                  perms, linkpath, origpath, del, pathname, labels);

                    // Set labels
                    // NB : Item del is empty if we are not in trash can
                    //      Item pathname is not displayed but is used in the tooltip

                    item->label.format("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s",
                                       labels[0].text(), labels[1].text(), labels[2].text(), labels[3].text(),
                                       labels[4].text(), labels[5].text(), labels[6].text(), labels[7].text(),
                                       labels[8].text(), labels[9].text(), labels[10].text(), labels[11].text());

                    // Dotdot folders have a specific icon
                    if ((name[0] == '.') && (name[1] == '.') && (name[2] == 0))
                    {
                        item->setBigIcon(bigfolderupicon);
                        item->setMiniIcon(minifolderupicon);
                    }
                }

                // Create item
                if (id())
                {
                    item->create();
                }

                // Refresh the GUI if an image has to be drawn and recompute the icon height
                // Don't redraw if there are too many images
                if (displaythumbnails)
                {
                    if (bigthumb && minithumb && (items.no() < REFRESH_COUNT))
                    {
                        update();
                        recompute();
                        makeItemVisible(0); // Fix some refresh problems
                        repaint();
                    }
                }
            }
            closedir(dirp);
        }
    }

    // Wipe items remaining in list:- they have disappeared!!
    for (item = oldlist; item; item = link)
    {
        link = item->link;
        delete item;
    }

    // Validate
    if (current >= items.no())
    {
        current = -1;
    }
    if (anchor >= items.no())
    {
        anchor = -1;
    }
    if (extent >= items.no())
    {
        extent = -1;
    }

    // Remember new list
    list = newlist;

    // Gotta recalc size of content
    recalc();
}
