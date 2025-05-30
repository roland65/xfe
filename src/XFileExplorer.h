#ifndef XFILEEXPLORER_H
#define XFILEEXPLORER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>

#include "xfedefs.h"
#include "xfeutils.h"
#include "ComboBox.h"
#include "FileDict.h"
#include "FilePanel.h"
#include "BookmarkDialog.h"
#include "InputDialog.h"
#include "HistInputDialog.h"
#include "BrowseInputDialog.h"
#include "ConnectDialog.h"
#include "TabButtons.h"
#include "Properties.h"
#include "DirPanel.h"
#include "Preferences.h"
#include "TextWindow.h"
#include "SearchWindow.h"

// Typedef for the map between program string identifiers and integer indexes
typedef std::map<FXString, int> progsmap;


// Application object
class XFileExplorer : public FXMainWindow
{
    FXDECLARE(XFileExplorer)
protected:
    enum
    {
        TREE_PANEL,
        ONE_PANEL,
        TWO_PANELS,
        TREE_TWO_PANELS,
        FILEPANEL_FOCUS,
        DIRPANEL_FOCUS,
    };
    int panel_mode = -1;
    int panel_view = 0;
    FXbool vertpanels = false;
    FXSplitter* panelsplit = NULL;
    FXMenuBar* menubar = NULL;
    FXMenuPane* toolsmenu = NULL;
    FXMenuPane* filemenu = NULL;
    FXMenuPane* trashmenu = NULL;
    FXMenuPane* editmenu = NULL;
    FXMenuPane* bookmarksmenu = NULL;
    FXMenuPane* viewmenu = NULL;
    FXMenuPane* lpanelmenu = NULL;
    FXMenuPane* rpanelmenu = NULL;
    FXMenuPane* scriptsmenu = NULL;
    FXMenuPane* helpmenu = NULL;
    FXMenuTitle* toolsmenutitle = NULL;
    FXMenuTitle* filemenutitle = NULL;
    FXMenuTitle* trashmenutitle = NULL;
    FXMenuTitle* editmenutitle = NULL;
    FXMenuTitle* bookmarksmenutitle = NULL;
    FXMenuTitle* viewmenutitle = NULL;
    FXMenuTitle* lpanelmenutitle = NULL;
    FXMenuTitle* rpanelmenutitle = NULL;
    FXMenuTitle* scriptsmenutitle = NULL;
    FXMenuTitle* helpmenutitle = NULL;
    FXToolBar* generaltoolbar = NULL;
    FXToolBar* toolstoolbar = NULL;
    FXToolBar* paneltoolbar = NULL;
    FXToolBar* addresstoolbar = NULL;
    FXToolBar* tabtoolbar = NULL;
    TabButtons* tabbuttons = NULL;
    HistComboBox* addressbox = NULL;
    DirPanel* dirpanel = NULL;
    FilePanel* lpanel = NULL;
    FilePanel* rpanel = NULL;
    FXDockSite* topdock = NULL;
    FXDockSite* bottomdock = NULL;
    FXDockSite* leftdock = NULL;
    FXDockSite* rightdock = NULL;
    FXString trashfileslocation;
    FXString trashinfolocation;
    FXString startlocation;
    FXuint liststyle = 0;
    FXColor listbackcolor = FXRGB(0, 0, 0);
    FXColor listforecolor = FXRGB(0, 0, 0);
    FXColor highlightcolor = FXRGB(0, 0, 0);
    FXColor pbarcolor = FXRGB(0, 0, 0);
    FXColor attentioncolor = FXRGB(0, 0, 0);
    FXColor scrollbarcolor = FXRGB(0, 0, 0);
    FXArrowButton* btnbackhist = NULL;
    FXArrowButton* btnforwardhist = NULL;
    HistInputDialog* rundialog = NULL;
    BookmarkDialog* addbookmarkdialog = NULL;
    ConnectDialog* connectdialog = NULL;
    PreferencesBox* prefsdialog = NULL;
    TextWindow* helpwindow = NULL;
    FXString message;
    FXuint panelfocus = 0;
    FXString startdir1;
    FXString startdir2;
    FXbool startdirs = false;
    FXString startdir2_tab;
    vector_FXString startURIs;
    FXbool starticonic = false;
    FXbool startmaximized = false;
    FXbool smoothscroll = false;
    double twopanels_lpanel_pct = 0;                // Panel sizes, relatively to the window width (in percent)
    double treepanel_tree_pct = 0;
    double treetwopanels_tree_pct = 0;
    double treetwopanels_lpanel_pct = 0;
    FXString prevdir;
    int prev_width = 0;
    FXuint search_xpos = 0;
    FXuint search_ypos = 0;
    FXuint search_width = 0;
    FXuint search_height = 0;
    SearchWindow* searchwindow = NULL;
    progsmap progs;                                 // Map between program string identifiers and integer indexes
    FXbool winshow = true;                          // If false, do not show the Xfe window
    FXbool stop = false;                            // If true, stop Xfe immediately
    int nbstartfiles = 0;                           // Number of files to open on startup
    FXMenuCommand* copynamesmc = NULL;
    FXMenuSeparator* bookmarkssep = NULL;

