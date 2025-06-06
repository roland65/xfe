#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "File.h"
#include "DirList.h"
#include "Places.h"
#include "Properties.h"
#include "BrowseInputDialog.h"
#include "ArchInputDialog.h"
#include "XFileExplorer.h"
#include "MessageBox.h"
#include "DirPanel.h"



// Refresh interval for the directory size (ms)
#define DIRSIZE_REFRESH_INTERVAL      1000

// Duration (in ms) before we can stop refreshing the file list
// Used for file operations on a large list of files
#define STOP_LIST_REFRESH_INTERVAL    5000

// Number of files before stopping the file list refresh
#define STOP_LIST_REFRESH_NBMAX       100

// Clipboard
extern FXString clipboard;
extern FXuint clipboard_type;

// Global variables
extern FXMainWindow* mainWindow;

extern FXString xdgdatahome;

#if defined(linux)
extern FXStringDict* fsdevices;
extern FXStringDict* mtdevices;
#endif


// Dirty hack to change the KEY_up and KEY_down behaviour
// These keys are no more associated with the mouse click action
#define SELECT_MASK    (TREELIST_SINGLESELECT | TREELIST_BROWSESELECT)
FXbool fromKeyPress = false;
long FXTreeList::onKeyPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXTreeItem* item = currentitem;
    FXTreeItem* succ;
    int page;

    flags &= ~FLAG_TIP;
    if (!isEnabled())
    {
        return 0;
    }
    if (target && target->tryHandle(this, FXSEL(SEL_KEYPRESS, message), ptr))
    {
        return 1;
    }
    if (item == NULL)
    {
        item = firstitem;
    }
    switch (event->code)
    {
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
        if (flags & FLAG_DODRAG)
        {
            handle(this, FXSEL(SEL_DRAGGED, 0), ptr);
        }
        return 1;

    case KEY_Page_Up:
    case KEY_KP_Page_Up:
        for (succ = item, page = verticalScrollBar()->getPage(); succ && 0 < page; )
        {
            item = succ;
            page -= succ->getHeight(this);
            if (succ->prev)
            {
                succ = succ->prev;
                while (succ->last && ((options & TREELIST_AUTOSELECT) || succ->isExpanded()))
                {
                    succ = succ->last;
                }
            }
            else if (succ->parent)
            {
                succ = succ->parent;
            }
        }
        goto hop;

    case KEY_Page_Down:
    case KEY_KP_Page_Down:
        for (succ = item, page = verticalScrollBar()->getPage(); succ && 0 < page; )
        {
            item = succ;
            page -= succ->getHeight(this);
            if (succ->first && ((options & TREELIST_AUTOSELECT) || succ->isExpanded()))
            {
                succ = succ->first;
            }
            else
            {
                while (!succ->next && succ->parent)
                {
                    succ = succ->parent;
                }
                succ = succ->next;
            }
        }
        goto hop;

    case KEY_Up:                      // Move up
    case KEY_KP_Up:
        if (item)
        {
            if (item->prev)
            {
                item = item->prev;
                while (item->last && ((options & TREELIST_AUTOSELECT) || item->isExpanded()))
                {
                    item = item->last;
                }
            }
            else if (item->parent)
            {
                item = item->parent;
            }
        }
        goto hop;

    case KEY_Down:                    // Move down
    case KEY_KP_Down:
        if (item)
        {
            if (item->first && ((options & TREELIST_AUTOSELECT) || item->isExpanded()))
            {
                item = item->first;
            }
            else
            {
                while (!item->next && item->parent)
                {
                    item = item->parent;
                }
                item = item->next;
            }
        }
        goto hop;

    case KEY_Right:                   // Move right/down and open subtree
    case KEY_KP_Right:
        if (item)
        {
            if (!(options & TREELIST_AUTOSELECT) && !item->isExpanded() && (item->hasItems() || item->getFirst()))
            {
                expandTree(item, true);
            }
            else if (item->first)
            {
                item = item->first;
            }
            else
            {
                while (!item->next && item->parent)
                {
                    item = item->parent;
                }
                item = item->next;
            }
        }
        goto hop;

    case KEY_Left:                    // Move left/up and close subtree
    case KEY_KP_Left:
        if (item)
        {
            if (!(options & TREELIST_AUTOSELECT) && item->isExpanded() && (item->hasItems() || item->getFirst()))
            {
                collapseTree(item, true);
            }
            else if (item->parent)
            {
                item = item->parent;
            }
            else if (item->prev)
            {
                item = item->prev;
            }
        }
        goto hop;

    case KEY_Home:                    // Move to first
    case KEY_KP_Home:
        item = firstitem;
        goto hop;

    case KEY_End:                     // Move to last
    case KEY_KP_End:
        item = lastitem;
        while (item)
        {
            if (item->last && ((options & TREELIST_AUTOSELECT) || item->isExpanded()))
            {
                item = item->last;
            }
            else if (item->next)
            {
                item = item->next;
            }
            else
            {
                break;
            }
        }
hop:
        lookup = FXString::null;
        if (item)
        {
            setCurrentItem(item, true);
            makeItemVisible(item);
            if ((options & SELECT_MASK) == TREELIST_EXTENDEDSELECT)
            {
                if (item->isEnabled())
                {
                    if (event->state & SHIFTMASK)
                    {
                        if (anchoritem)
                        {
                            selectItem(anchoritem, true);
                            extendSelection(item, true);
                        }
                        else
                        {
                            selectItem(item, true);
                            setAnchorItem(item);
                        }
                    }
                    else if (!(event->state & CONTROLMASK))
                    {
                        killSelection(true);
                        selectItem(item, true);
                        setAnchorItem(item);
                    }
                }
            }
        }

        // !!!! Hack to change the KEY_up and KEY_down behaviour !!!
        fromKeyPress = true;
        // !!!! End of hack !!!
        handle(this, FXSEL(SEL_CLICKED, 0), (void*)currentitem);

        if (currentitem && currentitem->isEnabled())
        {
            handle(this, FXSEL(SEL_COMMAND, 0), (void*)currentitem);
        }
        return 1;

    case KEY_space:
    case KEY_KP_Space:
        lookup = FXString::null;
        if (item && item->isEnabled())
        {
            switch (options & SELECT_MASK)
            {
            case TREELIST_EXTENDEDSELECT:
                if (event->state & SHIFTMASK)
                {
                    if (anchoritem)
                    {
                        selectItem(anchoritem, true);
                        extendSelection(item, true);
                    }
                    else
                    {
                        selectItem(item, true);
                    }
                }
                else if (event->state & CONTROLMASK)
                {
                    toggleItem(item, true);
                }
                else
                {
                    killSelection(true);
                    selectItem(item, true);
                }
                break;

            case TREELIST_MULTIPLESELECT:
            case TREELIST_SINGLESELECT:
                toggleItem(item, true);
                break;
            }
            setAnchorItem(item);
        }
        handle(this, FXSEL(SEL_CLICKED, 0), (void*)currentitem);
        if (currentitem && currentitem->isEnabled())
        {
            handle(this, FXSEL(SEL_COMMAND, 0), (void*)currentitem);
        }
        return 1;

    case KEY_Return:
    case KEY_KP_Enter:
        lookup = FXString::null;
        handle(this, FXSEL(SEL_DOUBLECLICKED, 0), (void*)currentitem);
        if (currentitem && currentitem->isEnabled())
        {
            handle(this, FXSEL(SEL_COMMAND, 0), (void*)currentitem);
        }
        return 1;

    default:
        if ((FXuchar)event->text[0] < ' ')
        {
            return 0;
        }
        if (event->state & (CONTROLMASK | ALTMASK))
        {
            return 0;
        }
        if (!Ascii::isPrint(event->text[0]))
        {
            return 0;
        }
        lookup.append(event->text);
        getApp()->addTimeout(this, ID_LOOKUPTIMER, getApp()->getTypingSpeed());
        item = findItem(lookup, currentitem, SEARCH_FORWARD | SEARCH_WRAP | SEARCH_PREFIX);
        if (item)
        {
            setCurrentItem(item, true);
            makeItemVisible(item);
            if ((options & SELECT_MASK) == TREELIST_EXTENDEDSELECT)
            {
                if (item->isEnabled())
                {
                    killSelection(true);
                    selectItem(item, true);
                }
            }
            setAnchorItem(item);
        }
        handle(this, FXSEL(SEL_CLICKED, 0), (void*)currentitem);
        if (currentitem && currentitem->isEnabled())
        {
            handle(this, FXSEL(SEL_COMMAND, 0), (void*)currentitem);
        }
        return 1;
    }
    return 0;
}


