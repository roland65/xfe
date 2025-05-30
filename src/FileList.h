#ifndef FILELIST_H
#define FILELIST_H

#include "xfedefs.h"
#include "StringList.h"
#include "IconList.h"


struct FileAssoc;
class FileDict;
class FileList;

// File List options
enum
{
    /* These options are already declared in FXFileList.h
    FILELIST_SHOWHIDDEN    = 0x04000000,          // Show hidden files or directories
    FILELIST_SHOWDIRS      = 0x08000000,          // Show only directories
    */
    FILELIST_SEARCH        = 0x10000000,          // File list is a search list (must be the same value as in IconList)
};

// File item
class FXAPI FileItem : public IconItem
{
    FXDECLARE(FileItem)
    friend class FileList;
    friend class SearchPanel;
protected:
    FileAssoc* assoc = NULL;                       // File association record
    FileItem* link = NULL;                         // Link to next item
    FXulong size = 0;                              // File size
    FXTime date = 0;                               // File date (mtime)
    FXTime cdate = 0;                              // Changed date (ctime)
    FXTime deldate = 0;                            // Deletion date
protected:
    FileItem()
    {
    }
protected:
    enum
    {
        FOLDER       = 64,                         // Directory item
        EXECUTABLE   = 128,                        // Executable item
        SYMLINK      = 256,                        // Symbolic linked item
        CHARDEV      = 512,                        // Character special item
        BLOCKDEV     = 1024,                       // Block special item
        FIFO         = 2048,                       // FIFO item
        SOCK         = 4096                        // Socket item
    };
public:
    // Constructor
    FileItem(FXApp* a, const FXString& text, FXIcon* bi = NULL, FXIcon* mi = NULL, void* ptr = NULL) :
        IconItem(a, text, bi, mi, ptr), assoc(NULL), link(NULL), size(0), date(0), cdate(0), deldate(0)
    {
    }

    // Return true if this is a file item
    FXbool isFile() const
    {
        return (state & (FOLDER | BLOCKDEV | CHARDEV | FIFO | SOCK)) == 0;
    }

    // Return true if this is a directory item
    FXbool isDirectory() const
    {
        return (state & FOLDER) != 0;
    }

    // Return true if this is an executable item
    FXbool isExecutable() const
    {
        return (state & EXECUTABLE) != 0;
    }

    // Return true if this is a symbolic link item
    FXbool isSymlink() const
    {
        return (state & SYMLINK) != 0;
    }

    // Return true if this is a character device item
    FXbool isChardev() const
    {
        return (state & CHARDEV) != 0;
    }

    // Return true if this is a block device item
    FXbool isBlockdev() const
    {
        return (state & BLOCKDEV) != 0;
    }

    // Return true if this is an FIFO item
    FXbool isFifo() const
    {
        return (state & FIFO) != 0;
    }

    // Return true if this is a socket
    FXbool isSocket() const
    {
        return (state & SOCK) != 0;
    }

    // Return the file-association object for this item
    FileAssoc* getAssoc() const
    {
        return assoc;
    }

    // Return the file size for this item
    FXulong getSize() const
    {
        return size;
    }

    // Return the date for this item
    FXTime getDate() const
    {
        return date;
    }
};


// File List object
class FXAPI FileList : public IconList
{
    FXDECLARE(FileList)
protected:
    FileItem* list = NULL;                              // File item list
    int prevIndex = 0;
    FXString directory;                                 // Current directory
    FXString orgdirectory;                              // Original directory
    FXString dropdirectory;                             // Drop directory
    FXDragAction dropaction;                            // Drop action
    FXString dragfiles;                                 // Dragged files
    FileDict* associations = NULL;                      // Association table
    FXString pattern;                                   // Pattern of file names
    FXuint matchmode = 0;                               // File wildcard match mode
    FXTime timestamp = 0;                               // Time when last refreshed
    FXuint counter = 0;                                 // Refresh counter
    FXbool allowrefresh = false;                        // Allow or disallow periodic refresh
    FXbool displaythumbnails = false;                   // Display thumbnails
    FXString trashfileslocation;                        // Location of the trash files directory
    FXString trashinfolocation;                         // Location of the trash info directory
    FXbool dirsfirst = false;                           // Sort directories first
    FXbool filter_folders = false;                      // Filter folders
    double scalefrac = 1.0;                             // Icon scaling factor
    FXuint deldate_size = 0;
    FXuint origpath_size = 0;
    FXWindow* focuswindow = NULL;                       // Window used to test focus
    FXuint idCol[NMAX_COLS + 1] = { 0 };
    FXuint nbCols = 0;
    FXuint idColTrash[NMAX_COLS + 2] = { 0 };
    FXuint nbColsTrash = 0;

