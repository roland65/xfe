#ifndef DIRPANEL_H
#define DIRPANEL_H
#include "DirList.h"
#include "Places.h"
#include "Properties.h"
#include "InputDialog.h"
#include "ArchInputDialog.h"
#include "BookmarkDialog.h"
#include "BrowseInputDialog.h"

#include <sys/types.h>


class DirPanel : public FXVerticalFrame
{
    FXDECLARE(DirPanel)
protected:
    DirList* dirlist = NULL;
    PlacesList* placeslist = NULL;
    MountsList* mountslist = NULL;
    BookmarksList* bookmarkslist = NULL;
    FXPacker* statusbar = NULL;
    FXLabel* status = NULL;
    FXButton* activeicon = NULL;
    FXString trashlocation;
    FXString trashfileslocation;
    FXString trashinfolocation;
    FXString startlocation;
    FXDragType urilistType = 0;                    // Standard uri-list type
    FXDragType xfelistType = 0;                    // Xfe, Gnome and XFCE list type
    FXDragType kdelistType = 0;                    // KDE list type
    FXDragType utf8Type = 0;                       // UTF-8 text type
    FXbool clipboard_locked = false;               // Clipboard locked to prevent changes when viewing it
    InputDialog* newdirdialog = NULL;
    ArchInputDialog* archdialog = NULL;
    BrowseInputDialog* operationdialog = NULL;
    InputDialog* operationdialogrename = NULL;
    BookmarkDialog* addbookmarkdialog = NULL;
    BookmarkDialog* modifybookmarkdialog = NULL;
    FXbool fromPaste = false;
    FXWindow* focuswindow = NULL;
    TextLabel* paneltitle = NULL;                  // Panel title
    FXTabBook* tabbook = NULL;                     // Tab book
    FXbool isactive = false;                       // Flag to indicate is panel has keyboard focus
    FXbool stopListRefresh = false;                // To stop refreshing in some cases
    time_t curr_mtime = 0;                         // Current directory mtime
    FXString curr_dirpath;                         // Current directory path
    FXbool allowDirsizeRefresh = true;             // Allow or avoid directory size refresh
    FXuint dirpanel_mode = 0;                      // Panel type: 0 => places and tree, 1 => places only, 2 => tree only

    FXSplitter* splitter = NULL;                   // Splitter for mounts and bookmarks list

    FXuint single_click = SINGLE_CLICK_NONE;

public:
    DirPanel(FXWindow* owner, FXComposite* p, FXColor listbackcolor = FXRGB(255, 255, 255),
             FXColor listforecolor = FXRGB(0, 0, 0), FXbool smoothscroll = true,
             FXuint opts = 0, int x = 0, int y = 0, int w = 0, int h = 0);

    DirPanel()
    {
    }

    virtual void create();