// Map
FXDEFMAP(DirPanel) DirPanelMap[] =
{
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, DirPanel::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, DirPanel::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, DirPanel::onClipboardRequest),
    FXMAPFUNC(SEL_TIMEOUT, DirPanel::ID_STOP_LIST_REFRESH_TIMER, DirPanel::onCmdStopListRefreshTimer),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_EXPANDTREE, DirPanel::onExpandTree),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_COLLAPSETREE, DirPanel::onCollapseTree),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_PROPERTIES, DirPanel::onCmdProperties),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_ARCHIVE, DirPanel::onCmdAddToArch),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, DirPanel::ID_FILELIST, DirPanel::onCmdPopupMenuDirList),
    FXMAPFUNC(SEL_CLICKED, DirPanel::ID_FILELIST, DirPanel::onCmdDirectory),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, DirPanel::ID_PLACES, DirPanel::onCmdPopupMenuPlaces),
    FXMAPFUNC(SEL_CLICKED, DirPanel::ID_PLACES, DirPanel::onCmdGotoPlace),
    FXMAPFUNC(SEL_FOCUSIN, DirPanel::ID_PLACES, DirPanel::onCmdFocus),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_PLACE_NEW_TAB, DirPanel::onCmdOpenPlaceNewTab),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_PLACE_NEW_WINDOW, DirPanel::onCmdOpenPlaceNewWindow),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_PLACE_PROPERTIES, DirPanel::onCmdPlaceProperties),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_PLACES, DirPanel::onUpdPlaces),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, DirPanel::ID_MOUNTS, DirPanel::onCmdPopupMenuMounts),
    FXMAPFUNC(SEL_CLICKED, DirPanel::ID_MOUNTS, DirPanel::onCmdGotoMount),
    FXMAPFUNC(SEL_FOCUSIN, DirPanel::ID_MOUNTS, DirPanel::onCmdFocus),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_MOUNT_NEW_TAB, DirPanel::onCmdOpenMountNewTab),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_MOUNT_NEW_WINDOW, DirPanel::onCmdOpenMountNewWindow),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_MOUNT_PROPERTIES, DirPanel::onCmdMountProperties),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_MOUNTS, DirPanel::onUpdMounts),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, DirPanel::ID_BOOKMARKS, DirPanel::onCmdPopupMenuBookmarks),
    FXMAPFUNC(SEL_CLICKED, DirPanel::ID_BOOKMARKS, DirPanel::onCmdGotoBookmark),
    FXMAPFUNC(SEL_FOCUSIN, DirPanel::ID_BOOKMARKS, DirPanel::onCmdFocus),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_ADD_BOOKMARK, DirPanel::onCmdAddBookmark),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_BOOKMARK_NEW_TAB, DirPanel::onCmdOpenBookmarkNewTab),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_OPEN_BOOKMARK_NEW_WINDOW, DirPanel::onCmdOpenBookmarkNewWindow),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_BOOKMARK_PROPERTIES, DirPanel::onCmdBookmarkProperties),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_MODIFY_BOOKMARK, DirPanel::onCmdModifyBookmark),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_REMOVE_BOOKMARK, DirPanel::onCmdRemoveBookmark),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_MOVEUP_BOOKMARK, DirPanel::onCmdMoveUpBookmark),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_MOVEDOWN_BOOKMARK, DirPanel::onCmdMoveDownBookmark),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_MOVEUP_BOOKMARK, DirPanel::onUpdMoveUpBookmark),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_MOVEDOWN_BOOKMARK, DirPanel::onUpdMoveDownBookmark),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_BOOKMARKS, DirPanel::onUpdBookmarks),
    FXMAPFUNC(SEL_EXPANDED, DirPanel::ID_FILELIST, DirPanel::onExpand),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_COPY_CLIPBOARD, DirPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_COPYNAME_CLIPBOARD, DirPanel::onCmdCopyName),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_CUT_CLIPBOARD, DirPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_ADDCOPY_CLIPBOARD, DirPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_ADDCUT_CLIPBOARD, DirPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_PASTE_CLIPBOARD, DirPanel::onCmdPaste),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_COPY, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_CUT, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_COPYTO, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_MOVETO, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_RENAME, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_SYMLINK, DirPanel::onCmdDirMan),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_DELETE, DirPanel::onCmdDirDelete),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_TRASH, DirPanel::onCmdDirTrash),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_DIR_RESTORE, DirPanel::onCmdDirRestore),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_NEW_DIR, DirPanel::onCmdNewDir),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_XTERM, DirPanel::onCmdXTerm),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_TOGGLE_HIDDEN, DirPanel::onCmdToggleHidden),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_TOGGLE_PANEL, DirPanel::onCmdTogglePanel),
    FXMAPFUNC(SEL_TIMEOUT, DirPanel::ID_DIRSIZE_REFRESH, DirPanel::onCmdDirsizeRefresh),
    FXMAPFUNC(SEL_FOCUSIN, DirPanel::ID_FILELIST, DirPanel::onCmdFocus),
#if defined(linux)
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_MOUNT, DirPanel::onCmdMount),
    FXMAPFUNC(SEL_COMMAND, DirPanel::ID_UMOUNT, DirPanel::onCmdMount),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_MOUNT, DirPanel::onUpdMount),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_UMOUNT, DirPanel::onUpdUnmount),
#endif
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_PASTE_CLIPBOARD, DirPanel::onUpdPaste),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_TOGGLE_HIDDEN, DirPanel::onUpdToggleHidden),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_TOGGLE_PANEL, DirPanel::onUpdTogglePanel),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_CUT_CLIPBOARD, DirPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_ARCHIVE, DirPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_DIR_MOVETO, DirPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_DIR_RENAME, DirPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_DIR_TRASH, DirPanel::onUpdDirTrash),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_DIR_RESTORE, DirPanel::onUpdDirRestore),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_DIR_DELETE, DirPanel::onUpdDirDelete),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_NEW_DIR, DirPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, DirPanel::ID_TITLE, DirPanel::onUpdTitle),
    FXMAPFUNC(SEL_UPDATE, 0, DirPanel::onUpdDirsizeRefresh),
};

// Object implementation
FXIMPLEMENT(DirPanel, FXVerticalFrame, DirPanelMap, ARRAYNUMBER(DirPanelMap))

