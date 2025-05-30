#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "startupnotification.h"
#include "File.h"
#include "FileList.h"
#include "FileDict.h"
#include "Preferences.h"
#include "FilePanel.h"
#include "InputDialog.h"
#include "HistInputDialog.h"
#include "DirPanel.h"
#include "MessageBox.h"
#include "TextWindow.h"
#include "CommandWindow.h"
#include "Places.h"
#include "FileDialog.h"
#include "help.h"
#include "DirHistBox.h"
#include "SearchWindow.h"
#include "XFileExplorer.h"


// Number of toolbars
#define NB_TOOLBARS     5

// Address bar length
#define ADDRESS_BAR_LENGTH    68
#define ADDRESS_BAR_HEIGHT    6

// Global variables
FXString clipboard = "";

// Run history
int RunNum = 0;
char RunHistory[RUN_HIST_SIZE][MAX_COMMAND_SIZE];

// Open history
char OpenHistory[OPEN_HIST_SIZE][MAX_COMMAND_SIZE];
int OpenNum = 0;

// Filter history
char FilterHistory[FILTER_HIST_SIZE][MAX_PATTERN_SIZE];
int FilterNum = 0;

// Search history
char FindHistory[FIND_HIST_SIZE][MAX_FIND_SIZE];
int FindNum = 0;

// Server history
char ServerHistory[SERVER_HIST_SIZE][MAX_SERVER_SIZE];
int ServerNum = 0;

// Share history
char ShareHistory[SHARE_HIST_SIZE][MAX_SHARE_SIZE];
int ShareNum = 0;

// Domain history
char DomainHistory[DOMAIN_HIST_SIZE][MAX_DOMAIN_SIZE];
int DomainNum = 0;

// User history
char UserHistory[USER_HIST_SIZE][MAX_USER_SIZE];
int UserNum = 0;


// External global variables
extern char** args;
extern FXString homedir;
extern FXString xdgdatahome;
extern FXString xdgconfighome;

#if defined(linux)
extern FXStringDict* fsdevices;
extern FXStringDict* updevices;
#endif


// Helper function to draw a toolbar separator
static void toolbarSeparator(FXToolBar* tb)
{
#define SEP_SPACE    1
    new FXFrame(tb, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
    new FXVerticalSeparator(tb, LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | SEPARATOR_GROOVE | LAYOUT_FILL_Y);
    new FXFrame(tb, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
}



// Map
FXDEFMAP(XFileExplorer) XFileExplorerMap[] =
{
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_TOGGLE_STATUS, XFileExplorer::onCmdToggleStatus),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_RUN, XFileExplorer::onCmdRun),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SU, XFileExplorer::onCmdSu),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_CONNECT_TO_SERVER, XFileExplorer::onCmdConnectToServer),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_COPY, XFileExplorer::onCmdFileCopyClp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_COPYNAME, XFileExplorer::onCmdFileCopyName),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_CUT, XFileExplorer::onCmdFileCutClp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_ADDCOPY, XFileExplorer::onCmdFileAddCopyClp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_ADDCUT, XFileExplorer::onCmdFileAddCutClp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_PASTE, XFileExplorer::onCmdFilePasteClp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_RENAME, XFileExplorer::onCmdFileRename),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_COPYTO, XFileExplorer::onCmdFileCopyto),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_MOVETO, XFileExplorer::onCmdFileMoveto),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_SYMLINK, XFileExplorer::onCmdFileSymlink),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_PROPERTIES, XFileExplorer::onCmdFileProperties),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_DELETE, XFileExplorer::onCmdFileDelete),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_TRASH, XFileExplorer::onCmdFileTrash),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_RESTORE, XFileExplorer::onCmdFileRestore),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_QUIT, XFileExplorer::onQuit),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_HELP, XFileExplorer::onCmdHelp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_ABOUT, XFileExplorer::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC, XFileExplorer::onCmdFileAssoc),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_REFRESH, XFileExplorer::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_EMPTY_TRASH, XFileExplorer::onCmdEmptyTrash),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_TRASH_SIZE, XFileExplorer::onCmdTrashSize),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_XTERM, XFileExplorer::onCmdXTerm),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_CLEAR_ADDRESS, XFileExplorer::onCmdClearAddress),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_GOTO_ADDRESS, XFileExplorer::onCmdGotoAddress),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_PREFS, XFileExplorer::onCmdPrefs),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_HORZ_PANELS, XFileExplorer::onCmdHorzVertPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_VERT_PANELS, XFileExplorer::onCmdHorzVertPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SHOW_ONE_PANEL, XFileExplorer::onCmdShowPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SHOW_TWO_PANELS, XFileExplorer::onCmdShowPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SHOW_FOLDERS_ONE_PANEL, XFileExplorer::onCmdShowPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SHOW_FOLDERS_TWO_PANELS, XFileExplorer::onCmdShowPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SYNCHRONIZE_PANELS, XFileExplorer::onCmdSynchronizePanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_SWITCH_PANELS, XFileExplorer::onCmdSwitchPanels),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_RESTART, XFileExplorer::onCmdRestart),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_NEW_WIN, XFileExplorer::onCmdNewWindow),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_ADD_BOOKMARK, XFileExplorer::onCmdAddBookmark),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_REMOVE_ALL_BOOKMARKS, XFileExplorer::onCmdRemoveAllBookmarks),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_BOOKMARK, XFileExplorer::onCmdGotoBookmark),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_REBUILD_BOOKMARKS_MENU, XFileExplorer::onCmdRebuildBookmarksMenu),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_REMOVE_ALL_BOOKMARKS, XFileExplorer::onUpdBookmarksMenu),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_DIR_UP, XFileExplorer::onCmdDirUp),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_DIR_BACK, XFileExplorer::onCmdDirBack),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_DIR_FORWARD, XFileExplorer::onCmdDirForward),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_DIR_BACK_HIST, XFileExplorer::onCmdDirBackHist),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_DIR_FORWARD_HIST, XFileExplorer::onCmdDirForwardHist),
    FXMAPFUNC(SEL_KEYPRESS, 0, XFileExplorer::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, XFileExplorer::onKeyRelease),
    FXMAPFUNC(SEL_SIGNAL, XFileExplorer::ID_HARVEST, XFileExplorer::onSigHarvest),
    FXMAPFUNC(SEL_SIGNAL, XFileExplorer::ID_QUIT, XFileExplorer::onQuit),
    FXMAPFUNC(SEL_CLOSE, 0, XFileExplorer::onQuit),
    FXMAPFUNC(SEL_COMMAND, XFileExplorer::ID_FILE_SEARCH, XFileExplorer::onCmdFileSearch),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_DIR_FORWARD_HIST, XFileExplorer::onUpdDirForwardHist),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_DIR_BACK_HIST, XFileExplorer::onUpdDirBackHist),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_DIR_BACK, XFileExplorer::onUpdDirBack),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_DIR_FORWARD, XFileExplorer::onUpdDirForward),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_TOGGLE_STATUS, XFileExplorer::onUpdToggleStatus),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_HORZ_PANELS, XFileExplorer::onUpdHorzVertPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_VERT_PANELS, XFileExplorer::onUpdHorzVertPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SHOW_ONE_PANEL, XFileExplorer::onUpdShowPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SHOW_TWO_PANELS, XFileExplorer::onUpdShowPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SHOW_FOLDERS_ONE_PANEL, XFileExplorer::onUpdShowPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SHOW_FOLDERS_TWO_PANELS, XFileExplorer::onUpdShowPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_GOTO_ADDRESS, XFileExplorer::onUpdAddress),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_EMPTY_TRASH, XFileExplorer::onUpdEmptyTrash),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_EMPTY_TRASH, XFileExplorer::onUpdTrashSize),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_DELETE, XFileExplorer::onUpdFileDelete),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_TRASH, XFileExplorer::onUpdFileTrash),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_RESTORE, XFileExplorer::onUpdFileRestore),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_RENAME, XFileExplorer::onUpdFileRename),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_PROPERTIES, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_COPYTO, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_MOVETO, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_SYMLINK, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_COPY, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_COPYNAME, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_CUT, XFileExplorer::onUpdFileMan),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_PASTE, XFileExplorer::onUpdFilePaste),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SYNCHRONIZE_PANELS, XFileExplorer::onUpdSynchronizePanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SWITCH_PANELS, XFileExplorer::onUpdSwitchPanels),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_SU, XFileExplorer::onUpdSu),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_QUIT, XFileExplorer::onUpdQuit),
    FXMAPFUNC(SEL_UPDATE, XFileExplorer::ID_FILE_SEARCH, XFileExplorer::onUpdFileSearch),

};


// Object implementation
FXIMPLEMENT(XFileExplorer, FXMainWindow, XFileExplorerMap, ARRAYNUMBER(XFileExplorerMap))