    FXuint single_click = SINGLE_CLICK_NONE;        // Single click navigation
    FXbool file_tooltips = true;                    // File tooltips
    FXbool relative_resize = true;                  // Relative resizing of the panels and columns in detailed mode
    FXbool always_show_tabbar = true;                      // Always show tab bar
    FXbool save_win_pos = false;                    // Save window position

    FXuint idCol[NMAX_COLS] = { 0 };
    FXbool colShown[FileList::ID_COL_NAME + NMAX_COLS] = { 0 };
    FXuint nbCols = 0;

    typedef std::vector<FXMenuCommand*> vector_FXMenuCommand;
    vector_FXMenuCommand bookmarkmc;

public:
    enum
    {
        ID_ABOUT=FXMainWindow::ID_LAST,
        ID_HELP,
        ID_REFRESH,
        ID_EMPTY_TRASH,
        ID_TRASH_SIZE,
        ID_XTERM,
        ID_CONNECT_TO_SERVER,
        ID_DIR_UP,
        ID_DIR_BACK,
        ID_DIR_FORWARD,
        ID_DIR_BACK_HIST,
        ID_DIR_FORWARD_HIST,
        ID_FILE_PROPERTIES,
        ID_FILE_COPY,
        ID_FILE_COPYNAME,
        ID_FILE_RENAME,
        ID_FILE_MOVETO,
        ID_FILE_COPYTO,
        ID_FILE_CUT,
        ID_FILE_PASTE,
        ID_FILE_SYMLINK,
        ID_FILE_DELETE,
        ID_FILE_TRASH,
        ID_FILE_RESTORE,
        ID_FILE_ASSOC,
        ID_FILE_SEARCH,
        ID_CLEAR_ADDRESS,
        ID_GOTO_ADDRESS,
        ID_CHANGED,
        ID_RUN,
        ID_SU,
        ID_PREFS,
        ID_DIR_BOX,
        ID_TOGGLE_STATUS,
        ID_SHOW_ONE_PANEL,
        ID_SHOW_TWO_PANELS,
        ID_SHOW_FOLDERS_ONE_PANEL,
        ID_SHOW_FOLDERS_TWO_PANELS,
        ID_SYNCHRONIZE_PANELS,
        ID_SWITCH_PANELS,
        ID_RESTART,
        ID_NEW_WIN,
        ID_BOOKMARK,
        ID_ADD_BOOKMARK,
        ID_REMOVE_ALL_BOOKMARKS,
        ID_REBUILD_BOOKMARKS_MENU,
        ID_HARVEST,
        ID_QUIT,
        ID_FILE_ADDCOPY,
        ID_FILE_ADDCUT,
        ID_HORZ_PANELS,
        ID_VERT_PANELS,
        ID_LAST
    };

protected:
    XFileExplorer()
    {
    }

public:
    XFileExplorer(FXApp* app, vector_FXString URIs, const int pm = -1, const FXbool iconic = false,
                  const FXbool maximized = false, const char* title = "X File Explorer",
                  FXIcon* bigicon = NULL, FXIcon* miniicon = NULL);
    virtual void create();

    ~XFileExplorer();
    void saveConfig();