// Construct Directory Panel
DirPanel::DirPanel(FXWindow* owner, FXComposite* p, FXColor listbackcolor, FXColor listforecolor, FXbool smoothscroll,
                   FXuint opts, int x, int y, int w, int h) :
    FXVerticalFrame(p, opts, x, y, w, h, 0, 0, 0, 0)
{
    // Construct directory panel
    FXVerticalFrame* cont = new FXVerticalFrame(this, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0,
                                                -1);
    FXPacker* packer = new FXHorizontalFrame(cont, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0,
                                             0, 0, 1, 0);

    // Visually indicate if the panel is active
    activeicon = new FXButton(packer, "", minigreenbuttonicon, this, DirPanel::ID_FILELIST,
                              BUTTON_TOOLBAR | JUSTIFY_LEFT | LAYOUT_LEFT);

    // Panel title
    paneltitle = new TextLabel(packer, 0, this, ID_FILELIST, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    paneltitle->setText(_("Folders"));
    paneltitle->setBackColor(getApp()->getBaseColor());

    // Options
    FXuint treeoptions = 0, placesoptions = 0;
    if (smoothscroll)
    {
        treeoptions = LAYOUT_FILL_X | LAYOUT_FILL_Y | TREELIST_BROWSESELECT | TREELIST_SHOWS_LINES |
                      TREELIST_SHOWS_BOXES | FRAME_NONE;
        placesoptions = LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE;
    }
    else
    {
        treeoptions = LAYOUT_FILL_X | LAYOUT_FILL_Y | TREELIST_BROWSESELECT | TREELIST_SHOWS_LINES |
                      TREELIST_SHOWS_BOXES | SCROLLERS_DONT_TRACK;
        placesoptions = LAYOUT_FILL_X | LAYOUT_FILL_Y | SCROLLERS_DONT_TRACK | FRAME_NONE;
    }

    // Get panel view mode
    dirpanel_mode = getApp()->reg().readUnsignedEntry("OPTIONS", "dirpanel_mode", 0);

    // Places and tree mode
    if (dirpanel_mode == 0)
    {
        // Switcher
        tabbook = new FXTabBook(cont, this, ID_FILELIST, LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_RIGHT | FRAME_NONE, 0,
                                0, 0, 0, 0, 0, 0, 0);

        // First tab - Places, mounts and bookmarks lists

        // Places list (fixed size)
        new FXTabItem(tabbook, _("Places") + FXString("  "), miniplacesicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 0, 0, 3, 5);
        FXVerticalFrame* frame = new FXVerticalFrame(tabbook, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0,
                                                     0, 0, 0);
        frame->setBackColor(getApp()->getBackColor());
        FXVerticalFrame* plframe = new FXVerticalFrame(frame, LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 2, 0);
        placeslist = new PlacesList(owner, plframe, this, ID_PLACES, listbackcolor, listforecolor, placesoptions);

        // Splitter with mounts and bookmarks lists
        splitter = new FXSplitter(frame,
                                  LAYOUT_SIDE_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_TRACKING |
                                  SPLITTER_VERTICAL | FRAME_NONE);
        FXVerticalFrame* mtframe = new FXVerticalFrame(splitter, LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
        new FXLabel(mtframe, _("Mount Points"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X | FRAME_GROOVE, 0, 0,
                    0, 0, 0, 0, 0, 0);
        mountslist = new MountsList(owner, mtframe, this, ID_MOUNTS, listbackcolor, listforecolor, placesoptions);
        FXVerticalFrame* bkframe = new FXVerticalFrame(splitter, LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
        new FXLabel(bkframe, _("Bookmarks"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X | FRAME_GROOVE, 0, 0, 0,
                    0, 0, 0, 0, 0);
        bookmarkslist = new BookmarksList(owner, bkframe, this, ID_BOOKMARKS, listbackcolor, listforecolor,
                                          placesoptions);

        // Second tab - Directory list
        new FXTabItem(tabbook, _("Tree") + FXString("  "), minitreeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 0, 0, 3, 5);
        frame = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 2, 0);
        dirlist = new DirList(owner, frame, this, ID_FILELIST, treeoptions);
        dirlist->setTextColor(listforecolor);
        dirlist->setBackColor(listbackcolor);

        // Add keyboard shortcuts to select first or second tab
        FXAccelTable* table = getShell()->getAccelTable();
        if (table)
        {
            FXString key;
            FXHotKey hotkey;

            key = getApp()->reg().readStringEntry("KEYBINDINGS", "dirpanel_first_tab", "Ctrl-F9");
            hotkey = xf_parseaccel(key);
            table->addAccel(hotkey, tabbook, FXSEL(SEL_COMMAND, FXTabBar::ID_OPEN_FIRST));

            key = getApp()->reg().readStringEntry("KEYBINDINGS", "dirpanel_second_tab", "Ctrl-F10");
            hotkey = xf_parseaccel(key);
            table->addAccel(hotkey, tabbook, FXSEL(SEL_COMMAND, FXTabBar::ID_OPEN_SECOND));
        }

        // Open preferred list at startup (0 => places, 1 => tree)
        FXbool dirpanel_list_startup = getApp()->reg().readUnsignedEntry("OPTIONS", "dirpanel_list_startup", 0);
        if (dirpanel_list_startup == 1)
        {
            tabbook->handle(this, FXSEL(SEL_COMMAND, FXTabBar::ID_OPEN_SECOND), NULL);
        }
    }
    // Places or tree mode
    else
    {
        // Places list (fixed size)
        FXVerticalFrame* placesframe = new FXVerticalFrame(cont, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0,
                                                           0, 0, 0, 0);
        placesframe->setBackColor(getApp()->getBackColor());
        FXVerticalFrame* plframe = new FXVerticalFrame(placesframe, LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 2, 0);
        placeslist = new PlacesList(owner, plframe, this, ID_PLACES, listbackcolor, listforecolor, placesoptions);

        // Splitter with mounts and bookmarks lists
        splitter = new FXSplitter(placesframe,
                                  LAYOUT_SIDE_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_TRACKING |
                                  SPLITTER_VERTICAL | FRAME_NONE);
        FXVerticalFrame* mtframe = new FXVerticalFrame(splitter, LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
        new FXLabel(mtframe, _("Mount Points"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X | FRAME_GROOVE, 0, 0,
                    0, 0, 0, 0, 0, 0);
        mountslist = new MountsList(owner, mtframe, this, ID_MOUNTS, listbackcolor, listforecolor, placesoptions);
        FXVerticalFrame* bkframe = new FXVerticalFrame(splitter, LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
        new FXLabel(bkframe, _("Bookmarks"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X | FRAME_GROOVE, 0, 0, 0,
                    0, 0, 0, 0, 0);
        bookmarkslist = new BookmarksList(owner, bkframe, this, ID_BOOKMARKS, listbackcolor, listforecolor,
                                          placesoptions);

        FXVerticalFrame* treeframe = new FXVerticalFrame(cont, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0,
                                                         0, 0, 2, 0);
        dirlist = new DirList(owner, treeframe, this, ID_FILELIST, treeoptions);
        dirlist->setTextColor(listforecolor);
        dirlist->setBackColor(listbackcolor);

        // Display places
        if (dirpanel_mode == 1)
        {
            treeframe->hide();
        }
        // Display tree
        else
        {
            placesframe->hide();
        }
    }

    // Status bar
    statusbar = new FXHorizontalFrame(cont, JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 3, 3, 3, 3);

    FXString key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_dirs", "Ctrl-F5");
    new FXToggleButton(statusbar, TAB + _("Show Hidden Folders") + PARS(key),
                       TAB + _("Hide hidden Folders") + PARS(key),
                       minishowhiddenicon, minihidehiddenicon, this, ID_TOGGLE_HIDDEN, BUTTON_TOOLBAR | LAYOUT_LEFT |
                       ICON_BEFORE_TEXT | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 3);

    status = new FXLabel(statusbar, _("Free space:"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X | FRAME_NONE,
                         0, 0, 0, 0, 0, 0, 0, 4);
    status->setTarget(this);
    status->setSelector(FXSEL(SEL_UPDATE, DirPanel::ID_TITLE));

    // Home and trahscan locations
    trashlocation = xdgdatahome + PATHSEPSTRING TRASHPATH;
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;

    // Start location (we return to the start location after each chdir)
    startlocation = FXSystem::getCurrentDirectory();

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        dirlist->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
        placeslist->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
        mountslist->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
        bookmarkslist->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }

    // Initializations
    focuswindow = owner;
    curr_dirpath = "";
}


// Destructor
DirPanel::~DirPanel()
{
    getApp()->removeTimeout(this, ID_DIRSIZE_REFRESH);
    delete dirlist;
    delete statusbar;
    delete status;
    delete newdirdialog;
    delete archdialog;
    delete operationdialog;
    delete operationdialogrename;
    delete paneltitle;
    delete addbookmarkdialog;
    delete modifybookmarkdialog;
    delete placeslist;
    delete mountslist;
    delete bookmarkslist;
    delete splitter;
    delete tabbook;
}


// Create X window
void DirPanel::create()
{
    // Register standard uri-list type
    urilistType = getApp()->registerDragType("text/uri-list");

    // Register special uri-list type used for Gnome, XFCE and Xfe
    xfelistType = getApp()->registerDragType("x-special/gnome-copied-files");

    // Register special uri-list type used for KDE
    kdelistType = getApp()->registerDragType("application/x-kde-cutselection");

    // Register standard UTF-8 text type used for file dialogs
    utf8Type = getApp()->registerDragType("UTF8_STRING");

    // Set mounts list height
    FXuint mounts_height = getApp()->reg().readUnsignedEntry("DIR PANEL", "mounts_height", 200);
    splitter->setSplit(0, mounts_height);

    getApp()->addTimeout(this, ID_DIRSIZE_REFRESH, DIRSIZE_REFRESH_INTERVAL);

    FXVerticalFrame::create();
}


// Make DirPanel active
void DirPanel::setActive()
{
    // Set active icon
    activeicon->setIcon(minigreenbuttonicon);
    activeicon->setTipText(_("Panel is active"));
    isactive = true;

    // Set focus on directory list, if displayed
    if ( (dirpanel_mode == 0 && tabbook->getCurrent() == 1) || (dirpanel_mode == 2) )
    {
        dirlist->setFocus();        
    }

    // Current panel must get an inactive icon (but not get the inactive status!)
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    currentpanel->setInactive(false);
}


// Make DirPanel inactive
void DirPanel::setInactive()
{
    // Set active icon
    activeicon->setIcon(minigraybuttonicon);
    activeicon->setTipText(_("Activate panel"));
    isactive = false;
}


// Focus on DirPanel when clicked (i.e. make panel active)
long DirPanel::onCmdFocus(FXObject* sender, FXSelector sel, void* ptr)
{
    setActive();
    return 1;
}


// To pass the expand message to DirList
long DirPanel::onExpand(FXObject*, FXSelector, void* ptr)
{
    dirlist->handle(this, FXSEL(SEL_EXPANDED, 0), (void*)ptr);
    return 1;
}


// Change the directory when clicking on the tree list
long DirPanel::onCmdDirectory(FXObject*, FXSelector, void* ptr)
{
    TreeItem* item = (TreeItem*)ptr;

    if (item)
    {
        FXString directory = dirlist->getItemPathname(item);
        if (!xf_isreadexecutable(directory))
        {
            MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
            return 0;
        }
        FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
        ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
        currentpanel->setDirectory(directory, true);
        currentpanel->updatePath();

        // Remember latest directory in the location address
        FXString item;
        int i = 0;
        int count = address->getNumItems();
        FXString p = currentpanel->getDirectory();

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
            address->insertItem(0, currentpanel->getDirectory());
        }
    }

    // Manage single click navigation
    if (item && (single_click != SINGLE_CLICK_NONE) && !fromKeyPress)
    {
        dirlist->handle(this, FXSEL(SEL_EXPANDED, 0), (void*)ptr);
    }
    fromKeyPress = false;

    return 1;
}


// Open place in new tab
long DirPanel::onCmdOpenPlaceNewTab(FXObject*, FXSelector, void*)
{
    FXString key = placeslist->getCurrentItem();
    FXString directory = placeslist->getItemPathname(key);

    TabButtons* tabbuttons = ((XFileExplorer*)mainWindow)->getTabButtons();

    tabbuttons->addTab(directory);

    return 1;
}


// Open place in new window
long DirPanel::onCmdOpenPlaceNewWindow(FXObject*, FXSelector, void*)
{
    FXString key = placeslist->getCurrentItem();
    FXString directory = placeslist->getItemPathname(key);

    FXString cmd = "xfe " + directory + " &";

    int ret = system(cmd.text());

    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    return 1;
}


// Go to the directory pointed by the selected place
long DirPanel::onCmdGotoPlace(FXObject*, FXSelector, void*)
{
    FXString key = placeslist->getCurrentItem();

    // Place location
    FXString directory = placeslist->getItemPathname(key);

    // Permission problem or does not exist
    if (!xf_existfile(directory))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
        return 0;
    }
    else if (!xf_isreadexecutable(directory))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
        return 0;
    }

    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
    currentpanel->setDirectory(directory, true);
    currentpanel->updatePath();
    setDirectory(directory, true);

    // Set place (select and set current)
    placeslist->setPlace(key);

    // Update address list
    FXString item;
    int i = 0;
    int count = address->getNumItems();
    if (!count)
    {
        count++;
        address->insertItem(0, address->getText());
    }
    while (i < count)
    {
        item = address->getItem(i++);
        if (xf_strequal(directory.text(), (const char*)&item[0]))
        {
            i--;
            break;
        }
    }
    if (i == count)
    {
        address->insertItem(0, directory);
    }

    return 1;
}


// Properties of the folder pointed by the place
long DirPanel::onCmdPlaceProperties(FXObject*, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString pathname = currentpanel->getDirectory();

    // Properties dialog
    PropertiesBox* attrdlg = new PropertiesBox(this, FXPath::name(pathname), FXPath::directory(pathname), true);

    attrdlg->create();
    attrdlg->show(PLACEMENT_OWNER);

    return 1;
}


// Update places list selection
long DirPanel::onUpdPlaces(FXObject* sender, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString currentdir = currentpanel->getDirectory();

    placeslist->deselectAll();

    for (int i = 0; i < placeslist->getNumItems(); i++)
    {
        if (placeslist->getItemPathname(placeslist->getItemText(i)) == currentdir)
        {
            placeslist->selectItem(i);
            break;
        }
    }

    return 1;
}


// Open mount in new tab
long DirPanel::onCmdOpenMountNewTab(FXObject*, FXSelector, void*)
{
    FXString key = mountslist->getCurrentItem();
    FXString directory = mountslist->getItemPathname(key);

    TabButtons* tabbuttons = ((XFileExplorer*)mainWindow)->getTabButtons();

    tabbuttons->addTab(directory);

    return 1;
}


// Open mount in new window
long DirPanel::onCmdOpenMountNewWindow(FXObject*, FXSelector, void*)
{
    FXString key = mountslist->getCurrentItem();
    FXString directory = mountslist->getItemPathname(key);

    FXString cmd = "xfe " + directory + " &";

    int ret = system(cmd.text());

    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    return 1;
}


// Go to the directory pointed by the selected mount
long DirPanel::onCmdGotoMount(FXObject*, FXSelector, void*)
{
    FXString key = mountslist->getCurrentItem();

    // Mount location
    FXString directory = mountslist->getItemPathname(key);

    // Permission problem or does not exist
    if (!xf_existfile(directory))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
        return 0;
    }
    else if (!xf_isreadexecutable(directory))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
        return 0;
    }

    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
    currentpanel->setDirectory(directory, true);
    currentpanel->updatePath();
    setDirectory(directory, true);

    // Set mount (select and set current)
    mountslist->setMount(key);

    // Update address list
    FXString item;
    int i = 0;
    int count = address->getNumItems();
    if (!count)
    {
        count++;
        address->insertItem(0, address->getText());
    }
    while (i < count)
    {
        item = address->getItem(i++);
        if (xf_strequal(directory.text(), (const char*)&item[0]))
        {
            i--;
            break;
        }
    }
    if (i == count)
    {
        address->insertItem(0, directory);
    }

    return 1;
}


// Properties of the folder pointed by the mount
long DirPanel::onCmdMountProperties(FXObject*, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString pathname = currentpanel->getDirectory();

    // Properties dialog
    PropertiesBox* attrdlg = new PropertiesBox(this, FXPath::name(pathname), FXPath::directory(pathname), true);

    attrdlg->create();
    attrdlg->show(PLACEMENT_OWNER);

    return 1;
}


// Update mounts list selection
long DirPanel::onUpdMounts(FXObject* sender, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString currentdir = currentpanel->getDirectory();

    mountslist->deselectAll();

    for (int i = 0; i < mountslist->getNumItems(); i++)
    {
        if (mountslist->getItemPathname(mountslist->getItemText(i)) == currentdir)
        {
            mountslist->selectItem(i);
            break;
        }
    }

    return 1;
}


// Open bookmark in new tab
long DirPanel::onCmdOpenBookmarkNewTab(FXObject*, FXSelector, void*)
{
    FXString name = bookmarkslist->getCurrentItem();
    FXString directory = bookmarkslist->getItemPathname(name);

    TabButtons* tabbuttons = ((XFileExplorer*)mainWindow)->getTabButtons();

    tabbuttons->addTab(directory);

    return 1;
}


// Open bookmark in new window
long DirPanel::onCmdOpenBookmarkNewWindow(FXObject*, FXSelector, void*)
{
    FXString name = bookmarkslist->getCurrentItem();
    FXString directory = bookmarkslist->getItemPathname(name);

    FXString cmd = "xfe " + directory + " &";

    int ret = system(cmd.text());

    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    return 1;
}

// Properties of the folder pointed by the bookmark
long DirPanel::onCmdBookmarkProperties(FXObject*, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString pathname = currentpanel->getDirectory();

    // Properties dialog
    PropertiesBox* attrdlg = new PropertiesBox(this, FXPath::name(pathname), FXPath::directory(pathname), true);

    attrdlg->create();
    attrdlg->show(PLACEMENT_OWNER);

    return 1;
}


// Modify bookmark
long DirPanel::onCmdModifyBookmark(FXObject*, FXSelector, void*)
{
    FXString oldname = bookmarkslist->getCurrentItem();
    FXString oldiconpathname = bookmarkslist->getItemIconPathname(oldname);
    FXString pathname = bookmarkslist->getItemPathname(oldname);

    // Bookmark dialog
    if (modifybookmarkdialog == NULL)
    {
        modifybookmarkdialog = new BookmarkDialog(this, "", "", _("Modify Bookmark"), _("Name:"),
                                                  _("Icon:"), bigeditbookicon);
    }
    modifybookmarkdialog->setName(oldname);
    modifybookmarkdialog->selectAll();
    if (oldiconpathname != "")
    {
        modifybookmarkdialog->setIconPathname(oldiconpathname);
    }
    else
    {
        modifybookmarkdialog->setIconPathname("minibookmark.png");
    }

    FXString msg = FXString(_("Location:")) + " " + pathname;
    modifybookmarkdialog->setMessage(msg);

    // Accept was pressed
    if (modifybookmarkdialog->execute(PLACEMENT_CURSOR))
    {
        // Rename bookmark
        FXString newname = modifybookmarkdialog->getName();
        FXString newiconpathname = modifybookmarkdialog->getIconPathname();
        int ret = bookmarkslist->modifyBookmark(oldname, newname, oldiconpathname, newiconpathname, pathname);
        if (ret == 0)
        {
            return 0;
        }

        // Rebuild bookmarks menu
        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_REBUILD_BOOKMARKS_MENU), NULL);
    }
    // Cancel was pressed
    else
    {
        return 0;
    }

    return 1;
}


// Remove bookmark
long DirPanel::onCmdRemoveBookmark(FXObject*, FXSelector, void*)
{
    FXString name = bookmarkslist->getCurrentItem();

    // Confirm dialog
    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);

    if (confirm_del)
    {
        FXString message;
        message.format(_("Remove bookmark '%s' ?"), name.text());
        MessageBox box(this, _("Confirm Remove"), message + "     ", bigremovebookicon,
                       BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER | LAYOUT_FILL_X | LAYOUT_FILL_Y);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return 0;
        }
    }

    // Remove bookmark
    bookmarkslist->removeItem(name);

    return 1;
}