    // Static variables
    static int name_index;                              // Index of name column (always 0)
    static int size_index;                              // Index of size column
    static int type_index;                              // Index of type column
    static int ext_index;                               // Index of ext column
    static int date_index;                              // Index of date column
    static int user_index;                              // Index of user column
    static int group_index;                             // Index of group column
    static int perms_index;                             // Index of permissions column
    static int link_index;                              // Index of link column
    static int origpath_index;                          // Index of original path column (in trash)
    static int deldate_index;                           // Index of deletion column (in trash)
    static int dirname_index;                           // Index of directory path column (in search list)

public:
    StringList* backhist = NULL;                        // Back history
    StringList* forwardhist = NULL;                     // Forward history

protected:
    FileList()
    {
    }
    virtual IconItem* createItem(const FXString& text, FXIcon* big, FXIcon* mini, void* ptr);

    FXbool updateItems(FXbool);
    void   listItems(FXbool);

private:
    FileList(const FileList&);
    FileList& operator=(const FileList&);

public:
    long onCmdRefresh(FXObject*, FXSelector, void*);
    long onCmdRefreshTimer(FXObject*, FXSelector, void*);
    long onOpenTimer(FXObject*, FXSelector, void*);
    long onDNDEnter(FXObject*, FXSelector, void*);
    long onDNDLeave(FXObject*, FXSelector, void*);
    long onDNDMotion(FXObject*, FXSelector, void*);
    long onDNDDrop(FXObject*, FXSelector, void*);
    long onDNDRequest(FXObject*, FXSelector, void*);
    long onBeginDrag(FXObject*, FXSelector, void*);
    long onEndDrag(FXObject*, FXSelector, void*);
    long onDragged(FXObject*, FXSelector, void*);
    long onCmdDirectoryUp(FXObject*, FXSelector, void*);
    long onUpdDirectoryUp(FXObject*, FXSelector, void*);
    long onCmdSortByName(FXObject*, FXSelector, void*);
    long onCmdSortByDirName(FXObject*, FXSelector, void*);
    long onUpdSortByName(FXObject*, FXSelector, void*);
    long onUpdSortByDirName(FXObject*, FXSelector, void*);
    long onCmdSortByType(FXObject*, FXSelector, void*);
    long onUpdSortByType(FXObject*, FXSelector, void*);
    long onCmdSortBySize(FXObject*, FXSelector, void*);
    long onUpdSortBySize(FXObject*, FXSelector, void*);
    long onCmdSortByExt(FXObject*, FXSelector, void*);
    long onUpdSortByExt(FXObject*, FXSelector, void*);
    long onCmdSortByDate(FXObject*, FXSelector, void*);
    long onUpdSortByDate(FXObject*, FXSelector, void*);
    long onCmdSortByUser(FXObject*, FXSelector, void*);
    long onUpdSortByUser(FXObject*, FXSelector, void*);
    long onCmdSortByGroup(FXObject*, FXSelector, void*);
    long onUpdSortByGroup(FXObject*, FXSelector, void*);
    long onCmdSortByPerms(FXObject*, FXSelector, void*);
    long onUpdSortByPerms(FXObject*, FXSelector, void*);
    long onCmdSortByLink(FXObject*, FXSelector, void*);
    long onUpdSortByLink(FXObject*, FXSelector, void*);
    long onCmdSortByDeldate(FXObject*, FXSelector, void*);
    long onUpdSortByDeldate(FXObject*, FXSelector, void*);
    long onCmdSortByOrigpath(FXObject*, FXSelector, void*);
    long onUpdSortByOrigpath(FXObject*, FXSelector, void*);
    long onCmdSortReverse(FXObject*, FXSelector, void*);
    long onUpdSortReverse(FXObject*, FXSelector, void*);
    long onCmdSortCase(FXObject*, FXSelector, void*);
    long onUpdSortCase(FXObject*, FXSelector, void*);
    long onCmdSetPattern(FXObject*, FXSelector, void*);
    long onUpdSetPattern(FXObject*, FXSelector, void*);
    long onCmdToggleHidden(FXObject*, FXSelector, void*);
    long onUpdToggleHidden(FXObject*, FXSelector, void*);
    long onCmdShowHidden(FXObject*, FXSelector, void*);
    long onUpdShowHidden(FXObject*, FXSelector, void*);
    long onCmdHideHidden(FXObject*, FXSelector, void*);
    long onUpdHideHidden(FXObject*, FXSelector, void*);
    long onCmdHeader(FXObject*, FXSelector, void*);
    long onUpdHeader(FXObject*, FXSelector, void*);
    long onCmdToggleThumbnails(FXObject*, FXSelector, void*);
    long onUpdToggleThumbnails(FXObject*, FXSelector, void*);
    long onCmdDirsFirst(FXObject*, FXSelector, void*);
    long onUpdDirsFirst(FXObject*, FXSelector, void*);
    long onCmdDragCopy(FXObject*, FXSelector, void*);
    long onCmdDragMove(FXObject*, FXSelector, void*);
    long onCmdDragLink(FXObject*, FXSelector, void*);
    long onCmdDragReject(FXObject*, FXSelector, void*);
    long onUpdRefreshTimer(FXObject*, FXSelector, void*);
    long onCmdCloseFilter(FXObject*, FXSelector, void*);

public:
    static int compare(const IconItem*, const IconItem*, FXbool, FXbool, FXbool, FXuint);
    static int ascending(const IconItem*, const IconItem*);
    static int descending(const IconItem*, const IconItem*);
    static int ascendingCase(const IconItem*, const IconItem*);
    static int descendingCase(const IconItem*, const IconItem*);
    static int ascendingDir(const IconItem*, const IconItem*);
    static int descendingDir(const IconItem*, const IconItem*);
    static int ascendingDirCase(const IconItem*, const IconItem*);
    static int descendingDirCase(const IconItem*, const IconItem*);
    static int ascendingType(const IconItem*, const IconItem*);
    static int descendingType(const IconItem*, const IconItem*);
    static int ascendingSize(const IconItem*, const IconItem*);
    static int descendingSize(const IconItem*, const IconItem*);
    static int ascendingExt(const IconItem*, const IconItem*);
    static int descendingExt(const IconItem*, const IconItem*);
    static int ascendingDate(const IconItem*, const IconItem*);
    static int descendingDate(const IconItem*, const IconItem*);
    static int ascendingUser(const IconItem*, const IconItem*);
    static int descendingUser(const IconItem*, const IconItem*);
    static int ascendingGroup(const IconItem*, const IconItem*);
    static int descendingGroup(const IconItem*, const IconItem*);
    static int ascendingPerms(const IconItem*, const IconItem*);
    static int descendingPerms(const IconItem*, const IconItem*);
    static int ascendingLink(const IconItem*, const IconItem*);
    static int descendingLink(const IconItem*, const IconItem*);
    static int ascendingLinkCase(const IconItem*, const IconItem*);
    static int descendingLinkCase(const IconItem*, const IconItem*);
    static int ascendingDeldate(const IconItem*, const IconItem*);
    static int descendingDeldate(const IconItem*, const IconItem*);
    static int ascendingOrigpath(const IconItem*, const IconItem*);
    static int descendingOrigpath(const IconItem*, const IconItem*);
    static int ascendingOrigpathCase(const IconItem*, const IconItem*);
    static int descendingOrigpathCase(const IconItem*, const IconItem*);
    static int ascendingMix(const IconItem*, const IconItem*);
    static int descendingMix(const IconItem*, const IconItem*);
    static int ascendingCaseMix(const IconItem*, const IconItem*);
    static int descendingCaseMix(const IconItem*, const IconItem*);
    static int ascendingDirMix(const IconItem*, const IconItem*);
    static int descendingDirMix(const IconItem*, const IconItem*);
    static int ascendingDirCaseMix(const IconItem*, const IconItem*);
    static int descendingDirCaseMix(const IconItem*, const IconItem*);
    static int ascendingTypeMix(const IconItem*, const IconItem*);
    static int descendingTypeMix(const IconItem*, const IconItem*);
    static int ascendingSizeMix(const IconItem*, const IconItem*);
    static int descendingSizeMix(const IconItem*, const IconItem*);
    static int ascendingExtMix(const IconItem*, const IconItem*);
    static int descendingExtMix(const IconItem*, const IconItem*);
    static int ascendingDateMix(const IconItem*, const IconItem*);
    static int descendingDateMix(const IconItem*, const IconItem*);
    static int ascendingUserMix(const IconItem*, const IconItem*);
    static int descendingUserMix(const IconItem*, const IconItem*);
    static int ascendingGroupMix(const IconItem*, const IconItem*);
    static int descendingGroupMix(const IconItem*, const IconItem*);
    static int ascendingPermsMix(const IconItem*, const IconItem*);
    static int descendingPermsMix(const IconItem*, const IconItem*);
    static int ascendingLinkMix(const IconItem*, const IconItem*);
    static int descendingLinkMix(const IconItem*, const IconItem*);
    static int ascendingLinkCaseMix(const IconItem*, const IconItem*);
    static int descendingLinkCaseMix(const IconItem*, const IconItem*);
    static int ascendingDeldateMix(const IconItem*, const IconItem*);
    static int descendingDeldateMix(const IconItem*, const IconItem*);
    static int ascendingOrigpathMix(const IconItem*, const IconItem*);
    static int descendingOrigpathMix(const IconItem*, const IconItem*);
    static int ascendingOrigpathCaseMix(const IconItem*, const IconItem*);
    static int descendingOrigpathCaseMix(const IconItem*, const IconItem*);