// Construct
XFileExplorer::XFileExplorer(FXApp* app, vector_FXString URIs, const int pm, const FXbool iconic, const FXbool maximized,
                             const char* title, FXIcon* bigicon, FXIcon* miniicon) :
    FXMainWindow(app, title, bigicon, miniicon,
                 DECOR_TITLE | DECOR_MINIMIZE | DECOR_MAXIMIZE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE)
{
    // Panel mode
    panel_mode = pm;

    // Menu bar
    menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE);

    
    // Position toolbars
    
    // Sites where to dock toolbars
    topdock = new FXDockSite(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    bottomdock = new FXDockSite(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    leftdock = new FXDockSite(this, LAYOUT_SIDE_LEFT | LAYOUT_FILL_Y);
    rightdock = new FXDockSite(this, LAYOUT_SIDE_RIGHT | LAYOUT_FILL_Y);
    
    // Get toolbar shown status and galley placement from registry
    // shown (0 or 1); docking side (top, bottom, left or right) ; galley (same or next)

    FXString key;
    FXString keydef = FXString("1") + ";" + FXStringVal(LAYOUT_SIDE_TOP) + ";" + FXStringVal(LAYOUT_DOCK_SAME); // Same galley

    key = getApp()->reg().readStringEntry("OPTIONS", "generaltoolbar", keydef.text());
    FXbool general_shown = FXIntVal(key.section(';', 0));
    int general_galley = FXIntVal(key.section(';', 2));

    key = getApp()->reg().readStringEntry("OPTIONS", "toolstoolbar", keydef.text());
    FXbool tools_shown = FXIntVal(key.section(';', 0));
    int tools_galley = FXIntVal(key.section(';', 2));
    
    key = getApp()->reg().readStringEntry("OPTIONS", "paneltoolbar", keydef.text());
    FXbool panel_shown = FXIntVal(key.section(';', 0));
    int panel_galley = FXIntVal(key.section(';', 2));

    keydef = FXString("1") + ";" + FXStringVal(LAYOUT_SIDE_TOP) + ";" + FXStringVal(LAYOUT_DOCK_NEXT); // Next galley
    key = getApp()->reg().readStringEntry("OPTIONS", "addresstoolbar", keydef.text());
    FXbool address_shown = FXIntVal(key.section(';', 0));
    int address_galley = FXIntVal(key.section(';', 2));
  
    keydef = FXString("1") + ";" + FXStringVal(LAYOUT_SIDE_TOP) + ";" + FXStringVal(LAYOUT_DOCK_SAME); // Same galley
    key = getApp()->reg().readStringEntry("OPTIONS", "tabtoolbar", keydef.text());
    FXbool tab_shown = FXIntVal(key.section(';', 0));
    int tab_galley = FXIntVal(key.section(';', 2));
    
    // Read toolbar positions from registry

    std::vector<int> top, bottom, left, right;

    key = getApp()->reg().readStringEntry("OPTIONS", "top_toolbars", "0;1;2;3;4");
    for (int i = 0; i < NB_TOOLBARS; i++)
    {
       FXString str = key.section(';', i);
       
       if (str != "")
        {
            top.push_back(FXIntVal(str));
        }
        else
        {
            break;
        }
    }

    key = getApp()->reg().readStringEntry("OPTIONS", "bottom_toolbars", "");
    for (int i = 0; i < NB_TOOLBARS; i++)
    {
       FXString str = key.section(';', i);
       
       if (str != "")
        {
            bottom.push_back(FXIntVal(str));
        }
        else
        {
            break;
        }
    }

    key = getApp()->reg().readStringEntry("OPTIONS", "left_toolbars", "");
    for (int i = 0; i < NB_TOOLBARS; i++)
    {
       FXString str = key.section(';', i);
       
       if (str != "")
        {
            left.push_back(FXIntVal(str));
        }
        else
        {
            break;
        }
    }
    
    key = getApp()->reg().readStringEntry("OPTIONS", "right_toolbars", "");
    for (int i = 0; i < NB_TOOLBARS; i++)
    {
       FXString str = key.section(';', i);
       
       if (str != "")
        {
            right.push_back(FXIntVal(str));
        }
        else
        {
            break;
        }
    }

    // Construct toolbars at each docking side
    
    // Toolbars at top side
    for (FXuint i = 0; i < top.size(); i++)
    {
        // General toolbar
        if (top[i] == 0)
        {            
            FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
            generaltoolbar = new FXToolBar(topdock, dragshell1, general_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
            new FXToolBarGrip(generaltoolbar, generaltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
            generaltoolbar->show();
        }
        
        // Tools toolbar
        if (top[i] == 1)
        {            
            FXToolBarShell* dragshell2 = new FXToolBarShell(this, FRAME_NONE);
            toolstoolbar = new FXToolBar(topdock, dragshell2, tools_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
            new FXToolBarGrip(toolstoolbar, toolstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
       
        // Panel toolbar
        if (top[i] == 2)
        {
            FXToolBarShell* dragshell3 = new FXToolBarShell(this, FRAME_NONE);
            paneltoolbar = new FXToolBar(topdock, dragshell3, panel_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
            new FXToolBarGrip(paneltoolbar, paneltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Address toolbar
        if (top[i] == 3)
        {
            FXToolBarShell* dragshell4 = new FXToolBarShell(this, FRAME_NONE);
            addresstoolbar = new FXToolBar(topdock, dragshell4, address_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
            new FXToolBarGrip(addresstoolbar, addresstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }

        // Tab toolbar
        if (top[i] == 4)
        {
            FXToolBarShell* dragshell5 = new FXToolBarShell(this, FRAME_NONE);
            tabtoolbar = new FXToolBar(topdock, dragshell5, tab_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
            new FXToolBarGrip(tabtoolbar, tabtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
    }

    // Toolbars at bottom side
    for (FXuint i = 0; i < bottom.size(); i++)
    {
        // General toolbar
        if (bottom[i] == 0)
        {            
            FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
            generaltoolbar = new FXToolBar(bottomdock, dragshell1, general_galley | FRAME_NONE | LAYOUT_SIDE_BOTTOM);
            new FXToolBarGrip(generaltoolbar, generaltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Tools toolbar
        if (bottom[i] == 1)
        {            
            FXToolBarShell* dragshell2 = new FXToolBarShell(this, FRAME_NONE);
            toolstoolbar = new FXToolBar(bottomdock, dragshell2, tools_galley | FRAME_NONE | LAYOUT_SIDE_BOTTOM);
            new FXToolBarGrip(toolstoolbar, toolstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Panel toolbar
        if (bottom[i] == 2)
        {
            FXToolBarShell* dragshell3 = new FXToolBarShell(this, FRAME_NONE);
            paneltoolbar = new FXToolBar(bottomdock, dragshell3, panel_galley | FRAME_NONE | LAYOUT_SIDE_BOTTOM);
            new FXToolBarGrip(paneltoolbar, paneltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }

        // Address toolbar
        if (bottom[i] == 3)
        {
            FXToolBarShell* dragshell4 = new FXToolBarShell(this, FRAME_NONE);
            addresstoolbar = new FXToolBar(bottomdock, dragshell4, address_galley | FRAME_NONE | LAYOUT_SIDE_BOTTOM);
            new FXToolBarGrip(addresstoolbar, addresstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Tab toolbar
        if (bottom[i] == 4)
        {
            FXToolBarShell* dragshell5 = new FXToolBarShell(this, FRAME_NONE);
            tabtoolbar = new FXToolBar(bottomdock, dragshell5, tab_galley | FRAME_NONE | LAYOUT_SIDE_BOTTOM);
            new FXToolBarGrip(tabtoolbar, tabtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
    }

    // Toolbars at left side
    for (FXuint i = 0; i < left.size(); i++)
    {
        // General toolbar
        if (left[i] == 0)
        {            
            FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
            generaltoolbar = new FXToolBar(leftdock, dragshell1, general_galley | FRAME_NONE | LAYOUT_SIDE_LEFT);
            new FXToolBarGrip(generaltoolbar, generaltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Tools toolbar
        if (left[i] == 1)
        {            
            FXToolBarShell* dragshell2 = new FXToolBarShell(this, FRAME_NONE);
            toolstoolbar = new FXToolBar(leftdock, dragshell2, tools_galley | FRAME_NONE | LAYOUT_SIDE_LEFT);
            new FXToolBarGrip(toolstoolbar, toolstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
       
        // Panel toolbar
        if (left[i] == 2)
        {
            FXToolBarShell* dragshell3 = new FXToolBarShell(this, FRAME_NONE);
            paneltoolbar = new FXToolBar(leftdock, dragshell3, panel_galley | FRAME_NONE | LAYOUT_SIDE_LEFT);
            new FXToolBarGrip(paneltoolbar, paneltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Address toolbar
        if (left[i] == 3)
        {
            FXToolBarShell* dragshell4 = new FXToolBarShell(this, FRAME_NONE);
            addresstoolbar = new FXToolBar(leftdock, dragshell4, address_galley | FRAME_NONE | LAYOUT_SIDE_LEFT);
            new FXToolBarGrip(addresstoolbar, addresstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }

        // Tab toolbar
        if (left[i] == 4)
        {
            FXToolBarShell* dragshell5 = new FXToolBarShell(this, FRAME_NONE);
            tabtoolbar = new FXToolBar(leftdock, dragshell5, tab_galley | FRAME_NONE | LAYOUT_SIDE_LEFT);
            new FXToolBarGrip(tabtoolbar, tabtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
    }

    // Toolbars at right side
    for (FXuint i = 0; i < right.size(); i++)
    {
        // General toolbar
        if (right[i] == 0)
        {            
            FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
            generaltoolbar = new FXToolBar(rightdock, dragshell1, general_galley | FRAME_NONE | LAYOUT_SIDE_RIGHT);
            new FXToolBarGrip(generaltoolbar, generaltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Tools toolbar
        if (right[i] == 1)
        {            
            FXToolBarShell* dragshell2 = new FXToolBarShell(this, FRAME_NONE);
            toolstoolbar = new FXToolBar(rightdock, dragshell2, tools_galley | FRAME_NONE | LAYOUT_SIDE_RIGHT);
            new FXToolBarGrip(toolstoolbar, toolstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Panel toolbar
        if (right[i] == 2)
        {
            FXToolBarShell* dragshell3 = new FXToolBarShell(this, FRAME_NONE);
            paneltoolbar = new FXToolBar(rightdock, dragshell3, panel_galley | FRAME_NONE | LAYOUT_SIDE_RIGHT);
            new FXToolBarGrip(paneltoolbar, paneltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }

        // Address toolbar
        if (right[i] == 3)
        {
            FXToolBarShell* dragshell4 = new FXToolBarShell(this, FRAME_NONE);
            addresstoolbar = new FXToolBar(rightdock, dragshell4, address_galley | FRAME_NONE | LAYOUT_SIDE_RIGHT);
            new FXToolBarGrip(addresstoolbar, addresstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
        
        // Tab toolbar
        if (right[i] == 4)
        {
            FXToolBarShell* dragshell5 = new FXToolBarShell(this, FRAME_NONE);
            tabtoolbar = new FXToolBar(rightdock, dragshell5, tab_galley | FRAME_NONE | LAYOUT_SIDE_RIGHT);
            new FXToolBarGrip(tabtoolbar, tabtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        }
    }

    // Show / hide toolbars (construct toolbars if they are not shown)
    
    // General toolbar
    if (!general_shown)
    {
        FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
        generaltoolbar = new FXToolBar(topdock, dragshell1, general_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
        new FXToolBarGrip(generaltoolbar, generaltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);        
        
        generaltoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);
    }
    else
    {
        generaltoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);        
    }
        
    // Tools toolbar
    if (!tools_shown)
    {            
        FXToolBarShell* dragshell2 = new FXToolBarShell(this, FRAME_NONE);
        toolstoolbar = new FXToolBar(topdock, dragshell2, tools_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
        new FXToolBarGrip(toolstoolbar, toolstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        
        toolstoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);
    }
    else
    {
        toolstoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);        
    }
   
    // Panel toolbar
    if (!panel_shown)
    {
        FXToolBarShell* dragshell3 = new FXToolBarShell(this, FRAME_NONE);
        paneltoolbar = new FXToolBar(topdock, dragshell3, panel_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
        new FXToolBarGrip(paneltoolbar, paneltoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        
        paneltoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);
    }
    else
    {
        paneltoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);        
    }
    
    // Address toolbar
    if (!address_shown)
    {
        FXToolBarShell* dragshell4 = new FXToolBarShell(this, FRAME_NONE);
        addresstoolbar = new FXToolBar(topdock, dragshell4, address_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
        new FXToolBarGrip(addresstoolbar, addresstoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        
        addresstoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);
    }
    else
    {
        addresstoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);        
    }

    // Tab toolbar
    if (!tab_shown)
    {
        FXToolBarShell* dragshell5 = new FXToolBarShell(this, FRAME_NONE);
        tabtoolbar = new FXToolBar(topdock, dragshell5, tab_galley | FRAME_NONE | LAYOUT_SIDE_TOP);
        new FXToolBarGrip(tabtoolbar, tabtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
        
        tabtoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);
    }
    else
    {
        tabtoolbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);        
    }


    // Main splitter
    FXHorizontalFrame* hframe = new FXHorizontalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0);
    FXSplitter* mainsplit = new FXSplitter(hframe, LAYOUT_SIDE_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                           SPLITTER_TRACKING | FRAME_NONE);

    // File list background, foreground, highlight, progress bar and attention colors
    listbackcolor = getApp()->reg().readColorEntry("SETTINGS", "listbackcolor", FXRGB(255, 255, 255));
    listforecolor = getApp()->reg().readColorEntry("SETTINGS", "listforecolor", FXRGB(0, 0, 0));
    highlightcolor = getApp()->reg().readColorEntry("SETTINGS", "highlightcolor", FXRGB(238, 238, 238));
    pbarcolor = getApp()->reg().readColorEntry("SETTINGS", "pbarcolor", FXRGB(10, 36, 106));
    attentioncolor = getApp()->reg().readColorEntry("SETTINGS", "attentioncolor", FXRGB(255, 0, 0));
    scrollbarcolor = getApp()->reg().readColorEntry("SETTINGS", "scrollbarcolor", FXRGB(237, 236, 235));

    // Smooth scrolling
    smoothscroll = getApp()->reg().readUnsignedEntry("SETTINGS", "smooth_scroll", true);

    // Directory panel on the left (with minimum size)
    dirpanel = new DirPanel(this, mainsplit, listbackcolor, listforecolor, smoothscroll,
                            LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0);

    // Splitter containing the two panels
    panelsplit = new FXSplitter(mainsplit, LAYOUT_SIDE_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                SPLITTER_TRACKING | FRAME_NONE);

    // Stack file panels horizontally or vertically (directory panel is always vertical)
    vertpanels = getApp()->reg().readUnsignedEntry("OPTIONS", "vert_panels", true);
    if (vertpanels)
    {
        panelsplit->setSplitterStyle(panelsplit->getSplitterStyle() & ~SPLITTER_VERTICAL);
    }
    else
    {
        panelsplit->setSplitterStyle(panelsplit->getSplitterStyle() | SPLITTER_VERTICAL);
    }

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);

    // Read file list columns order and shown status
    FXuint i = FileList::ID_COL_NAME;
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_name", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);

    FXuint id = 0;
    i = 0;

    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_0", FileList::ID_COL_NAME);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_1", FileList::ID_COL_SIZE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_2", FileList::ID_COL_TYPE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_3", FileList::ID_COL_EXT);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_4", FileList::ID_COL_DATE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_5", FileList::ID_COL_USER);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_6", FileList::ID_COL_GROUP);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_7", FileList::ID_COL_PERMS);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_8", FileList::ID_COL_LINK);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    nbCols = i;

    // File panels on the right : remembers size of each field
    lpanel = new FilePanel(this, "LEFT PANEL", panelsplit, dirpanel, idCol, nbCols,
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "name_size", MIN_NAME_SIZE),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "size_size", 60),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "type_size", 100),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "ext_size", 100),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "date_size", 150),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "user_size", 50),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "group_size", 50),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "perms_size", 100),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "link_size", 100),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "deldate_size", 150),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "origpath_size", 200),
                           getApp()->reg().readUnsignedEntry("LEFT PANEL", "showthumbnails", 0),
                           listbackcolor, listforecolor, attentioncolor, smoothscroll,
                           LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0);

    rpanel = new FilePanel(this, "RIGHT PANEL", panelsplit, dirpanel, idCol, nbCols,
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "name_size", MIN_NAME_SIZE),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "size_size", 60),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "type_size", 100),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "ext_size", 100),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "date_size", 150),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "user_size", 50),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "group_size", 50),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "perms_size", 100),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "link_size", 100),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "deldate_size", 150),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "origpath_size", 200),
                           getApp()->reg().readUnsignedEntry("RIGHT PANEL", "showthumbnails", 0),
                           listbackcolor, listforecolor, attentioncolor, smoothscroll,
                           LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0);

    lpanel->Next(rpanel);
    rpanel->Next(lpanel);
    lpanel->setActive();


    // Dir panel options
    FXString sort_func;
    sort_func = getApp()->reg().readStringEntry("DIR PANEL", "sort_func", "ascendingCase");
    if (sort_func == "ascendingCase")
    {
        dirpanel->setSortFunc(DirList::ascendingCase);
    }
    else if (sort_func == "descendingCase")
    {
        dirpanel->setSortFunc(DirList::descendingCase);
    }
    else if (sort_func == "ascending")
    {
        dirpanel->setSortFunc(DirList::ascending);
    }
    else if (sort_func == "descending")
    {
        dirpanel->setSortFunc(DirList::descending);
    }
    else
    {
        dirpanel->setSortFunc(DirList::ascendingCase);
    }

    // Left panel options
    sort_func = getApp()->reg().readStringEntry("LEFT PANEL", "sort_func", "ascendingCase");
    if (sort_func == "ascendingCase")
    {
        lpanel->setSortFunc(FileList::ascendingCase);
    }
    else if (sort_func == "ascendingCaseMix")
    {
        lpanel->setSortFunc(FileList::ascendingCaseMix);
    }
    else if (sort_func == "descendingCase")
    {
        lpanel->setSortFunc(FileList::descendingCase);
    }
    else if (sort_func == "descendingCaseMix")
    {
        lpanel->setSortFunc(FileList::descendingCaseMix);
    }
    else if (sort_func == "ascending")
    {
        lpanel->setSortFunc(FileList::ascending);
    }
    else if (sort_func == "ascendingMix")
    {
        lpanel->setSortFunc(FileList::ascendingMix);
    }
    else if (sort_func == "descending")
    {
        lpanel->setSortFunc(FileList::descending);
    }
    else if (sort_func == "descendingMix")
    {
        lpanel->setSortFunc(FileList::descendingMix);
    }
    else if (sort_func == "ascendingSize")
    {
        lpanel->setSortFunc(FileList::ascendingSize);
    }
    else if (sort_func == "ascendingSizeMix")
    {
        lpanel->setSortFunc(FileList::ascendingSizeMix);
    }
    else if (sort_func == "descendingSize")
    {
        lpanel->setSortFunc(FileList::descendingSize);
    }
    else if (sort_func == "descendingSizeMix")
    {
        lpanel->setSortFunc(FileList::descendingSizeMix);
    }
    else if (sort_func == "ascendingType")
    {
        lpanel->setSortFunc(FileList::ascendingType);
    }
    else if (sort_func == "ascendingTypeMix")
    {
        lpanel->setSortFunc(FileList::ascendingTypeMix);
    }
    else if (sort_func == "descendingType")
    {
        lpanel->setSortFunc(FileList::descendingType);
    }
    else if (sort_func == "descendingTypeMix")
    {
        lpanel->setSortFunc(FileList::descendingTypeMix);
    }
    else if (sort_func == "ascendingExt")
    {
        lpanel->setSortFunc(FileList::ascendingExt);
    }
    else if (sort_func == "ascendingExtMix")
    {
        lpanel->setSortFunc(FileList::ascendingExtMix);
    }
    else if (sort_func == "descendingExt")
    {
        lpanel->setSortFunc(FileList::descendingExt);
    }
    else if (sort_func == "descendingExtMix")
    {
        lpanel->setSortFunc(FileList::descendingExtMix);
    }
    else if (sort_func == "ascendingDate")
    {
        lpanel->setSortFunc(FileList::ascendingDate);
    }
    else if (sort_func == "ascendingDateMix")
    {
        lpanel->setSortFunc(FileList::ascendingDateMix);
    }
    else if (sort_func == "descendingDate")
    {
        lpanel->setSortFunc(FileList::descendingDate);
    }
    else if (sort_func == "descendingDateMix")
    {
        lpanel->setSortFunc(FileList::descendingDateMix);
    }
    else if (sort_func == "ascendingUser")
    {
        lpanel->setSortFunc(FileList::ascendingUser);
    }
    else if (sort_func == "ascendingUserMix")
    {
        lpanel->setSortFunc(FileList::ascendingUserMix);
    }
    else if (sort_func == "descendingUser")
    {
        lpanel->setSortFunc(FileList::descendingUser);
    }
    else if (sort_func == "descendingUserMix")
    {
        lpanel->setSortFunc(FileList::descendingUserMix);
    }
    else if (sort_func == "ascendingGroup")
    {
        lpanel->setSortFunc(FileList::ascendingGroup);
    }
    else if (sort_func == "ascendingGroupMix")
    {
        lpanel->setSortFunc(FileList::ascendingGroupMix);
    }
    else if (sort_func == "descendingGroup")
    {
        lpanel->setSortFunc(FileList::descendingGroup);
    }
    else if (sort_func == "descendingGroupMix")
    {
        lpanel->setSortFunc(FileList::descendingGroupMix);
    }
    else if (sort_func == "ascendingPerms")
    {
        lpanel->setSortFunc(FileList::ascendingPerms);
    }
    else if (sort_func == "ascendingPermsMix")
    {
        lpanel->setSortFunc(FileList::ascendingPermsMix);
    }
    else if (sort_func == "descendingPerms")
    {
        lpanel->setSortFunc(FileList::descendingPerms);
    }
    else if (sort_func == "descendingPermsMix")
    {
        lpanel->setSortFunc(FileList::descendingPermsMix);
    }
    else if (sort_func == "ascendingLink")
    {
        lpanel->setSortFunc(FileList::ascendingLink);
    }
    else if (sort_func == "ascendingLinkMix")
    {
        lpanel->setSortFunc(FileList::ascendingLinkMix);
    }
    else if (sort_func == "descendingLink")
    {
        lpanel->setSortFunc(FileList::descendingLink);
    }
    else if (sort_func == "descendingLinkMix")
    {
        lpanel->setSortFunc(FileList::descendingLinkMix);
    }
    else
    {
        lpanel->setSortFunc(FileList::ascendingCase);
    }
    FXuint ignore_case = getApp()->reg().readUnsignedEntry("LEFT PANEL", "ignore_case", 1);
    lpanel->setIgnoreCase(ignore_case);
    FXuint dirs_first = getApp()->reg().readUnsignedEntry("LEFT PANEL", "dirs_first", 1);
    lpanel->setDirsFirst(dirs_first);

    // Right panel options
    sort_func = getApp()->reg().readStringEntry("RIGHT PANEL", "sort_func", "ascendingCase");
    if (sort_func == "ascendingCase")
    {
        rpanel->setSortFunc(FileList::ascendingCase);
    }
    else if (sort_func == "ascendingCaseMix")
    {
        rpanel->setSortFunc(FileList::ascendingCaseMix);
    }
    else if (sort_func == "descendingCase")
    {
        rpanel->setSortFunc(FileList::descendingCase);
    }
    else if (sort_func == "descendingCaseMix")
    {
        rpanel->setSortFunc(FileList::descendingCaseMix);
    }
    else if (sort_func == "ascending")
    {
        rpanel->setSortFunc(FileList::ascending);
    }
    else if (sort_func == "ascendingMix")
    {
        rpanel->setSortFunc(FileList::ascendingMix);
    }
    else if (sort_func == "descending")
    {
        rpanel->setSortFunc(FileList::descending);
    }
    else if (sort_func == "descendingMix")
    {
        rpanel->setSortFunc(FileList::descendingMix);
    }
    else if (sort_func == "ascendingSize")
    {
        rpanel->setSortFunc(FileList::ascendingSize);
    }
    else if (sort_func == "ascendingSizeMix")
    {
        rpanel->setSortFunc(FileList::ascendingSizeMix);
    }
    else if (sort_func == "descendingSize")
    {
        rpanel->setSortFunc(FileList::descendingSize);
    }
    else if (sort_func == "descendingSizeMix")
    {
        rpanel->setSortFunc(FileList::descendingSizeMix);
    }
    else if (sort_func == "ascendingType")
    {
        rpanel->setSortFunc(FileList::ascendingType);
    }
    else if (sort_func == "ascendingTypeMix")
    {
        rpanel->setSortFunc(FileList::ascendingTypeMix);
    }
    else if (sort_func == "descendingType")
    {
        rpanel->setSortFunc(FileList::descendingType);
    }
    else if (sort_func == "descendingTypeMix")
    {
        rpanel->setSortFunc(FileList::descendingTypeMix);
    }
    else if (sort_func == "ascendingExt")
    {
        rpanel->setSortFunc(FileList::ascendingExt);
    }
    else if (sort_func == "ascendingExtMix")
    {
        rpanel->setSortFunc(FileList::ascendingExtMix);
    }
    else if (sort_func == "descendingExt")
    {
        rpanel->setSortFunc(FileList::descendingExt);
    }
    else if (sort_func == "descendingExtMix")
    {
        rpanel->setSortFunc(FileList::descendingExtMix);
    }
    else if (sort_func == "ascendingDate")
    {
        rpanel->setSortFunc(FileList::ascendingDate);
    }
    else if (sort_func == "ascendingDateMix")
    {
        rpanel->setSortFunc(FileList::ascendingDateMix);
    }
    else if (sort_func == "descendingDate")
    {
        rpanel->setSortFunc(FileList::descendingDate);
    }
    else if (sort_func == "descendingDateMix")
    {
        rpanel->setSortFunc(FileList::descendingDateMix);
    }
    else if (sort_func == "ascendingUser")
    {
        rpanel->setSortFunc(FileList::ascendingUser);
    }
    else if (sort_func == "ascendingUserMix")
    {
        rpanel->setSortFunc(FileList::ascendingUserMix);
    }
    else if (sort_func == "descendingUser")
    {
        rpanel->setSortFunc(FileList::descendingUser);
    }
    else if (sort_func == "descendingUserMix")
    {
        rpanel->setSortFunc(FileList::descendingUserMix);
    }
    else if (sort_func == "ascendingGroup")
    {
        rpanel->setSortFunc(FileList::ascendingGroup);
    }
    else if (sort_func == "ascendingGroupMix")
    {
        rpanel->setSortFunc(FileList::ascendingGroupMix);
    }
    else if (sort_func == "descendingGroup")
    {
        rpanel->setSortFunc(FileList::descendingGroup);
    }
    else if (sort_func == "descendingGroupMix")
    {
        rpanel->setSortFunc(FileList::descendingGroupMix);
    }
    else if (sort_func == "ascendingPerms")
    {
        rpanel->setSortFunc(FileList::ascendingPerms);
    }
    else if (sort_func == "ascendingPermsMix")
    {
        rpanel->setSortFunc(FileList::ascendingPermsMix);
    }
    else if (sort_func == "descendingPerms")
    {
        rpanel->setSortFunc(FileList::descendingPerms);
    }
    else if (sort_func == "descendingPermsMix")
    {
        rpanel->setSortFunc(FileList::descendingPermsMix);
    }
    else if (sort_func == "ascendingLink")
    {
        rpanel->setSortFunc(FileList::ascendingLink);
    }
    else if (sort_func == "ascendingLinkMix")
    {
        rpanel->setSortFunc(FileList::ascendingLinkMix);
    }
    else if (sort_func == "descendingLink")
    {
        rpanel->setSortFunc(FileList::descendingLink);
    }
    else if (sort_func == "descendingLinkMix")
    {
        rpanel->setSortFunc(FileList::descendingLinkMix);
    }
    else
    {
        rpanel->setSortFunc(FileList::ascendingCase);
    }
    ignore_case = getApp()->reg().readUnsignedEntry("RIGHT PANEL", "ignore_case", 1);
    rpanel->setIgnoreCase(ignore_case);
    dirs_first = getApp()->reg().readUnsignedEntry("RIGHT PANEL", "dirs_first", 1);
    rpanel->setDirsFirst(dirs_first);

    FXButton* btn = NULL;
    FXHotKey hotkey;

    // Tab buttons
    tabbuttons = new TabButtons(tabtoolbar, lpanel, dirpanel, FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    // General toolbar
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_back", "Ctrl-Backspace");
    btn = new FXButton(generaltoolbar, TAB + _("Go to Previous Folder") + PARS(key), minidirbackicon, this,
                       XFileExplorer::ID_DIR_BACK, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    btnbackhist = new FXArrowButton(generaltoolbar, this, XFileExplorer::ID_DIR_BACK_HIST,
                                    LAYOUT_FILL_Y | FRAME_NONE | ARROW_DOWN | ARROW_TOOLBAR);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_forward", "Shift-Backspace");
    btn = new FXButton(generaltoolbar, TAB + _("Go to Next Folder") + PARS(key), minidirforwardicon, this,
                       XFileExplorer::ID_DIR_FORWARD, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    btnforwardhist = new FXArrowButton(generaltoolbar, this, XFileExplorer::ID_DIR_FORWARD_HIST,
                                       LAYOUT_FILL_Y | FRAME_NONE | ARROW_DOWN | ARROW_TOOLBAR);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    btn = new FXButton(generaltoolbar, TAB + _("Go to Parent Folder") + PARS(key), minidirupicon, this,
                       XFileExplorer::ID_DIR_UP, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    toolbarSeparator(generaltoolbar);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    new FXButton(generaltoolbar, TAB + _("Go to Home Folder") + PARS(key), minihomeicon, lpanel, FilePanel::ID_GO_HOME,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    new FXButton(generaltoolbar, TAB + _("Refresh Panels") + PARS(key), minireloadicon, this, XFileExplorer::ID_REFRESH,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    toolbarSeparator(generaltoolbar);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_file", "Ctrl-N");
    new FXButton(generaltoolbar, TAB + _("Create New File") + PARS(key), mininewfileicon, lpanel,
                 FilePanel::ID_NEW_FILE, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_folder", "F7");
    new FXButton(generaltoolbar, TAB + _("Create New Folder") + PARS(key), minifoldernewicon, lpanel,
                 FilePanel::ID_NEW_DIR, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_symlink", "Ctrl-J");
    new FXButton(generaltoolbar, TAB + _("Create New Symlink") + PARS(key), mininewlinkicon, lpanel,
                 FilePanel::ID_NEW_SYMLINK, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    toolbarSeparator(generaltoolbar);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    new FXButton(generaltoolbar, TAB + _("Copy Selected Files to Clipboard") + PARS(key), minicopyicon, this,
                 XFileExplorer::ID_FILE_COPY,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    // Shift + copy key binding can be used to add files to the copy clipboard
    // but this feature is disabled if the key binding already uses the Shift key
    if (key.lower().find("shift") < 0)
    {
        key = "Shift-" + key;
        hotkey = xf_parseaccel(key);
        getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ADDCOPY));
    }

    // Shift + cut key binding can be used to add files to the cut clipboard
    // but this feature is disabled if the key binding already uses the Shift key
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    new FXButton(generaltoolbar, TAB + _("Cut Selected Files to Clipboard") + PARS(key), minicuticon, this,
                 XFileExplorer::ID_FILE_CUT,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    if (key.lower().find("shift") < 0)
    {
        key = "Shift-" + key;
        hotkey = xf_parseaccel(key);
        getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ADDCUT));
    }

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "paste", "Ctrl-V");
    new FXButton(generaltoolbar, TAB + _("Paste From Clipboard") + PARS(key), minipasteicon, this,
                 XFileExplorer::ID_FILE_PASTE,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "properties", "F9");
    new FXButton(generaltoolbar, TAB + _("Show Properties of Selected Files") + PARS(key), miniattribicon, this,
                 XFileExplorer::ID_FILE_PROPERTIES,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    toolbarSeparator(generaltoolbar);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    new FXButton(generaltoolbar, TAB + _("Move Selected Files to Trash Can") + PARS(key), minideleteicon, this,
                 XFileExplorer::ID_FILE_TRASH,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "restore_from_trash", "Alt-Del");
    new FXButton(generaltoolbar, TAB + _("Restore Selected Files From Trash Can") + PARS(key), minirestoreicon, this,
                 XFileExplorer::ID_FILE_RESTORE,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    new FXButton(generaltoolbar, TAB + _("Delete Selected Files") + PARS(key), minideletepermicon, this,
                 XFileExplorer::ID_FILE_DELETE,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_window", "F3");
    new FXButton(toolstoolbar, TAB + _("Launch Xfe") + PARS(key), minixfeicon, this, XFileExplorer::ID_NEW_WIN,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_root_window", "Shift-F3");
    new FXButton(toolstoolbar, TAB + _("Launch Xfe as root") + PARS(key), minixferooticon, this, XFileExplorer::ID_SU,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "execute_command", "Ctrl-E");
    new FXButton(toolstoolbar, TAB + _("Execute Command") + PARS(key), minirunicon, this, XFileExplorer::ID_RUN,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "terminal", "Ctrl-T");
    new FXButton(toolstoolbar, TAB + _("Launch Terminal") + PARS(key), minishellicon, this, XFileExplorer::ID_XTERM,
                 BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "search", "Ctrl-F");
    new FXButton(toolstoolbar, TAB + _("Search Files and Folders...") + PARS(key), minisearchicon, this,
                 XFileExplorer::ID_FILE_SEARCH,
                 BUTTON_TOOLBAR | ICON_BEFORE_TEXT | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

#if defined(linux)
    toolbarSeparator(toolstoolbar);

    // Mount and unmount buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mount", "Ctrl-M");
    btn = new FXButton(toolstoolbar, TAB + _("Mount") + PARS(key), minimounticon, lpanel, FilePanel::ID_MOUNT,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "unmount", "Ctrl-U");
    btn = new FXButton(toolstoolbar, TAB + _("Unmount") + PARS(key), miniunmounticon, lpanel, FilePanel::ID_UMOUNT,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Connect to network server
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "connect_to_server", "Shift-F5");
    btn = new FXButton(toolstoolbar, TAB + _("Connect to Server...") + PARS(key), miniservericon, this,
                       ID_CONNECT_TO_SERVER, BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);
#endif

    // Panel toolbar

    // Show one file panel
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "one_panel", "Ctrl-F1");
    btn = new FXButton(paneltoolbar, TAB + _("Show One File Panel") + PARS(key), minionepanelicon, this,
                       XFileExplorer::ID_SHOW_ONE_PANEL,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Show folders panel and file panel
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_panel", "Ctrl-F2");
    btn = new FXButton(paneltoolbar, TAB + _("Show Folder Panel and File Panel") + PARS(key), minitreeonepanelicon,
                       this, XFileExplorer::ID_SHOW_FOLDERS_ONE_PANEL,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Show two file panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "two_panels", "Ctrl-F3");
    btn = new FXButton(paneltoolbar, TAB + _("Show Two File Panels") + PARS(key), minitwopanelsicon, this,
                       XFileExplorer::ID_SHOW_TWO_PANELS,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Show folders panel and two file panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_two_panels", "Ctrl-F4");
    btn = new FXButton(paneltoolbar, TAB + _("Show Folder Panel and Two File Panels") + PARS(key),
                       minitreetwopanelsicon, this, XFileExplorer::ID_SHOW_FOLDERS_TWO_PANELS,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    toolbarSeparator(paneltoolbar);

    // Vertical panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    btn = new FXButton(paneltoolbar, TAB + _("Vertical Panels") + PARS(key), minivertpanelsicon, this,
                       XFileExplorer::ID_VERT_PANELS,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Horizontal panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");
    btn = new FXButton(paneltoolbar, TAB + _("Horizontal Panels") + PARS(key), minihorzpanelsicon, this,
                       XFileExplorer::ID_HORZ_PANELS,
                       BUTTON_TOOLBAR | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    toolbarSeparator(paneltoolbar);

    // Switch display modes
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    btn = new FXButton(paneltoolbar, TAB + _("Big Icon List") + PARS(key), minibigiconsicon, lpanel,
                       FilePanel::ID_SHOW_BIG_ICONS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    btn = new FXButton(paneltoolbar, TAB + _("Small Icon List") + PARS(key), minismalliconsicon, lpanel,
                       FilePanel::ID_SHOW_MINI_ICONS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    btn = new FXButton(paneltoolbar, TAB + _("Detailed File List") + PARS(key), minidetailsicon, lpanel,
                       FilePanel::ID_SHOW_DETAILS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_NONE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Address combo box
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "clear_address", "Ctrl-L");
    btn = new FXButton(addresstoolbar, TAB + _("Clear Address Bar") + PARS(key), NULL, this, ID_CLEAR_ADDRESS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_RAISED);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);
    btn->hide(); // Clear address button not shown

    addressbox = new HistComboBox(addresstoolbar, ADDRESS_BAR_LENGTH, true, false, this, ID_GOTO_ADDRESS,
                                  COMBOBOX_INSERT_LAST | JUSTIFY_LEFT | LAYOUT_CENTER_Y);
    addressbox->setNumVisible(5);

    // Menus

    // File menu
    filemenu = new FXMenuPane(this);
    FXMenuCommand* mc = NULL;

    mc = new FXMenuCommand(filemenu, _("New &File..."), mininewfileicon, lpanel, FilePanel::ID_NEW_FILE);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_file", "Ctrl-N");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("New Fo&lder..."), minifoldernewicon, lpanel, FilePanel::ID_NEW_DIR);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_folder", "F7");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("New S&ymlink..."), mininewlinkicon, lpanel, FilePanel::ID_NEW_SYMLINK);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_symlink", "Ctrl-J");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("Go &Home"), minihomeicon, lpanel, FilePanel::ID_GO_HOME);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Refresh"), minireloadicon, this, XFileExplorer::ID_REFRESH);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(filemenu);

    mc = new FXMenuCommand(filemenu, _("&Open"), minifileopenicon, lpanel, FilePanel::ID_OPEN);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("Re&name..."), minirenameicon, this, XFileExplorer::ID_FILE_RENAME);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rename", "F2");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Copy to..."), minicopyicon, this, XFileExplorer::ID_FILE_COPYTO);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_to", "F5");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Move to..."), minimoveicon, this, XFileExplorer::ID_FILE_MOVETO);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to", "F6");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Symlink to..."), minilinkicon, this, XFileExplorer::ID_FILE_SYMLINK);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "symlink_to", "Ctrl-S");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("Mo&ve to Trash"), minideleteicon, this, XFileExplorer::ID_FILE_TRASH);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("R&estore From Trash"), minirestoreicon, this, XFileExplorer::ID_FILE_RESTORE);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "restore_from_trash", "Alt-Del");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Delete"), minideletepermicon, this, XFileExplorer::ID_FILE_DELETE);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    copynamesmc = new FXMenuCommand(filemenu, _("Cop&y Names"), minicopyicon, this, XFileExplorer::ID_FILE_COPYNAME);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_names", "Ctrl-Shift-N");
    copynamesmc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, copynamesmc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(filemenu, _("&Properties"), miniattribicon, this, XFileExplorer::ID_FILE_PROPERTIES);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "properties", "F9");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(filemenu);

    mc = new FXMenuCommand(filemenu, _("&Quit"), miniquiticon, this, XFileExplorer::ID_QUIT);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "quit", "Ctrl-Q");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    filemenutitle = new FXMenuTitle(menubar, _("&File"), NULL, filemenu);

    // Edit menu
    editmenu = new FXMenuPane(this);

    mc = new FXMenuCommand(editmenu, _("&Copy"), minicopyicon, this, XFileExplorer::ID_FILE_COPY);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(editmenu, _("C&ut"), minicuticon, this, XFileExplorer::ID_FILE_CUT);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(editmenu, _("&Paste"), minipasteicon, this, XFileExplorer::ID_FILE_PASTE);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "paste", "Ctrl-V");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(editmenu);

    mc = new FXMenuCommand(editmenu, _("&Select All"), miniselallicon, lpanel, FilePanel::ID_SELECT_ALL);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "select_all", "Ctrl-A");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(editmenu, _("&Deselect All"), miniunselicon, lpanel, FilePanel::ID_DESELECT_ALL);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "deselect_all", "Ctrl-Z");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(editmenu, _("&Invert Selection"), miniinvselicon, lpanel, FilePanel::ID_SELECT_INVERSE);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "invert_selection", "Ctrl-I");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(editmenu);
    new FXMenuCommand(editmenu, _("P&references"), miniprefsicon, this, XFileExplorer::ID_PREFS);

    editmenutitle = new FXMenuTitle(menubar, _("&Edit"), NULL, editmenu);

    // View menu
    viewmenu = new FXMenuPane(this);
    new FXMenuCheck(viewmenu, _("&General Toolbar"), generaltoolbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(viewmenu, _("&Tools Toolbar"), toolstoolbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(viewmenu, _("&Panel Toolbar"), paneltoolbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(viewmenu, _("&Address Bar"), addresstoolbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(viewmenu, _("&Status Bar"), this, XFileExplorer::ID_TOGGLE_STATUS);

    new FXMenuSeparator(viewmenu);

    mc = new FXMenuRadio(viewmenu, _("&One Panel"), this, XFileExplorer::ID_SHOW_ONE_PANEL);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "one_panel", "Ctrl-F1");
    mc->setAccelText(key);

    mc = new FXMenuRadio(viewmenu, _("Folde&rs and Panel"), this, XFileExplorer::ID_SHOW_FOLDERS_ONE_PANEL);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_panel", "Ctrl-F2");
    mc->setAccelText(key);

    mc = new FXMenuRadio(viewmenu, _("Two &Panels"), this, XFileExplorer::ID_SHOW_TWO_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "two_panels", "Ctrl-F3");
    mc->setAccelText(key);

    mc = new FXMenuRadio(viewmenu, _("Fold&ers and Two Panels"), this, XFileExplorer::ID_SHOW_FOLDERS_TWO_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_two_panels", "Ctrl-F4");
    mc->setAccelText(key);

    new FXMenuSeparator(viewmenu);

    mc = new FXMenuRadio(viewmenu, _("&Vertical Panels"), this, XFileExplorer::ID_VERT_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    mc->setAccelText(key);

    mc = new FXMenuRadio(viewmenu, _("&Horizontal Panels"), this, XFileExplorer::ID_HORZ_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");
    mc->setAccelText(key);

    viewmenutitle = new FXMenuTitle(menubar, _("&View"), NULL, viewmenu);

    // Bookmarks menu
    bookmarksmenu = new FXMenuPane(this);
    mc = new FXMenuCommand(bookmarksmenu, _("&Add Bookmark"), miniaddbookicon, this, ID_ADD_BOOKMARK);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "add_bookmark", "Ctrl-B");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuCommand(bookmarksmenu, _("&Remove All Bookmarks"), miniremovebookicon, this, ID_REMOVE_ALL_BOOKMARKS);
    bookmarkssep = new FXMenuSeparator(bookmarksmenu);

    for (int i = 0; i < dirpanel->getNumBookmarks(); i++)
    {
        bookmarkmc.push_back(new FXMenuCommand(bookmarksmenu, dirpanel->getBookmarkName(i), NULL, this, ID_BOOKMARK));
    }

    bookmarksmenutitle = new FXMenuTitle(menubar, _("&Bookmarks"), NULL, bookmarksmenu);

    // Left Panel Menu
    lpanelmenu = new FXMenuPane(this);
    new FXMenuCommand(lpanelmenu, _("&Filter..."), minifiltericon, lpanel, FilePanel::ID_FILTER);
    new FXMenuCheck(lpanelmenu, _("&Hidden Files"), lpanel->getList(), FileList::ID_TOGGLE_HIDDEN);
    new FXMenuCheck(lpanelmenu, _("&Thumbnails"), lpanel->getList(), FileList::ID_TOGGLE_THUMBNAILS);
    new FXMenuSeparator(lpanelmenu);
    new FXMenuRadio(lpanelmenu, _("&Big Icons"), lpanel->getList(), IconList::ID_SHOW_BIG_ICONS);
    new FXMenuRadio(lpanelmenu, _("&Small Icons"), lpanel->getList(), IconList::ID_SHOW_MINI_ICONS);
    new FXMenuRadio(lpanelmenu, _("&Detailed File List"), lpanel->getList(), IconList::ID_SHOW_DETAILS);
    new FXMenuSeparator(lpanelmenu);
    new FXMenuCheck(lpanelmenu, _("Autos&ize"), lpanel->getList(), FileList::ID_AUTOSIZE);
    new FXMenuRadio(lpanelmenu, _("&Rows"), lpanel->getList(), FileList::ID_ARRANGE_BY_ROWS);
    new FXMenuRadio(lpanelmenu, _("&Columns"), lpanel->getList(), FileList::ID_ARRANGE_BY_COLUMNS);
    new FXMenuSeparator(lpanelmenu);
    new FXMenuRadio(lpanelmenu, _("&Name"), lpanel->getList(), FileList::ID_COL_NAME);
    new FXMenuRadio(lpanelmenu, _("Si&ze"), lpanel->getList(), FileList::ID_COL_SIZE);
    new FXMenuRadio(lpanelmenu, _("T&ype"), lpanel->getList(), FileList::ID_COL_TYPE);
    new FXMenuRadio(lpanelmenu, _("E&xtension"), lpanel->getList(), FileList::ID_COL_EXT);
    new FXMenuRadio(lpanelmenu, _("D&ate"), lpanel->getList(), FileList::ID_COL_DATE);
    new FXMenuRadio(lpanelmenu, _("Us&er"), lpanel->getList(), FileList::ID_COL_USER);
    new FXMenuRadio(lpanelmenu, _("Gr&oup"), lpanel->getList(), FileList::ID_COL_GROUP);
    new FXMenuRadio(lpanelmenu, _("&Permissions"), lpanel->getList(), FileList::ID_COL_PERMS);
    new FXMenuRadio(lpanelmenu, _("&Link"), lpanel->getList(), FileList::ID_COL_LINK);
    new FXMenuRadio(lpanelmenu, _("Deletion Date"), lpanel->getList(), FileList::ID_COL_DELDATE);
    new FXMenuRadio(lpanelmenu, _("Original Path"), lpanel->getList(), FileList::ID_COL_ORIGPATH);
    new FXMenuSeparator(lpanelmenu);
    new FXMenuCheck(lpanelmenu, _("I&gnore Case"), lpanel->getList(), FileList::ID_SORT_CASE);
    new FXMenuCheck(lpanelmenu, _("Fol&ders First"), lpanel->getList(), FileList::ID_DIRS_FIRST);
    new FXMenuCheck(lpanelmenu, _("Re&verse Order"), lpanel->getList(), FileList::ID_SORT_REVERSE);
    lpanelmenutitle = new FXMenuTitle(menubar, _("&Left Panel"), NULL, lpanelmenu);

    // Right Panel Menu
    rpanelmenu = new FXMenuPane(this);
    new FXMenuCommand(rpanelmenu, _("&Filter"), minifiltericon, rpanel, FilePanel::ID_FILTER);
    new FXMenuCheck(rpanelmenu, _("&Hidden Files"), rpanel->getList(), FileList::ID_TOGGLE_HIDDEN);
    new FXMenuCheck(rpanelmenu, _("&Thumbnails"), rpanel->getList(), FileList::ID_TOGGLE_THUMBNAILS);
    new FXMenuSeparator(rpanelmenu);
    new FXMenuRadio(rpanelmenu, _("&Big Icons"), rpanel->getList(), IconList::ID_SHOW_BIG_ICONS);
    new FXMenuRadio(rpanelmenu, _("&Small Icons"), rpanel->getList(), IconList::ID_SHOW_MINI_ICONS);
    new FXMenuRadio(rpanelmenu, _("&Detailed File List"), rpanel->getList(), IconList::ID_SHOW_DETAILS);
    new FXMenuSeparator(rpanelmenu);
    new FXMenuCheck(rpanelmenu, _("Autos&ize"), rpanel->getList(), FileList::ID_AUTOSIZE);
    new FXMenuRadio(rpanelmenu, _("&Rows"), rpanel->getList(), FileList::ID_ARRANGE_BY_ROWS);
    new FXMenuRadio(rpanelmenu, _("&Columns"), rpanel->getList(), FileList::ID_ARRANGE_BY_COLUMNS);
    new FXMenuSeparator(rpanelmenu);
    new FXMenuRadio(rpanelmenu, _("&Name"), rpanel->getList(), FileList::ID_COL_NAME);
    new FXMenuRadio(rpanelmenu, _("Si&ze"), rpanel->getList(), FileList::ID_COL_SIZE);
    new FXMenuRadio(rpanelmenu, _("T&ype"), rpanel->getList(), FileList::ID_COL_TYPE);
    new FXMenuRadio(rpanelmenu, _("E&xtension"), rpanel->getList(), FileList::ID_COL_EXT);
    new FXMenuRadio(rpanelmenu, _("D&ate"), rpanel->getList(), FileList::ID_COL_DATE);
    new FXMenuRadio(rpanelmenu, _("Us&er"), rpanel->getList(), FileList::ID_COL_USER);
    new FXMenuRadio(rpanelmenu, _("Gr&oup"), rpanel->getList(), FileList::ID_COL_GROUP);
    new FXMenuRadio(rpanelmenu, _("&Permissions"), rpanel->getList(), FileList::ID_COL_PERMS);
    new FXMenuRadio(rpanelmenu, _("&Link"), rpanel->getList(), FileList::ID_COL_LINK);
    new FXMenuRadio(rpanelmenu, _("Deletion Date"), rpanel->getList(), FileList::ID_COL_DELDATE);
    new FXMenuRadio(rpanelmenu, _("Original Path"), rpanel->getList(), FileList::ID_COL_ORIGPATH);
    new FXMenuSeparator(rpanelmenu);
    new FXMenuCheck(rpanelmenu, _("I&gnore Case"), rpanel->getList(), FileList::ID_SORT_CASE);
    new FXMenuCheck(rpanelmenu, _("&Folders First"), rpanel->getList(), FileList::ID_DIRS_FIRST);
    new FXMenuCheck(rpanelmenu, _("Re&verse Order"), rpanel->getList(), FileList::ID_SORT_REVERSE);
    rpanelmenutitle = new FXMenuTitle(menubar, _("&Right Panel"), NULL, rpanelmenu);

    // Scripts menu
    scriptsmenu = new FXMenuPane(this);
    new FXMenuSeparator(scriptsmenu);
    new FXMenuCommand(scriptsmenu, _("&Go to Script Folder"), minigotodiricon, lpanel->getCurrent(),
                      FilePanel::ID_GO_SCRIPTDIR);
    scriptsmenutitle = new FXMenuTitle(menubar, _("&Scripts"), NULL, scriptsmenu);

    // Tools menu
    toolsmenu = new FXMenuPane(this);

    mc = new FXMenuCommand(toolsmenu, _("&New Tab"), mininewtabicon, tabbuttons, TabButtons::ID_NEW_TAB);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_tab", "Shift-F1");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));
    new FXMenuCommand(toolsmenu, _("Remove &All Tabs"), miniremovetabicon, tabbuttons, TabButtons::ID_REMOVE_ALL_TABS);

    new FXMenuSeparator(toolsmenu);

    mc = new FXMenuCommand(toolsmenu, _("New &Window"), minixfeicon, this, XFileExplorer::ID_NEW_WIN);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_window", "F3");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(toolsmenu, _("New &Root Window"), minixferooticon, this, XFileExplorer::ID_SU);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_root_window", "Shift-F3");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(toolsmenu);

    mc = new FXMenuCommand(toolsmenu, _("E&xecute Command..."), minirunicon, this, ID_RUN);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "execute_command", "Ctrl-E");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(toolsmenu, _("&Terminal"), minishellicon, this, XFileExplorer::ID_XTERM);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "terminal", "Ctrl-T");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(toolsmenu, _("&Synchronize Panels"), minisyncpanelsicon, this,
                           XFileExplorer::ID_SYNCHRONIZE_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "synchronize_panels", "Ctrl-Y");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    mc = new FXMenuCommand(toolsmenu, _("Sw&itch Panels"), miniswitchpanelsicon, this, XFileExplorer::ID_SWITCH_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "switch_panels", "Ctrl-K");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuCommand(toolsmenu, _("&Go to Script Folder"), minigotodiricon, lpanel, FilePanel::ID_GO_SCRIPTDIR);

    mc = new FXMenuCommand(toolsmenu, _("Search &Files..."), minisearchicon, this, XFileExplorer::ID_FILE_SEARCH);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "search", "Ctrl-F");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

#if defined(linux)
    new FXMenuSeparator(toolsmenu);

    mc = new FXMenuCommand(toolsmenu, _("&Mount"), minimounticon, lpanel, FilePanel::ID_MOUNT);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mount", "Ctrl-M");
    mc->setAccelText(key);

    mc = new FXMenuCommand(toolsmenu, _("&Unmount"), miniunmounticon, lpanel, FilePanel::ID_UMOUNT);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "unmount", "Ctrl-U");
    mc->setAccelText(key);

    new FXMenuSeparator(toolsmenu);

    mc = new FXMenuCommand(toolsmenu, _("&Connect to Server..."), miniservericon, this, ID_CONNECT_TO_SERVER);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "connect_to_server", "Shift-F5");
    mc->setAccelText(key);
#endif
    toolsmenutitle = new FXMenuTitle(menubar, _("&Tools"), NULL, toolsmenu);

    // Trash menu
    trashmenu = new FXMenuPane(this);

    mc = new FXMenuCommand(trashmenu, _("&Go to Trash"), minitotrashicon, lpanel, FilePanel::ID_GO_TRASH);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_to_trash", "Ctrl-F8");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuCommand(trashmenu, _("&Trash Size"), minideleteicon, this, XFileExplorer::ID_TRASH_SIZE);

    mc = new FXMenuCommand(trashmenu, _("&Empty Trash"), minitrashfullicon, this, XFileExplorer::ID_EMPTY_TRASH);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "empty_trash_can", "Ctrl-Del");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    trashmenutitle = new FXMenuTitle(menubar, _("T&rash"), NULL, trashmenu);

    // Help menu
    helpmenu = new FXMenuPane(this);

    mc = new FXMenuCommand(helpmenu, _("&Help"), minihelpicon, this, ID_HELP);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "help", "F1");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuCommand(helpmenu, _("&About X File Explorer"), NULL, this, ID_ABOUT);
    helpmenutitle = new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    // Other accelerators
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "edit", "F4");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_EDIT));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view", "Shift-F4");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_VIEW));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "compare", "F8");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_COMPARE));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_dirs", "Ctrl-F5");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, dirpanel, FXSEL(SEL_COMMAND, DirPanel::ID_TOGGLE_HIDDEN));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "filter", "Ctrl-D");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_FILTER_CURRENT));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_TOGGLE_HIDDEN));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_TOGGLE_THUMBNAILS));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, XFileExplorer::ID_QUIT));

    // Escape key
    getAccelTable()->addAccel(KEY_Escape, lpanel, FXSEL(SEL_COMMAND, FilePanel::ID_DESELECT_ALL));

    // Make a tool tip
    new FXToolTip(app, 0);

    // Initial focus is on (left) file panel
    panelfocus = FILEPANEL_FOCUS;

    // Trashcan locations
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;

    // Start location (we return to the start location after each chdir)
    startlocation = FXSystem::getCurrentDirectory();

    // Other initializations
    starticonic = iconic;
    startmaximized = maximized;
    startdir1 = "";
    startdir2 = "";
    startURIs = URIs;

    // Read URIs to open on startup
    // Find if startdir1 and startdir2 are specified
    // Get the number of files to open, if any
    for (FXuint n = 0; n < startURIs.size(); n++)
    {
        if (xf_isdirectory(startURIs[n]))
        {
            if (startdir1 == "")
            {
                startdir1 = startURIs[n];
                continue;
            }
            if (startdir2 == "")
            {
                startdir2 = startURIs[n];
                continue;
            }
        }
        if (xf_isfile(startURIs[n]))
        {
            nbstartfiles++;
        }
    }

    // Flag used when restoring tabs
    if (startdir1 != "")
    {
        startdirs = true;
    }

    prevdir = FXString::null;
    prev_width = getWidth();

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