// Add bookmark
long DirPanel::onCmdAddBookmark(FXObject*, FXSelector, void*)
{
    FXString dirpathname = dirlist->getDirectory();

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
        FXString name = addbookmarkdialog->getName();
        FXString iconpathname = addbookmarkdialog->getIconPathname();
        int ret = bookmarkslist->appendBookmark(name, dirpathname, iconpathname);
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


// Go to the directory pointed by the selected bookmark
long DirPanel::onCmdGotoBookmark(FXObject*, FXSelector, void*)
{
    FXString name = bookmarkslist->getCurrentItem();

    // Bookmark location
    FXString directory = bookmarkslist->getItemPathname(name);

    // Permission problem or does not exist
    if (!xf_existfile(directory))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
        return 0;
    }
    else if (!xf_isreadexecutable(directory))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
        return 0;
    }

    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
    currentpanel->setDirectory(directory, true);
    currentpanel->updatePath();
    setDirectory(directory, true);

    // Set bookmark (select and set current)
    bookmarkslist->setBookmark(name);

    // Update address list
    FXString item;
    int i = 0;
    int count = address->getNumItems();
    if (!count)
    {
        count++;
        address->insertItem(0, address->getText());
    }
    while (i < count)
    {
        item = address->getItem(i++);
        if (xf_strequal(directory.text(), (const char*)&item[0]))
        {
            i--;
            break;
        }
    }
    if (i == count)
    {
        address->insertItem(0, directory);
    }

    return 1;
}


// Move up bookmark
long DirPanel::onCmdMoveUpBookmark(FXObject*, FXSelector, void*)
{
    // Move current bookmark
    FXString name = bookmarkslist->getCurrentItem();

    bookmarkslist->moveUp(name);

    // Rebuild bookmarks menu
    mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_REBUILD_BOOKMARKS_MENU), NULL);


    return 1;
}


// Move down bookmark
long DirPanel::onCmdMoveDownBookmark(FXObject*, FXSelector, void*)
{
    // Move current bookmark
    FXString name = bookmarkslist->getCurrentItem();

    bookmarkslist->moveDown(name);

    // Rebuild bookmarks menu
    mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_REBUILD_BOOKMARKS_MENU), NULL);

    return 1;
}


// Update bookmarks move up menu
long DirPanel::onUpdMoveUpBookmark(FXObject* sender, FXSelector, void*)
{
    int index = bookmarkslist->FXList::getCurrentItem();

    (index == 0 ? sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL) :
     sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL));

    return 1;
}


// Update bookmarks move down menu
long DirPanel::onUpdMoveDownBookmark(FXObject* sender, FXSelector, void*)
{
    int index = bookmarkslist->FXList::getCurrentItem();

    (index == bookmarkslist->getNumItems() - 1 ? sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL) :
     sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL));

    return 1;
}


// Update bookmarks list selection
long DirPanel::onUpdBookmarks(FXObject* sender, FXSelector, void*)
{
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString currentdir = currentpanel->getDirectory();

    bookmarkslist->deselectAll();

    for (int i = 0; i < bookmarkslist->getNumItems(); i++)
    {
        // Bookmark directory no more exists
        if (!xf_existfile(bookmarkslist->getItemPathname(bookmarkslist->getItemText(i))))
        {
            // Remove it
            bookmarkslist->removeItem(bookmarkslist->getItemText(i));
        }
        else
        {
            // Select bookmark if current directory
            if (bookmarkslist->getItemPathname(bookmarkslist->getItemText(i)) == currentdir)
            {
                bookmarkslist->selectItem(i);
                break;
            }
        }
    }

    return 1;
}


// Toggle hidden files
long DirPanel::onCmdToggleHidden(FXObject*, FXSelector, void*)
{
    dirlist->showHiddenFiles(!dirlist->shownHiddenFiles());
    return 1;
}


// Update toggle hidden files widget
long DirPanel::onUpdToggleHidden(FXObject* sender, FXSelector, void*)
{
    if (dirlist->shownHiddenFiles())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Show/hide panel
long DirPanel::onCmdTogglePanel(FXObject* sender, FXSelector sel, void* ptr)
{
    return this->handle(sender, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), ptr);
}


// Update menu button
long DirPanel::onUpdTogglePanel(FXObject* sender, FXSelector sel, void* ptr)
{
    if (this->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_CHECK), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_UNCHECK), ptr);
    }
    return 1;
}


// Directory list context menu
long DirPanel::onCmdPopupMenuDirList(FXObject* sender, FXSelector sel, void* ptr)
{
    // Check if control key was pressed
    FXbool ctrlflag = false;

    if (ptr != NULL)
    {
        FXEvent* event = (FXEvent*)ptr;
        if (event->state & CONTROLMASK)
        {
            ctrlflag = true;
        }
    }

    // Current item becomes item under cursor
    int x, y, xitem, yitem;
    FXuint state;
    getRoot()->getCursorPosition(x, y, state);
    dirlist->getCursorPosition(xitem, yitem, state);
    DirItem* item = (DirItem*)dirlist->getItemAt(xitem, yitem);

    // If item, then set it current and set directory in DirList and FileList
    FXString dir;
    if (item)
    {
        dirlist->setCurrentItem(item, true);
        dir = dirlist->getItemPathname((TreeItem*)item);
        dirlist->setDirectory(dir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(dir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
    }
    else
    {
        ctrlflag = true;
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);

    // Menu

    // Control flag set
    if (ctrlflag)
    {
        // Reset control flag
        ctrlflag = false;

        // Panel menu items
        new FXMenuCommand(menu, _("New &Folder..."), minifoldernewicon, this, DirPanel::ID_NEW_DIR);
        new FXMenuCommand(menu, _("&Add Bookmark..."), miniaddbookicon, this, DirPanel::ID_ADD_BOOKMARK);

        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("&Hidden Folders"), this, DirPanel::ID_TOGGLE_HIDDEN);
        new FXMenuCheck(menu, _("Ignore C&ase"), dirlist, DirList::ID_SORT_CASE);
        new FXMenuCheck(menu, _("&Reverse Order"), dirlist, DirList::ID_SORT_REVERSE);
        new FXMenuCommand(menu, _("E&xpand Tree"), miniexptreeicon, this, DirPanel::ID_EXPANDTREE);
        new FXMenuCommand(menu, _("Collap&se Tree"), minicolltreeicon, this, DirPanel::ID_COLLAPSETREE);
    }
    else
    {
        // Panel submenu items
        FXMenuPane* submenu = new FXMenuPane(this);

        new FXMenuCommand(submenu, _("New &Folder..."), minifoldernewicon, this, DirPanel::ID_NEW_DIR);
        new FXMenuCommand(submenu, _("&Add Bookmark..."), miniaddbookicon, this, DirPanel::ID_ADD_BOOKMARK);

        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("&Hidden Folders"), this, DirPanel::ID_TOGGLE_HIDDEN);
        new FXMenuCheck(submenu, _("Ignore C&ase"), dirlist, DirList::ID_SORT_CASE);
        new FXMenuCheck(submenu, _("&Reverse Order"), dirlist, DirList::ID_SORT_REVERSE);
        new FXMenuCommand(submenu, _("E&xpand Tree"), miniexptreeicon, this, DirPanel::ID_EXPANDTREE);
        new FXMenuCommand(submenu, _("Collap&se Tree"), minicolltreeicon, this, DirPanel::ID_COLLAPSETREE);

        new FXMenuCascade(menu, _("Pane&l"), NULL, submenu);
        new FXMenuSeparator(menu);

#if defined(linux)
        if (xf_islink(dir))
        {
            dir = xf_readlink(dir);
        }
        if (fsdevices->find(dir.text()) || mtdevices->find(dir.text()))
        {
            new FXMenuCommand(menu, _("M&ount"), minimounticon, this, ID_MOUNT);
            new FXMenuCommand(menu, _("Unmoun&t"), miniunmounticon, this, ID_UMOUNT);
            new FXMenuSeparator(menu);
        }
#endif
        new FXMenuCommand(menu, _("&Add to Archive..."), miniarchaddicon, this, DirPanel::ID_ARCHIVE);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Copy"), minicopyicon, this, DirPanel::ID_COPY_CLIPBOARD);
        new FXMenuCommand(menu, _("C&ut"), minicuticon, this, DirPanel::ID_CUT_CLIPBOARD);
        new FXMenuCommand(menu, _("&Paste"), minipasteicon, this, DirPanel::ID_PASTE_CLIPBOARD);
        new FXMenuCommand(menu, _("Cop&y Name"), minicopyicon, this, DirPanel::ID_COPYNAME_CLIPBOARD);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Re&name..."), minirenameicon, this, DirPanel::ID_DIR_RENAME);
        new FXMenuCommand(menu, _("Cop&y to..."), minicopyicon, this, DirPanel::ID_DIR_COPYTO);
        new FXMenuCommand(menu, _("&Move to..."), minimoveicon, this, DirPanel::ID_DIR_MOVETO);
        new FXMenuCommand(menu, _("Symlin&k to..."), minilinktoicon, this, DirPanel::ID_DIR_SYMLINK);
        new FXMenuCommand(menu, _("Mo&ve to Trash"), minideleteicon, this, DirPanel::ID_DIR_TRASH);
        new FXMenuCommand(menu, _("R&estore From Trash"), minirestoreicon, this, DirPanel::ID_DIR_RESTORE);
        new FXMenuCommand(menu, _("&Delete"), minideletepermicon, this, DirPanel::ID_DIR_DELETE);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Prop&erties"), miniattribicon, this, DirPanel::ID_PROPERTIES);
    }

    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);
    return 1;
}


