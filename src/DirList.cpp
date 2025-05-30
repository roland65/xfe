// Directory list. Taken from the FOX library and slightly modified.
// The compare(), compare_nolocale() and compare_locale() functions are adapted from a patch
// submitted by Vladimir Támara Patiño

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <FXPNGIcon.h>
#if defined(linux)
#include <mntent.h>
#endif

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "File.h"
#include "FileDict.h"
#include "InputDialog.h"
#include "MessageBox.h"
#include "DirList.h"


#define SELECT_MASK    (TREELIST_SINGLESELECT | TREELIST_BROWSESELECT)

// Interval between updevices and mtdevices read (s)
#define UPDEVICES_INTERVAL    300
#define MTDEVICES_INTERVAL    5

// Interval between refreshes (ms)
#define REFRESH_INTERVAL      1000

// File systems not supporting mod-time, refresh every nth time
#define REFRESH_FREQUENCY     30

// Time interval before expanding a folder (ms)
#define EXPAND_INTERVAL       500

// Global variables
extern FXString xdgdatahome;

#if defined(linux)
extern FXStringDict* fsdevices;
extern FXStringDict* mtdevices;
extern FXStringDict* updevices;
#endif



// Object implementation
FXIMPLEMENT(DirItem, FXTreeItem, NULL, 0)



// Map
FXDEFMAP(DirList) DirListMap[] =
{
    FXMAPFUNC(SEL_DRAGGED, 0, DirList::onDragged),
    FXMAPFUNC(SEL_TIMEOUT, DirList::ID_REFRESH_TIMER, DirList::onCmdRefreshTimer),
#if defined(linux)
    FXMAPFUNC(SEL_TIMEOUT, DirList::ID_MTDEVICES_REFRESH, DirList::onMtdevicesRefresh),
    FXMAPFUNC(SEL_TIMEOUT, DirList::ID_UPDEVICES_REFRESH, DirList::onUpdevicesRefresh),
#endif
    FXMAPFUNC(SEL_TIMEOUT, DirList::ID_EXPAND_TIMER, DirList::onExpandTimer),
    FXMAPFUNC(SEL_DND_ENTER, 0, DirList::onDNDEnter),
    FXMAPFUNC(SEL_DND_LEAVE, 0, DirList::onDNDLeave),
    FXMAPFUNC(SEL_DND_DROP, 0, DirList::onDNDDrop),
    FXMAPFUNC(SEL_DND_MOTION, 0, DirList::onDNDMotion),
    FXMAPFUNC(SEL_DND_REQUEST, 0, DirList::onDNDRequest),
    FXMAPFUNC(SEL_BEGINDRAG, 0, DirList::onBeginDrag),
    FXMAPFUNC(SEL_ENDDRAG, 0, DirList::onEndDrag),
    FXMAPFUNC(SEL_OPENED, 0, DirList::onOpened),
    FXMAPFUNC(SEL_CLOSED, 0, DirList::onClosed),
    FXMAPFUNC(SEL_EXPANDED, 0, DirList::onExpanded),
    FXMAPFUNC(SEL_COLLAPSED, 0, DirList::onCollapsed),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_SHOW_HIDDEN, DirList::onUpdShowHidden),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_HIDE_HIDDEN, DirList::onUpdHideHidden),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_TOGGLE_HIDDEN, DirList::onUpdToggleHidden),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_SHOW_FILES, DirList::onUpdShowFiles),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_HIDE_FILES, DirList::onUpdHideFiles),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_TOGGLE_FILES, DirList::onUpdToggleFiles),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_SET_PATTERN, DirList::onUpdSetPattern),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_SORT_REVERSE, DirList::onUpdSortReverse),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_SHOW_HIDDEN, DirList::onCmdShowHidden),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_DRAG_COPY, DirList::onCmdDragCopy),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_DRAG_MOVE, DirList::onCmdDragMove),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_DRAG_LINK, DirList::onCmdDragLink),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_DRAG_REJECT, DirList::onCmdDragReject),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_HIDE_HIDDEN, DirList::onCmdHideHidden),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_TOGGLE_HIDDEN, DirList::onCmdToggleHidden),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_SHOW_FILES, DirList::onCmdShowFiles),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_HIDE_FILES, DirList::onCmdHideFiles),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_TOGGLE_FILES, DirList::onCmdToggleFiles),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_SET_PATTERN, DirList::onCmdSetPattern),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_SORT_REVERSE, DirList::onCmdSortReverse),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_REFRESH, DirList::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, DirList::ID_SORT_CASE, DirList::onCmdSortCase),
    FXMAPFUNC(SEL_UPDATE, DirList::ID_SORT_CASE, DirList::onUpdSortCase),
    FXMAPFUNC(SEL_UPDATE, 0, DirList::onUpdRefreshTimers),
    FXMAPFUNC(SEL_QUERY_TIP, 0, DirList::onQueryTip)
};


// Object implementation
FXIMPLEMENT(DirList, FXTreeList, DirListMap, ARRAYNUMBER(DirListMap))


// Construct
DirList::DirList(FXWindow* focuswin, FXComposite* p, FXObject* tgt, FXSelector sel,
                 FXuint opts, int x, int y, int w, int h) :
    FXTreeList(p, tgt, sel, opts, x, y, w, h), pattern("*")
{
    flags |= FLAG_ENABLED | FLAG_DROPTARGET;
    matchmode = FILEMATCH_FILE_NAME | FILEMATCH_NOESCAPE;
    associations = NULL;
    if (!(options & DIRLIST_NO_OWN_ASSOC))
    {
        associations = new FileDict(getApp());
    }
    list = NULL;
    sortfunc = (FXTreeListSortFunc)ascendingCase;
    dropaction = DRAG_MOVE;
    counter = 0;
    prevSelItem = NULL;
    focuswindow = focuswin;

#if defined(linux)

    // Initialize the fsdevices, mtdevices and updevices lists
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
                if (xf_keepmount(mnt->mnt_dir, mnt->mnt_fsname))
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

    // Read settings
    file_tooltips = getApp()->reg().readUnsignedEntry("SETTINGS", "file_tooltips", true);
}