    ~DirPanel();
    enum
    {
        ID_FILELIST=FXVerticalFrame::ID_LAST,
        ID_STOP_LIST_REFRESH_TIMER,
        ID_BOOKMARKS,
        ID_PLACES,
        ID_MOUNTS,
        ID_EXPANDTREE,
        ID_TOGGLE_HIDDEN,
        ID_COLLAPSEDIR,
        ID_COLLAPSETREE,
        ID_PROPERTIES,
        ID_ARCHIVE,
        ID_DIR_COPY,
        ID_DIR_CUT,
        ID_DIR_COPYTO,
        ID_DIR_MOVETO,
        ID_DIR_RENAME,
        ID_DIR_SYMLINK,
        ID_DIR_DELETE,
        ID_DIR_TRASH,
        ID_DIR_RESTORE,
        ID_NEW_DIR,
        ID_XTERM,
        ID_COPY_CLIPBOARD,
        ID_COPYNAME_CLIPBOARD,
        ID_CUT_CLIPBOARD,
        ID_ADDCOPY_CLIPBOARD,
        ID_ADDCUT_CLIPBOARD,
        ID_PASTE_CLIPBOARD,
        ID_TOGGLE_PANEL,
        ID_TITLE,
        ID_DIRSIZE_REFRESH,
        ID_POPUP_MENU_DIRLIST,
        ID_POPUP_MENU_PLACES,
        ID_OPEN_PLACE_NEW_TAB,
        ID_OPEN_PLACE_NEW_WINDOW,
        ID_PLACE_PROPERTIES,
        ID_POPUP_MENU_MOUNTS,
        ID_OPEN_MOUNT_NEW_TAB,
        ID_OPEN_MOUNT_NEW_WINDOW,
        ID_MOUNT_PROPERTIES,
        ID_POPUP_MENU_BOOKMARKS,
        ID_OPEN_BOOKMARK_NEW_TAB,
        ID_OPEN_BOOKMARK_NEW_WINDOW,
        ID_MODIFY_BOOKMARK,
        ID_REMOVE_BOOKMARK,
        ID_BOOKMARK_PROPERTIES,
        ID_ADD_BOOKMARK,
        ID_MOVEUP_BOOKMARK,
        ID_MOVEDOWN_BOOKMARK,
#if defined(linux)
        ID_MOUNT,
        ID_UMOUNT,
#endif
        ID_LAST,
    };
    long exploreUp(DirItem* item, const DirItem* rootitem, const int task);
    long exploreDown(DirItem* item, const DirItem* rootitem, const int task);

public:
    long onClipboardGained(FXObject*, FXSelector, void*);
    long onClipboardLost(FXObject*, FXSelector, void*);
    long onClipboardRequest(FXObject*, FXSelector, void*);
    long onCmdToggleHidden(FXObject*, FXSelector, void*);
    long onUpdToggleHidden(FXObject*, FXSelector, void*);
    long onCmdPopupMenuDirList(FXObject*, FXSelector, void*);
    long onExpandTree(FXObject*, FXSelector, void*);
    long onCollapseTree(FXObject*, FXSelector, void*);
    long onCmdProperties(FXObject*, FXSelector, void*);
    long onCmdAddToArch(FXObject*, FXSelector, void*);

    long onCmdPopupMenuPlaces(FXObject*, FXSelector, void*);
    long onCmdGotoPlace(FXObject*, FXSelector, void*);
    long onCmdOpenPlaceNewTab(FXObject*, FXSelector, void*);
    long onCmdOpenPlaceNewWindow(FXObject*, FXSelector, void*);
    long onCmdPlaceProperties(FXObject*, FXSelector, void*);
    long onUpdPlaces(FXObject*, FXSelector, void*);

    long onCmdPopupMenuMounts(FXObject*, FXSelector, void*);
    long onCmdGotoMount(FXObject*, FXSelector, void*);
    long onCmdOpenMountNewTab(FXObject*, FXSelector, void*);
    long onCmdOpenMountNewWindow(FXObject*, FXSelector, void*);
    long onCmdMountProperties(FXObject*, FXSelector, void*);
    long onUpdMounts(FXObject*, FXSelector, void*);

    long onCmdPopupMenuBookmarks(FXObject*, FXSelector, void*);
    long onCmdAddBookmark(FXObject*, FXSelector, void*);
    long onCmdOpenBookmarkNewTab(FXObject*, FXSelector, void*);
    long onCmdOpenBookmarkNewWindow(FXObject*, FXSelector, void*);
    long onCmdBookmarkProperties(FXObject*, FXSelector, void*);
    long onCmdModifyBookmark(FXObject*, FXSelector, void*);
    long onCmdRemoveBookmark(FXObject*, FXSelector, void*);
    long onCmdGotoBookmark(FXObject*, FXSelector, void*);
    long onCmdMoveUpBookmark(FXObject*, FXSelector, void*);
    long onCmdMoveDownBookmark(FXObject*, FXSelector, void*);
    long onUpdMoveUpBookmark(FXObject*, FXSelector, void*);
    long onUpdMoveDownBookmark(FXObject*, FXSelector, void*);
    long onUpdBookmarks(FXObject*, FXSelector, void*);

    long onCmdDirMan(FXObject*, FXSelector, void*);
    long onCmdDirDelete(FXObject*, FXSelector, void*);
    long onCmdDirTrash(FXObject*, FXSelector, void*);
    long onCmdDirRestore(FXObject*, FXSelector, void*);
    long onCmdNewDir(FXObject*, FXSelector, void*);
    long onCmdXTerm(FXObject*, FXSelector, void*);
    long onCmdCopyCut(FXObject*, FXSelector, void*);
    long onCmdCopyName(FXObject*, FXSelector sel, void*);
    long onCmdPaste(FXObject*, FXSelector, void*);
    long onUpdPaste(FXObject*, FXSelector, void*);
    long onCmdDirectory(FXObject*, FXSelector, void*);
    long onCmdTogglePanel(FXObject*, FXSelector sel, void*);
    long onCmdDirsizeRefresh(FXObject*, FXSelector, void*);