// Places list context menu
long DirPanel::onCmdPopupMenuPlaces(FXObject* sender, FXSelector sel, void* ptr)
{
    // Current item becomes item under cursor
    int x, y, xitem, yitem;
    FXuint state;

    getRoot()->getCursorPosition(x, y, state);
    placeslist->getCursorPosition(xitem, yitem, state);

    int item = placeslist->getItemAt(xitem, yitem);
    if (item > -1)
    {
        FXString key = placeslist->getItemText(item);
        FXString directory = placeslist->getItemPathname(key);

        // Permission problem or does not exist
        if (!xf_existfile(directory))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
            return 0;
        }
        else if (!xf_isreadexecutable(directory))
        {
            MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
            return 0;
        }

        FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
        ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
        currentpanel->setDirectory(directory, true);
        currentpanel->updatePath();
        setDirectory(directory, true);
        placeslist->setPlace(key);

        // Update address list
        FXString item;
        int i = 0;
        int count = address->getNumItems();
        if (!count)
        {
            count++;
            address->insertItem(0, address->getText());
        }
        while (i < count)
        {
            item = address->getItem(i++);
            if (xf_strequal(directory.text(), (const char*)&item[0]))
            {
                i--;
                break;
            }
        }
        if (i == count)
        {
            address->insertItem(0, directory);
        }
    }
    else
    {
        return 0;
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);

    // Menu
    new FXMenuCommand(menu, _("New &Tab"), mininewtabicon, this, DirPanel::ID_OPEN_PLACE_NEW_TAB);
    new FXMenuCommand(menu, _("New &Window"), minixfeicon, this, DirPanel::ID_OPEN_PLACE_NEW_WINDOW);
    new FXMenuSeparator(menu);
    new FXMenuCommand(menu, _("&Properties"), miniattribicon, this, DirPanel::ID_PLACE_PROPERTIES);

    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);

    return 1;
}


// Mounts list context menu
long DirPanel::onCmdPopupMenuMounts(FXObject* sender, FXSelector sel, void* ptr)
{
    // Current item becomes item under cursor
    int x, y, xitem, yitem;
    FXuint state;

    getRoot()->getCursorPosition(x, y, state);
    mountslist->getCursorPosition(xitem, yitem, state);

    int item = mountslist->getItemAt(xitem, yitem);
    if (item > -1)
    {
        FXString key = mountslist->getItemText(item);
        FXString directory = mountslist->getItemPathname(key);

        // Permission problem or does not exist
        if (!xf_existfile(directory))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
            return 0;
        }
        else if (!xf_isreadexecutable(directory))
        {
            MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
            return 0;
        }

        FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
        ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
        currentpanel->setDirectory(directory, true);
        currentpanel->updatePath();
        setDirectory(directory, true);
        mountslist->setMount(key);

        // Update address list
        FXString item;
        int i = 0;
        int count = address->getNumItems();
        if (!count)
        {
            count++;
            address->insertItem(0, address->getText());
        }
        while (i < count)
        {
            item = address->getItem(i++);
            if (xf_strequal(directory.text(), (const char*)&item[0]))
            {
                i--;
                break;
            }
        }
        if (i == count)
        {
            address->insertItem(0, directory);
        }
    }
    else
    {
        return 0;
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);

    // Menu
    new FXMenuCommand(menu, _("New &Tab"), mininewtabicon, this, DirPanel::ID_OPEN_MOUNT_NEW_TAB);
    new FXMenuCommand(menu, _("New &Window"), minixfeicon, this, DirPanel::ID_OPEN_MOUNT_NEW_WINDOW);
    new FXMenuSeparator(menu);
#if defined(linux)
    new FXMenuCommand(menu, _("M&ount"), minimounticon, this, DirPanel::ID_MOUNT);
    new FXMenuCommand(menu, _("&Unmount"), miniunmounticon, this, DirPanel::ID_UMOUNT);
    new FXMenuSeparator(menu);
#endif
    new FXMenuCommand(menu, _("&Properties"), miniattribicon, this, DirPanel::ID_MOUNT_PROPERTIES);

    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);

    return 1;
}


// Bookmarks list context menu
long DirPanel::onCmdPopupMenuBookmarks(FXObject* sender, FXSelector sel, void* ptr)
{
    // Check if control key was pressed
    FXbool ctrlflag = false;

    if (ptr != NULL)
    {
        FXEvent* event = (FXEvent*)ptr;
        if (event->state & CONTROLMASK)
        {
            ctrlflag = true;
        }
    }

    // Current item becomes item under cursor
    int x, y, xitem, yitem;
    FXuint state;

    getRoot()->getCursorPosition(x, y, state);
    bookmarkslist->getCursorPosition(xitem, yitem, state);

    int item = bookmarkslist->getItemAt(xitem, yitem);
    if (item > -1)
    {
        FXString key = bookmarkslist->getItemText(item);
        FXString directory = bookmarkslist->getItemPathname(key);

        // Permission problem or does not exist
        if (!xf_existfile(directory))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), directory.text());
            return 0;
        }
        else if (!xf_isreadexecutable(directory))
        {
            MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), directory.text());
            return 0;
        }

        FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
        ComboBox* address = ((XFileExplorer*)mainWindow)->getAddressBox();
        currentpanel->setDirectory(directory, true);
        currentpanel->updatePath();
        setDirectory(directory, true);
        bookmarkslist->setBookmark(key);

        // Update address list
        FXString item;
        int i = 0;
        int count = address->getNumItems();
        if (!count)
        {
            count++;
            address->insertItem(0, address->getText());
        }
        while (i < count)
        {
            item = address->getItem(i++);
            if (xf_strequal(directory.text(), (const char*)&item[0]))
            {
                i--;
                break;
            }
        }
        if (i == count)
        {
            address->insertItem(0, directory);
        }
    }
    else
    {
        ctrlflag = true;
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);

    // Menu

    // Control flag set
    if (ctrlflag)
    {
        // Reset control flag
        ctrlflag = false;

        new FXMenuCommand(menu, _("&Add Bookmark..."), miniaddbookicon, this, DirPanel::ID_ADD_BOOKMARK);
    }
    else
    {
        new FXMenuCommand(menu, _("New &Tab"), mininewtabicon, this, DirPanel::ID_OPEN_BOOKMARK_NEW_TAB);
        new FXMenuCommand(menu, _("New &Window"), minixfeicon, this, DirPanel::ID_OPEN_BOOKMARK_NEW_WINDOW);
        new FXMenuCommand(menu, _("&Modify..."), minieditbookicon, this, DirPanel::ID_MODIFY_BOOKMARK);
        new FXMenuCommand(menu, _("&Remove"), miniremovebookicon, this, DirPanel::ID_REMOVE_BOOKMARK);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Move &Up"), miniupicon, this, DirPanel::ID_MOVEUP_BOOKMARK);
        new FXMenuCommand(menu, _("Move &Down"), minidownicon, this, DirPanel::ID_MOVEDOWN_BOOKMARK);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Properties"), miniattribicon, this, DirPanel::ID_BOOKMARK_PROPERTIES);
    }

    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);

    return 1;
}


// Helper function used to explore the directory tree and expand or collapse it
long DirPanel::exploreUp(DirItem* item, const DirItem* rootitem, const int task)
{
    DirItem* parentitem = item;

    if (task == ID_EXPANDTREE)
    {
        dirlist->expandTree((TreeItem*)item, true);
    }
    else
    {
        dirlist->collapseTree((TreeItem*)item, true);
    }

    item = (DirItem*)item->getFirst();
    if (!item)
    {
        exploreDown(parentitem, rootitem, task);
    }
    else
    {
        exploreUp(item, rootitem, task);
    }
    return 1;
}


// Helper function used to explore the directory tree and expand or collapse it
long DirPanel::exploreDown(DirItem* item, const DirItem* rootitem, const int task)
{
    if (item == rootitem)
    {
        return 1;
    }

    DirItem* parentitem = (DirItem*)item->getParent();

    if (task == ID_EXPANDTREE)
    {
        dirlist->expandTree((TreeItem*)item, true);
    }
    else
    {
        dirlist->collapseTree((TreeItem*)item, true);
    }
    item = (DirItem*)item->getNext();

    if (!item)
    {
        exploreDown(parentitem, rootitem, task);
    }
    else
    {
        if (task == ID_EXPANDTREE)
        {
            dirlist->expandTree((TreeItem*)item, true);
        }
        else
        {
            dirlist->collapseTree((TreeItem*)item, true);
        }
        if (!dirlist->isItemLeaf(item))
        {
            exploreUp(item, rootitem, task);
        }
        else
        {
            exploreDown(item, rootitem, task);
        }
    }
    return 1;
}


// Expand the directory tree under cursor
long DirPanel::onExpandTree(FXObject* sender, FXSelector sel, void*)
{
    DirItem* rootitem = (DirItem*)dirlist->getCurrentItem();

    getApp()->beginWaitCursor();
    exploreUp(rootitem, rootitem, ID_EXPANDTREE);
    getApp()->endWaitCursor();

    return 1;
}