// Save configuration when quitting
void XFileExplorer::saveConfig()
{
    // Check if Xfe was restarted from Preferences dialog
    FXbool restarted = getApp()->reg().readUnsignedEntry("SETTINGS", "restarted", false);

    if (!restarted)
    {
        // Columns order and shown status
        FXuint i = FileList::ID_COL_NAME;
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_name", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_size", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_type", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_ext", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_date", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_user", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_group", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_perms", colShown[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_link", colShown[i++]);

        i = 0;
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_0", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_1", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_2", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_3", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_4", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_5", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_6", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_7", idCol[i++]);
        getApp()->reg().writeUnsignedEntry("SETTINGS", "id_col_8", idCol[i++]);
    }

    // Get autosave flag
    FXbool auto_save_layout = getApp()->reg().readUnsignedEntry("OPTIONS", "auto_save_layout", true);

    if (auto_save_layout == true)
    {
        FXString sort_func;

        // Dir panel options
        if (dirpanel->getSortFunc() == DirList::ascendingCase)
        {
            sort_func = "ascendingCase";
        }
        else if (dirpanel->getSortFunc() == DirList::descendingCase)
        {
            sort_func = "descendingCase";
        }
        else if (dirpanel->getSortFunc() == DirList::ascending)
        {
            sort_func = "ascending";
        }
        else if (dirpanel->getSortFunc() == DirList::descending)
        {
            sort_func = "descending";
        }
        else
        {
            sort_func = "ascendingCase";
        }
        getApp()->reg().writeStringEntry("DIR PANEL", "sort_func", sort_func.text());
        getApp()->reg().writeUnsignedEntry("DIR PANEL", "hidden_dir", dirpanel->shownHiddenFiles());

        // Search panel options
        if (searchwindow)
        {
            // Search dialog properties
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "search_xpos", (FXuint)searchwindow->getX());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "search_ypos", (FXuint)searchwindow->getY());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "search_width", (FXuint)searchwindow->getWidth());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "search_height", (FXuint)searchwindow->getHeight());

            // Search panel options
            if (!restarted)
            {
                FXuint i = FileList::ID_COL_NAME;
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "name_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_NAME)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "size_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_SIZE)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "type_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_TYPE)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "ext_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_EXT)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "date_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_DATE)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "user_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_USER)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "group_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_GROUP)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "perms_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_PERMS)));
                }
                if (colShown[i++])
                {
                    getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "link_size",
                    (FXuint)searchwindow->getHeaderSize(searchwindow->getHeaderIndex(FileList::ID_COL_LINK)));
                }
            }
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "dir_size", (FXuint)searchwindow->getHeaderSize(1)); // Directory is at position 1
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "showthumbnails",
                                               (FXuint)searchwindow->shownThumbnails());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "find_ignorecase",
                                               (FXuint)searchwindow->getFindIgnoreCase());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "find_hidden", (FXuint)searchwindow->getFindHidden());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "grep_ignorecase",
                                               (FXuint)searchwindow->getGrepIgnoreCase());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "moreoptions", (FXuint)searchwindow->shownMoreOptions());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "liststyle", (FXuint)searchwindow->getListStyle());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "dirs_first", (FXuint)searchwindow->getDirsFirst());
            getApp()->reg().writeUnsignedEntry("SEARCH PANEL", "ignore_case", (FXuint)searchwindow->getIgnoreCase());

            // Get and write sort function for search window
            if (searchwindow->getSortFunc() == FileList::ascendingCase)
            {
                sort_func = "ascendingCase";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingCaseMix)
            {
                sort_func = "ascendingCaseMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingCase)
            {
                sort_func = "descendingCase";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingCaseMix)
            {
                sort_func = "descendingCaseMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascending)
            {
                sort_func = "ascending";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingMix)
            {
                sort_func = "ascendingMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descending)
            {
                sort_func = "descending";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingMix)
            {
                sort_func = "descendingMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDirCase)
            {
                sort_func = "ascendingDirCase";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDirCaseMix)
            {
                sort_func = "ascendingDirCaseMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDirCase)
            {
                sort_func = "descendingDirCase";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDirCaseMix)
            {
                sort_func = "descendingDirCaseMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDir)
            {
                sort_func = "ascendingDir";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDirMix)
            {
                sort_func = "ascendingDirMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDir)
            {
                sort_func = "descendingDir";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDirMix)
            {
                sort_func = "descendingDirMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingSize)
            {
                sort_func = "ascendingSize";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingSizeMix)
            {
                sort_func = "ascendingSizeMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingSize)
            {
                sort_func = "descendingSize";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingSizeMix)
            {
                sort_func = "descendingSizeMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingType)
            {
                sort_func = "ascendingType";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingTypeMix)
            {
                sort_func = "ascendingTypeMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingType)
            {
                sort_func = "descendingType";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingTypeMix)
            {
                sort_func = "descendingTypeMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingExt)
            {
                sort_func = "ascendingExt";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingExtMix)
            {
                sort_func = "ascendingExtMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingExt)
            {
                sort_func = "descendingExt";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingExtMix)
            {
                sort_func = "descendingExtMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDate)
            {
                sort_func = "ascendingDate";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingDateMix)
            {
                sort_func = "ascendingDateMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDate)
            {
                sort_func = "descendingDate";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingDateMix)
            {
                sort_func = "descendingDateMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingUser)
            {
                sort_func = "ascendingUser";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingUserMix)
            {
                sort_func = "ascendingUserMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingUser)
            {
                sort_func = "descendingUser";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingUserMix)
            {
                sort_func = "descendingUserMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingGroup)
            {
                sort_func = "ascendingGroup";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingGroupMix)
            {
                sort_func = "ascendingGroupMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingGroup)
            {
                sort_func = "descendingGroup";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingGroupMix)
            {
                sort_func = "descendingGroupMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingPerms)
            {
                sort_func = "ascendingPerms";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingPermsMix)
            {
                sort_func = "ascendingPermsMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingPerms)
            {
                sort_func = "descendingPerms";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingPermsMix)
            {
                sort_func = "descendingPermsMix";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingLink)
            {
                sort_func = "ascendingLink";
            }
            else if (searchwindow->getSortFunc() == FileList::ascendingLinkMix)
            {
                sort_func = "ascendingLinkMix";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingLink)
            {
                sort_func = "descendingLink";
            }
            else if (searchwindow->getSortFunc() == FileList::descendingLinkMix)
            {
                sort_func = "descendingLinkMix";
            }
            else
            {
                sort_func = "ascendingCase";
            }
            getApp()->reg().writeStringEntry("SEARCH PANEL", "sort_func", sort_func.text());
        }

        // Left panel options
        if (!restarted)
        {
            FXuint i = FileList::ID_COL_NAME;
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "name_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_NAME)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "size_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_SIZE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "type_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_TYPE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "ext_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_EXT)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "date_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_DATE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "user_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_USER)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "group_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_GROUP)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "perms_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_PERMS)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("LEFT PANEL", "link_size",
                (FXuint)lpanel->getHeaderSize((FXuint)lpanel->getHeaderIndex(FileList::ID_COL_LINK)));
            }
        }

        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "deldate_size", lpanel->getDeldateSize());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "origpath_size", lpanel->getOrigpathSize());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "liststyle", lpanel->getListStyle());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "hiddenfiles", lpanel->shownHiddenFiles());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "showthumbnails", lpanel->shownThumbnails());

        // Get and write sort function for left panel
        if (lpanel->getSortFunc() == FileList::ascendingCase)
        {
            sort_func = "ascendingCase";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingCaseMix)
        {
            sort_func = "ascendingCaseMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingCase)
        {
            sort_func = "descendingCase";
        }
        else if (lpanel->getSortFunc() == FileList::descendingCaseMix)
        {
            sort_func = "descendingCaseMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascending)
        {
            sort_func = "ascending";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingMix)
        {
            sort_func = "ascendingMix";
        }
        else if (lpanel->getSortFunc() == FileList::descending)
        {
            sort_func = "descending";
        }
        else if (lpanel->getSortFunc() == FileList::descendingMix)
        {
            sort_func = "descendingMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingSize)
        {
            sort_func = "ascendingSize";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingSizeMix)
        {
            sort_func = "ascendingSizeMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingSize)
        {
            sort_func = "descendingSize";
        }
        else if (lpanel->getSortFunc() == FileList::descendingSizeMix)
        {
            sort_func = "descendingSizeMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingType)
        {
            sort_func = "ascendingType";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingTypeMix)
        {
            sort_func = "ascendingTypeMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingType)
        {
            sort_func = "descendingType";
        }
        else if (lpanel->getSortFunc() == FileList::descendingTypeMix)
        {
            sort_func = "descendingTypeMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingExt)
        {
            sort_func = "ascendingExt";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingExtMix)
        {
            sort_func = "ascendingExtMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingExt)
        {
            sort_func = "descendingExt";
        }
        else if (lpanel->getSortFunc() == FileList::descendingExtMix)
        {
            sort_func = "descendingExtMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingDate)
        {
            sort_func = "ascendingDate";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingDateMix)
        {
            sort_func = "ascendingDateMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingDate)
        {
            sort_func = "descendingDate";
        }
        else if (lpanel->getSortFunc() == FileList::descendingDateMix)
        {
            sort_func = "descendingDateMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingUser)
        {
            sort_func = "ascendingUser";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingUserMix)
        {
            sort_func = "ascendingUserMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingUser)
        {
            sort_func = "descendingUser";
        }
        else if (lpanel->getSortFunc() == FileList::descendingUserMix)
        {
            sort_func = "descendingUserMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingGroup)
        {
            sort_func = "ascendingGroup";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingGroupMix)
        {
            sort_func = "ascendingGroupMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingGroup)
        {
            sort_func = "descendingGroup";
        }
        else if (lpanel->getSortFunc() == FileList::descendingGroupMix)
        {
            sort_func = "descendingGroupMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingPerms)
        {
            sort_func = "ascendingPerms";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingPermsMix)
        {
            sort_func = "ascendingPermsMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingPerms)
        {
            sort_func = "descendingPerms";
        }
        else if (lpanel->getSortFunc() == FileList::descendingPermsMix)
        {
            sort_func = "descendingPermsMix";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingLink)
        {
            sort_func = "ascendingLink";
        }
        else if (lpanel->getSortFunc() == FileList::ascendingLinkMix)
        {
            sort_func = "ascendingLinkMix";
        }
        else if (lpanel->getSortFunc() == FileList::descendingLink)
        {
            sort_func = "descendingLink";
        }
        else if (lpanel->getSortFunc() == FileList::descendingLinkMix)
        {
            sort_func = "descendingLinkMix";
        }
        else
        {
            sort_func = "ascendingCase";
        }
        getApp()->reg().writeStringEntry("LEFT PANEL", "sort_func", sort_func.text());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "ignore_case", lpanel->getIgnoreCase());
        getApp()->reg().writeUnsignedEntry("LEFT PANEL", "dirs_first", lpanel->getDirsFirst());

        // Right panel options
        if (!restarted)
        {
            FXuint i = FileList::ID_COL_NAME;
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "name_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_NAME)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "size_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_SIZE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "type_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_TYPE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "ext_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_EXT)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "date_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_DATE)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "user_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_USER)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "group_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_GROUP)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "perms_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_PERMS)));
            }
            if (colShown[i++])
            {
                getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "link_size",
                rpanel->getHeaderSize(rpanel->getHeaderIndex(FileList::ID_COL_LINK)));
            }
        }

        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "deldate_size", rpanel->getDeldateSize());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "origpath_size", rpanel->getOrigpathSize());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "liststyle", rpanel->getListStyle());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "hiddenfiles", rpanel->shownHiddenFiles());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "showthumbnails", rpanel->shownThumbnails());

        // Get and write sort function for right panel
        if (rpanel->getSortFunc() == FileList::ascendingCase)
        {
            sort_func = "ascendingCase";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingCaseMix)
        {
            sort_func = "ascendingCaseMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingCase)
        {
            sort_func = "descendingCase";
        }
        else if (rpanel->getSortFunc() == FileList::descendingCaseMix)
        {
            sort_func = "descendingCaseMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascending)
        {
            sort_func = "ascending";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingMix)
        {
            sort_func = "ascendingMix";
        }
        else if (rpanel->getSortFunc() == FileList::descending)
        {
            sort_func = "descending";
        }
        else if (rpanel->getSortFunc() == FileList::descendingMix)
        {
            sort_func = "descendingMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingSize)
        {
            sort_func = "ascendingSize";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingSizeMix)
        {
            sort_func = "ascendingSizeMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingSize)
        {
            sort_func = "descendingSize";
        }
        else if (rpanel->getSortFunc() == FileList::descendingSizeMix)
        {
            sort_func = "descendingSizeMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingType)
        {
            sort_func = "ascendingType";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingTypeMix)
        {
            sort_func = "ascendingTypeMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingType)
        {
            sort_func = "descendingType";
        }
        else if (rpanel->getSortFunc() == FileList::descendingTypeMix)
        {
            sort_func = "descendingTypeMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingExt)
        {
            sort_func = "ascendingExt";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingExtMix)
        {
            sort_func = "ascendingExtMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingExt)
        {
            sort_func = "descendingExt";
        }
        else if (rpanel->getSortFunc() == FileList::descendingExtMix)
        {
            sort_func = "descendingExtMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingDate)
        {
            sort_func = "ascendingDate";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingDateMix)
        {
            sort_func = "ascendingDateMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingDate)
        {
            sort_func = "descendingDate";
        }
        else if (rpanel->getSortFunc() == FileList::descendingDateMix)
        {
            sort_func = "descendingDateMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingUser)
        {
            sort_func = "ascendingUser";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingUserMix)
        {
            sort_func = "ascendingUserMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingUser)
        {
            sort_func = "descendingUser";
        }
        else if (rpanel->getSortFunc() == FileList::descendingUserMix)
        {
            sort_func = "descendingUserMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingGroup)
        {
            sort_func = "ascendingGroup";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingGroupMix)
        {
            sort_func = "ascendingGroupMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingGroup)
        {
            sort_func = "descendingGroup";
        }
        else if (rpanel->getSortFunc() == FileList::descendingGroupMix)
        {
            sort_func = "descendingGroupMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingPerms)
        {
            sort_func = "ascendingPerms";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingPermsMix)
        {
            sort_func = "ascendingPermsMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingPerms)
        {
            sort_func = "descendingPerms";
        }
        else if (rpanel->getSortFunc() == FileList::descendingPermsMix)
        {
            sort_func = "descendingPermsMix";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingLink)
        {
            sort_func = "ascendingLink";
        }
        else if (rpanel->getSortFunc() == FileList::ascendingLinkMix)
        {
            sort_func = "ascendingLinkMix";
        }
        else if (rpanel->getSortFunc() == FileList::descendingLink)
        {
            sort_func = "descendingLink";
        }
        else if (rpanel->getSortFunc() == FileList::descendingLinkMix)
        {
            sort_func = "descendingLinkMix";
        }
        else
        {
            sort_func = "ascendingCase";
        }
        getApp()->reg().writeStringEntry("RIGHT PANEL", "sort_func", sort_func.text());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "ignore_case", rpanel->getIgnoreCase());
        getApp()->reg().writeUnsignedEntry("RIGHT PANEL", "dirs_first", rpanel->getDirsFirst());

        // Global options
        getApp()->reg().writeUnsignedEntry("OPTIONS", "width", (FXuint)getWidth());
        getApp()->reg().writeUnsignedEntry("OPTIONS", "height", (FXuint)getHeight());

        // Window position
        if (save_win_pos)
        {
            // Account for the Window Manager border size
            XWindowAttributes xwattr;
            if (XGetWindowAttributes((Display*)getApp()->getDisplay(), this->id(), &xwattr))
            {
                getApp()->reg().writeIntEntry("OPTIONS", "xpos", getX() - xwattr.x);
                getApp()->reg().writeIntEntry("OPTIONS", "ypos", getY() - xwattr.y);
            }
            else
            {
                getApp()->reg().writeIntEntry("OPTIONS", "xpos", getX());
                getApp()->reg().writeIntEntry("OPTIONS", "ypos", getY());
            }
        }

        
        // Save toolbar positions
        
        // Vector of toolbar parameters
        std::vector<std::vector<int>> tb;

        FXString key;
        int galley = LAYOUT_DOCK_SAME;  // Default galley placement
        
        tb.push_back({generaltoolbar->shown(), (int)generaltoolbar->getDockingSide(), galley, generaltoolbar->getX(), generaltoolbar->getY(), generaltoolbar->getWidth(), generaltoolbar->getHeight()});
        tb.push_back({toolstoolbar->shown(), (int)toolstoolbar->getDockingSide(), galley, toolstoolbar->getX(), toolstoolbar->getY(), toolstoolbar->getWidth(), toolstoolbar->getHeight()});       
        tb.push_back({paneltoolbar->shown(), (int)paneltoolbar->getDockingSide(), galley, paneltoolbar->getX(), paneltoolbar->getY(), paneltoolbar->getWidth(), paneltoolbar->getHeight()});
        tb.push_back({addresstoolbar->shown(), (int)addresstoolbar->getDockingSide(), galley, addresstoolbar->getX(), addresstoolbar->getY(), addresstoolbar->getWidth(), addresstoolbar->getHeight()});
        tb.push_back({tabtoolbar->shown(), (int)tabtoolbar->getDockingSide(), galley, tabtoolbar->getX(), tabtoolbar->getY(), tabtoolbar->getWidth(), tabtoolbar->getHeight()});

        // Sort toolbars at each docking side
        
        // Screen dimensions
        int screen_width = getRoot()->getDefaultWidth();
        int screen_height = getRoot()->getDefaultHeight();

        // Vectors of toolbar positions at docking sides
        std::vector<std::pair<int, int>> top;
        std::vector<std::pair<int, int>> bottom;
        std::vector<std::pair<int, int>> left;
        std::vector<std::pair<int, int>> right;
        
        for (int i = 0; i < NB_TOOLBARS; i++)
        {
            FXbool shown = tb[i][0];
            FXuint dockingside = tb[i][1];
            
            if (shown)
            {
                int x = tb[i][3];
                int y = tb[i][4];
                int w = tb[i][5];
                int h = tb[i][6];

                if (dockingside == LAYOUT_SIDE_BOTTOM)
                {
                    int nl = (int)round((double)y / double(h));  // Line number
                    bottom.push_back(std::make_pair(nl * screen_width + x, i));                   
                }
                else if (dockingside == LAYOUT_SIDE_LEFT)
                {
                    int nc = (int)round((double)x / double(w));  // Column number
                    left.push_back(std::make_pair(nc * screen_height + y, i));                   
                }
                else if (dockingside == LAYOUT_SIDE_RIGHT)
                {
                    int nc = (int)round((double)x / double(w));  // Column number
                    right.push_back(std::make_pair(nc * screen_height + y, i));                   
                }
                else // LAYOUT_SIDE_TOP
                {
                    int nl = (int)round((double)y / double(h));  // Line number
                    top.push_back(std::make_pair(nl * screen_width + x, i));                   
                }
            }
        }
        
        // Sort positions
        sort(top.begin(), top.end());
        sort(bottom.begin(), bottom.end());
        sort(left.begin(), left.end());
        sort(right.begin(), right.end());

        // Find toolbars having next galley placement
        for (int i = 0; i < NB_TOOLBARS; i++)
        {
            FXbool shown = tb[i][0];
            FXuint dockingside = tb[i][1];

            if (shown)
            {                
                if (dockingside == LAYOUT_SIDE_BOTTOM)
                {
                    for (FXuint j = 0; j < bottom.size(); j++)
                    {
                        if (bottom[j].second == i)
                        {                            
                            // Next galley?
                            if (j < bottom.size() - 1)
                            {
                                int i_next = bottom[j + 1].second;
                                int y_next = tb[i_next][4];
                                int y = tb[i][4];
                                
                                if (y_next - y > tb[i][6] / 2)  // Compare to toolbar height / 2
                                {
                                    tb[i_next][2] = LAYOUT_DOCK_NEXT;
                                }
                            }
                        }
                    }
                }
                else if (dockingside == LAYOUT_SIDE_LEFT)
                {
                    for (FXuint j = 0; j < left.size(); j++)
                    {
                        if (left[j].second == i)
                        {
                            // Next galley?
                            if (j > 1)
                            {
                                int i_prev = left[j - 1].second;
                                int x_prev = tb[i_prev][3];
                                int x = tb[i][3];
                                
                                if (x - x_prev > tb[i][5] / 2)  // Compare to toolbar width / 2
                                {
                                    tb[i][2] = LAYOUT_DOCK_NEXT;
                                }
                            }
                        }
                    }
                }
                else if (dockingside == LAYOUT_SIDE_RIGHT)
                {
                    for (FXuint j = 0; j < right.size(); j++)
                    {
                        if (right[j].second == i)
                        {
                            // Next galley?
                            if (j < right.size() - 1)
                            {
                                int i_next = right[j + 1].second;                               
                                int x_next = tb[i_next][3];
                                int x = tb[i][3];
                                
                                if (x_next - x > tb[i][5] / 2)  // Compare to toolbar width / 2
                                {
                                    tb[i_next][2] = LAYOUT_DOCK_NEXT;
                                }
                            }
                        }
                    }
                }
                else // LAYOUT_SIDE_TOP
                {
                    for (FXuint j = 0; j < top.size(); j++)
                    {
                        if (top[j].second == i)
                        {
                            // Next galley?
                            if (j > 1)
                            {
                                int i_prev = top[j - 1].second;
                                int y_prev = tb[i_prev][4];
                                int y = tb[i][4];
                                
                                if (y - y_prev > tb[i][6] / 2)  // Compare to toolbar height / 2
                                {
                                    tb[i][2] = LAYOUT_DOCK_NEXT;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Save toolbar parameters
        key = FXStringVal(tb[0][0]) + ";" + FXStringVal(tb[0][1]) + ";" + FXStringVal(tb[0][2]);
        getApp()->reg().writeStringEntry("OPTIONS", "generaltoolbar", key.text());
        
        key = FXStringVal(tb[1][0]) + ";" + FXStringVal(tb[1][1]) + ";" + FXStringVal(tb[1][2]);
        getApp()->reg().writeStringEntry("OPTIONS", "toolstoolbar", key.text());
       
        key = FXStringVal(tb[2][0]) + ";" + FXStringVal(tb[2][1]) + ";" + FXStringVal(tb[2][2]);
        getApp()->reg().writeStringEntry("OPTIONS", "paneltoolbar", key.text());

        key = FXStringVal(tb[3][0]) + ";" + FXStringVal(tb[3][1]) + ";" + FXStringVal(tb[3][2]);
        getApp()->reg().writeStringEntry("OPTIONS", "addresstoolbar", key.text());

        key = FXStringVal(tb[4][0]) + ";" + FXStringVal(tb[4][1]) + ";" + FXStringVal(tb[4][2]);
        getApp()->reg().writeStringEntry("OPTIONS", "tabtoolbar", key.text());

        // Save toolbar positions at docking sides
        key = "";
        for (FXuint i = 0; i < top.size(); i++)
        {
            key += FXStringVal(top[i].second) + ";";
        }
        getApp()->reg().writeStringEntry("OPTIONS", "top_toolbars", key.text());

        key = "";
        for (FXuint i = 0; i < bottom.size(); i++)
        {
            key += FXStringVal(bottom[i].second) + ";";
        }
        getApp()->reg().writeStringEntry("OPTIONS", "bottom_toolbars", key.text());

        key = "";
        for (FXuint i = 0; i < left.size(); i++)
        {
            key += FXStringVal(left[i].second) + ";";
        }
        getApp()->reg().writeStringEntry("OPTIONS", "left_toolbars", key.text());

        key = "";
        for (FXuint i = 0; i < right.size(); i++)
        {
            key += FXStringVal(right[i].second) + ";";
        }
        getApp()->reg().writeStringEntry("OPTIONS", "right_toolbars", key.text());


        // Status and panels
        getApp()->reg().writeUnsignedEntry("OPTIONS", "status", (FXuint)lpanel->statusbarShown());
        getApp()->reg().writeRealEntry("OPTIONS", "treepanel_tree_pct", (int)(treepanel_tree_pct * 1000) / 1000.0);
        getApp()->reg().writeRealEntry("OPTIONS", "twopanels_lpanel_pct", (int)(twopanels_lpanel_pct * 1000) / 1000.0);
        getApp()->reg().writeRealEntry("OPTIONS", "treetwopanels_tree_pct",
                                       (int)(treetwopanels_tree_pct * 1000) / 1000.0);
        getApp()->reg().writeRealEntry("OPTIONS", "treetwopanels_lpanel_pct",
                                       (int)(treetwopanels_lpanel_pct * 1000) / 1000.0);

        // Panel stacking
        getApp()->reg().writeUnsignedEntry("OPTIONS", "vert_panels", vertpanels);

        // Save panel view only if not given from command line
        if (panel_mode == -1)
        {
            getApp()->reg().writeUnsignedEntry("OPTIONS", "panel_view", (FXuint)panel_view);
        }

        FXString history = "";
        for (int i = 0; i < RunNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(RunHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + RunHistory[i] + ":") == 0) )
            {
                history += RunHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "run", history.text());

        history = "";
        for (int i = 0; i < OpenNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(OpenHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + OpenHistory[i] + ":") == 0) )
            {
                history += OpenHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "open", history.text());

        history = "";
        for (int i = 0; i < FilterNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(FilterHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + FilterHistory[i] + ":") == 0) )
            {
                history += FilterHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "filter", history.text());

        history = "";
        for (int i = 0; i < FindNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(FindHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + FindHistory[i] + ":") == 0) )
            {
                history += FindHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "find", history.text());

        history = "";
        for (int i = 0; i < ServerNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(ServerHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + ServerHistory[i] + ":") == 0) )
            {
                history += ServerHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "server", history.text());

        history = "";
        for (int i = 0; i < ShareNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(ShareHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + ShareHistory[i] + ":") == 0) )
            {
                history += ShareHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "share", history.text());

        history = "";
        for (int i = 0; i < DomainNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(DomainHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + DomainHistory[i] + ":") == 0) )
            {
                history += DomainHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "domain", history.text());

        history = "";
        for (int i = 0; i < UserNum; i++)
        {
            // Avoid duplicates
            if ( (i == 1 && history.contains(FXString(UserHistory[i]) + ":") == 0) ||
                 (i != 1 && history.contains(FXString(":") + UserHistory[i] + ":") == 0) )
            {
                history += UserHistory[i];
                history += ":";
            }
        }
        getApp()->reg().writeStringEntry("HISTORY", "user", history.text());
            

        // Folders panel mounts list height
        getApp()->reg().writeUnsignedEntry("DIR PANEL", "mounts_height", dirpanel->getMountsHeight());
    }

    // Last visited directories
    getApp()->reg().writeStringEntry("LEFT PANEL", "lastdir", lpanel->getDirectory().text());
    getApp()->reg().writeStringEntry("RIGHT PANEL", "lastdir", rpanel->getDirectory().text());

    // Save tabs if required
    FXbool restore_tabs = getApp()->reg().readUnsignedEntry("OPTIONS", "restore_tabs", false);
    if (restore_tabs && tabbuttons->getNumActiveTabs() > 0)
    {
        if (getApp()->reg().readStringEntry("TABS", "tab0", NULL))
        {
            getApp()->reg().deleteSection("TABS");
        }

        for (FXuint i = 0; i < tabbuttons->getNumActiveTabs(); i++)
        {
            FXString regkey = "tab" + FXStringVal(i);
            FXString data = tabbuttons->getPath(i);
            getApp()->reg().writeStringEntry("TABS", regkey.text(), data.text());
       }        

        // Check consistency between current tab index and number of tabs
        FXuint currentTab = FXMIN(tabbuttons->getCurrentTab(), tabbuttons->getNumActiveTabs() - 1);
        
        // Save current tab index
        getApp()->reg().writeUnsignedEntry("OPTIONS", "current_tab", currentTab);
    }
        
    // Don't save tabs
    else
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "current_tab", 0);
        getApp()->reg().deleteSection("TABS");
    }

    // Read registry file to get bookmarks that could have been changed from another Xfe instance
    FXRegistry* reg = new FXRegistry(XFEAPPNAME, XFEVDRNAME);
    reg->read();

    if (reg->readStringEntry("BOOKMARKS", "bookmark0", NULL))
    {
        getApp()->reg().deleteSection("BOOKMARKS");

        FXuint i = 0;
        while (1)
        {
            FXString regkey = "bookmark" + FXStringVal(i);
            FXString data = reg->readStringEntry("BOOKMARKS", regkey.text(), NULL);

            if (data != "")
            {
                getApp()->reg().writeStringEntry("BOOKMARKS", regkey.text(), data.text());
                i++;
            }
            else
            {
                break;
            }
        }
    }

    // Reset restarted flag
    getApp()->reg().writeUnsignedEntry("SETTINGS", "restarted", false);

    // Update registry
    getApp()->reg().write();
}


// Make application
void XFileExplorer::create()
{
    // Switch to two panels mode if startdir2 was specified
    // and no particular panel mode was selected
    if ((startdir2 != "") && (panel_mode == -1))
    {
        panel_mode = 2;
    }

    // Possibly select panel mode from the command line option
    // or revert to last saved panel view
    switch (panel_mode)
    {
    case 0:
        panel_view = TREE_PANEL;
        break;

    case 1:
        panel_view = ONE_PANEL;
        break;

    case 2:
        panel_view = TWO_PANELS;
        break;

    case 3:
        panel_view = TREE_TWO_PANELS;
        break;

    default:
        panel_view = getApp()->reg().readUnsignedEntry("OPTIONS", "panel_view", TREE_PANEL);
    }

    int width = getApp()->reg().readUnsignedEntry("OPTIONS", "width", DEFAULT_WINDOW_WIDTH);
    int height = getApp()->reg().readUnsignedEntry("OPTIONS", "height", DEFAULT_WINDOW_HEIGHT);
    save_win_pos = getApp()->reg().readUnsignedEntry("SETTINGS", "save_win_pos", false);
    if (save_win_pos)
    {
        int xpos = getApp()->reg().readIntEntry("OPTIONS", "xpos", DEFAULT_WINDOW_XPOS);
        int ypos = getApp()->reg().readIntEntry("OPTIONS", "ypos", DEFAULT_WINDOW_YPOS);
        position(xpos, ypos, width, height);
    }
    else
    {
        position(getX(), getY(), width, height);
    }

    // Search dialog geometry
    search_xpos = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "search_xpos", 100);
    search_ypos = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "search_ypos", 100);
    search_width = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "search_width", 800);
    search_height = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "search_height", 600);

    FXMainWindow::create();

    twopanels_lpanel_pct = getApp()->reg().readRealEntry("OPTIONS", "twopanels_lpanel_pct", 0.50);
    treepanel_tree_pct = getApp()->reg().readRealEntry("OPTIONS", "treepanel_tree_pct", 0.20);
    treetwopanels_tree_pct = getApp()->reg().readRealEntry("OPTIONS", "treetwopanels_tree_pct", 0.20);
    treetwopanels_lpanel_pct = getApp()->reg().readRealEntry("OPTIONS", "treetwopanels_lpanel_pct", 0.40);

    // Complete scripts menu
    FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;
    lpanel->getCurrent()->readScriptDir(this, scriptsmenu, scriptpath);

    // Window width and height
    int window_width = getWidth();
    int window_height = getHeight();

    switch (panel_view)
    {
    case ONE_PANEL:
        rpanel->hide();
        dirpanel->hide();

        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);

        // Handle active icon
        lpanel->showActiveIcon(false);
        lpanel->setWidth((int)round(1.0 * window_width));
        break;

    case TWO_PANELS:
        dirpanel->hide();
        if (vertpanels)
        {
            lpanel->setWidth((int)round(twopanels_lpanel_pct * window_width));
        }
        else
        {
            lpanel->setHeight((int)round(twopanels_lpanel_pct * window_height));
        }

        // Handle drag corner
        rpanel->showCorner(true);
        lpanel->showCorner(false);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case TREE_PANEL:
        rpanel->hide();
        dirpanel->setWidth((int)round(treepanel_tree_pct * window_width));
        lpanel->setWidth((int)round((1.0 - treepanel_tree_pct) * window_width));

        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case TREE_TWO_PANELS:
        dirpanel->setWidth((int)round(treetwopanels_tree_pct * window_width));
        if (vertpanels)
        {
            lpanel->setWidth((int)round(treetwopanels_lpanel_pct * window_width));
        }
        else
        {
            lpanel->setHeight((int)round(treetwopanels_lpanel_pct * window_height));
        }

        // Handle drag corner
        rpanel->showCorner(true);
        lpanel->showCorner(false);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;
    }

    if (!getApp()->reg().readUnsignedEntry("OPTIONS", "status", true))
    {
        handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_TOGGLE_STATUS), NULL);
    }

    file_tooltips = getApp()->reg().readUnsignedEntry("SETTINGS", "file_tooltips", true);
    relative_resize = getApp()->reg().readUnsignedEntry("SETTINGS", "relative_resize", true);
    always_show_tabbar = getApp()->reg().readUnsignedEntry("OPTIONS", "always_show_tabbar", true);

    // Wheel scrolling
    int wheellines = getApp()->reg().readUnsignedEntry("SETTINGS", "wheellines", 5);
    getApp()->setWheelLines(wheellines);

    // Scrollbar size
    int barsize = getApp()->reg().readUnsignedEntry("SETTINGS", "scrollbarsize", 12);
    getApp()->setScrollBarSize(barsize);

    // If there are only files to open, tell Xfe not to show its window
    winshow = true;
    if ((startdir1 == "") && (startdir2 == "") && (nbstartfiles > 0))
    {
        winshow = false;
    }

    // Set file panels list style
    liststyle = getApp()->reg().readUnsignedEntry("LEFT PANEL", "liststyle", ICONLIST_DETAILED | ICONLIST_AUTOSIZE);
    lpanel->setListStyle(liststyle);
    liststyle = getApp()->reg().readUnsignedEntry("RIGHT PANEL", "liststyle", ICONLIST_DETAILED | ICONLIST_AUTOSIZE);
    rpanel->setListStyle(liststyle);

    // Show or hide hidden files listed in panels
    FXbool hiddenfiles = getApp()->reg().readUnsignedEntry("LEFT PANEL", "hiddenfiles", 0);
    lpanel->showHiddenFiles(hiddenfiles);
    hiddenfiles = getApp()->reg().readUnsignedEntry("RIGHT PANEL", "hiddenfiles", 0);
    rpanel->showHiddenFiles(hiddenfiles);

    // Show or hide hidden directories listed in dirpanel
    FXbool hidden_dir = getApp()->reg().readUnsignedEntry("DIR PANEL", "hidden_dir", 0);
    dirpanel->showHiddenFiles(hidden_dir);

    // Read start directory mode
    FXuint startdirmode = getApp()->reg().readUnsignedEntry("OPTIONS", "startdir_mode", START_HOMEDIR);

    // Open left and right panels in starting directories (if specified) or in home / current or last directory
    if (startdir1 == "")
    {
        switch (startdirmode)
        {
        case START_HOMEDIR:
            startdir1 = FXSystem::getHomeDirectory();
            break;

        case START_CURRENTDIR:
            startdir1 = FXSystem::getCurrentDirectory();
            break;

        case START_LASTDIR:
            startdir1 = getApp()->reg().readStringEntry("LEFT PANEL", "lastdir", ROOTDIR);
            break;
        }
    }
    if (startdir2 == "")
    {
        switch (startdirmode)
        {
        case START_HOMEDIR:
            startdir2 = FXSystem::getHomeDirectory();
            break;

        case START_CURRENTDIR:
            startdir2 = FXSystem::getCurrentDirectory();
            break;

        case START_LASTDIR:
            startdir2 = getApp()->reg().readStringEntry("RIGHT PANEL", "lastdir", ROOTDIR);
            break;
        }
    }

    lpanel->setDirectory(startdir1, true);
    lpanel->setPathLink(startdir1);
    lpanel->setPathText(startdir1);
    rpanel->setDirectory(startdir2, true);
    rpanel->setPathLink(startdir2);
    rpanel->setPathText(startdir2);
    dirpanel->setDirectory(startdir1, true);

    // Open file on startup, if any
    if (nbstartfiles > 0)
    {
        openFiles(startURIs);
    }

    // History
    FXString history = getApp()->reg().readStringEntry("HISTORY", "run", "");
    FXString histent;
    RunNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < RUN_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(RunHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        RunNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "open", "");
    histent = "";
    OpenNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < OPEN_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(OpenHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        OpenNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "filter", "");
    histent = "";
    FilterNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < FILTER_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(FilterHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        FilterNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "find", "");
    histent = "";
    FindNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < FIND_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(FindHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        FindNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "server", "");
    histent = "";
    ServerNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < SERVER_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(ServerHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        ServerNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "share", "");
    histent = "";
    ShareNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < SHARE_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(ShareHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        ShareNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "domain", "");
    histent = "";
    DomainNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < DOMAIN_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(DomainHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        DomainNum = i;
    }

    history = getApp()->reg().readStringEntry("HISTORY", "user", "");
    histent = "";
    UserNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < USER_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(UserHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        UserNum = i;
    }

    getApp()->forceRefresh();

    // Running as root?
    FXbool root_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "root_warn", true);
    if ((getuid() == 0) && root_warn)
    {
        MessageBox::information(this, BOX_OK, _("Warning"), _("Running Xfe as root!"));
    }

    // Initial focus is always on the left panel
    lpanel->setFocusOnList();

#if defined(linux)
    // Warning message if a mount point is down
    FXbool mount_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_warn", true);
    if (mount_warn)
    {
        int d;
        for (d = updevices->first(); d < updevices->size(); d = updevices->next(d))
        {
            if (xf_strequal(updevices->data(d), "down"))
            {
                MessageBox::warning(this, BOX_OK, _("Warning"), _("Mount point %s is not responding..."),
                                    updevices->key(d));
            }
        }
    }
#endif

    // If no Xfe local configuration exists (i.e. at first call or after a purge of the configuration files),
    // copy the global xferc file to the local configuration directory, and read / write the registry
    int mask;
    FXString configlocation = xdgconfighome + PATHSEPSTRING XFECONFIGPATH;
    FXString configpath = configlocation + PATHSEPSTRING XFECONFIGNAME;

    if (!xf_existfile(configpath))
    {
        // Create ~/.config/xfe directory if it doesn't exist
        if (!xf_existfile(configlocation))
        {
            // Create the ~/.config/xfe directory according to the umask
            mask = umask(0);
            umask(mask);
            errno = 0;
            int ret = xf_mkpath(configlocation.text(), 511 & ~mask);
            int errcode = errno;
            if (ret == -1)
            {
                if (errcode)
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't create Xfe config folder %s: %s"),
                                      configlocation.text(), strerror(errcode));
                }
                else
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't create Xfe config folder %s"),
                                      configlocation.text());
                }
            }
        }

        // Copy the global xfrec file (three possible locations) to the local configuration file
        if (xf_existfile("/usr/share/xfe/xferc"))
        {
            FXFile::copy("/usr/share/xfe/xferc", configpath, false);
        }
        else if (xf_existfile("/usr/local/share/xfe/xferc"))
        {
            FXFile::copy("/usr/local/share/xfe/xferc", configpath, false);
        }
        else if (xf_existfile("/opt/local/share/xfe/xferc"))
        {
            FXFile::copy("/opt/local/share/xfe/xferc", configpath, false);
        }
        // If nothing is found, display a file dialog to let the user choose the right place
        else
        {
            FileDialog browse(this, _("No global xferc file found! Please select a configuration file..."));

            const char* patterns[] =
            {
                _("XFE configuration file"), "*xferc*", NULL
            };
            browse.setFilename(ROOTDIR);
            browse.setPatternList(patterns);

            if (browse.execute())
            {
                FXString path = browse.getFilename();
                FXFile::copy(path, configpath, false);
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("Xfe cannot run without a global xferc configuration file"));
                exit(EXIT_FAILURE);
            }
        }

        // Read and write the registry
        getApp()->reg().read();
        getApp()->reg().write();
    }

    // Create trash can files directory if it doesn't exist
    if (!xf_existfile(trashfileslocation))
    {
        // Create the trash can files directory according to the umask
        mask = umask(0);
        umask(mask);
        errno = 0;
        int ret = xf_mkpath(trashfileslocation.text(), 511 & ~mask);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'files' folder %s: %s"),
                                  trashfileslocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'files' folder %s"),
                                  trashfileslocation.text());
            }
        }
    }

    // Create trash can info directory if it doesn't exist
    if (!xf_existfile(trashinfolocation))
    {
        // Create the trash can info directory according to the umask
        mask = umask(0);
        umask(mask);
        errno = 0;
        int ret = xf_mkpath(trashinfolocation.text(), 511 & ~mask);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'info' folder %s: %s"),
                                  trashinfolocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'info' folder %s"),
                                  trashinfolocation.text());
            }
        }
    }

    // Possibly start iconic or maximized
    if (starticonic)
    {
        minimize();
    }
    if (startmaximized)
    {
        maximize();
    }

#ifdef STARTUP_NOTIFICATION
    startup_completed();
#endif

    // Restore tabs if required
    FXbool restore_tabs = getApp()->reg().readUnsignedEntry("OPTIONS", "restore_tabs", false);
    if (restore_tabs && getApp()->reg().readStringEntry("TABS", "tab0", NULL) && !startdirs)
    {
        FXuint i = 0;
        while (1)
        {
            FXString regkey = "tab" + FXStringVal(i);
            FXString path = getApp()->reg().readStringEntry("TABS", regkey.text(), NULL);

            if (path != "")
            {
                tabbuttons->addTab(path);
                i++;
            }
            else
            {
                break;
            }
        }

        // Current tab
        FXuint index = getApp()->reg().readUnsignedEntry("OPTIONS", "current_tab", 0);
        tabbuttons->setCurrentTab(index);
    }

    // Show tab according to start directory mode
    else
    {
        // Show tab bar if needed
        if (always_show_tabbar)
        {
            // Add first tab
            if (startdirmode == START_HOMEDIR)
            {
                tabbuttons->addTab(homedir);
            }
            else if (startdirmode == START_CURRENTDIR)
            {
                tabbuttons->addTab(lpanel->getCurrent()->getDirectory());       
            }
            else // Last directory
            {
                FXString lastdir = getApp()->reg().readStringEntry("LEFT PANEL", "lastdir", ROOTDIR);
                tabbuttons->addTab(lastdir);
            }       
        }
    }

    // Show window
    if (winshow)
    {
        show();
    }

    // Tell Xfe to stop, if we didn't show its window
    if (!winshow)
    {
        stop = true;
    }

#if defined(linux) && defined(XFE_AUTOMOUNTER)

    // Non root user
    if (getuid())
    {        
        // Automount status 
        FXbool automount = getApp()->reg().readUnsignedEntry("OPTIONS", "automount", false);

        // Check if udisks2 is installed
        if (automount)
        {
            FXString cmd = "udisksctl status";
            FXString str = xf_getcommandoutput(cmd);

            if (str.find("DEVICE") == -1)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Unable to find udisksctl command on your system.\
\nPackage udisks2 is probably not installed, Xfe automounter is disabled."));       
                
                getApp()->reg().writeUnsignedEntry("OPTIONS", "automount", false);
                automount = false;
            }            
        }
        
        // Check if Xfe automounter is running for the actual user    
        FXString cmd = "pgrep -u "+ FXSystem::userName(FXSystem::user()) + " xfe-automount";
        FXString pid = xf_getcommandoutput(cmd);

        // Restart flag
        FXbool restart_automount = getApp()->reg().readUnsignedEntry("OPTIONS", "restart_automount", false);
       
        // Stop automounter if running and (disabled or (enabled and restart))
        if ( (pid != "") && ( !automount || (automount && restart_automount) ) )
        {
            cmd = "killall -q xfe-automount";
            int ret = system(cmd.text());
            if (ret < 0)
            {
                fprintf(stderr, _("Error: Can't execute command %s"), cmd.text());
            }            
        }
        
        // Start automounter if enabled and (not running or (running and restart)
        if (automount && ( (pid == "") || ( (pid != "") && restart_automount) ) )
        {
            // Run  automounter
            cmd = "xfe-automount &";
            int ret = system(cmd.text());
            if (ret < 0)
            {
                fprintf(stderr, _("Error: Can't execute command %s"), cmd.text());
            }            
        }

        // Reset restart flag
        getApp()->reg().writeUnsignedEntry("OPTIONS", "restart_automount", false);
    }

#endif
}


// Destructor
XFileExplorer::~XFileExplorer()
{
    delete menubar;
    delete addresstoolbar;
    delete addressbox;
    delete filemenu;
    delete toolsmenu;
    delete trashmenu;
    delete editmenu;
    delete viewmenu;
    delete lpanelmenu;
    delete rpanelmenu;
    delete scriptsmenu;
    delete helpmenu;
    delete filemenutitle;
    delete trashmenutitle;
    delete editmenutitle;
    delete viewmenutitle;
    delete lpanelmenutitle;
    delete rpanelmenutitle;
    delete scriptsmenutitle;
    delete helpmenutitle;
    delete generaltoolbar;
    delete paneltoolbar;
    delete tabtoolbar;
    delete toolstoolbar;
    delete tabbuttons;
    delete dirpanel;
    delete lpanel;
    delete rpanel;
    delete btnbackhist;
    delete btnforwardhist;
    delete rundialog;
    delete prefsdialog;
    delete helpwindow;
    delete searchwindow;
    delete bookmarkssep;
    delete bookmarksmenutitle;
    delete bookmarksmenu;
    delete addbookmarkdialog;
    delete connectdialog;
}


// If Tab pressed, cycle through the panels
long XFileExplorer::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    int current;

    // Tab was pressed : cycle through the panels from left to right
    if (event->code == KEY_Tab)
    {
        if (dirpanel->shown())
        {
            if (dirpanel->isActive())
            {
                lpanel->setFocusOnList();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->setCurrentItem(current);
                lpanel->selectItem(current);
            }
            else if ((rpanel->shown()) && (lpanel->isActive()))
            {
                rpanel->setFocusOnList();
                current = rpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                rpanel->setCurrentItem(current);
                rpanel->selectItem(current);
            }
            else
            {
                dirpanel->setFocusOnList();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->deselectItem(current);
            }
        }
        else if (rpanel->shown())
        {
            if (lpanel->getCurrent() == rpanel)
            {
                lpanel->setActive();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->setCurrentItem(current);
                lpanel->selectItem(current);
            }
            else
            {
                rpanel->setActive();
                current = rpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                rpanel->setCurrentItem(current);
                rpanel->selectItem(current);
            }
        }
        return 1;
    }
    // Shift-Tab was pressed : cycle through the panels from right to left
    else if (((event->state & SHIFTMASK) && (event->code == KEY_Tab)) ||
             ((event->state & SHIFTMASK) && (event->code == KEY_ISO_Left_Tab)))
    {
        if (rpanel->shown())
        {
            if (rpanel->isActive())
            {
                lpanel->setFocusOnList();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->setCurrentItem(current);
                lpanel->selectItem(current);
            }
            else if (dirpanel->shown() && dirpanel->isActive())
            {
                rpanel->setFocusOnList();
                current = rpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                rpanel->setCurrentItem(current);
                rpanel->selectItem(current);
            }
            else if (lpanel->isActive())
            {
                if (dirpanel->shown())
                {
                    dirpanel->setFocusOnList();
                    current = lpanel->getCurrentItem();
                    if (current < 0)
                    {
                        current = 0;
                    }
                    lpanel->deselectItem(current);
                }
                else
                {
                    rpanel->setFocusOnList();
                    current = rpanel->getCurrentItem();
                    if (current < 0)
                    {
                        current = 0;
                    }
                    rpanel->setCurrentItem(current);
                    rpanel->selectItem(current);
                }
            }
        }
        else
        {
            if (dirpanel->isActive() && dirpanel->shown())
            {
                lpanel->setFocusOnList();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->setCurrentItem(current);
                lpanel->selectItem(current);
            }
            else if (dirpanel->shown())
            {
                dirpanel->setFocusOnList();
                current = lpanel->getCurrentItem();
                if (current < 0)
                {
                    current = 0;
                }
                lpanel->deselectItem(current);
            }
        }

        return 1;
    }
    // Ctrl-Shift-F10 or Menu was pressed : open popup menu
    else if ((event->state & SHIFTMASK && event->code == KEY_F10) || event->code == KEY_Menu)
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_POPUP_MENU), ptr);
        return 1;
    }
    // Any other key was pressed : handle the pressed key in the usual way
    else
    {
        if (FXTopWindow::onKeyPress(sender, sel, ptr))
        {
            return 1;
        }
    }

    return 0;
}


long XFileExplorer::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXTopWindow::onKeyRelease(sender, sel, ptr))
    {
        return 1;
    }

    return 0;
}