// Create the directory list
void DirList::create()
{
    FXTreeList::create();
    if (!deleteType)
    {
        deleteType = getApp()->registerDragType(deleteTypeName);
    }
    if (!urilistType)
    {
        urilistType = getApp()->registerDragType(urilistTypeName);
    }
    getApp()->addTimeout(this, ID_REFRESH_TIMER, REFRESH_INTERVAL);
#if defined(linux)
    getApp()->addTimeout(this, ID_MTDEVICES_REFRESH, MTDEVICES_INTERVAL * 1000);
    getApp()->addTimeout(this, ID_UPDEVICES_REFRESH, UPDEVICES_INTERVAL * 1000);
#endif
    dropEnable();

    // Scan root directory
    scan(false);
}


// Expand folder tree when hovering long over a folder
long DirList::onExpandTimer(FXObject* sender, FXSelector sel, void* ptr)
{
    int xx, yy;
    FXuint state;
    DirItem* item;

    getCursorPosition(xx, yy, state);
    item = (DirItem*)getItemAt(xx, yy);

    if (!(item->state & DirItem::FOLDER))
    {
        return 0;
    }

    // Expand tree item
    expandDirTree((TreeItem*)item, true);
    scan(true);

    // Set open timer
    getApp()->addTimeout(this, ID_EXPAND_TIMER, EXPAND_INTERVAL);

    return 1;
}


// Create item
TreeItem* DirList::createItem(const FXString& text, FXIcon* oi, FXIcon* ci, void* ptr)
{
    return (TreeItem*)new DirItem(text, oi, ci, ptr);
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
static inline int compare_nolocale(char* p, char* q, FXbool igncase, FXbool asc)
{
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
    int ret = xf_comparenat(p, q, igncase);

    // Restore saved characters
    *pp = pw;
    *qq = qw;

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
static inline int compare_locale(wchar_t* p, wchar_t* q, FXbool igncase, FXbool asc)
{
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
    int ret = xf_comparewnat(p, q, igncase);

    // Restore saved characters
    *pp = pw;
    *qq = qw;

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
 *
 * @return 0 if equal, negative if pa<pb, positive if pa>pb
 * Requires to allocate some space, if there is no memory this
 * function returns 0 and errno will be ENOMEM
 * If jmp has an invalid value returns 0 and errno will be EINVAL
 */
int DirList::compareItem(const FXTreeItem* pa, const FXTreeItem* pb, FXbool igncase, FXbool asc)
{
    const DirItem* a = (DirItem*)pa;
    const DirItem* b = (DirItem*)pb;
    char* p = (char*)a->label.text();
    char* q = (char*)b->label.text();

    // Prepare wide char strings
    wchar_t* wa = NULL;
    wchar_t* wb = NULL;
    size_t an, bn;

    an = mbstowcs(NULL, (const char*)p, 0);
    if (an == (size_t)-1)
    {
        return compare_nolocale(p, q, igncase, asc); // If error, fall back to no locale comparison
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
        return compare_nolocale(p, q, igncase, asc); // If error, fall back to no locale comparison
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
    int ret = compare_locale(wa, wb, igncase, asc);

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


// Sort ascending order, keeping directories first
int DirList::ascending(const FXTreeItem* pa, const FXTreeItem* pb)
{
    return compareItem(pa, pb, false, true);
}


// Sort descending order, keeping directories first
int DirList::descending(const FXTreeItem* pa, const FXTreeItem* pb)
{
    return compareItem(pa, pb, false, false);
}


// Sort ascending order, case insensitive, keeping directories first
int DirList::ascendingCase(const FXTreeItem* pa, const FXTreeItem* pb)
{
    return compareItem(pa, pb, true, true);
}


// Sort descending order, case insensitive, keeping directories first
int DirList::descendingCase(const FXTreeItem* pa, const FXTreeItem* pb)
{
    return compareItem(pa, pb, true, false);
}


// Handle drag-and-drop enter
long DirList::onDNDEnter(FXObject* sender, FXSelector sel, void* ptr)
{
    FXTreeList::onDNDEnter(sender, sel, ptr);
    return 1;
}


// Handle drag-and-drop leave
long DirList::onDNDLeave(FXObject* sender, FXSelector sel, void* ptr)
{
    // Cancel open up timer
    getApp()->removeTimeout(this, ID_EXPAND_TIMER);

    stopAutoScroll();
    FXTreeList::onDNDLeave(sender, sel, ptr);
    if (prevSelItem)
    {
        if (!isItemCurrent(prevSelItem))
        {
            closeItem(prevSelItem);
        }
        prevSelItem = NULL;
    }
    return 1;
}


// Handle drag-and-drop motion
long DirList::onDNDMotion(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    TreeItem* item;

    // Cancel open up timer
    getApp()->removeTimeout(this, ID_EXPAND_TIMER);

    // Start autoscrolling
    if (startAutoScroll(event, false))
    {
        return 1;
    }

    // Give base class a shot
    if (FXTreeList::onDNDMotion(sender, sel, ptr))
    {
        return 1;
    }

    // Dropping list of filenames
    if (offeredDNDType(FROM_DRAGNDROP, urilistType))
    {
        // Locate drop place
        item = (TreeItem*)getItemAt(event->win_x, event->win_y);

        // We can drop in a directory
        if (item && isItemDirectory(item))
        {
            // Get drop directory
            dropdirectory = getItemPathname(item);

            // What is being done (move,copy,link)
            dropaction = inquireDNDAction();

            // Set open up timer
            getApp()->addTimeout(this, ID_EXPAND_TIMER, EXPAND_INTERVAL);

            // Set icon to open folder icon
            setItemOpenIcon(item, minifolderopenicon);

            // See if this is writable
            if (xf_iswritable(dropdirectory))
            {
                acceptDrop(DRAG_ACCEPT);
                int x, y;
                FXuint state;
                getCursorPosition(x, y, state);
                TreeItem* item = (TreeItem*)getItemAt(x, y);

                if (prevSelItem && (prevSelItem != item))
                {
                    if (!isItemCurrent(prevSelItem))
                    {
                        closeItem(prevSelItem);
                        prevSelItem->setSelected(FALSE);
                    }
                    prevSelItem = NULL;
                }
                if (item && (prevSelItem != item))
                {
                    openItem(item);
                    item->setSelected(TRUE);
                    prevSelItem = item;
                }
            }
        }
        return 1;
    }
    return 0;
}


// Set drag type to copy
long DirList::onCmdDragCopy(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_COPY;
    return 1;
}


// Set drag type to move
long DirList::onCmdDragMove(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_MOVE;
    return 1;
}


// Set drag type to symlink
long DirList::onCmdDragLink(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_LINK;
    return 1;
}


// Cancel drag action
long DirList::onCmdDragReject(FXObject* sender, FXSelector sel, void* ptr)
{
    dropaction = DRAG_REJECT;
    return 1;
}


// Handle drag-and-drop drop
long DirList::onDNDDrop(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuchar* data;
    FXuint len;
    FXbool showdialog = true;
    int ret;
    File* f = NULL;

    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);
    FXbool confirm_dnd = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_drag_and_drop", true);

    // Cancel open up timer
    getApp()->removeTimeout(this, ID_EXPAND_TIMER);

    // Stop scrolling
    stopAutoScroll();

    // Perhaps target wants to deal with it
    if (FXTreeList::onDNDDrop(sender, sel, ptr))
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
    // This is done before displaying the popup menu to fix a drag and drop problem with konqueror and dolphin file managers
    FXbool dnd = getDNDData(FROM_DRAGNDROP, urilistType, data, len);

    int xx, yy;
    DirItem* item = NULL;

    // Display the dnd dialog if the control or shift key were not pressed
    if (confirm_dnd && !ctrlshiftkey)
    {
        // Get item
        FXuint state;
        getCursorPosition(xx, yy, state);
        item = (DirItem*)getItemAt(xx, yy);

        // Display a popup to select the drag type
        dropaction = DRAG_REJECT;
        FXMenuPane menu(this);
        int x, y;
        getRoot()->getCursorPosition(x, y, state);
        new FXMenuCommand(&menu, _("Copy Here"), minicopyicon, this, DirList::ID_DRAG_COPY);
        new FXMenuCommand(&menu, _("Move Here"), minimoveicon, this, DirList::ID_DRAG_MOVE);
        new FXMenuCommand(&menu, _("Link Here"), minilinktoicon, this, DirList::ID_DRAG_LINK);
        new FXMenuSeparator(&menu);
        new FXMenuCommand(&menu, _("Cancel"), NULL, this, DirList::ID_DRAG_REJECT);
        menu.create();
        menu.popup(NULL, x, y);
        getApp()->runModalWhileShown(&menu);
    }

    // Close item
    if (prevSelItem)
    {
        if (!isItemCurrent(prevSelItem))
        {
            closeItem(prevSelItem);
        }
        prevSelItem = NULL;
    }

    // Get uri-list of files being dropped
    //if (getDNDData(FROM_DRAGNDROP,urilistType,data,len))
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
            if (item)
            {
                item->setSelected(FALSE);
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
            FXString source(FXURL::fileFromURL(url));
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
                        message.format(_("Copy %s files/folders from: %s"), FXStringVal(num).text(), sourcedir.text());
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
                        message.format(_("Move %s files/folders from: %s"), FXStringVal(num).text(), sourcedir.text());
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

                // If source file is located at trash location, try to also remove the corresponding trashinfo if it exists
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

                // Set directory to the source parent
                setDirectory(sourcedir, false);
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
                f->create();
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

        // Deselect item
        if (item)
        {
            item->setSelected(FALSE);
        }

        // Force a refresh of the DirList
        onCmdRefresh(0, 0, 0);

        return 1;
    }
    return 0;
}


// Somebody wants our dragged data
long DirList::onDNDRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuchar* data;
    FXuint len;

    // Perhaps the target wants to supply its own data
    if (FXTreeList::onDNDRequest(sender, sel, ptr))
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
long DirList::onBeginDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    TreeItem* item;

    if (FXTreeList::onBeginDrag(sender, sel, ptr))
    {
        return 1;
    }
    if (beginDrag(&urilistType, 1))
    {
        dragfiles = FXString::null;
        item = (TreeItem*)firstitem;
        while (item)
        {
            if (item->isSelected())
            {
                if (!dragfiles.empty())
                {
                    dragfiles += "\r\n";
                }
                dragfiles += FXURL::encode(::xf_filetouri(getItemPathname(item)));
            }
            if (item->first)
            {
                item = (TreeItem*)item->first;
            }
            else
            {
                while (!item->next && item->parent)
                {
                    item = (TreeItem*)item->parent;
                }
                item = (TreeItem*)item->next;
            }
        }
        return 1;
    }
    return 0;
}


// End drag operation
long DirList::onEndDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXTreeList::onEndDrag(sender, sel, ptr))
    {
        return 1;
    }
    endDrag((didAccept() != DRAG_REJECT));
    setDragCursor(getDefaultCursor());

    return 1;
}


// Dragged stuff around
long DirList::onDragged(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXDragAction action;

    if (FXTreeList::onDragged(sender, sel, ptr))
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


// Toggle hidden files
long DirList::onCmdToggleHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(!shownHiddenFiles());
    return 1;
}


// Update toggle hidden files widget
long DirList::onUpdToggleHidden(FXObject* sender, FXSelector, void*)
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
long DirList::onCmdShowHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(true);
    return 1;
}