    long onUpdTogglePanel(FXObject*, FXSelector, void*);
    long onUpdMount(FXObject*, FXSelector, void*);
    long onUpdUnmount(FXObject*, FXSelector, void*);
    long onUpdMenu(FXObject*, FXSelector, void*);
    long onUpdDirTrash(FXObject*, FXSelector, void*);
    long onUpdDirRestore(FXObject*, FXSelector, void*);
    long onUpdDirDelete(FXObject*, FXSelector, void*);
    long onUpdTitle(FXObject*, FXSelector, void*);
    long onUpdStatus(FXObject*, FXSelector, void*);
    long onExpand(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onCmdFocus(FXObject*, FXSelector, void*);
    long onCmdStopListRefreshTimer(FXObject*, FXSelector, void*);
    long onUpdDirsizeRefresh(FXObject*, FXSelector, void*);

#if defined(linux)
    long onCmdMount(FXObject*, FXSelector, void*);
#endif

public:

    void setActive();
    void setInactive();

    // Toggle dirsize refresh and force refresh if flag is true
    void setAllowDirsizeRefresh(FXbool flag);

    // Change sort function
    void setSortFunc(FXTreeListSortFunc func)
    {
        dirlist->setSortFunc(func);
    }

    // Return sort function
    FXTreeListSortFunc getSortFunc() const
    {
        return dirlist->getSortFunc();
    }

    // Change default cursor
    void setDefaultCursor(FXCursor* cur)
    {
        dirlist->setDefaultCursor(cur);
        bookmarkslist->setDefaultCursor(cur);
    }

    // Set current directory
    void setDirectory(const FXString& pathname, FXbool notify = false)
    {
        dirlist->setDirectory(pathname, notify);
    }

    // Get current directory
    FXString getDirectory(void) const
    {
        return dirlist->getDirectory();
    }

    // Get current item
    DirItem* getCurrentItem(void) const
    {
        return (DirItem*)dirlist->getCurrentItem();
    }

    // Get current path name
    FXString getItemPathname(const DirItem* item) const
    {
        return dirlist->getItemPathname((TreeItem*)item);
    }

    // Hidden files shown?
    FXbool shownHiddenFiles() const
    {
        return dirlist->shownHiddenFiles();
    }

    // Show hidden files
    void showHiddenFiles(FXbool shown)
    {
        dirlist->showHiddenFiles(shown);
    }

    // Set focus on list
    void setFocusOnList(void)
    {
        dirlist->setFocus();
    }

    // Is panel active?
    FXbool isActive(void)
    {
        return isactive;
    }

    // Force dir panel refresh
    void forceRefresh(void)
    {
        dirlist->onCmdRefresh(0, 0, 0);
    }

    DirList* getList(void)
    {
        return dirlist;
    }

#if defined(linux)
    // Force devices refresh
    void forceDevicesRefresh(void)
    {
        dirlist->onMtdevicesRefresh(0, 0, 0);
        dirlist->onUpdevicesRefresh(0, 0, 0);
    }

#endif

    // Toggle status bar
    void toggleStatusbar(void)
    {
        statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), NULL);
    }

    // Get bookmarks list
    BookmarksList* getBookmarksList(void)
    {
        return bookmarkslist;
    }

    // Get number of bookmarks items
    int getNumBookmarks(void)
    {
        return bookmarkslist->getNumItems();
    }

    // Return bookmark item text at index
    FXString getBookmarkName(int index)
    {
        return bookmarkslist->getItemText(index);
    }

    // Return bookmark location
    FXString getBookmarkLocation(FXString key)
    {
        return bookmarkslist->getItemPathname(key);
    }

    // Return mounts list height
    FXuint getMountsHeight(void)
    {
        return splitter->getSplit(0);
    }
};
#endif