// Collapse the directory tree under cursor
long DirPanel::onCollapseTree(FXObject* sender, FXSelector sel, void*)
{
    DirItem* rootitem = (DirItem*)dirlist->getCurrentItem();

    getApp()->beginWaitCursor();
    exploreUp(rootitem, rootitem, ID_COLLAPSETREE);
    getApp()->endWaitCursor();

    return 1;
}


// Directory properties
long DirPanel::onCmdProperties(FXObject* sender, FXSelector, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);

    // Properties dialog
    PropertiesBox* attrdlg = new PropertiesBox(this, FXPath::name(pathname), FXPath::directory(pathname));

    attrdlg->create();
    attrdlg->show(PLACEMENT_OWNER);
    dirlist->setDirectory(pathname, true);

    return 1;
}


// Add files or directory to an archive
long DirPanel::onCmdAddToArch(FXObject* sender, FXSelector, void*)
{
    int ret;
    FXString ext1, ext2, cmd, archive;
    File* f;

    // Name and path of the current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString name = dirlist->getItemText(item);
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    FXString parentdir = FXPath::directory(pathname);

    // Initial archive name with full path and default extension
    if (parentdir == PATHSEPSTRING)
    {
        if (name == PATHSEPSTRING) // Archive is root file system
        {
            archive = parentdir + "ROOT" + ".tar.gz";
        }
        else
        {
            archive = parentdir + name + ".tar.gz";
        }
    }
    else
    {
        archive = parentdir + PATHSEPSTRING + name + ".tar.gz";
    }

    ret = chdir(parentdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), parentdir.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), parentdir.text());
        }

        return 0;
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
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z"))
        {
            cmd = "tar -acvf " + archive + " ";
        }
        else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                 (ext1 == "tzst"))
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
        else if (ext1 == "z")
        {
            cmd = "compress -v ";
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

        // Archive command name
        cmd = cmd + xf_quote(name);

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Create Archive"), ARCHIVE);
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

        getApp()->endWaitCursor();
        delete f;

        // Display parent directory in DirList and FileList
        dirlist->setDirectory(parentdir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
    }
    return 1;
}


// We now really do have the clipboard, keep clipboard content
long DirPanel::onClipboardGained(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardGained(sender, sel, ptr);
    return 1;
}


// We lost the clipboard, free clipboard content
long DirPanel::onClipboardLost(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardLost(sender, sel, ptr);
    return 1;
}


// Somebody wants our clipboard content
long DirPanel::onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr)
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
                end = str.length();
                str = str.mid(0, end - 2); // Why is it end-2 here????
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
long DirPanel::onCmdCopyCut(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard
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

    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    clipboard += FXURL::encode(::xf_filetouri(pathname));

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


// Copy directory name (without path) to clipboard
long DirPanel::onCmdCopyName(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard and set clipboard type
    clipboard.clear();
    clipboard_type = COPYNAME_CLIPBOARD;

    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    clipboard += FXURL::encode(::xf_filetouri(FXPath::name(pathname)));

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
long DirPanel::onCmdPaste(FXObject*, FXSelector sel, void*)
{
    FXuchar* data;
    FXuint len;
    int beg, end, pos;
    FXString chaine, url, param;
    int num = 0;
    FXbool from_kde = false;

    // Target directory
    FXString targetdir = ((XFileExplorer*)mainWindow)->getCurrentPanel()->getDirectory();

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

            // Remove the trailing '\r' if any
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
        param = targetdir + "\n" + FXStringVal(num - 1) + "\n" + param;

        // Copy or cut operation depending on the clipboard type
        switch (clipboard_type)
        {
        case COPY_CLIPBOARD:
            sel = FXSEL(SEL_COMMAND, DirPanel::ID_DIR_COPY);
            break;

        case CUT_CLIPBOARD:
            clipboard.clear();
            sel = FXSEL(SEL_COMMAND, DirPanel::ID_DIR_CUT);
            break;
        }
        fromPaste = true;
        this->handle(this, sel, (void*)param.text());

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

            // Remove the trailing '\r' if any
            if ((pos = url.rfind('\r')) > 0)
            {
                url.erase(pos);
            }

            // Update the param string
            param += FXURL::decode(FXURL::fileFromURL(url)) + "\n";
            num++;
        }

        // Construct the final param string passed to the file management routine
        param = targetdir + "\n" + FXStringVal(num) + "\n" + param;

        // Copy or cut operation depending on the clipboard type
        switch (clipboard_type)
        {
        case COPY_CLIPBOARD:
            sel = FXSEL(SEL_COMMAND, DirPanel::ID_DIR_COPY);
            break;

        case CUT_CLIPBOARD:
            clipboard.clear();
            sel = FXSEL(SEL_COMMAND, DirPanel::ID_DIR_CUT);
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

            // Remove the trailing '\r' if any
            if ((pos = filepath.rfind('\r')) > 0)
            {
                filepath.erase(pos);
            }

            // Update the param string
            param += filepath + "\n";
            num++;
        }

        // Construct the final param string passed to the file management routine
        param = targetdir + "\n" + FXStringVal(num) + "\n" + param;

        // Copy
        sel = FXSEL(SEL_COMMAND, DirPanel::ID_DIR_COPY);
        fromPaste = true;
        handle(this, sel, (void*)param.text());

        FXFREE(&data);
        return 1;
    }

    return 0;
}


// Set the flag that allows to stop the file list refresh
long DirPanel::onCmdStopListRefreshTimer(FXObject*, FXSelector, void*)
{
    stopListRefresh = true;
    return 0;
}


// Copy/Move/Rename/Symlink directory
long DirPanel::onCmdDirMan(FXObject* sender, FXSelector sel, void* ptr)
{
    int num;
    FXString src, targetdir, target, name, source;

    // Confirmation dialog?
    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);

    // Copy suffix
    FXString copysuffix = getApp()->reg().readStringEntry("OPTIONS", "copysuffix", DEFAULT_COPY_SUFFIX);
    FXuint copysuffix_pos = getApp()->reg().readUnsignedEntry("OPTIONS", "copysuffix_pos", 0);

    // If we are called from the paste command, get the parameters from the pointer
    // Multiple sources are allowed
    if (fromPaste)
    {
        // Reset the flag
        fromPaste = false;

        // Get the parameters
        FXString str = (char*)ptr;
        targetdir = str.section('\n', 0);
        num = FXUIntVal(str.section('\n', 1));
        src = str.after('\n', 2);
        source = src.section('\n', 0);

        // If no item, return
        if (num <= 0)
        {
            return 0;
        }
    }
    // Obtain the parameters from the dir panel (only one source)
    else
    {
        // Current item
        DirItem* item = (DirItem*)dirlist->getCurrentItem();

        // Number of items
        if (item)
        {
            num = 1;
        }
        else
        {
            return 0;
        }

        // Source directory
        source = dirlist->getItemPathname((TreeItem*)item);

        // Target directory
        targetdir = FXPath::directory(source);
    }

    // Go to target directory
    //chdir(targetdir.text());

    // Name and directory of the first source file
    name = FXPath::name(source);
    FXString dir = FXPath::directory(source);

    // Initialise target name
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
    if (FXSELID(sel) == ID_DIR_COPY)
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
            FXString tgt = xf_cleanpath(target); // Remove trailing / if any
            if ((xf_isidentical(source, tgt)) ||
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
    if (FXSELID(sel) == ID_DIR_RENAME)
    {
        command = "rename";
        title = _("Rename");
        icon = bigrenameicon;
        if (num == 1)
        {
            message = _("Rename ");
            message += name;
            target = name;
            title = _("Rename");
        }
        else
        {
            return 0;
        }
    }
    if (FXSELID(sel) == ID_DIR_COPYTO)
    {
        command = "copy";
        title = _("Copy to");
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
    if (FXSELID(sel) == ID_DIR_MOVETO)
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
    if (FXSELID(sel) == ID_DIR_CUT)
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
    if (FXSELID(sel) == ID_DIR_SYMLINK)
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
    if (ask_before_copy || (source == target) || (FXSELID(sel) == ID_DIR_COPYTO) || (FXSELID(sel) == ID_DIR_MOVETO) ||
        (FXSELID(sel) == ID_DIR_RENAME) || (FXSELID(sel) == ID_DIR_SYMLINK))
    {
        if (FXSELID(sel) == ID_DIR_RENAME)
        {
            if (operationdialogrename == NULL)
            {
                operationdialogrename = new InputDialog(this, "", "", title, _("To:"), icon);
            }
            operationdialogrename->setTitle(title);
            operationdialogrename->setIcon(icon);
            operationdialogrename->setMessage(message);
            operationdialogrename->setText(target);
            operationdialogrename->selectAll();
            operationdialogrename->CursorEnd();
            int rc = operationdialogrename->execute(PLACEMENT_CURSOR);
            target = operationdialogrename->getText();

            // Target name contains '/'
            if (target.contains(PATHSEPCHAR))
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The / character is not allowed in folder names, operation cancelled"));
                return 0;
            }

            if (!rc)
            {
                return 0;
            }
        }
        else
        {
            if (operationdialog == NULL)
            {
                operationdialog = new BrowseInputDialog(this, "", "", title, _("To:"), icon, BROWSE_INPUT_FOLDER);
            }
            operationdialog->setTitle(title);
            operationdialog->setIcon(icon);
            operationdialog->setMessage(message);
            operationdialog->setText(target);
            operationdialog->CursorEnd();
            operationdialog->setDirectory(targetdir);
            int rc = operationdialog->execute(PLACEMENT_CURSOR);
            target = operationdialog->getText();
            if (!rc)
            {
                return 0;
            }
        }
    }

    // Update target and target parent directory
    target = xf_filepath(target, targetdir);
    if (xf_isdirectory(target))
    {
        targetdir = target;
    }
    else
    {
        targetdir = FXPath::directory(target);
    }

    // Target directory not writable
    if (!xf_iswritable(targetdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), targetdir.text());
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

    // Target parent directory doesn't exist
    if (!xf_existfile(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), targetdir.text());
        return 0;
    }

    // Target parent directory is not a directory
    if (!xf_isdirectory(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("%s is not a folder"), targetdir.text());
        return 0;
    }

    // Current and next panel
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FilePanel* nextpanel = ((XFileExplorer*)mainWindow)->getNextPanel();

    FXString hsourcesize;
    FXulong sourcesize = 0, tstart = 0;

    // One source
    File* f;
    int ret;
    f = NULL;
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
            f = new File(this, _("Copy Files"), COPY);
            f->create();

            // Get total source size and start time
            hsourcesize = f->sourcesize(source, &sourcesize);
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

            // An error has occurred
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
            f = new File(this, _("Rename Files"), RENAME);
            f->create();
            ret = f->rename(source, target);

            // If file is located at trash location, try to also remove the corresponding trashinfo if it exists
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
            f = new File(this, _("Move Files"), MOVE);
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

            // An error has occurred
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
            f = new File(this, _("Symlink"), SYMLINK);
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
    // Note : rename cannot be used in this case!
    else if (num > 1)
    {
        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        if (command == "copy")
        {
            f = new File(this, _("Copy Files"), COPY);
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
            f = new File(this, _("Move Files"), MOVE);
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
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK);
            f->create();
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }

        if (f == NULL)
        {
            fprintf(stderr, "%s::onCmdDirMan: NULL pointer specified.\n", getClassName());
            exit(EXIT_FAILURE);
        }

        // Initialize file list stop refresh timer and flag
        stopListRefresh = false;
        getApp()->addTimeout(this, ID_STOP_LIST_REFRESH_TIMER, STOP_LIST_REFRESH_INTERVAL);

        // Loop on the multiple files
        for (int i = 0; i < num; i++)
        {
            // Stop refreshing the file list and directory size if file operation is long and has many files
            // This avoids flickering and speeds up things a bit
            if (stopListRefresh && (i > STOP_LIST_REFRESH_NBMAX))
            {
                // Force a last refresh if current panel is destination
                if (currentpanel->getDirectory() == targetdir)
                {
                    currentpanel->getList()->onCmdRefresh(0, 0, 0);
                }

                // Force a last refresh if next panel is destination
                if (nextpanel->getDirectory() == targetdir)
                {
                    nextpanel->getList()->onCmdRefresh(0, 0, 0);
                }

                // Tell the file list to not refresh anymore
                currentpanel->setAllowRefresh(false);
                nextpanel->setAllowRefresh(false);

                // Avoid to refresh the dirsize
                setAllowDirsizeRefresh(false);

                // Don't need to stop again
                stopListRefresh = false;
            }

            // Individual source file
            //~ source = src.section('\n', i);

            // NB: src string is altered by strtok()
            char* token;
            (i == 0 ? token = strtok((char*)src.text(), "\n") : token = strtok(NULL, "\n"));
            source = FXString(token);

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
                        ret = f->fmove(source, trashtarget);
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
                else if (command == "symlink")
                {
                    ret = f->symlink(source, target);

                    // An error has occurred
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
        currentpanel->setAllowRefresh(true);
        nextpanel->setAllowRefresh(true);
        setAllowDirsizeRefresh(true);

        getApp()->endWaitCursor();
        delete f;
    }

    // Refresh the path link and the directory list
    currentpanel->updatePath();
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);
    return 1;
}