// Update show hidden files widget
long DirList::onUpdShowHidden(FXObject* sender, FXSelector, void*)
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
long DirList::onCmdHideHidden(FXObject*, FXSelector, void*)
{
    showHiddenFiles(false);
    return 1;
}


// Update hide hidden files widget
long DirList::onUpdHideHidden(FXObject* sender, FXSelector, void*)
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


// Toggle files display
long DirList::onCmdToggleFiles(FXObject*, FXSelector, void*)
{
    showFiles(!showFiles());
    return 1;
}


// Update toggle files widget
long DirList::onUpdToggleFiles(FXObject* sender, FXSelector, void*)
{
    if (showFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Show files
long DirList::onCmdShowFiles(FXObject*, FXSelector, void*)
{
    showFiles(true);
    return 1;
}


// Update show files widget
long DirList::onUpdShowFiles(FXObject* sender, FXSelector, void*)
{
    if (showFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Hide files
long DirList::onCmdHideFiles(FXObject*, FXSelector, void*)
{
    showFiles(false);
    return 1;
}


// Update hide files widget
long DirList::onUpdHideFiles(FXObject* sender, FXSelector, void*)
{
    if (!showFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Change pattern
long DirList::onCmdSetPattern(FXObject*, FXSelector, void* ptr)
{
    if (!ptr)
    {
        return 0;
    }
    setPattern((const char*)ptr);
    return 1;
}


// Update pattern
long DirList::onUpdSetPattern(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SETVALUE), (void*)pattern.text());
    return 1;
}


// Reverse sort order
long DirList::onCmdSortReverse(FXObject*, FXSelector, void*)
{
    if (sortfunc == (FXTreeListSortFunc)ascending)
    {
        sortfunc = (FXTreeListSortFunc)descending;
    }
    else if (sortfunc == (FXTreeListSortFunc)descending)
    {
        sortfunc = (FXTreeListSortFunc)ascending;
    }
    else if (sortfunc == (FXTreeListSortFunc)ascendingCase)
    {
        sortfunc = (FXTreeListSortFunc)descendingCase;
    }
    else if (sortfunc == (FXTreeListSortFunc)descendingCase)
    {
        sortfunc = (FXTreeListSortFunc)ascendingCase;
    }
    scan(true);
    return 1;
}


// Update sender
long DirList::onUpdSortReverse(FXObject* sender, FXSelector, void* ptr)
{
    sender->handle(this,
                   (sortfunc == (FXTreeListSortFunc)descending || sortfunc == (FXTreeListSortFunc)descendingCase) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
    return 1;
}


// Toggle case sensitivity
long DirList::onCmdSortCase(FXObject*, FXSelector, void*)
{
    if (sortfunc == (FXTreeListSortFunc)ascending)
    {
        sortfunc = (FXTreeListSortFunc)ascendingCase;
    }
    else if (sortfunc == (FXTreeListSortFunc)descending)
    {
        sortfunc = (FXTreeListSortFunc)descendingCase;
    }
    else if (sortfunc == (FXTreeListSortFunc)ascendingCase)
    {
        sortfunc = (FXTreeListSortFunc)ascending;
    }
    else if (sortfunc == (FXTreeListSortFunc)descendingCase)
    {
        sortfunc = (FXTreeListSortFunc)descending;
    }
    scan(true);
    return 1;
}


// Check if case sensitive
long DirList::onUpdSortCase(FXObject* sender, FXSelector, void* ptr)
{
    sender->handle(this,
                   (sortfunc == (FXTreeListSortFunc)ascendingCase || sortfunc == (FXTreeListSortFunc)descendingCase) ?
                   FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK), ptr);
    return 1;
}


// Close directory
long DirList::onClosed(FXObject*, FXSelector, void* ptr)
{
    DirItem* item = (DirItem*)ptr;

    if (item->state & DirItem::FOLDER)
    {
        return target && target->handle(this, FXSEL(SEL_CLOSED, message), ptr);
    }

    return 1;
}


// Open directory
long DirList::onOpened(FXObject*, FXSelector, void* ptr)
{
    DirItem* item = (DirItem*)ptr;

    if (item->state & DirItem::FOLDER)
    {
        return target && target->handle(this, FXSEL(SEL_OPENED, message), ptr);
    }
    return 1;
}


// Item opened
long DirList::onExpanded(FXObject* sender, FXSelector sel, void* ptr)
{
    DirItem* item = (DirItem*)ptr;

    if (!(item->state & DirItem::FOLDER))
    {
        return 0;
    }

    // Expand tree item
    expandDirTree((TreeItem*)item, true);
    listChildItems(item);

    // Now we know for sure whether we really have subitems or not
    if (!item->first)
    {
        item->state &= ~DirItem::HASITEMS;
    }
    else
    {
        item->state |= DirItem::HASITEMS;
    }

    sortChildItems(item);
    return 1;
}


// Item closed
long DirList::onCollapsed(FXObject* sender, FXSelector sel, void* ptr)
{
    DirItem* item = (DirItem*)ptr;

    if (!(item->state & DirItem::FOLDER))
    {
        return 0;
    }

    // Collapse tree item
    collapseDirTree((TreeItem*)item, true);

    return 1;
}


// Expand directory tree
FXbool DirList::expandDirTree(TreeItem* tree, FXbool notify)
{
    if (FXTreeList::expandTree(tree, notify))
    {
        if (isItemDirectory(tree))
        {
            listChildItems((DirItem*)tree);
            sortChildItems(tree);
        }
        return true;
    }
    return false;
}


// Collapse directory tree
FXbool DirList::collapseDirTree(TreeItem* tree, FXbool notify)
{
    if (FXTreeList::collapseTree(tree, notify))
    {
        if (isItemDirectory(tree))
        {
            // As a memory saving feature, all knowledge below this item
            // is deleted; we'll just recreate it when its reexpanded!
            FXTreeList::removeItems(tree->first, tree->last);
            recalc();
        }
        return true;
    }
    return false;
}


#if defined(linux)

// To periodically scan /proc/mounts and refresh the mtdevices list
long DirList::onMtdevicesRefresh(FXObject*, FXSelector, void*)
{
    // Don't refresh if window is minimized
    if (((FXTopWindow*)focuswindow)->isMinimized())
    {
        return 0;
    }

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

    // Restart timer
    getApp()->addTimeout(this, ID_MTDEVICES_REFRESH, MTDEVICES_INTERVAL * 1000);
    return 0;
}


// To periodically scan /proc/mounts and detect up and down mounted devices
// NB : the refresh period is much longer than for onMtdevicesRefresh
long DirList::onUpdevicesRefresh(FXObject*, FXSelector, void*)
{
    // Don't refresh if window is minimized
    if (((FXTopWindow*)focuswindow)->isMinimized())
    {
        return 0;
    }

    struct mntent* mnt;
    FXString mtstate;

    FXbool mount_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_warn", true);

    FXStringDict* tmpdict = new FXStringDict();
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
            tmpdict->insert(mntlist[i].text(), "");

            if (xf_statvfs_timeout(mntlist[i].text(), timeout) == 1)
            {
                mtstate = "down";
                if (mount_warn)
                {
                    MessageBox::warning(this, BOX_OK, _("Warning"), _("Mount point %s is not responding..."),
                                        mntlist[i].text());
                }
            }
            else
            {
                mtstate = "up";
            }

            if (updevices->find(mntlist[i].text()))
            {
                updevices->remove(mntlist[i].text());
            }
            updevices->insert(mntlist[i].text(), mtstate.text());
        }
    }

    // Remove mount points that don't exist anymore
    int s;
    for (s = updevices->first(); s < updevices->size(); s = updevices->next(s))
    {
        if (!tmpdict->find(updevices->key(s)))
        {
            updevices->remove(updevices->key(s));
        }
    }
    delete tmpdict;

    // Restart timer
    getApp()->addTimeout(this, ID_UPDEVICES_REFRESH, UPDEVICES_INTERVAL * 1000);
    return 0;
}


#endif


// Refresh with timer
long DirList::onCmdRefreshTimer(FXObject*, FXSelector, void*)
{
    // Don't refresh if window is minimized
    if (((FXTopWindow*)focuswindow)->isMinimized())
    {
        return 0;
    }

    if (flags & FLAG_UPDATE)
    {
        scan(false);
        counter = (counter + 1) % REFRESH_FREQUENCY;
    }

    // Restart timer
    getApp()->addTimeout(this, ID_REFRESH_TIMER, REFRESH_INTERVAL);
    return 0;
}


// Force refresh
long DirList::onCmdRefresh(FXObject*, FXSelector, void*)
{
    scan(true);
    return 0;
}


// Scan items to see if listing is necessary
void DirList::scan(FXbool force)
{
    FXString pathname;
    struct stat info;
    DirItem* item;

    // Do root first time
    if (!firstitem || force)
    {
        listRootItems();
        sortRootItems();
    }

    // Check all items
    item = (DirItem*)firstitem;
    while (item)
    {
        // Is expanded directory?
        if (item->isDirectory() && item->isExpanded())
        {
            // Get the full path of the item
            pathname = getItemPathname((TreeItem*)item);

            // Stat this directory
            if (xf_stat(pathname.text(), &info) == 0)
            {
                // Get the mod date of the item
                FXTime newdate = (FXTime)FXMAX(info.st_mtime, info.st_ctime);

                // Forced, date was changed, or failed to get proper date and counter expired
                if (force || (item->date != newdate) || (counter == 0))
                {
                    // And do the refresh
#if defined(linux)
                    onMtdevicesRefresh(0, 0, 0); // Force mtdevices list refresh
#endif
                    listChildItems(item);
                    sortChildItems(item);

                    // Remember when we did this
                    item->date = newdate;
                }

                // Go deeper
                if (item->first)
                {
                    item = (DirItem*)item->first;
                    continue;
                }
            }
            // Directory does not exist
            else
            {
                // Go to parent and rescan
                setDirectory(FXPath::directory(pathname), false);
                scan(true);
                break;
            }
        }

        // Go up
        while (!item->next && item->parent)
        {
            item = (DirItem*)item->parent;
        }

        // Go to next
        item = (DirItem*)item->next;
    }
}


// List root directories
void DirList::listRootItems()
{
    DirItem* item = (DirItem*)firstitem;
    FXIcon* openicon, * closedicon;
    FileAssoc* fileassoc;

    // First time, make root node
    if (!item)
    {
        item = list = (DirItem*)appendItem(NULL, PATHSEPSTRING, miniharddriveicon, miniharddriveicon, NULL, true);
    }

    // Root is a directory, has items under it, and is searchable
    item->state |= DirItem::FOLDER | DirItem::HASITEMS;
    item->state &= ~(DirItem::CHARDEV | DirItem::BLOCKDEV | DirItem::FIFO | DirItem::SOCK | DirItem::SYMLINK |
                     DirItem::EXECUTABLE);

    // Determine associations, icons and type
    fileassoc = NULL;
    openicon = miniharddriveicon;
    closedicon = miniharddriveicon;
    if (associations)
    {
        fileassoc = associations->findDirBinding(PATHSEPSTRING);
    }

    // If association is found, use it
    if (fileassoc)
    {
        if (fileassoc->miniicon)
        {
            closedicon = fileassoc->miniicon;
        }
        if (fileassoc->miniiconopen)
        {
            openicon = fileassoc->miniiconopen;
        }
    }

    // Update item information
    item->openIcon = openicon;
    item->closedIcon = closedicon;
    item->size = 0L;
    item->assoc = fileassoc;
    item->date = 0;

    // Create item
    if (id())
    {
        item->create();
    }

    // Need to layout
    recalc();
}


// List child items
void DirList::listChildItems(DirItem* par)
{
    DirItem* oldlist, * newlist, ** po, ** pp, ** pn, * item, * link;
    FXIcon* openicon, * closedicon;
    FileAssoc* fileassoc;
    DIR* dirp;
    struct dirent* dp;
    struct stat info;
    FXString pathname, directory, name;
    FXString type, mod, usrid, grpid, perms, del;
    FXString timeformat;
    int islink;
    FXlong deldate;

    // Path to parent node
    directory = getItemPathname((TreeItem*)par);

    // Read time format
    timeformat = getApp()->reg().readStringEntry("SETTINGS", "time_format", DEFAULT_TIME_FORMAT);

    // Build new insert-order list
    oldlist = par->list;
    newlist = NULL;

    // Assemble lists
    po = &oldlist;
    pn = &newlist;

    // Get directory stream pointer
    dirp = opendir(directory.text());

    // Managed to open directory
    if (dirp)
    {
        // Process directory entries
        while ((dp = readdir(dirp)) != NULL)
        {
            // Get name of entry
            name = dp->d_name;

            // A dot special file?
            if ((name[0] == '.') && ((name[1] == 0) || ((name[1] == '.') && (name[2] == 0))))
            {
                continue;
            }

            // Hidden file or directory normally not shown
            if ((name[0] == '.') && !(options & DIRLIST_SHOWHIDDEN))
            {
                continue;
            }

            // Build full pathname of entry
            pathname = directory;
            if (!ISPATHSEP(pathname[pathname.length() - 1]))
            {
                pathname += PATHSEPSTRING;
            }
            pathname += name;

            // Get file/link info
            if (xf_lstat(pathname.text(), &info) != 0)
            {
                continue;
            }

            // If its a link, get the info on file itself
            islink = S_ISLNK(info.st_mode);
            if (islink && (xf_stat(pathname.text(), &info) != 0))
            {
                continue;
            }

            // If it is not a directory, and not showing files and matching pattern skip it
            if (!S_ISDIR(info.st_mode) && !((options & DIRLIST_SHOWFILES) && FXPath::match(pattern, name, matchmode)))
            {
                continue;
            }

            // Find it, and take it out from the old list if found
            for (pp = po; (item = *pp) != NULL; pp = &item->link)
            {
                if (compare(item->label, name) == 0)
                {
                    *pp = item->link;
                    item->link = NULL;
                    po = pp;
                    goto fnd;
                }
            }

            // Not found; prepend before list
            item = (DirItem*)appendItem(par, name, minifolderopenicon, minifoldericon, NULL, true);

            // Next gets hung after this one
fnd:
            *pn = item;
            pn = &item->link;

            // Item flags
            if (info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            {
                item->state |= DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~DirItem::EXECUTABLE;
            }

            if (S_ISDIR(info.st_mode))
            {
                item->state |= DirItem::FOLDER;
                item->state &= ~DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~(DirItem::FOLDER | DirItem::HASITEMS);
            }

            if (S_ISCHR(info.st_mode))
            {
                item->state |= DirItem::CHARDEV;
                item->state &= ~DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~DirItem::CHARDEV;
            }

            if (S_ISBLK(info.st_mode))
            {
                item->state |= DirItem::BLOCKDEV;
                item->state &= ~DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~DirItem::BLOCKDEV;
            }

            if (S_ISFIFO(info.st_mode))
            {
                item->state |= DirItem::FIFO;
                item->state &= ~DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~DirItem::FIFO;
            }

            if (S_ISSOCK(info.st_mode))
            {
                item->state |= DirItem::SOCK;
                item->state &= ~DirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~DirItem::SOCK;
            }

            if (islink)
            {
                item->state |= DirItem::SYMLINK;
            }
            else
            {
                item->state &= ~DirItem::SYMLINK;
            }

            // We can drag items
            item->state |= DirItem::DRAGGABLE;

            // Assume no associations
            fileassoc = NULL;

            // Determine icons and type
            if (item->state & DirItem::FOLDER)
            {
                if (!xf_isreadexecutable(pathname))
                {
                    openicon = minifolderlockedicon;
                    closedicon = minifolderlockedicon;
                }
                else
                {
                    openicon = minifolderopenicon;
                    closedicon = minifoldericon;
                }
                if (associations)
                {
                    fileassoc = associations->findDirBinding(pathname.text());
                }
            }
            else if (item->state & DirItem::EXECUTABLE)
            {
                openicon = miniappicon;
                closedicon = miniappicon;
                if (associations)
                {
                    fileassoc = associations->findExecBinding(pathname.text());
                }
            }
            else
            {
                openicon = minidocicon;
                closedicon = minidocicon;
                if (associations)
                {
                    fileassoc = associations->findFileBinding(pathname.text());
                }
            }

            // If association is found, use it
            if (fileassoc)
            {
                if (fileassoc->miniicon)
                {
                    closedicon = fileassoc->miniicon;
                }
                if (fileassoc->miniiconopen)
                {
                    openicon = fileassoc->miniiconopen;
                }
            }

            // Update item information
            item->openIcon = openicon;
            item->closedIcon = closedicon;
            item->size = (FXulong)info.st_size;
            item->assoc = fileassoc;
            item->date = info.st_mtime;

            // Set the HASITEMS flag (too slow on network drives)
            //(xf_hassubdirs(pathname.text()) == 1 ? item->setHasItems(true) : item->setHasItems(false));

            // Default folder type
            type = _("Folder");

            // Obtain user name
            FXString usrid = FXSystem::userName(info.st_uid);

            // Obtain group name
            FXString grpid = FXSystem::groupName(info.st_gid);

            // Permissions (caution : we don't use the FXSystem::modeString() function because
            // it seems to be incompatible with the info.st_mode format)
            FXString perms = xf_permissions(info.st_mode);

            // Modification time
            mod = FXSystem::time(timeformat.text(), item->date);

            // If we are in trash can, obtain the deletion time
            deldate = 0;
            del = "";
            if (FXPath::directory(pathname) == trashfileslocation)
            {
                char* endptr;
                FXString str;
                str = pathname.rafter('_');
                str = str.rbefore('-');
                deldate = strtol(str.text(), &endptr, 10);
                if (deldate != 0)
                {
                    del = FXSystem::time(timeformat.text(), deldate);
                }
            }

#if defined(linux)
            // Mounted devices may have a specific icon
            if (mtdevices->find(pathname.text()))
            {
                type = _("Mount Point");

                if (xf_strequal(mtdevices->find(pathname.text()), "cifs"))
                {
                    item->closedIcon = mininetdriveicon;
                    item->openIcon = mininetdriveicon;
                }
                else
                {
                    item->closedIcon = miniharddriveicon;
                    item->openIcon = miniharddriveicon;
                }
            }

            // Devices found in fstab may have a specific icon
            if (fsdevices->find(pathname.text()))
            {
                type = _("Mount Point");

                if (xf_strequal(fsdevices->find(pathname.text()), "harddrive"))
                {
                    item->closedIcon = miniharddriveicon;
                    item->openIcon = miniharddriveicon;
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "nfsdisk"))
                {
                    item->closedIcon = mininetdriveicon;
                    item->openIcon = mininetdriveicon;
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "smbdisk"))
                {
                    item->closedIcon = mininetdriveicon;
                    item->openIcon = mininetdriveicon;
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "floppy"))
                {
                    item->closedIcon = minifloppyicon;
                    item->openIcon = minifloppyicon;
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "cdrom"))
                {
                    item->closedIcon = minicdromicon;
                    item->openIcon = minicdromicon;
                }
                else if (xf_strequal(fsdevices->find(pathname.text()), "zip"))
                {
                    item->closedIcon = minizipdiskicon;
                    item->openIcon = minizipdiskicon;
                }
            }
#endif

            // Symbolic links have a specific icon
            if (islink)
            {
                type = _("Link to Folder");
                item->closedIcon = minifolderlinkicon;
                item->openIcon = minifolderlinkicon;
            }

            // Data used to update the tooltip
            item->tdata = item->label + "\t" + type + "\t" + mod + "\t" + usrid + "\t" + grpid + "\t" + perms + "\t" +
                          del + "\t" + pathname;
            item->setData(&item->tdata);

            // Create item
            if (id())
            {
                item->create();
            }
        }

        // Close it
        closedir(dirp);
    }

    // Wipe items remaining in list:- they have disappeared!!
    for (item = oldlist; item; item = link)
    {
        link = item->link;
        removeItem(item, true);
    }

    // Now we know for sure whether we really have subitems or not
    if (par->first)
    {
        par->state |= DirItem::HASITEMS;
    }
    else
    {
        par->state &= ~DirItem::HASITEMS;
    }

    // Remember new list
    par->list = newlist;

    // Need to layout
    recalc();
}