    int getHeaderIndex(FXuint);
    int getHeaderSize(FXuint);
    void setHeaderSize(FXuint, FXuint);

    FXString labels[NMAX_COLS + 3];

    void setItemLabels(FXuint*, FXuint*, FXbool, FXString, FXString, FXString,
                       FXString, FXString, FXString, FXString, FXString, FXString,
                       FXString, FXString, FXString, FXString, FXString*);

public:
    enum
    {
        // Note : the order of the 10 following column IDs must be kept
        ID_COL_NAME = IconList::ID_LAST,
        ID_COL_SIZE,
        ID_COL_TYPE,
        ID_COL_EXT,
        ID_COL_DATE,
        ID_COL_USER,
        ID_COL_GROUP,
        ID_COL_PERMS,
        ID_COL_LINK,
        ID_COL_ORIGPATH,
        ID_COL_DELDATE,
        ID_COL_DIRNAME,
        ID_SORT_REVERSE,
        ID_SORT_CASE,
        ID_DIRS_FIRST,
        ID_DIRECTORY_UP,
        ID_SET_PATTERN,
        ID_SET_DIRECTORY,
        ID_SHOW_HIDDEN,
        ID_HIDE_HIDDEN,
        ID_TOGGLE_HIDDEN,
        ID_TOGGLE_THUMBNAILS,
        ID_REFRESH_TIMER,
        ID_REFRESH,
        ID_OPEN_TIMER,
        ID_DRAG_COPY,
        ID_DRAG_MOVE,
        ID_DRAG_LINK,
        ID_DRAG_REJECT,
        ID_CLOSE_FILTER,
        ID_LAST
    };
public:

    // Construct a file list
    FileList(FXWindow*, FXComposite*, FXuint*, FXuint, FXObject* tgt = NULL, FXSelector sel = 0,
             FXbool showthumbs = false, FXuint = 0, FXuint = 0, FXuint opts = 0,
             int x = 0, int y = 0, int w = 0, int h = 0);

    // Create server-side resources
    virtual void create();

    // Scan the current directory and update the items if needed, or if force is true
    void scan(FXbool force = true);

    // Set current file
    void setCurrentFile(const FXString& file);

    // Return current file
    FXString getCurrentFile() const;

    // Set current directory
    void setDirectory(const FXString& path, const FXbool histupdate = true, FXString prevpath = "");

    // Return current directory
    FXString getDirectory() const
    {
        return directory;
    }

    // Change wildcard matching pattern
    void setPattern(const FXString& ptrn);

    // Return wildcard pattern
    FXString getPattern() const
    {
        return pattern;
    }

    // Get filter folders
    FXbool getFilterFolders(void)
    {
        return filter_folders;
    }

    // Toggle filter folders
    void setFilterFolders(FXbool filt)
    {
        filter_folders = filt;
    }

    // Return true if item is a directory
    FXbool isItemDirectory(int index) const;

    // Return true if item is a file
    FXbool isItemFile(int index) const;