// Harvest the zombies
long XFileExplorer::onSigHarvest(FXObject*, FXSelector, void*)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
    return 1;
}


// Handle quitting
long XFileExplorer::onQuit(FXObject*, FXSelector, void*)
{
    saveConfig();

    getApp()->exit(EXIT_SUCCESS);
    return 1;
}


// Directory up
long XFileExplorer::onCmdDirUp(FXObject* sender, FXSelector, void*)
{
    lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_DIRECTORY_UP), NULL);

    // Set focus on dirpanel or filepanel
    if (panelfocus == DIRPANEL_FOCUS)
    {
        dirpanel->setFocusOnList();
    }
    else
    {
        lpanel->getCurrent()->setFocusOnList();
    }

    return 1;
}


// Directory back
long XFileExplorer::onCmdDirBack(FXObject*, FXSelector, void*)
{
    StringItem* item;
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Get the previous directory
    item = filepanel->backhistGetFirst();
    if (item)
    {
        pathname = filepanel->backhistGetString(item);
    }

    // Update the history
    filepanel->backhistRemoveFirstItem();
    filepanel->forwardhistInsertFirstItem(filepanel->getDirectory());

    // Go to the previous directory
    filepanel->setDirectory(pathname, false);
    filepanel->updatePath();
    dirpanel->setDirectory(pathname, false);

    // Set focus on dirpanel or filepanel
    if (panelfocus == DIRPANEL_FOCUS)
    {
        dirpanel->setFocusOnList();
    }
    else
    {
        filepanel->setFocusOnList();
    }

    return 1;
}