    void openFiles(vector_FXString);
    long onSigHarvest(FXObject*, FXSelector, void*);
    long onQuit(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onCmdHelp(FXObject*, FXSelector, void*);
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdFileAssoc(FXObject*, FXSelector, void*);
    long onCmdRefresh(FXObject*, FXSelector, void*);
    long onCmdToggleStatus(FXObject*, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdPrefs(FXObject*, FXSelector, void*);
    long onCmdRun(FXObject*, FXSelector, void*);
    long onCmdSu(FXObject*, FXSelector, void*);
    long onCmdConnectToServer(FXObject*, FXSelector, void*);
    long onCmdXTerm(FXObject*, FXSelector, void*);
    long onCmdEmptyTrash(FXObject*, FXSelector, void*);
    long onCmdTrashSize(FXObject*, FXSelector, void*);
    long onCmdHorzVertPanels(FXObject*, FXSelector, void*);
    long onCmdShowPanels(FXObject*, FXSelector, void*);
    long onCmdRestart(FXObject*, FXSelector, void*);
    long onCmdNewWindow(FXObject*, FXSelector, void*);
    long onCmdNewTab(FXObject*, FXSelector, void*);
    long onCmdGotoBookmark(FXObject*, FXSelector, void*);
    long onCmdAddBookmark(FXObject*, FXSelector, void*);
    long onCmdRemoveAllBookmarks(FXObject*, FXSelector, void*);
    long onUpdBookmarksMenu(FXObject*, FXSelector, void*);
    long onCmdRebuildBookmarksMenu(FXObject*, FXSelector, void*);
    long onCmdClearAddress(FXObject*, FXSelector, void*);
    long onCmdGotoAddress(FXObject*, FXSelector, void*);
    long onUpdToggleStatus(FXObject*, FXSelector, void*);
    long onUpdHorzVertPanels(FXObject*, FXSelector, void*);
    long onUpdShowPanels(FXObject*, FXSelector, void*);
    long onUpdAddress(FXObject*, FXSelector, void*);
    long onUpdEmptyTrash(FXObject*, FXSelector, void*);
    long onUpdTrashSize(FXObject*, FXSelector, void*);
    long onCmdFileDelete(FXObject*, FXSelector, void*);
    long onCmdFileTrash(FXObject*, FXSelector, void*);
    long onCmdFileRestore(FXObject*, FXSelector, void*);
    long onUpdFileDelete(FXObject*, FXSelector, void*);
    long onUpdFileTrash(FXObject*, FXSelector, void*);
    long onUpdFileRestore(FXObject*, FXSelector, void*);
    long onCmdFileSearch(FXObject*, FXSelector, void*);
    long onCmdDirUp(FXObject*, FXSelector, void*);
    long onCmdDirBack(FXObject*, FXSelector, void*);
    long onUpdDirBack(FXObject*, FXSelector, void*);
    long onCmdDirForward(FXObject*, FXSelector, void*);
    long onUpdDirForward(FXObject*, FXSelector, void*);
    long onCmdDirBackHist(FXObject*, FXSelector, void*);
    long onUpdDirBackHist(FXObject*, FXSelector, void*);
    long onCmdDirForwardHist(FXObject*, FXSelector, void*);
    long onUpdDirForwardHist(FXObject*, FXSelector, void*);
    long onCmdFileCopyClp(FXObject*, FXSelector, void*);
    long onCmdFileCopyName(FXObject*, FXSelector, void*);
    long onCmdFileCutClp(FXObject*, FXSelector, void*);
    long onCmdFileAddCopyClp(FXObject*, FXSelector, void*);
    long onCmdFileAddCutClp(FXObject*, FXSelector, void*);
    long onCmdFilePasteClp(FXObject*, FXSelector, void*);
    long onCmdFileRename(FXObject*, FXSelector, void*);
    long onCmdFileMoveto(FXObject*, FXSelector, void*);
    long onCmdFileCopyto(FXObject*, FXSelector, void*);
    long onCmdFileSymlink(FXObject*, FXSelector, void*);
    long onUpdFileMan(FXObject*, FXSelector, void*);
    long onUpdFilePaste(FXObject*, FXSelector, void*);
    long onCmdFileProperties(FXObject*, FXSelector, void*);
    long onUpdFileRename(FXObject*, FXSelector, void*);
    long onCmdSynchronizePanels(FXObject*, FXSelector, void*);
    long onUpdSynchronizePanels(FXObject*, FXSelector, void*);
    long onCmdSwitchPanels(FXObject*, FXSelector, void*);
    long onUpdSwitchPanels(FXObject*, FXSelector, void*);
    long onUpdSu(FXObject*, FXSelector, void*);
    long onUpdQuit(FXObject*, FXSelector, void*);
    long onUpdFileSearch(FXObject*, FXSelector, void*);
    long onCmdChanged(FXObject*, FXSelector, void*);

public:
    // Get associations
    FileDict* getAssociations()
    {
        return lpanel->getCurrent()->getAssociations();
    }

    // Change to selected directory
    void setDirectory(FXString pathname)
    {
        lpanel->getCurrent()->setDirectory(pathname, false);
        lpanel->getCurrent()->updatePath();
        dirpanel->setDirectory(pathname, true);
    }

    // Change default cursor for file and dir panels
    void setDefaultCursor(FXCursor* cur)
    {
        lpanel->setDefaultCursor(cur);
        rpanel->setDefaultCursor(cur);
        dirpanel->setDefaultCursor(cur);
    }

    // Deselect all items
    void deselectAll(void)
    {
        lpanel->deselectAll();
        rpanel->deselectAll();

        if (searchwindow)
        {
            searchwindow->deselectAll();
        }
    }

    // Refresh file panels
    void refreshPanels(void)
    {
        lpanel->onCmdRefresh(0, 0, 0);
        rpanel->onCmdRefresh(0, 0, 0);
    }

    // Return a pointer on the current file panel
    FilePanel* getCurrentPanel(void)
    {
        return lpanel->getCurrent();
    }

    // Return a pointer on the next file panel
    FilePanel* getNextPanel(void)
    {
        return lpanel->getNext();
    }

    // Return the address box (from location bar)
    ComboBox* getAddressBox(void)
    {
        return addressbox;
    }

    // Return a pointer on the directory panel
    DirPanel* getDirPanel(void)
    {
        return dirpanel;
    }

    // Return a pointer on the left panel
    FilePanel* getLeftPanel(void)
    {
        return lpanel;
    }

    // Return a pointer on the right panel
    FilePanel* getRightPanel(void)
    {
        return rpanel;
    }

    // Return a pointer on the search window
    SearchWindow* getSearchWindow(void)
    {
        return searchwindow;
    }

    // Return a pointer on the tab buttons
    TabButtons* getTabButtons(void)
    {
        return tabbuttons;
    }
};
#endif