// Delete directory
long DirPanel::onCmdDirDelete(FXObject*, FXSelector, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    FXString parentdir = FXPath::directory(pathname);

    // If we don't have permission to write to the parent directory
    if (!xf_iswritable(parentdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), parentdir.text());
        return 0;
    }

    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);
    FXbool confirm_del_emptydir = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete_emptydir", true);

    if (confirm_del)
    {
        FXString message;
        message.format(_("Definitively delete folder %s ?"), pathname.text());
        MessageBox box(this, _("Confirm Delete"), message, bigdeletepermicon,
                       BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return 0;
        }
    }

    // File object
    File* f = new File(this, _("Delete files"), DELETE);
    f->create();

    // Confirm empty directory deletion
    if (confirm_del & confirm_del_emptydir)
    {
        if ((xf_isemptydir(pathname) == 0) && !xf_islink(pathname))
        {
            // Dialog to confirm file deletion
            f->hideProgressDialog();
            FXString msg;
            msg.format(_("Folder %s is not empty, delete it anyway?"), pathname.text());
            MessageBox box(this, _("Confirm Delete"), msg, bigdeletepermicon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) == BOX_CLICKED_CANCEL)
            {
                goto end;
            }
            f->showProgressDialog();
        }
    }

    // If we don't have permission to write to the directory
    if (!xf_iswritable(pathname))
    {
        // Dialog to confirm directory deletion
        f->hideProgressDialog();
        FXString msg;
        msg.format(_("Folder %s is write-protected, definitively delete it anyway?"), pathname.text());
        MessageBox box(this, _("Confirm Delete"), msg, bigerroricon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        FXuint answer = box.execute(PLACEMENT_OWNER);
        if (answer == BOX_CLICKED_OK)
        {
            f->showProgressDialog();
            f->remove(pathname);

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Delete folder operation cancelled!"));
            }

            // Return to parent directory in DirList and FileList
            dirlist->setDirectory(parentdir, true);
            ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
            ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
        }
    }
    // If we have permission to write
    else
    {
        f->remove(pathname);

        // If directory is located at trash location, try to also remove the trashinfo if it exists
        // Do it silently and don't report any error if it fails
        FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
        if (use_trash_can && (pathname.left(trashfileslocation.length()) == trashfileslocation))
        {
            FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(pathname) + ".trashinfo";
            unlink(trashinfopathname.text());
        }

        // If action is cancelled in progress dialog
        if (f->isCancelled())
        {
            f->hideProgressDialog();
            MessageBox::error(this, BOX_OK, _("Warning"), _("Delete folder operation cancelled!"));
        }
        // Return to parent directory in DirList and FileList
        dirlist->setDirectory(parentdir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
    }

end:
    delete f;

    // Force DirPanel and FilePanel refresh
    setAllowDirsizeRefresh(true);
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);
    return 1;
}


// Move directory to trash can
long DirPanel::onCmdDirTrash(FXObject*, FXSelector, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    FXString parentdir = FXPath::directory(pathname);

    // If we don't have permission to write to the parent directory
    if (!xf_iswritable(parentdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), parentdir.text());
        return 0;
    }

    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    if (confirm_trash)
    {
        FXString message;
        message.format(_("Move folder %s to trash can?"), pathname.text());
        MessageBox box(this, _("Confirm Trash"), message, bigdeleteicon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return 0;
        }
    }

    // File object
    File* f = new File(this, _("Move to Trash"), DELETE);
    f->create();

    // If we don't have permission to write to the directory
    if (!xf_iswritable(pathname))
    {
        // Dialog to confirm directory deletion
        f->hideProgressDialog();
        FXString str;
        str.format(_("Folder %s is write-protected, move it to trash can anyway?"), pathname.text());
        MessageBox box(this, _("Confirm Trash"), str, bigerroricon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        FXuint answer = box.execute(PLACEMENT_OWNER);
        if (answer == BOX_CLICKED_OK)
        {
            // Allow progress dialog
            f->showProgressDialog();

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
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move to trash folder operation cancelled!"));
            }

            // Return to parent directory in DirList and FileList
            dirlist->setDirectory(parentdir, true);
            ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
            ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
        }
    }
    // If we have permission to write
    else
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
            MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move to trash operation!"));
        }

        // If action is cancelled in progress dialog
        if (f->isCancelled())
        {
            f->hideProgressDialog();
            MessageBox::error(this, BOX_OK, _("Warning"), _("Move to trash folder operation cancelled!"));
        }
        // Return to parent directory in DirList and FileList
        dirlist->setDirectory(parentdir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
    }
    delete f;

    // Force DirPanel and FilePanel refresh
    setAllowDirsizeRefresh(true);
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);
    return 1;
}


// Restore directory from trash can
long DirPanel::onCmdDirRestore(FXObject*, FXSelector, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString pathname = dirlist->getItemPathname((TreeItem*)item);
    FXString parentdir = FXPath::directory(pathname);
    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    // File object
    File* f = new File(this, _("Restore From Trash"), DELETE);

    f->create();

    // Obtain trash base name and sub path
    FXString subpath = pathname;
    subpath.erase(0, trashfileslocation.length() + 1);
    FXString trashbasename = subpath.before('/');
    if (trashbasename == "")
    {
        trashbasename = FXPath::name(pathname);
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
    if (confirm_trash)
    {
        FXString message;
        message.format(_("Restore folder %s to its original location %s ?"), FXPath::name(pathname).text(),
                       origpathname.text());
        f->hideProgressDialog();
        MessageBox box(this, _("Confirm Restore"), message, bigrestoreicon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            getApp()->endWaitCursor();
            delete f;
            return 0;
        }
        f->showProgressDialog();
    }

    // Bracket used because of a compilation problem with gotos
    {
        if (origpathname == "")
        {
            f->hideProgressDialog();
            MessageBox::error(this, BOX_OK, _("Error"), _("Restore information not available for %s"), pathname.text());
            goto end;
        }

        // If parent dir of the original location does not exist
        FXString origparentdir = FXPath::directory(origpathname);
        if (!xf_existfile(origparentdir))
        {
            // Ask the user if he wants to create it
            f->hideProgressDialog();
            FXString message;
            message.format(_("Parent folder %s does not exist, do you want to create it?"), origparentdir.text());
            MessageBox box(this, _("Confirm Restore"), message, bigrestoreicon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                goto end;
            }
            else
            {
                f->showProgressDialog();
                errno = 0;
                int ret = xf_mkpath(origparentdir.text(), 0755);
                int errcode = errno;
                if (ret == -1)
                {
                    f->hideProgressDialog();
                    if (errcode)
                    {
                        MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s : %s"),
                                          origparentdir.text(), strerror(errcode));
                    }
                    else
                    {
                        MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s"), origparentdir.text());
                    }
                    goto end;
                }
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

        // If action is cancelled in progress dialog
        if (f->isCancelled())
        {
            f->hideProgressDialog();
            MessageBox::error(this, BOX_OK, _("Warning"), _("Restore from trash file operation cancelled!"));
            goto end;
        }

        // Return to parent directory in DirList and FileList
        dirlist->setDirectory(parentdir, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(parentdir);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();
    }

end:
    delete f;

    // Force DirPanel and FilePanel refresh
    setAllowDirsizeRefresh(true);
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);
    return 1;
}


// Create new directory
long DirPanel::onCmdNewDir(FXObject*, FXSelector, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();

    FXString dirpath = dirlist->getItemPathname((TreeItem*)item);

    if (dirpath != ROOTDIR)
    {
        dirpath += PATHSEPSTRING;
    }

    if (newdirdialog == NULL)
    {
        newdirdialog = new InputDialog(this, "", _("Create new folder:"), _("New Folder"), "", bigfoldernewicon);
    }
    newdirdialog->setText("");
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

        FXString dirname = dirpath + newdirdialog->getText();
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
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s : %s"),
                                      dirname.text(), strerror(errcode));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't create folder %s"), dirname.text());
                }
                return 0;
            }
        }
    }

    // Force dirpanel refresh
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);
    return 1;
}


// Run Terminal in the selected directory
long DirPanel::onCmdXTerm(FXObject*, FXSelector, void*)
{
    int ret;

    getApp()->beginWaitCursor();
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString buf = dirlist->getItemPathname((TreeItem*)item);
    ret = chdir(buf.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), buf.text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), buf.text());
        }

        return 0;
    }

    FXString cmd = getApp()->reg().readStringEntry("PROGS", "xterm", "xterm -sb");
    cmd += " &";

    ret = system(cmd.text());
    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

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
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
        }

        return 0;
    }

    getApp()->endWaitCursor();
    return 1;
}