// Update directory back
long XFileExplorer::onUpdDirBack(FXObject* sender, FXSelector, void* ptr)
{
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Gray out the button if no item in history
    if (filepanel->backhistGetNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory forward
long XFileExplorer::onCmdDirForward(FXObject*, FXSelector, void*)
{
    StringItem* item;
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Get the next directory
    item = filepanel->forwardhistGetFirst();
    if (item)
    {
        pathname = filepanel->forwardhistGetString(item);
    }

    // Update the history
    filepanel->forwardhistRemoveFirstItem();
    filepanel->backhistInsertFirstItem(lpanel->getCurrent()->getDirectory());

    // Go to the next directory
    filepanel->setDirectory(pathname, false);
    filepanel->updatePath();
    dirpanel->setDirectory(pathname, true);

    // Set focus on dirpanel or filepanel
    if (panelfocus == DIRPANEL_FOCUS)
    {
        dirpanel->setFocusOnList();
    }
    else
    {
        filepanel->setFocusOnList();
    }

    return 1;
}


// Update directory forward
long XFileExplorer::onUpdDirForward(FXObject* sender, FXSelector, void* ptr)
{
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Gray out the button if no item in history
    if (filepanel->forwardhistGetNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory back history
long XFileExplorer::onCmdDirBackHist(FXObject* sender, FXSelector, void*)
{
    StringItem* item;
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Get all string items and display them in a list box
    int num = filepanel->backhistGetNumItems();

    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = filepanel->backhistGetFirst();
        int nb = 0;
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = filepanel->backhistGetString(item);
                FXbool flag = true;
                for (int j = 0; j <= nb - 1; j++)
                {
                    if (str == dirs[j])
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag)
                {
                    dirs[nb] = str;
                    strlist = strlist + str + "\n";
                    nb++;
                }
                item = filepanel->backhistGetNext(item);
            }
        }

        // Display list box
        int pos = DirHistBox::box(btnbackhist, DECOR_NONE, strlist, getX() + 40, getY() + 60);

        // If an item was selected
        if (pos != -1)
        {
            // Update back history
            if (pos == num - 1)
            {
                filepanel->backhistRemoveAllItems();
            }
            else
            {
                item = filepanel->backhistGetItemAtPos(pos + 1);
                filepanel->backhistRemoveAllItemsBefore(item);
            }

            // Update forward history
            filepanel->forwardhistInsertFirstItem(filepanel->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    filepanel->forwardhistInsertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            filepanel->setDirectory(pathname, false);
            filepanel->updatePath();
            dirpanel->setDirectory(pathname, true);
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory back
long XFileExplorer::onUpdDirBackHist(FXObject* sender, FXSelector, void* ptr)
{
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Gray out the button if no item in history
    if (filepanel->backhistGetNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory forward history
long XFileExplorer::onCmdDirForwardHist(FXObject* sender, FXSelector, void*)
{
    StringItem* item;
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Get all string items and display them in a list box
    int num = filepanel->forwardhistGetNumItems();

    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = filepanel->forwardhistGetFirst();
        int nb = 0;
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = filepanel->forwardhistGetString(item);
                FXbool flag = true;
                for (int j = 0; j <= nb - 1; j++)
                {
                    if (str == dirs[j])
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag)
                {
                    dirs[nb] = str;
                    strlist = strlist + str + "\n";
                    nb++;
                }
                item = filepanel->forwardhistGetNext(item);
            }
        }

        // Display list box
        int pos = DirHistBox::box(btnforwardhist, DECOR_NONE, strlist, getX() + 85, getY() + 60);

        // If an item was selected
        if (pos != -1)
        {
            // Update forward history
            if (pos == num - 1)
            {
                filepanel->forwardhistRemoveAllItems();
            }
            else
            {
                item = filepanel->forwardhistGetItemAtPos(pos + 1);
                filepanel->forwardhistRemoveAllItemsBefore(item);
            }

            // Update back history
            filepanel->backhistInsertFirstItem(filepanel->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    filepanel->backhistInsertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            filepanel->setDirectory(pathname, false);
            filepanel->updatePath();
            dirpanel->setDirectory(pathname, true);
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory forward
long XFileExplorer::onUpdDirForwardHist(FXObject* sender, FXSelector, void* ptr)
{
    FXString pathname;
    FilePanel* filepanel = lpanel->getCurrent();

    // Gray out the button if no item in history
    if (filepanel->forwardhistGetNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Add bookmark
long XFileExplorer::onCmdAddBookmark(FXObject*, FXSelector, void*)
{
    FXString dirpathname = lpanel->getCurrent()->getDirectory();

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


// Remove all bookmarks
long XFileExplorer::onCmdRemoveAllBookmarks(FXObject*, FXSelector, void*)
{
    if (BOX_CLICKED_CANCEL == MessageBox::question(FXApp::instance()->getActiveWindow(), BOX_OK_CANCEL,
                                                   _("Confirm Remove Bookmarks"),
                                                   _("Do you really want to remove all bookmarks?")))
    {
        return 0;
    }
    else
    {
        // Remove all menu items
        for (int i = bookmarkmc.size() - 1; i >= 0; i--)
        {
            bookmarkmc.pop_back();
            delete bookmarkmc[bookmarkmc.size()]; // pop_back does not delete the menu item
        }

        // Remove all list items
        dirpanel->getBookmarksList()->removeAllItems();
    }

    return 1;
}


// Update remove all bookmarks button and bookmarks menu items
long XFileExplorer::onUpdBookmarksMenu(FXObject* sender, FXSelector, void* ptr)
{
    // Update remove all button
    if (bookmarkmc.size() > 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
        bookmarkssep->show();
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
        bookmarkssep->hide();
    }

    // Update bookmarks menu items if the sizes don't match
    if ((FXint)bookmarkmc.size() != dirpanel->getNumBookmarks())
    {
        // Remove all bookmark menu items
        for (int i = bookmarkmc.size() - 1; i >= 0; i--)
        {
            bookmarkmc.pop_back();
            delete bookmarkmc[bookmarkmc.size()]; // pop_back does not delete the menu item
        }

        // Add items to the menu
        for (int i = 0; i < dirpanel->getNumBookmarks(); i++)
        {
            bookmarkmc.push_back(new FXMenuCommand(bookmarksmenu, dirpanel->getBookmarkName(i), NULL, this,
                                                   ID_BOOKMARK));
            bookmarkmc[i]->create();
        }
    }

    return 1;
}


// Rebuild bookmarks menu (called by DirPanel)
long XFileExplorer::onCmdRebuildBookmarksMenu(FXObject*, FXSelector, void*)
{
    // Remove all bookmark menu items
    for (int i = bookmarkmc.size() - 1; i >= 0; i--)
    {
        bookmarkmc.pop_back();
        delete bookmarkmc[bookmarkmc.size()]; // pop_back does not delete the menu item
    }

    // Add sorted items to the menu
    for (int i = 0; i < dirpanel->getNumBookmarks(); i++)
    {
        bookmarkmc.push_back(new FXMenuCommand(bookmarksmenu, dirpanel->getBookmarkName(i), NULL, this, ID_BOOKMARK));
        bookmarkmc[i]->create();
    }

    return 1;
}


// Goto bookmark location
long XFileExplorer::onCmdGotoBookmark(FXObject* sender, FXSelector, void*)
{
    // Bookmark name
    FXString key = dynamic_cast<FXMenuCommand*>(sender)->getText();

    // Bookmark location
    FXString pathname = dirpanel->getBookmarkLocation(key);

    // Permission problem or does not exist
    if (!xf_existfile(pathname))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), pathname.text());
        return 0;
    }
    else if (!xf_isreadexecutable(pathname))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), pathname.text());
        return 0;
    }

    // Go to bookmark location
    lpanel->getCurrent()->setDirectory(pathname);
    lpanel->getCurrent()->updatePath();
    dirpanel->setDirectory(pathname, true);

    // Set bookmark (select and set current)
    dirpanel->getBookmarksList()->setBookmark(key);

    // Update address list
    FXString item;
    int i = 0;
    int count = addressbox->getNumItems();
    if (!count)
    {
        count++;
        addressbox->insertItem(0, addressbox->getText());
    }
    while (i < count)
    {
        item = addressbox->getItem(i++);
        if (xf_strequal(pathname.text(), (const char*)&item[0]))
        {
            i--;
            break;
        }
    }
    if (i == count)
    {
        addressbox->insertItem(0, pathname);
    }

    // Set focus to the active panel
    lpanel->getCurrent()->setFocusOnList();

    return 1;
}


// Clear address bar
long XFileExplorer::onCmdClearAddress(FXObject*, FXSelector, void*)
{
    addressbox->setText(FXString::null);
    addressbox->CursorEnd();

    return(1);
}


// Goto address entered into the text field
long XFileExplorer::onCmdGotoAddress(FXObject*, FXSelector, void*)
{
    // Address where we want to go
    FXString address = addressbox->getText();

    // In case address is given in URI form, convert it
    address = ::xf_filefromuri(address);

    // If first two characters are '~/' then replace '~' with home directory
    if (address.find("~/") == 0)
    {
        address.replace(0, 1, FXSystem::getHomeDirectory());
    }

    // Get complete path
    FXString path = FXPath::absolute(lpanel->getCurrent()->getDirectory(), address);
    FXString dir = path;

    // Go up to the lowest directory which still exists
    while (!FXPath::isTopDirectory(dir) && !xf_isdirectory(dir))
    {
        dir = FXPath::upLevel(dir);
    }

    // Move to this existing directory
    lpanel->getCurrent()->setDirectory(dir);
    lpanel->getCurrent()->updatePath();
    dirpanel->setDirectory(dir, true);
    addressbox->setText(dir);

    return 1;
}


// Restart the application when required
long XFileExplorer::onCmdRestart(FXObject*, FXSelector, void*)
{
    saveConfig();

    if (fork() == 0) // Child
    {
        execvp("xfe", args);
    }
    else // Parent
    {
        exit(EXIT_SUCCESS);
    }
    return 1;
}


// Start a new Xfe session
long XFileExplorer::onCmdNewWindow(FXObject*, FXSelector, void*)
{
    FXString startdir;

    // Read start directory mode
    FXuint startdirmode = getApp()->reg().readUnsignedEntry("OPTIONS", "startdir_mode", START_HOMEDIR);

    if (startdirmode == START_CURRENTDIR)
    {
        startdir = FXSystem::getCurrentDirectory();
    }
    else if (startdirmode == START_LASTDIR)
    {
        startdir = getApp()->reg().readStringEntry("LEFT PANEL", "lastdir", ROOTDIR);
    }
    else
    {
        startdir = FXSystem::getHomeDirectory();
    }

    // Run
    FXString cmd = "xfe " + startdir + " &";

    int ret = system(cmd.text());

    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    return 1;
}


// Add a new tab
long XFileExplorer::onCmdNewTab(FXObject*, FXSelector, void*)
{
    // Current directory
    FXString dirpathname = lpanel->getCurrent()->getDirectory();
   
    // Add tab
    tabbuttons->addTab(dirpathname);

    return 1;
}


// Run Terminal
long XFileExplorer::onCmdXTerm(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();
    FXString xterm = getApp()->reg().readStringEntry("PROGS", "xterm", "xterm -sb");

    // By default use current directory as target directory
    FXString tgtdirectory = lpanel->getCurrent()->getDirectory();

    // If one item is selected and it is a directory, use it as target directory
    if (lpanel->getCurrent()->getNumSelectedItems() == 1)
    {
        int index = lpanel->getCurrent()->getCurrentItem();
        if (index > 0)
        {
            FXString pathname = lpanel->getCurrent()->getItemPathname(index);
            if (xf_isdirectory(pathname))
            {
                tgtdirectory = pathname;
            }
        }
    }

    int ret = chdir(tgtdirectory.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), tgtdirectory.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), tgtdirectory.text());
        }

        return 0;
    }

    FXString cmd = xterm;
    cmd += " &";

    ret = system(cmd.text());
    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    lpanel->getCurrent()->setFocusOnList();
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


// Help menu
long XFileExplorer::onCmdHelp(FXObject*, FXSelector, void*)
{
    // Display help window
    if (helpwindow == NULL)
    {
        helpwindow = new TextWindow(getApp(), _("Help"), 40, 120);
    }
    helpwindow->setIcon(minihelpicon);

    // Set text font
    FXString fontspec;
    fontspec = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    if (!fontspec.empty())
    {
        FXFont* font = new FXFont(getApp(), fontspec);
        font->create();
        helpwindow->setFont(font);
    }

    // NB: The HELP_TEXT macro is defined in help.h
    FXString str = (FXString)"                         " + COPYRIGHT + HELP_TEXT;
    helpwindow->setText(str.text());
    // Non modal window
    helpwindow->create();
    helpwindow->show(PLACEMENT_OWNER);
    lpanel->getCurrent()->setFocusOnList();
    return 1;
}


// About menu
long XFileExplorer::onCmdAbout(FXObject*, FXSelector, void*)
{
    FXString msg;
    msg.format(_("X File Explorer Version %s"), VERSION);

    FXString copyright = (FXString)"\n" + COPYRIGHT + "\n\n" +
                         _("X Win Commander by Maxim Baranov\n\n") +
                         _("FOX Toolkit by Jeroen van der Zijp\n");

    FXString web = "\n\n<http://roland65.free.fr/xfe>\n<http://sourceforge.net/projects/xfe>\n";

    FXString translators =
        _("\nTranslators\n\
-------------\n\
Argentinian Spanish: Bruno Gilberto Luciani\n\
Brazilian Portuguese: Eduardo R.B.S., Jose Carlos Medeiros,\n\
Phantom X, Tomas Acauan Schertel\n\
Bosnian: Samir Ribi, Bajrami Emran, Balagija Jasmina,\n\
Bilalovi, Omar Cogo Emir\n\
Catalan: Pere Orga\n\
Chinese: Xin Li\n\
Chinese (Taïwan): Wei-Lun Chao\n\
Colombian Spanish: Vladimir Támara (Pasos de Jesús)\n\
Czech: David Vachulka\n\
Danish: Jonas Bardino, Vidar Jon Bauge\n\
Dutch: Hans Strijards\n\
Finnish: Kimmo Siira\n\
French: Claude Leo Mercier, Roland Baudin\n\
German: Bastian Kleineidam, Joachim Wiedorn, Tim Benke, Jens Körner\n\
Greek: Nikos Papadopoulos\n\
Hungarian: Attila Szervac, Sandor Sipos\n\
Italian: Claudio Fontana, Giorgio Moscardi\n\
Japanese: Karl Skewes\n\
Norwegian: Vidar Jon Bauge\n\
Polish: Jacek Dziura, Franciszek Janowski\n\
Portuguese: Miguel Santinho\n\
Russian: Dimitri Sertolov, Vad Vad\n\
Spanish: Felix Medrano Sanz, Lucas 'Basurero' Vieites,\n\
Martin Carr\n\
Swedish: Anders F. Bjorklund\n\
Turkish: erkaN\n\
");

    msg = msg + web + copyright + translators;
    MessageBox about(this, _("About X File Explorer"), msg.text(), xfeicon, BOX_OK | DECOR_TITLE | DECOR_BORDER,
                     JUSTIFY_CENTER_X | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FILL_X |
                     LAYOUT_FILL_Y);
    about.execute(PLACEMENT_OWNER);
    lpanel->getCurrent()->setFocusOnList();
    return 1;
}


// Handle file association (called by Properties.cpp and FilePanel.cpp)
long XFileExplorer::onCmdFileAssoc(FXObject*, FXSelector, void* ptr)
{
    char** str = (char**)ptr;
    char* ext = str[0];
    char* cmd = str[1];

    // ext=extension, cmd=associated command
    // replace : to allow immediate association in Xfe
    FileDict* associations = lpanel->getAssociations();

    associations->replace(ext, cmd);
    associations = rpanel->getAssociations();
    associations->replace(ext, cmd);

    saveConfig();

    return 1;
}


// FilePanel and DirPanel refresh
long XFileExplorer::onCmdRefresh(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();

#if defined(linux)
    dirpanel->forceDevicesRefresh();
#endif

    lpanel->onCmdRefresh(0, 0, 0);
    rpanel->onCmdRefresh(0, 0, 0);
    dirpanel->forceRefresh();
    getApp()->endWaitCursor();

    return 1;
}


// Update address
long XFileExplorer::onUpdAddress(FXObject* sender, FXSelector, void*)
{
    FXString currentdir = lpanel->getCurrent()->getDirectory();

    if (currentdir != prevdir)
    {
        // Update address
        addressbox->setText(xf_cleanpath(currentdir));

        // Update address list
        FXString item;
        int i = 0;
        int count = addressbox->getNumItems();
        if (!count)
        {
            count++;
            addressbox->insertItem(0, addressbox->getText());
        }
        while (i < count)
        {
            item = addressbox->getItem(i++);
            if (xf_strequal(currentdir.text(), (const char*)&item[0]))
            {
                i--;
                break;
            }
        }
        if (i == count)
        {
            addressbox->insertItem(0, currentdir);
        }

        prevdir = currentdir;
    }

    return 1;
}


// Switch between vertical and horizontal panels
long XFileExplorer::onCmdHorzVertPanels(FXObject* sender, FXSelector sel, void*)
{
    FXuint width, height;

    switch (FXSELID(sel))
    {
    case ID_VERT_PANELS:

        width = lpanel->getWidth();
        height = getHeight();

        panelsplit->setSplitterStyle(panelsplit->getSplitterStyle() & ~SPLITTER_VERTICAL);

        lpanel->setWidth(width / 2);
        lpanel->setHeight(height);

        vertpanels = true;
        break;

    case ID_HORZ_PANELS:

        width = getWidth();
        height = lpanel->getHeight();

        panelsplit->setSplitterStyle(panelsplit->getSplitterStyle() | SPLITTER_VERTICAL);

        lpanel->setHeight(height / 2);
        lpanel->setWidth(width);

        vertpanels = false;
        break;
    }

    return 1;
}


// Switch between the four possible panel views
long XFileExplorer::onCmdShowPanels(FXObject* sender, FXSelector sel, void* ptr)
{
    // Get window width and height
    int window_width = getWidth();
    int window_height = getHeight();

    switch (FXSELID(sel))
    {
    case ID_SHOW_ONE_PANEL:
        panel_view = ONE_PANEL;
        if (dirpanel->shown())
        {
            dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_TOGGLE_PANEL), ptr);
        }
        if (rpanel->shown())
        {
            rpanel->handle(sender, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), ptr);
        }

        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);

        // Handle active icon
        lpanel->showActiveIcon(false);

        break;

    case ID_SHOW_TWO_PANELS:
        panel_view = TWO_PANELS;
        if (vertpanels)
        {
            lpanel->setWidth((int)round(twopanels_lpanel_pct * window_width));
        }
        else
        {
            lpanel->setHeight((int)round(twopanels_lpanel_pct * window_height));
        }
        if (dirpanel->shown())
        {
            dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_TOGGLE_PANEL), ptr);
        }
        if (!rpanel->shown())
        {
            rpanel->handle(sender, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), ptr);
        }

        // Handle drag corner
        rpanel->showCorner(true);
        lpanel->showCorner(false);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case ID_SHOW_FOLDERS_ONE_PANEL:
        panel_view = TREE_PANEL;
        dirpanel->setWidth((int)round(treepanel_tree_pct * window_width));
        if (!dirpanel->shown())
        {
            dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_TOGGLE_PANEL), ptr);
        }
        if (rpanel->shown())
        {
            rpanel->handle(sender, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), ptr);
        }

        // Handle drag corner
        rpanel->showCorner(false);
        lpanel->showCorner(true);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;

    case ID_SHOW_FOLDERS_TWO_PANELS:
        panel_view = TREE_TWO_PANELS;
        dirpanel->setWidth((int)round(treetwopanels_tree_pct * window_width));
        if (vertpanels)
        {
            lpanel->setWidth((int)round(treetwopanels_lpanel_pct * window_width));
        }
        else
        {
            lpanel->setHeight((int)round(treetwopanels_lpanel_pct * window_height));
        }
        if (!dirpanel->shown())
        {
            dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_TOGGLE_PANEL), ptr);
        }
        if (!rpanel->shown())
        {
            rpanel->handle(sender, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), ptr);
        }

        // Handle drag corner
        lpanel->showCorner(false);
        rpanel->showCorner(true);

        // Handle active icon
        lpanel->showActiveIcon(true);
        break;
    }

    // Set focus on current panel
    lpanel->getCurrent()->setFocusOnList();

    return 1;
}