    // Return true if item is executable
    FXbool isItemExecutable(int index) const;

    // Return true if item is a symbolic link
    FXbool isItemLink(int index) const;

    // Get number of selected items
    int getNumSelectedItems(void) const
    {
        int num = 0;

        for (int u = 0; u < getNumItems(); u++)
        {
            if (isItemSelected(u))
            {
                num++;
            }
        }
        return num;
    }

    // Get number of selected items and index of first selected item
    int getNumSelectedItems(int* index) const
    {
        int num = 0, itm = -1;

        for (int u = 0; u < getNumItems(); u++)
        {
            if (isItemSelected(u))
            {
                if (itm == -1)
                {
                    itm = u;
                }
                num++;
            }
        }
        (*index) = itm;
        return num;
    }

    // Return name of item at index
    FXString getItemFilename(int index) const;

    // Get pathname from item at index, relatively to the current directory
    FXString getItemPathname(int index) const;

    // Get full pathname from item at index, as obtained from the label string
    FXString getItemFullPathname(int index) const;

    // Return file association of item
    FileAssoc* getItemAssoc(int index) const;

    // Return file size of the item
    FXulong getItemFileSize(int index) const;

    // Return wildcard matching mode
    FXuint getMatchMode() const
    {
        return matchmode;
    }

    // Return directory first state for file name sorting
    FXbool getDirsFirst() const
    {
        return dirsfirst;
    }

    // Set directory first state for file name sorting
    void setDirsFirst(const FXbool dfirst)
    {
        dirsfirst = dfirst;
    }

    // Allow or disallow periodic refresh
    void setAllowRefresh(const FXbool allow);

    // Change wildcard matching mode
    void setMatchMode(FXuint mode);

    // Return true if showing hidden files
    FXbool shownHiddenFiles() const;

    // Show or hide hidden files
    void showHiddenFiles(FXbool showing);

    // Return true if displaying thumbnails
    FXbool shownThumbnails() const;

    // Display or not thumbnails
    void showThumbnails(FXbool display);

    // Return true if showing directories only
    FXbool showOnlyDirectories() const;

    // Show directories only
    void showOnlyDirectories(FXbool shown);

    // Change file associations
    void setAssociations(FileDict* assoc);

    // Return file associations
    FileDict* getAssociations() const
    {
        return associations;
    }

    // Return size of deletion date column
    FXuint getDeldateSize(void)
    {
        return deldate_size;
    }

    // Return size of original path column
    FXuint getOrigpathSize(void)
    {
        return origpath_size;
    }

#if defined(linux)
    // Force mtdevices list refresh
    void refreshMtdevices(void);

#endif

    // Destructor
    virtual ~FileList();
};

#endif