// Is directory
FXbool DirList::isItemDirectory(const TreeItem* item) const
{
    if (item == NULL)
    {
        fprintf(stderr, "%s::isItemDirectory: item is NULL.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (item->state & DirItem::FOLDER) != 0;
}


// Is file
FXbool DirList::isItemFile(const TreeItem* item) const
{
    if (item == NULL)
    {
        fprintf(stderr, "%s::isItemFile: item is NULL.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (item->state & (DirItem::FOLDER | DirItem::CHARDEV | DirItem::BLOCKDEV | DirItem::FIFO | DirItem::SOCK)) ==
           0;
}


// Is executable
FXbool DirList::isItemExecutable(const TreeItem* item) const
{
    if (item == NULL)
    {
        fprintf(stderr, "%s::isItemExecutable: item is NULL.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return (item->state & DirItem::EXECUTABLE) != 0;
}


// Return absolute pathname of item
FXString DirList::getItemPathname(const TreeItem* item) const
{
    FXString pathname;

    if (item)
    {
        while (1)
        {
            pathname.prepend(item->getText());
            item = (TreeItem*)item->parent;
            if (!item)
            {
                break;
            }
            if (item->parent)
            {
                pathname.prepend(PATHSEP);
            }
        }
    }
    return pathname;
}


// Return the item from the absolute pathname
TreeItem* DirList::getPathnameItem(const FXString& path)
{
    TreeItem* item, * it;
    int beg = 0, end = 0;
    FXString name;

    if (!path.empty())
    {
        if (ISPATHSEP(path[0]))
        {
            end++;
        }
        if (beg < end)
        {
            name = path.mid(beg, end - beg);
            for (it = (TreeItem*)firstitem; it; it = (TreeItem*)it->next)
            {
                if (compare(name, it->getText()) == 0)
                {
                    goto x;
                }
            }
            listRootItems();
            sortRootItems();
            for (it = (TreeItem*)firstitem; it; it = (TreeItem*)it->next)
            {
                if (compare(name, it->getText()) == 0)
                {
                    goto x;
                }
            }
            return NULL;

x:
            item = it;
            FXASSERT(item);
            while (end < path.length())
            {
                beg = end;
                while (end < path.length() && !ISPATHSEP(path[end]))
                {
                    end++;
                }
                name = path.mid(beg, end - beg);
                for (it = (TreeItem*)item->first; it; it = (TreeItem*)it->next)
                {
                    if (compare(name, it->getText()) == 0)
                    {
                        goto y;
                    }
                }
                listChildItems((DirItem*)item);
                sortChildItems(item);
                for (it = (TreeItem*)item->first; it; it = (TreeItem*)it->next)
                {
                    if (compare(name, it->getText()) == 0)
                    {
                        goto y;
                    }
                }
                return item;

y:
                item = it;
                FXASSERT(item);
                if ((end < path.length()) && ISPATHSEP(path[end]))
                {
                    end++;
                }
            }
            FXASSERT(item);
            return item;
        }
    }
    return NULL;
}


// Obtain item's file name only
FXString DirList::getItemFilename(const TreeItem* item) const
{
    if (item == NULL)
    {
        fprintf(stderr, "%s::getItemFilename: item is NULL.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return item->label;
}


// Open all intermediate directories down toward given one
void DirList::setDirectory(const FXString& pathname, FXbool notify)
{
    if (!pathname.empty())
    {
        FXString path = FXPath::absolute(getItemPathname((TreeItem*)currentitem), pathname);

        while (!FXPath::isTopDirectory(path) && !xf_isdirectory(path))
        {
            path = FXPath::upLevel(path);
        }

        TreeItem* item = getPathnameItem(path);
        if (id())
        {
            layout();
        }
        makeItemVisible(item);
        setCurrentItem(item, notify);
    }
}


// Return directory part of path to current item
FXString DirList::getDirectory() const
{
    const TreeItem* item = (TreeItem*)currentitem;

    while (item)
    {
        if (item->state & DirItem::FOLDER)
        {
            return getItemPathname(item);
        }
        item = (TreeItem*)item->parent;
    }
    return "";
}


// Set current (dir/file) name path
void DirList::setCurrentFile(const FXString& pathname, FXbool notify)
{
    if (!pathname.empty())
    {
        FXString path = FXPath::absolute(getItemPathname((TreeItem*)currentitem), pathname);
        while (!FXPath::isTopDirectory(path) && !xf_existfile(path))
        {
            path = FXPath::upLevel(path);
        }
        TreeItem* item = getPathnameItem(path);
        if (id())
        {
            layout();
        }
        makeItemVisible(item);
        setCurrentItem(item, notify);
    }
}


// Get current (dir/file) name path
FXString DirList::getCurrentFile() const
{
    return getItemPathname((TreeItem*)currentitem);
}


// Get list style
FXbool DirList::showFiles() const
{
    return (options & DIRLIST_SHOWFILES) != 0;
}


// Change list style
void DirList::showFiles(FXbool showing)
{
    FXuint opts = options;

    if (showing)
    {
        opts |= DIRLIST_SHOWFILES;
    }
    else
    {
        opts &= ~DIRLIST_SHOWFILES;
    }
    if (options != opts)
    {
        options = opts;
        scan(true);
    }
    setFocus();
}


// Return true if showing hidden files
FXbool DirList::shownHiddenFiles() const
{
    return (options & DIRLIST_SHOWHIDDEN) != 0;
}


// Change show hidden files mode
void DirList::showHiddenFiles(FXbool showing)
{
    FXuint opts = options;

    if (showing)
    {
        opts |= DIRLIST_SHOWHIDDEN;
    }
    else
    {
        opts &= ~DIRLIST_SHOWHIDDEN;
    }
    if (opts != options)
    {
        options = opts;
        scan(true);
    }
    setFocus();
}


// Set associations
void DirList::setAssociations(FileDict* assoc)
{
    associations = assoc;
    scan(true);
}


// Set the pattern to filter
void DirList::setPattern(const FXString& ptrn)
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
void DirList::setMatchMode(FXuint mode)
{
    if (matchmode != mode)
    {
        matchmode = mode;
        scan(true);
    }
    setFocus();
}


// Cleanup
DirList::~DirList()
{
    clearItems();
    getApp()->removeTimeout(this, ID_REFRESH_TIMER);
    getApp()->removeTimeout(this, ID_EXPAND_TIMER);
#if defined(linux)
    getApp()->removeTimeout(this, ID_MTDEVICES_REFRESH);
    getApp()->removeTimeout(this, ID_UPDEVICES_REFRESH);
#endif
    if (!(options & DIRLIST_NO_OWN_ASSOC))
    {
        delete associations;
    }
    associations = (FileDict*)-1;
}


// Update refresh timers if the window gains focus
long DirList::onUpdRefreshTimers(FXObject*, FXSelector, void*)
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

    // Update refresh timers
    if ((prevMinimized == false) && (minimized == true))
    {
        onCmdRefreshTimer(0, 0, 0);
#if defined(linux)
        onMtdevicesRefresh(0, 0, 0);
        onUpdevicesRefresh(0, 0, 0);
#endif
    }

    return 1;
}


// Remove all siblings from [fm,to]
void DirList::removeItems(TreeItem* fm, TreeItem* to, FXbool notify)
{
    TreeItem* olditem = (TreeItem*)currentitem;
    TreeItem* prv;
    TreeItem* nxt;
    TreeItem* par;

    if (fm && to)
    {
        if (fm->parent != to->parent)
        {
            fxerror("%s::removeItems: arguments have different parent.\n", getClassName());
        }

        // Delete items
        while (1)
        {
            // Scan till end
            while (to->last)
            {
                to = (TreeItem*)to->last;
            }
            do
            {
                // Notify item will be deleted
                if (notify && target)
                {
                    target->tryHandle(this, FXSEL(SEL_DELETED, message), (void*)to);
                }

                // Remember hookups
                nxt = (TreeItem*)to->next;
                prv = (TreeItem*)to->prev;
                par = (TreeItem*)to->parent;

                // Adjust pointers; suggested by Alan Ott <ott@acusoft.com>
                if (anchoritem == to)
                {
                    anchoritem = par;
                    if (prv)
                    {
                        anchoritem = prv;
                    }
                    if (nxt)
                    {
                        anchoritem = nxt;
                    }
                }

                // !!! Hack to go back to parent when an item disappeared !!!
                if (currentitem == to)
                {
                    currentitem = par;
                    //if(prv) currentitem=prv;
                    //if(nxt) currentitem=nxt;
                }
                // !!! End of hack !!!

                if (extentitem == to)
                {
                    extentitem = par;
                    if (prv)
                    {
                        extentitem = prv;
                    }
                    if (nxt)
                    {
                        extentitem = nxt;
                    }
                }
                if (viewableitem == to)
                {
                    viewableitem = par;
                    if (prv)
                    {
                        viewableitem = prv;
                    }
                    if (nxt)
                    {
                        viewableitem = nxt;
                    }
                }

                // Remove item from list
                if (prv)
                {
                    prv->next = nxt;
                }
                else if (par)
                {
                    par->first = nxt;
                }
                else
                {
                    firstitem = nxt;
                }
                if (nxt)
                {
                    nxt->prev = prv;
                }
                else if (par)
                {
                    par->last = prv;
                }
                else
                {
                    lastitem = prv;
                }

                // Delete it
                delete to;

                // Was last one?
                if (to == fm)
                {
                    goto x;
                }
                to = par;
            }
            while (!prv);
            to = prv;
        }

        // Current item has changed
x:
        if (olditem != currentitem)
        {
            if (notify && target)
            {
                target->tryHandle(this, FXSEL(SEL_CHANGED, message), (void*)currentitem);
            }
        }

        // Deleted current item
        if (currentitem && (currentitem != olditem))
        {
            if (hasFocus())
            {
                currentitem->setFocus(true);
            }
            if (((options & SELECT_MASK) == TREELIST_BROWSESELECT) && currentitem->isEnabled())
            {
                selectItem(currentitem, notify);
            }
        }

        // Redo layout
        recalc();
    }
}


// Display a tooltip with name, size, date, etc.
long DirList::onQueryTip(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXWindow::onQueryTip(sender, sel, ptr))
    {
        return 1;
    }

    // File tooltips are optional
    if (file_tooltips)
    {
        if ((flags & FLAG_TIP) && !(options & TREELIST_AUTOSELECT)) // No tip when autoselect!
        {
            int x, y;
            FXuint buttons;

            getCursorPosition(x, y, buttons);
            DirItem* item = (DirItem*)getItemAt(x, y);
            if (item)
            {
                // !!! Hack to display a tooltip with name, size, date, etc. !!!
                FXString string;

                // Root folder
                if (item->getText() == ROOTDIR)
                {
                    string = _("Root Folder");
                }
                // Other folders
                else
                {
                    // Get tooltip data
                    FXString str = item->getTooltipData();
                    if (str == "")
                    {
                        return 0;
                    }

                    // Add name, type, permissions, etc. to the tool tip
                    FXString name = str.section('\t', 0);
                    FXString type = str.section('\t', 1);
                    FXString date = str.section('\t', 2);
                    FXString user = str.section('\t', 3);
                    FXString group = str.section('\t', 4);
                    FXString perms = str.section('\t', 5);
                    FXString deldate = str.section('\t', 6);
                    FXString pathname = str.section('\t', 7);

                    // Compute root file size
                    FXulong dnsize;
                    char dsize[64];
                    dnsize = xf_dirsize(pathname.text());
#if __WORDSIZE == 64
                    snprintf(dsize, sizeof(dsize), "%lu", dnsize);
#else
                    snprintf(dsize, sizeof(dsize), "%llu", dnsize);
#endif
                    FXString size = xf_humansize(dsize);
                    if (deldate.empty())
                    {
                        string = _("Name: ") + name + "\n" + _("Size in root: ") + size + "\n" + _("Type: ") + type
                                 + "\n" + _("Modified date: ") + date + "\n" + _("User: ") + user + " - " +
                                 _("Group: ") + group
                                 + "\n" + _("Permissions: ") + perms;
                    }
                    else
                    {
                        string = _("Name: ") + name + "\n" + _("Size in root: ") + size + "\n" + _("Type: ") + type
                                 + "\n" + _("Modified date: ") + date + "\n" + _("Deletion date: ") + deldate + "\n"
                                 + _("User: ") + user + " - " + _("Group: ") + group
                                 + "\n" + _("Permissions: ") + perms;
                    }
                }

                sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&string);
                return 1;
            }
        }
    }
    return 0;
}