// Update the horizontal / vertical panel radio menus
long XFileExplorer::onUpdHorzVertPanels(FXObject* sender, FXSelector sel, void* ptr)
{
    if (rpanel->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

        if (vertpanels)
        {
            lpanel->hidePanelSeparator();
            rpanel->hidePanelSeparator();

            if (FXSELID(sel) == ID_HORZ_PANELS)
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_UNCHECK), ptr);
            }
            else
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_CHECK), ptr);
            }
        }
        else
        {
            lpanel->showPanelSeparator();
            rpanel->hidePanelSeparator();

            if (FXSELID(sel) == ID_VERT_PANELS)
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_UNCHECK), ptr);
            }
            else
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_CHECK), ptr);
            }
        }
    }
    else
    {
        lpanel->hidePanelSeparator();
        rpanel->hidePanelSeparator();

        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), ptr);
    }

    return 1;
}


// Update the panels
long XFileExplorer::onUpdShowPanels(FXObject* sender, FXSelector sel, void* ptr)
{
    // Keep the panel sizes relative to the window width or height (if option enabled)

    int width;
    int height;

    // Get the current window width and height
    width = getWidth();
    height = getHeight();

    // If width has changed and relative resizing option is enabled
    if (relative_resize && prev_width != width)
    {
        // One file panel mode is not relevant

        // Two file panels mode
        if (!dirpanel->shown() && rpanel->shown())
        {
            // Set left panel width / height to the new value
            if (vertpanels)
            {
                lpanel->setWidth((int)round(twopanels_lpanel_pct * width));
            }
            else
            {
                lpanel->setHeight((int)round(twopanels_lpanel_pct * height));
            }
        }
        // Folder panel mode
        else if (dirpanel->shown() && !rpanel->shown())
        {
            // Set dirpanel width to the new value
            dirpanel->setWidth((int)round(treepanel_tree_pct * width));
        }
        // Folder panel and two file panels mode
        else if (dirpanel->shown() && rpanel->shown())
        {
            // Set dirpanel width to the new value
            dirpanel->setWidth((int)round(treetwopanels_tree_pct * width));

            // Set left panel width / height to the new value
            if (vertpanels)
            {
                lpanel->setWidth((int)round(treetwopanels_lpanel_pct * width));
            }
            else
            {
                lpanel->setHeight((int)round(treetwopanels_lpanel_pct * height));
            }
        }
    }

    // Update previous window width
    prev_width = width;

    // Update the panel menus and the panel display
    FXuint msg = FXWindow::ID_UNCHECK;
    switch (FXSELID(sel))
    {
    case ID_SHOW_ONE_PANEL:
        if (!dirpanel->shown() && !rpanel->shown())
        {
            msg = FXWindow::ID_CHECK;
            if (rpanelmenutitle->shown())
            {
                rpanelmenutitle->hide();
                rpanelmenutitle->disable();
                lpanelmenutitle->setText(_("&Panel"));
                lpanel->show();
                //lpanel->repaint();
                lpanel->setActive();
            }
        }
        break;

    case ID_SHOW_TWO_PANELS:
        if (!dirpanel->shown() && rpanel->shown())
        {
            // Update the left panel relative size (only if the window size is sufficient)
            if (vertpanels)
            {
                if (getWidth() > 10)
                {
                    twopanels_lpanel_pct = (double)(lpanel->getWidth()) / (double)(getWidth());
                }
            }
            else
            {
                if (getHeight() > 10)
                {
                    twopanels_lpanel_pct = (double)(lpanel->getHeight()) / (double)(getHeight());
                }
            }

            msg = FXWindow::ID_CHECK;
            if (!rpanelmenutitle->shown())
            {
                rpanelmenutitle->enable();
                rpanelmenutitle->show();
                rpanelmenutitle->setText(_("&Right Panel"));
                lpanelmenutitle->setText(_("&Left Panel"));
                //lpanel->repaint();
                lpanel->setActive();
            }
        }
        break;

    case ID_SHOW_FOLDERS_ONE_PANEL:
        if (dirpanel->shown() && !rpanel->shown())
        {
            // Update the folders panel relative size (only if the window size is sufficient)
            if (getWidth() > 10)
            {
                treepanel_tree_pct = (double)(dirpanel->getWidth()) / (double)(getWidth());
            }

            msg = FXWindow::ID_CHECK;
            if (rpanelmenutitle->shown())
            {
                rpanelmenutitle->hide();
                rpanelmenutitle->disable();
                lpanelmenutitle->setText(_("&Panel"));
                //lpanel->repaint();
                lpanel->setActive();
            }
        }
        break;

    case ID_SHOW_FOLDERS_TWO_PANELS:
        if (dirpanel->shown() && rpanel->shown())
        {
            // Update the folders panel relative size (only if the window size is sufficient)
            if (getWidth() > 10)
            {
                treetwopanels_tree_pct = (double)(dirpanel->getWidth()) / (double)(getWidth());
            }

            // Update the left file panel relative size (only if the window size is sufficient)
            if (vertpanels)
            {
                if (getWidth() > 10)
                {
                    treetwopanels_lpanel_pct = (double)(lpanel->getWidth()) / (double)(getWidth());
                }
            }
            else
            {
                if (getHeight() > 10)
                {
                    treetwopanels_lpanel_pct = (double)(lpanel->getHeight()) / (double)(getHeight());
                }
            }

            msg = FXWindow::ID_CHECK;
            if (!rpanelmenutitle->shown())
            {
                rpanelmenutitle->enable();
                rpanelmenutitle->show();
                rpanelmenutitle->setText(_("&Right Panel"));
                lpanelmenutitle->setText(_("&Left Panel"));
                //lpanel->repaint();
                lpanel->setActive();
            }
        }
        break;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);

    return 1;
}