#if defined(linux)
// Mount/Unmount directory
long DirPanel::onCmdMount(FXObject*, FXSelector sel, void*)
{
    int ret;
    FXString cmd, msg, msgwait;
    FXuint op;
    File* f;

    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString dir = dirlist->getItemPathname((TreeItem*)item);

    // GVFS mount directory
    FXString gvfsdir = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";

    // GVFS mount name
    FXString gvfsname = FXPath::name(dir);

    if (FXPath::directory(dir) == gvfsdir)
    {
        // GVFS mount URI
        FXString mounturi = "";

        // Get mount type
        FXString mounttype = gvfsname.before(':');

        // Samba share
        if (mounttype == "smb-share")
        {
            FXString tmp;
            int pos = 0;

            // Domain name
            FXString domain;
            pos = gvfsname.find("domain=");
            if (pos < 0)
            {
                domain = "";
            }
            else
            {
                pos += 7;
                tmp = gvfsname.right(gvfsname.length() - pos);
                domain = tmp.before(',');
            }

            // User name
            FXString user;
            pos = gvfsname.find("user=");
            if (pos < 0)
            {
                user = "";
            }
            else
            {
                pos += 5;
                tmp = gvfsname.right(gvfsname.length() - pos);
                user = tmp.before(',');
            }

            // Server name
            FXString server;
            pos = gvfsname.find("server=");
            if (pos < 0)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't find server name"));
                return 0;
            }
            else
            {
                pos += 7;
                tmp = gvfsname.right(gvfsname.length() - pos);
                server = tmp.before(',');
            }

            // Share name
            FXString share;
            pos = gvfsname.find("share=");
            if (pos < 0)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't find share name"));
                return 0;
            }
            else
            {
                pos += 6;
                tmp = gvfsname.right(gvfsname.length() - pos);
                share = tmp.before(',');
            }

            // Build mount URI
            if (domain != "")
            {
                domain += ";";
            }
            if (user != "")
            {
                user += "@";
            }
            server += "/";
            share += "/";

            mounturi = "'smb://" + domain + user + server + share + "'";
        }

        // SSH mount
        if (mounttype == "sftp")
        {
            FXString tmp;
            int pos = 0;

            // Host name
            FXString host;
            pos = gvfsname.find("host=") + 5;
            if (pos < 0)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't find server name"));
                return 0;
            }
            else
            {
                tmp = gvfsname.right(gvfsname.length() - pos);
                host = tmp.before(',');
            }

            // Port
            FXString port = "";
            pos = gvfsname.find("port=") + 5;
            if (pos != -1)
            {
                tmp = gvfsname.right(gvfsname.length() - pos);
                port = tmp.before(',');
            }

            // User name
            FXString user;
            pos = gvfsname.find("user=") + 5;
            if (pos < 0)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't find user name"));
                return 0;
            }
            else
            {
                tmp = gvfsname.right(gvfsname.length() - pos);
                user = tmp.before(',');
            }

            // Build mount URI
            user += "@";
            if (port == "")
            {
                mounturi = "'sftp://" + user + host + "'";
            }
            else
            {
                mounturi = "'sftp://" + user + host + ":" + port + "'";
            }
        }

        // MTP device
        if (mounttype == "mtp")
        {
            FXString name = gvfsname.after('=');

            // BUILD mount URI
            mounturi = "'mtp://" + name + "/'";
        }

        // Select command and set appropriate message
        if (FXSELID(sel) == ID_MOUNT)
        {
            op = MOUNT;
            msg = _("Mount file system...");
            msgwait = _("Volume busy, please wait while mounting folder:");
            cmd = "gio mount ";
        }
        else
        {
            op = UNMOUNT;
            msg = _("Unmount file system...");
            msgwait = _("Volume busy, please wait while unmounting folder:");
            cmd = "gio mount -u ";
        }
        cmd += mounturi;
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
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                  startlocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
            }

            delete f;
            return 0;
        }

        getApp()->endWaitCursor();
        delete f;
    }
    // MTAB mount
    else
    {
        // If symbolic link
        if (xf_islink(dir))
        {
            dir = FXFile::symlink(dir);
        }

        // Select the command and set the appropriate message
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
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                  startlocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), startlocation.text());
            }

            delete f;
            return 0;
        }

        getApp()->endWaitCursor();
        delete f;
    }

    // If unmount, return to start location
    if (FXSELID(sel) == ID_UMOUNT)
    {
        dirlist->setDirectory(startlocation, true);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->setDirectory(startlocation);
        ((XFileExplorer*)mainWindow)->getCurrentPanel()->updatePath();        
    }

    // Force dirpanel refresh
    dirlist->handle(this, FXSEL(SEL_COMMAND, DirList::ID_REFRESH), NULL);

    return 1;
}


// Update the Mount menu item
long DirPanel::onUpdMount(FXObject* sender, FXSelector sel, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString dir = dirlist->getItemPathname((TreeItem*)item);

    if (fsdevices->find(dir.text()) && !mtdevices->find(dir.text()))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update the Unmount menu item
long DirPanel::onUpdUnmount(FXObject* sender, FXSelector sel, void*)
{
    // Current item
    DirItem* item = (DirItem*)dirlist->getCurrentItem();
    FXString dir = dirlist->getItemPathname((TreeItem*)item);

    // GVFS directory
    FXString gvfsdir = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";

    if ((FXPath::directory(dir) == gvfsdir) || (fsdevices->find(dir.text()) || mtdevices->find(dir.text())))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}

#endif


// Update the paste button
long DirPanel::onUpdPaste(FXObject* sender, FXSelector, void*)
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

        // Test if valid clipboard
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
long DirPanel::onUpdMenu(FXObject* sender, FXSelector, void*)
{
    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        return 0;
    }

    // Path name of the selected item
    FXString dir = dirlist->getItemPathname(item);
    return 1;
}


// Update menu items and toolbar buttons that are related to file operations
long DirPanel::onUpdDirDelete(FXObject* sender, FXSelector, void*)
{
    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        return 0;
    }

    // Path name of the selected item
    FXString dir = dirlist->getItemPathname(item);

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    FXbool use_trash_bypass = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_bypass", false);
    if ((!use_trash_can) | use_trash_bypass)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update menu items and toolbar buttons that are related to file operations
long DirPanel::onUpdDirTrash(FXObject* sender, FXSelector, void*)
{
    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        return 0;
    }

    // Path name of the selected item
    FXString dir = dirlist->getItemPathname(item);

    // Disable move to trash menu if we are in trash can
    // or if the trash can directory is selected
    FXbool trashenable = true;
    FXString trashparentdir = trashlocation.rbefore('/');

    if (dir.left(trashlocation.length()) == trashlocation)
    {
        trashenable = false;
    }

    if (dir == trashparentdir)
    {
        trashenable = false;
    }

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    if (use_trash_can && trashenable)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


long DirPanel::onUpdDirRestore(FXObject* sender, FXSelector, void*)
{
    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        return 0;
    }

    // Path name of the selected item
    FXString dir = dirlist->getItemPathname(item);

    // Enable restore from trash menu if we are in trash can
    FXbool restoreenable = false;
    if (dir.left(trashfileslocation.length() + 1) == trashfileslocation + PATHSEPSTRING)
    {
        restoreenable = true;
    }

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    if (use_trash_can && restoreenable)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Toggle dirsize refresh and force refresh if flag is true
void DirPanel::setAllowDirsizeRefresh(FXbool flag)
{
    allowDirsizeRefresh = flag;

    // Force refresh
    if (allowDirsizeRefresh)
    {
        curr_dirpath = "";
        onCmdDirsizeRefresh(0, 0, 0);
    }
}


// Refresh the directory size in the status bar
long DirPanel::onCmdDirsizeRefresh(FXObject* sender, FXSelector, void*)
{
    // Don't refresh if not allowed or window is minimized
    if (!allowDirsizeRefresh || ((FXTopWindow*)focuswindow)->isMinimized())
    {
        return 0;
    }

    FXulong dnsize = 0;
    char dsize[64];
    FXString hsize;

    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        status->setText(_("Free space:"));
        return 0;
    }

    // Path name of the selected item (without trailing '/' except for the root path)
    FXString path = xf_filepath(dirlist->getItemPathname(item), "");

    // Compute free space only if something has changed in directory
    struct stat statbuf;
    if (xf_lstat(path.text(), &statbuf) == 0)
    {
        if (!((path == curr_dirpath) && (statbuf.st_mtime == curr_mtime)))
        {
            // Update curr directory mtime
            curr_mtime = statbuf.st_mtime;

            // Update curr directory path
            curr_dirpath = path;

            // Free space on file system
            struct statvfs info;

            if (::statvfs(path.text(), &info) != -1)
            {
                dnsize = info.f_bavail * info.f_bsize;

                // Size in human readable form
#if __WORDSIZE == 64
                snprintf(dsize, sizeof(dsize), "%lu", dnsize);
#else
                snprintf(dsize, sizeof(dsize), "%llu", dnsize);
#endif
                hsize = xf_humansize(dsize);
            }
            else
            {
                hsize = "";
            }

            // Refresh the status label
            FXString str = _("Free space:") + FXString(" ") + hsize;
            status->setText(str);
        }
    }

    // Reset timer again
    getApp()->addTimeout(this, ID_DIRSIZE_REFRESH, DIRSIZE_REFRESH_INTERVAL);

    // Important : returning 0 here avoids to continuously update the GUI!
    return 0;
}


// Update the path name in the Window title
long DirPanel::onUpdTitle(FXObject* sender, FXSelector, void*)
{
    // Name of the current selected item
    TreeItem* item = (TreeItem*)dirlist->getCurrentItem();

    // There is no selected item
    if (item == NULL)
    {
        mainWindow->setTitle("Xfe - ");
        return 0;
    }

    // Path of the current directory in the file panel
    FilePanel* currentpanel = ((XFileExplorer*)mainWindow)->getCurrentPanel();
    FXString path = currentpanel->getDirectory();

    // Update the path in the window title
    if (getuid() == 0)
    {
        mainWindow->setTitle("Xfe (root) - " + path);
    }
    else
    {
        mainWindow->setTitle("Xfe - " + path);
    }

    return 1;
}


// Update dirsize refresh timer if the window gains focus
long DirPanel::onUpdDirsizeRefresh(FXObject*, FXSelector, void*)
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

    // Update timer if window is unminimized
    if ((prevMinimized == false) && (minimized == true))
    {
        onCmdDirsizeRefresh(0, 0, 0);
    }

    return 1;
}