// Synchronize the panels to the same directory
long XFileExplorer::onCmdSynchronizePanels(FXObject* sender, FXSelector, void*)
{
    FXString dir;

    // Left panel is active
    if (lpanel->getCurrent() == lpanel)
    {
        dir = lpanel->getDirectory();
        rpanel->setDirectory(dir);
        rpanel->updatePath();
    }
    // Right panel is active
    else
    {
        dir = rpanel->getDirectory();
        lpanel->setDirectory(dir);
        lpanel->updatePath();
    }
    return 1;
}


// Update the synchronize panels menu item
long XFileExplorer::onUpdSynchronizePanels(FXObject* sender, FXSelector, void*)
{
    if (rpanel->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Switch the panels
long XFileExplorer::onCmdSwitchPanels(FXObject* sender, FXSelector, void*)
{
    FXString leftdir, rightdir;

    leftdir = lpanel->getDirectory();
    rightdir = rpanel->getDirectory();
    lpanel->setDirectory(rightdir);
    lpanel->updatePath();
    rpanel->setDirectory(leftdir);
    rpanel->updatePath();

    return 1;
}


// Update the switch panels menu item
long XFileExplorer::onUpdSwitchPanels(FXObject* sender, FXSelector, void*)
{
    if (rpanel->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Preferences
long XFileExplorer::onCmdPrefs(FXObject*, FXSelector, void*)
{
    if (prefsdialog == NULL)
    {
        prefsdialog = new PreferencesBox(this, listbackcolor, listforecolor, highlightcolor, pbarcolor, attentioncolor,
                                         scrollbarcolor);
    }
    prefsdialog->execute(PLACEMENT_OWNER);
    lpanel->getCurrent()->setFocusOnList();
    return 1;
}


// Toggle status bar
long XFileExplorer::onCmdToggleStatus(FXObject*, FXSelector, void*)
{
    dirpanel->toggleStatusbar();
    lpanel->toggleStatusbar();
    rpanel->toggleStatusbar();
    return 1;
}


// Update toggle status bar
long XFileExplorer::onUpdToggleStatus(FXObject* sender, FXSelector, void*)
{
    FXMenuCheck* cmd = (FXMenuCheck*)sender;

    if (lpanel->statusbarShown())
    {
        cmd->setCheck(true);
    }
    else
    {
        cmd->setCheck(false);
    }

    return 1;
}


// Run shell command or X program
long XFileExplorer::onCmdRun(FXObject*, FXSelector, void*)
{
    int ret;

    ret = chdir(lpanel->getCurrent()->getDirectory().text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                              lpanel->getCurrent()->getDirectory().text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"),
                              lpanel->getCurrent()->getDirectory().text());
        }

        return 0;
    }

    FXString command = " ";
    if (rundialog == NULL)
    {
        rundialog = new HistInputDialog(this, "", _("Execute the command:"), _("Execute command"), "", bigrunicon,
                                        false, true, HIST_INPUT_EXECUTABLE_FILE, true, _("Console mode"));
    }
    rundialog->create();
    rundialog->setText(command);
    rundialog->CursorEnd();
    rundialog->selectAll();
    rundialog->clearItems();
    for (int i = 0; i < RunNum; i++)
    {
        rundialog->appendItem(RunHistory[i]);
    }
    rundialog->sortItems();

    rundialog->setDirectory(ROOTDIR);

    if (rundialog->execute())
    {
        command = rundialog->getText();
        if (command != " ")
        {
            // Execute command in command window
            if (rundialog->getOption())
            {
                // Make and show command window
                CommandWindow* cmdwin = new CommandWindow(getApp(), _("Command log"), command, 30, 80);
                cmdwin->create();
                cmdwin->setIcon(minirunicon);

                // The CommandWindow object will delete itself when closed!
            }
            // Execute silently in background
            else
            {
                command += " &";
                ret = system(command.text());
                if (ret < 0)
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), command.text());
                    return 0;
                }
            }
        }

        // Update history list
        RunNum = rundialog->getHistorySize();
        command = rundialog->getText();

        // Check if cmd is a new string, i.e. is not already in history
        FXbool newstr = true;
        for (int i = 0; i < RunNum - 1; i++)
        {
            if (xf_strequal(RunHistory[i], command.text()))
            {
                newstr = false;
                break;
            }
        }

        // No new string or history limit reached
        if (!newstr || RunNum > RUN_HIST_SIZE)
        {
            RunNum--;
        }

        // Restore original history order
        rundialog->clearItems();
        for (int i = 0; i < RunNum; i++)
        {
            rundialog->appendItem(RunHistory[i]);
        }

        // New string
        if (newstr)
        {
            // FIFO
            xf_strlcpy(RunHistory[0], command.text(), command.length() + 1);
            for (int i = 1; i < RunNum; i++)
            {
                xf_strlcpy(RunHistory[i], rundialog->getHistoryItem(i - 1).text(),
                         rundialog->getHistoryItem(i - 1).length() + 1);
            }
        }
    }
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

    lpanel->getCurrent()->setFocusOnList();

    // If list has been cleared, make sure history size is zero
    if (rundialog->getHistorySize() == 0)
    {
        RunNum = 0;
    }

    return 1;
}


// Run an Xfe as root
long XFileExplorer::onCmdSu(FXObject*, FXSelector, void*)
{
    int ret, status;

    // Wait cursor
    getApp()->beginWaitCursor();

    // Obtain preferred root authentication mode
    FXuint root_auth = getApp()->reg().readUnsignedEntry("OPTIONS", "root_auth", 0);

    // Current directory
    FXString currdir = lpanel->getCurrent()->getDirectory();

    // Select pkexec, sudo or su to launch xfe as root

    FXString title, cmd, command;

    // Use pkexec
    if (root_auth == 0)
    {
        // Check if pkexec exists
        if (!xf_existcommand("pkexec"))
        {
            MessageBox::error(this, BOX_OK, _("Error"),
            _("Command pkexec not found!\n\nPlease check that the pkexec package is installed (else use sudo or su root mode)"));
            getApp()->endWaitCursor();
            return 0;
        }

        // Build command from current directory
        cmd = getApp()->reg().readStringEntry("OPTIONS", "pkexec_cmd", DEFAULT_PKEXEC_CMD);
        cmd += " " + currdir;

#ifdef STARTUP_NOTIFICATION
        status = runcmd(cmd, "pkexec", currdir, startlocation, false, "");
#else
        status = runcmd(cmd, currdir, startlocation);
#endif
    }
    // Use sudo or su
    else
    {
        // Enter current directory
        ret = chdir(currdir.text());
        if (ret < 0)
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), currdir.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), currdir.text());
            }

            return 0;
        }

        // sudo
        if (root_auth == 1)
        {
            title = _("Enter the user password:");
            FXString sudo_cmd = getApp()->reg().readStringEntry("OPTIONS", "sudo_cmd", DEFAULT_SUDO_CMD);
            cmd = " -g 60x4 -e " + sudo_cmd;
        }
        // su
        else if (root_auth == 2)
        {
            title = _("Enter the root password:");
            FXString su_cmd = getApp()->reg().readStringEntry("OPTIONS", "su_cmd", DEFAULT_SU_CMD);
            cmd = " -g 60x4 -e " + su_cmd;
        }
        else
        {
            // Should not happen
        }

        // Get text font
        FXString fontspec = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
        if (fontspec.empty())
        {
            command = "st -t " + xf_quote(title) + cmd;
        }
        else
        {
            FXchar fontsize[32];
            FXFont* font = new FXFont(getApp(), fontspec);
            font->create();
            snprintf(fontsize, sizeof(fontsize), "%d", (int)(font->getSize() / 10)); // Size is in deci-points, thus divide by 10
            command = "st -t " + xf_quote(title) + " -f '" + (font->getFamily()).text() + ":pixelsize=" + fontsize +
                      "'" + cmd;
            delete font;
        }

        // Execute su or sudo command in an internal st terminal
        status = xf_runst(command);
    }

    // If error
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

        getApp()->beginWaitCursor();
        return 0;
    }

    if (status < 0)
    {
        MessageBox::error(getApp(), BOX_OK, _("Error"), _("An error has occurred!"));
        getApp()->endWaitCursor();
        return 0;
    }

    // Wait cursor
    getApp()->endWaitCursor();
    return 1;
}


// Connect to a server
long XFileExplorer::onCmdConnectToServer(FXObject*, FXSelector, void*)
{
    // Connect dialog
    if (connectdialog == NULL)
    {
        connectdialog = new ConnectDialog(this, _("Connect to Server"), lpanel, dirpanel);
    }
    
    connectdialog->execute(PLACEMENT_OWNER);

    return 1;
}


// File search dialog
long XFileExplorer::onCmdFileSearch(FXObject*, FXSelector, void*)
{
    // Display search box
    if (searchwindow == NULL)
    {
        searchwindow = new SearchWindow(getApp(), _("Search Files and Folders"),
                                        search_xpos, search_ypos, search_width, search_height, 0, 0, 0, 0, 0, 0);
    }

    // Non modal window
    searchwindow->create();
    searchwindow->show(PLACEMENT_DEFAULT);

    // Set search path in search window
    searchwindow->setSearchPath(lpanel->getCurrent()->getDirectory());

    return 1;
}


// Update file search button
long XFileExplorer::onUpdFileSearch(FXObject* sender, FXSelector, void*)
{
    if (searchwindow != NULL && searchwindow->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    return 1;
}


// Empty trash can
long XFileExplorer::onCmdEmptyTrash(FXObject*, FXSelector, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

    // Compute trash directory size
    char buf[MAXPATHLEN + 1];
    char size[64];
    FXString hsize;
    FXulong dirsize;
    FXuint nbfiles = 0, nbsubfolders = 0;
    FXulong totalsize = 0;

    FXString dirpath = trashfileslocation;
    xf_strlcpy(buf, dirpath.text(), dirpath.length() + 1);
    dirsize = xf_pathsize(buf, &nbfiles, &nbsubfolders, &totalsize);

#if __WORDSIZE == 64
    snprintf(size, sizeof(size), "%lu", dirsize);
#else
    snprintf(size, sizeof(size), "%llu", dirsize);
#endif
    hsize = xf_humansize(size);
#if __WORDSIZE == 64
    snprintf(size, sizeof(size), _("%s (%lu bytes)"), hsize.text(), dirsize);
#else
    snprintf(size, sizeof(size), _("%s (%llu bytes)"), hsize.text(), dirsize);
#endif
    snprintf(size, sizeof(size), _("%u files, %u subfolders"), nbfiles - nbsubfolders, nbsubfolders - 1);

    // Wait cursor
    getApp()->endWaitCursor();

    // Confirmation message
    FXString message = _("Do you really want to empty the trash can?") + FXString(" (") + hsize + _(" in ") +
                       FXString(size) + FXString(")")
                       + _("\n\nAll items will be definitively lost!");

    MessageBox box(this, _("Empty trash can"), message, bigtrashfullicon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
    if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
    {
        return 0;
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // Delete trash can files folder
    File* f;
    f = new File(this, _("Delete Files"), DELETE);
    f->create();
    f->remove(trashfileslocation);
    delete f;

    // Delete trash can info folder
    f = new File(this, _("Delete Files"), DELETE);
    f->create();
    f->remove(trashinfolocation);
    delete f;

    // Re-create the trash can files directory
    if (!xf_existfile(trashfileslocation))
    {
        errno = 0;
        int ret = xf_mkpath(trashfileslocation.text(), 0755);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'files' folder %s: %s"),
                                  trashfileslocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'files' folder %s"),
                                  trashfileslocation.text());
            }
        }
    }

    // Re-create the trash can info directory
    if (!xf_existfile(trashinfolocation))
    {
        errno = 0;
        int ret = xf_mkpath(trashinfolocation.text(), 0755);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'info' folder %s: %s"),
                                  trashinfolocation.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create trash can 'info' folder %s"),
                                  trashinfolocation.text());
            }
        }
    }

    // Wait cursor
    getApp()->endWaitCursor();

    onCmdRefresh(0, 0, 0);

    return 1;
}


// Display trash size
long XFileExplorer::onCmdTrashSize(FXObject*, FXSelector, void*)
{
    struct stat linfo;
    FXString trashsize, trashmtime, trashnbfiles, trashnbfolders;

    if (xf_lstat(trashfileslocation.text(), &linfo) == 0)
    {
        // Read time format
        FXString timeformat = getApp()->reg().readStringEntry("SETTINGS", "time_format", DEFAULT_TIME_FORMAT);

        // Trash files size
        trashmtime = FXSystem::time(timeformat.text(), linfo.st_mtime);
        char buf[MAXPATHLEN];
        FXulong dirsize = 0;
        FXuint nbfiles = 0, nbsubfolders = 0;
        FXulong totalsize = 0;
        xf_strlcpy(buf, trashfileslocation.text(), trashfileslocation.length() + 1);
        dirsize = xf_pathsize(buf, &nbfiles, &nbsubfolders, &totalsize);
#if __WORDSIZE == 64
        snprintf(buf, sizeof(buf), "%lu", dirsize);
#else
        snprintf(buf, sizeof(buf), "%llu", dirsize);
#endif
        trashsize = xf_humansize(buf);
        trashnbfiles = FXStringVal(nbfiles - nbsubfolders);
        trashnbfolders = FXStringVal(nbsubfolders - 1);

        // Dialog box
        FXString msg;
        msg.format(_("Trash size: %s (%s files, %s subfolders)\n\nModified date: %s"),
                   trashsize.text(), trashnbfiles.text(), trashnbfolders.text(), trashmtime.text());
        MessageBox dialog(this, _("Trash size"), msg.text(), bigdeleteicon, BOX_OK | DECOR_TITLE | DECOR_BORDER,
                          JUSTIFY_LEFT | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FILL_X |
                          LAYOUT_FILL_Y);
        dialog.execute(PLACEMENT_CURSOR);
    }
    else
    {
        // Error
        MessageBox::error(this, BOX_OK, _("Error"), _("Trash can 'files' folder %s is not readable!"),
                          trashfileslocation.text());
        return 0;
    }

    return 1;
}


// File copy to clipboard
long XFileExplorer::onCmdFileCopyClp(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_COPY_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_COPY_CLIPBOARD), NULL);
    }

    return 1;
}


// File name copy to clipboard
long XFileExplorer::onCmdFileCopyName(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_COPYNAME_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_COPYNAME_CLIPBOARD), NULL);
    }

    return 1;
}


// File cut to clipboard
long XFileExplorer::onCmdFileCutClp(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_CUT_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_CUT_CLIPBOARD), NULL);
    }

    return 1;
}


// File add copy to clipboard
long XFileExplorer::onCmdFileAddCopyClp(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_ADDCOPY_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_ADDCOPY_CLIPBOARD), NULL);
    }

    return 1;
}


// File add cut to clipboard
long XFileExplorer::onCmdFileAddCutClp(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_ADDCUT_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_ADDCUT_CLIPBOARD), NULL);
    }

    return 1;
}


// File paste from clipboard
long XFileExplorer::onCmdFilePasteClp(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_PASTE_CLIPBOARD), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_PASTE_CLIPBOARD), NULL);
    }

    return 1;
}


// File rename
long XFileExplorer::onCmdFileRename(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_RENAME), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_RENAME), NULL);
    }

    return 1;
}


// File move
long XFileExplorer::onCmdFileMoveto(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_MOVETO), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_MOVETO), NULL);
    }

    return 1;
}


// File copy to
long XFileExplorer::onCmdFileCopyto(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_COPYTO), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_COPYTO), NULL);
    }

    return 1;
}


// File symlink
long XFileExplorer::onCmdFileSymlink(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_SYMLINK), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_SYMLINK), NULL);
    }

    return 1;
}


// File trash
long XFileExplorer::onCmdFileTrash(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_TRASH), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_TRASH), ptr);
    }

    return 1;
}


// File restore
long XFileExplorer::onCmdFileRestore(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_RESTORE), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_RESTORE), ptr);
    }

    return 1;
}


// File delete
long XFileExplorer::onCmdFileDelete(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_DIR_DELETE), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_FILE_DELETE), ptr);
    }

    return 1;
}


// File properties
long XFileExplorer::onCmdFileProperties(FXObject* sender, FXSelector, void*)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_COMMAND, DirPanel::ID_PROPERTIES), NULL);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_COMMAND, FilePanel::ID_PROPERTIES), NULL);
    }

    return 1;
}


// Update the empty trash can and trash menus
long XFileExplorer::onUpdEmptyTrash(FXObject* sender, FXSelector, void*)
{
    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);

    if (use_trash_can)
    {
        // Update the empty trash can menu
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);

        // Update the trash menu title
        helpmenutitle->setText("");
        trashmenutitle->setText(_("T&rash"));
        trashmenutitle->enable();
        trashmenutitle->show();
        helpmenutitle->setText(_("&Help"));
    }
    else
    {
        // Update the empty trash can menu
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);

        // Update the trash menu title
        trashmenutitle->hide();
        trashmenutitle->disable();
        helpmenutitle->setText("");
        helpmenutitle->setText(_("&Help"));
    }

    return 1;
}


// Update the trash size menu
long XFileExplorer::onUpdTrashSize(FXObject* sender, FXSelector, void*)
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


// Update the file delete menu item
long XFileExplorer::onUpdFileDelete(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_UPDATE, DirPanel::ID_DIR_DELETE), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_UPDATE, FilePanel::ID_FILE_DELETE), ptr);
    }

    return 1;
}


// Update the move to trash menu item
long XFileExplorer::onUpdFileTrash(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_UPDATE, DirPanel::ID_DIR_TRASH), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_UPDATE, FilePanel::ID_FILE_TRASH), ptr);
    }

    return 1;
}


// Update the restore from trash menu item
long XFileExplorer::onUpdFileRestore(FXObject* sender, FXSelector, void* ptr)
{
    if (dirpanel->isActive())
    {
        dirpanel->handle(sender, FXSEL(SEL_UPDATE, DirPanel::ID_DIR_RESTORE), ptr);
    }
    else
    {
        lpanel->getCurrent()->handle(sender, FXSEL(SEL_UPDATE, FilePanel::ID_FILE_RESTORE), ptr);
    }

    return 1;
}


// Update file operation menu items
long XFileExplorer::onUpdFileMan(FXObject* sender, FXSelector, void*)
{
    // Update show tab bar
    if (always_show_tabbar) // Always show
    {
        tabtoolbar->show();
    }
    else
    {
        if (tabbuttons->getNumActiveTabs() == 0)
        {
            tabtoolbar->hide();
        }
        else
        {
            tabtoolbar->show();
        }        
    }
    
    // Update the panelfocus variable
    if (lpanel->getCurrent()->isActive())
    {
        panelfocus = FILEPANEL_FOCUS;
    }
    if (dirpanel->isActive())
    {
        panelfocus = DIRPANEL_FOCUS;
    }

    // Update the file operation menu items
    if (dirpanel->isActive())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);

        // Update copy names menu label when a directory is selected
        copynamesmc->setText(_("Cop&y Name"));
    }
    else
    {
        // Set default copy names menu label
        copynamesmc->setText(_("Cop&y Names"));

        int num = lpanel->getCurrent()->getNumSelectedItems();
        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else if (num == 1)
        {
            if (lpanel->getCurrent()->isItemSelected(0))
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
            }
            else
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);

                // Update copy names menu label when only one file is selected
                copynamesmc->setText(_("Cop&y Name"));
            }
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }

    return 1;
}


// Update the file rename menu items
long XFileExplorer::onUpdFileRename(FXObject* sender, FXSelector, void*)
{
    int num = lpanel->getCurrent()->getNumSelectedItems();

    if (num == 1)
    {
        if (lpanel->getCurrent()->isItemSelected(0))
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else if (num > 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        if (dirpanel->isActive())
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


// Update the paste menu and button
long XFileExplorer::onUpdFilePaste(FXObject* sender, FXSelector, void*)
{
    lpanel->getCurrent()->handle(sender, FXSEL(SEL_UPDATE, FilePanel::ID_PASTE_CLIPBOARD), NULL);
    return 1;
}


// Update the root menu items
long XFileExplorer::onUpdSu(FXObject* sender, FXSelector, void*)
{
    FXbool root_mode = getApp()->reg().readUnsignedEntry("OPTIONS", "root_mode", true);

    if (!root_mode || getuid() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    return 1;
}



// Open files URIS
void XFileExplorer::openFiles(vector_FXString startURIs)
{
    FXString pathname, samecmd, cmd, cmdname, itemslist = " ";
    FileAssoc* association;
    FXbool same = true;
    FXbool first = true;

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

    // Check if all files have the same association
    for (FXuint u = 0; u < startURIs.size(); u++)
    {
        if (xf_isfile(startURIs[u]))
        {
            // Increment number of selected items
            pathname = startURIs[u];
            association = assocdict->findFileBinding(pathname.text());

            if (association)
            {
                cmd = association->command.section(',', 0);

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

                if (cmd != "")
                {
                    // First item
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

                    // List of items
                    itemslist += xf_quote(pathname) + " ";
                }
                else
                {
                    same = false;
                    break;
                }
            }
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

    // Same command for all files: open them
    if (same)
    {
        cmdname = samecmd;

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            cmd = samecmd + itemslist;
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, lpanel->getDirectory(), startlocation, usesn, snexcepts);
#else
            runcmd(cmd, lpanel->getDirectory(), startlocation);
#endif
        }
        // Command does not exist
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Command not found: %s"), cmd.text());
        }
    }
    // Files have different commands: handle them separately
    else
    {
        for (FXuint u = 0; u < startURIs.size(); u++)
        {
            if (xf_isfile(startURIs[u]))
            {
                pathname = startURIs[u];
                association = assocdict->findFileBinding(pathname.text());
                if (association)
                {
                    // Use it to open the file
                    cmd = association->command.section(',', 0);

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

                    if (cmd != "")
                    {
                        cmdname = cmd;

                        // If command exists, run it
                        if (xf_existcommand(cmdname))
                        {
                            cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, lpanel->getDirectory(), startlocation, usesn, snexcepts);
#else
                            runcmd(cmd, lpanel->getDirectory(), startlocation);
#endif
                        }
                        // Command does not exist
                        else
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Command not found: %s"), cmdname.text());
                        }
                    }
                    // Command string is void
                    else
                    {
                        MessageBox::error(this, BOX_OK, _("Error"), _("Invalid file association: %s"),
                                          FXPath::extension(pathname).text());
                    }
                }
                // Other cases
                else
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("File association not found: %s"),
                                      FXPath::extension(pathname).text());
                }
            }
        }
    }
    delete assocdict;
}


// Quit immediately and properly, if asked
long XFileExplorer::onUpdQuit(FXObject* sender, FXSelector, void*)
{
    if (stop)
    {
        onQuit(0, 0, 0);
    }

    return 1;
}
