#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <fcntl.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "startupnotification.h"
#include "FileDialog.h"
#include "FileDict.h"
#include "MessageBox.h"
#include "CommandWindow.h"
#include "XFileArchive.h"


// Add FOX hacks
#include "foxhacks.cpp"
#include "moderncontrols.cpp"


// Global variables
char** args;
FXString homedir;
FXString xdgconfighome;
FXString xdgdatahome;
FXString execpath;

// Integer UI scaling factor
FXint scaleint = 1;

// Filter history
char FilterHistory[FILTER_HIST_SIZE][MAX_PATTERN_SIZE];
int FilterNum = 0;

// Open history
char OpenHistory[OPEN_HIST_SIZE][MAX_COMMAND_SIZE];
int OpenNum = 0;

// Main window (not used but necessary for compilation)
FXMainWindow* mainWindow = NULL;


// Hand cursor replacement (integer scaling factor = 1)
#define hand1_width     32
#define hand1_height    32
#define hand1_x_hot     6
#define hand1_y_hot     1
static const FXuchar hand1_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x90, 0x03, 0x00, 0x00,
    0x90, 0x1c, 0x00, 0x00, 0x10, 0xe4, 0x00, 0x00, 0x1c, 0x20, 0x01, 0x00,
    0x12, 0x00, 0x01, 0x00, 0x12, 0x00, 0x01, 0x00, 0x92, 0x24, 0x01, 0x00,
    0x82, 0x24, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x02, 0x00, 0x01, 0x00, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const FXuchar hand1_mask_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
    0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00,
    0xf0, 0x1f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xfc, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// Hand cursor replacement (integer scaling factor = 2)
#define hand2_width     32
#define hand2_height    32
#define hand2_x_hot     6
#define hand2_y_hot     1
static const FXuchar hand2_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00,
    0x60, 0x06, 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00,
    0x20, 0x1e, 0x00, 0x00, 0x60, 0x3e, 0x00, 0x00, 0x20, 0xe2, 0x03, 0x00,
    0x60, 0x62, 0x1e, 0x00, 0x38, 0x00, 0x74, 0x00, 0x7c, 0x00, 0x60, 0x00,
    0x24, 0x00, 0x40, 0x00, 0x64, 0x00, 0x60, 0x00, 0x26, 0x00, 0x40, 0x00,
    0x26, 0x22, 0x62, 0x00, 0x06, 0x22, 0x42, 0x00, 0x06, 0x00, 0x60, 0x00,
    0x06, 0x00, 0x40, 0x00, 0x06, 0x00, 0x60, 0x00, 0x04, 0x00, 0x60, 0x00,
    0xfc, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const FXuchar hand2_mask_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00,
    0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00,
    0xe0, 0x1f, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00,
    0xe0, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00,
    0xfc, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00,
    0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00,
    0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00,
    0xfc, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// Hand cursor replacement (integer scaling factor = 3 or more)
#define hand3_width     32
#define hand3_height    32
#define hand3_x_hot     6
#define hand3_y_hot     1
static const FXuchar hand3_bits[] =
{
    0x80, 0x1f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00,
    0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00,
    0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0xf0, 0x03, 0x00,
    0xc0, 0xf0, 0x07, 0x00, 0xc0, 0x30, 0xfe, 0x00, 0xc0, 0x10, 0xfe, 0x01,
    0xc0, 0x10, 0x8c, 0x3f, 0xc0, 0x10, 0x04, 0x7f, 0xfc, 0x00, 0x04, 0xe1,
    0xfe, 0x00, 0x04, 0xc1, 0xc6, 0x00, 0x04, 0xc0, 0xc6, 0x00, 0x00, 0xc0,
    0xc6, 0x00, 0x00, 0xc0, 0xc3, 0x00, 0x00, 0xc0, 0xc3, 0x00, 0x00, 0xc0,
    0xc3, 0x10, 0x04, 0xc1, 0x03, 0x10, 0x04, 0xc1, 0x03, 0x10, 0x04, 0xc1,
    0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0,
    0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0,
    0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f
};

static const FXuchar hand3_mask_bits[] =
{
    0x80, 0x1f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00,
    0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00,
    0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0x03, 0x00,
    0xc0, 0xff, 0x07, 0x00, 0xc0, 0xff, 0xff, 0x00, 0xc0, 0xff, 0xff, 0x01,
    0xc0, 0xff, 0xff, 0x3f, 0xc0, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f
};


// Map
FXDEFMAP(XFileArchive) XFileArchiveMap[] =
{
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_OPEN, XFileArchive::onCmdOpen),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_RECENTFILE, XFileArchive::onCmdRecentFile),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_EXTRACT_TO, XFileArchive::onCmdExtractTo),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_EXTRACT, XFileArchive::onCmdExtract),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_ABOUT, XFileArchive::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_QUIT, XFileArchive::onCmdQuit),
    FXMAPFUNC(SEL_CLICKED, XFileArchive::ID_LIST, XFileArchive::onCmdClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, XFileArchive::ID_LIST, XFileArchive::onCmdDoubleClicked),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_FILE_OPEN, XFileArchive::onCmdFileOpen),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_FILE_OPEN_WITH, XFileArchive::onCmdFileOpenWith),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_FILE_EXTRACT, XFileArchive::onCmdFileExtract),
    FXMAPFUNC(SEL_COMMAND, XFileArchive::ID_FILE_EXTRACT_TO, XFileArchive::onCmdFileExtractTo),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, XFileArchive::ID_LIST, XFileArchive::onCmdPopupMenu),
    FXMAPFUNC(SEL_QUERY_TIP, XFileArchive::ID_LIST, XFileArchive::onQueryTip),
    FXMAPFUNC(SEL_CHORE, XFileArchive::ID_WATCHPROCESS, XFileArchive::onWatchProcess),
    FXMAPFUNC(SEL_SIGNAL, XFileArchive::ID_HARVEST, XFileArchive::onSigHarvest),
    FXMAPFUNC(SEL_CLOSE, 0, XFileArchive::onCmdQuit),
    FXMAPFUNC(SEL_UPDATE, XFileArchive::ID_OPEN, XFileArchive::onUpdOpen),
    FXMAPFUNC(SEL_UPDATE, XFileArchive::ID_EXTRACT_TO, XFileArchive::onUpdExtractTo),
    FXMAPFUNC(SEL_UPDATE, XFileArchive::ID_EXTRACT, XFileArchive::onUpdExtract),
    FXMAPFUNC(SEL_UPDATE, XFileArchive::ID_QUIT, XFileArchive::onUpdQuit),
};


// Object implementation
FXIMPLEMENT(XFileArchive, FXMainWindow, XFileArchiveMap, ARRAYNUMBER(XFileArchiveMap))

// Construct
XFileArchive::XFileArchive(FXApp* a) : FXMainWindow(a, "Xfa", NULL, NULL, DECOR_TITLE | DECOR_MINIMIZE |
                                                    DECOR_MAXIMIZE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE)
{
    FXString key;
    FXHotKey hotkey;

    // Application icon
    setIcon(xfaicon);

    // Make menu bar
    menubar = new FXMenuBar(this, LAYOUT_DOCK_NEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE);

    // Sites where to dock
    FXDockSite* topdock = new FXDockSite(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    new FXDockSite(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    new FXDockSite(this, LAYOUT_SIDE_LEFT | LAYOUT_FILL_Y);
    new FXDockSite(this, LAYOUT_SIDE_RIGHT | LAYOUT_FILL_Y);

    // Tool bar
    FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
    toolbar = new FXToolBar(topdock, dragshell1, LAYOUT_DOCK_NEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X |
                            LAYOUT_FILL_Y | FRAME_NONE);
    new FXToolBarGrip(toolbar, toolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);
    int style = BUTTON_TOOLBAR;

    // File menu
    filemenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&File"), NULL, filemenu);

    // Preferences Menu
    prefsmenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&Preferences"), NULL, prefsmenu);

    // Help menu
    helpmenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    // Toolbar button: File open
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    new FXButton(toolbar, TAB + _("Open Archive File") + PARS(key), minifileopenicon, this,
                 ID_OPEN, style | FRAME_NONE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    // File Menu entries
    FXMenuCommand* mc = NULL;

    mc = new FXMenuCommand(filemenu, _("&Open..."), minifileopenicon, this, ID_OPEN);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    // Recent file menu; this automatically hides if there are no files
    FXMenuSeparator* sep1 = new FXMenuSeparator(filemenu);
    sep1->setTarget(&mrufiles);
    sep1->setSelector(FXRecentFiles::ID_ANYFILES);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_1);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_2);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_3);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_4);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_5);
    new FXMenuCommand(filemenu, _("&Clear Recent Files") + TAB2 + _("Clear recent file menu."), NULL, &mrufiles,
                      FXRecentFiles::ID_CLEAR);
    FXMenuSeparator* sep2 = new FXMenuSeparator(filemenu);
    sep2->setTarget(&mrufiles);
    sep2->setSelector(FXRecentFiles::ID_ANYFILES);

    mc = new FXMenuCommand(filemenu, _("&Quit"), miniquiticon, this, ID_QUIT);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "quit", "Ctrl-Q");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));
    getAccelTable()->addAccel(KEY_Escape, this, FXSEL(SEL_COMMAND, ID_QUIT));

    // Preferences menu
    new FXMenuCheck(prefsmenu, _("&Toolbar"), toolbar, FXWindow::ID_TOGGLESHOWN);

    // Help Menu entries
    mc = new FXMenuCommand(helpmenu, _("&About X File Archive"), NULL, this, ID_ABOUT, 0);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "help", "F1");
    mc->setAccelText(key);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    // Close accelerator
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, XFileArchive::ID_QUIT));

    // Make a tool tip
    new FXToolTip(getApp(), 0);

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    new FXDragCorner(buttons);

    // Status
    status = new FXLabel(buttons, "", NULL, JUSTIFY_LEFT);

    // Quit
    new FXButton(buttons, _("&Quit"), NULL, this, XFileArchive::ID_QUIT, FRAME_GROOVE | LAYOUT_RIGHT |
                 LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Extract
    extractbtn = new FXButton(buttons, _("&Extract"), NULL, this, XFileArchive::ID_EXTRACT,
                              FRAME_GROOVE | LAYOUT_RIGHT |
                              LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Extract to
    extracttobtn = new FXButton(buttons, _("Extract &to..."), NULL, this, XFileArchive::ID_EXTRACT_TO, FRAME_GROOVE |
                                LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Tree list
    list = new FXTreeList(this, this, XFileArchive::ID_LIST, LAYOUT_FILL_X | LAYOUT_FILL_Y |
                          TREELIST_SHOWS_LINES | TREELIST_SINGLESELECT);

    // Set cursor for single click
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }

    // Recent files
    mrufiles.setTarget(this);
    mrufiles.setSelector(ID_RECENTFILE);

    // Initialize archive path name
    archpathname = "";

    // Other initializations
    smoothscroll = true;
    errorflag = false;

    // Read settings
    save_win_pos = getApp()->reg().readUnsignedEntry("SETTINGS", "save_win_pos", false);

    // Default programs identifiers
    progs["<txtviewer>"] = TXTVIEWER;
    progs["<txteditor>"] = TXTEDITOR;
    progs["<imgviewer>"] = IMGVIEWER;
    progs["<imgeditor>"] = IMGEDITOR;
    progs["<pdfviewer>"] = PDFVIEWER;
    progs["<audioplayer>"] = AUDIOPLAYER;
    progs["<videoplayer>"] = VIDEOPLAYER;
    progs["<archiver>"] = ARCHIVER;

    // Tmp dir
    tmpdir = FXSystem::getTempDirectory() + "/xfa-tmp/";
}


// Destructor
XFileArchive::~XFileArchive()
{
    delete menubar;
    delete toolbar;
    delete status;
    delete filemenu;
    delete prefsmenu;
    delete helpmenu;
    delete opendialog;
    delete list;
    delete extractbtn;
    delete extracttobtn;
    if (cmdwin != NULL)
    {
        delete cmdwin;
    }
}


// About box
long XFileArchive::onCmdAbout(FXObject*, FXSelector, void*)
{
    FXString msg;

    msg.format(_("X File Archive Version %s is a simple archive viewer and extractor.\n\n"), VERSION);
    msg += COPYRIGHT;
    MessageBox about(this, _("About X File Archive"), msg.text(), xfaicon, BOX_OK | DECOR_TITLE | DECOR_BORDER,
                     JUSTIFY_CENTER_X | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FILL_X |
                     LAYOUT_FILL_Y);
    about.execute(PLACEMENT_OWNER);
    return 1;
}


// Open archive
long XFileArchive::onCmdOpen(FXObject*, FXSelector, void*)
{
    status->setText("");

    const char* patterns[] =
    {
        _("All Files"), "*",
        _("tar.gz Archives"), "*.tar.gz",
        _("tgz Archives"), "*.tgz",
        _("gz Archive"), "*.gz",
        _("zip Archives"), "*.zip",
        _("jar Archives"), "*.jar",
        _("iso Images"), "*.iso",
        _("tar.bz2 Archives"), "*.tar.bz2",
        _("tbz2 Archives"), "*.tbz2",
        _("bz2 Archives"), "*.bz2",
        _("tar.xz Archives"), "*.tar.xz",
        _("txz Archives"), "*.txz",
        _("xz Archives"), "*.xz",
        _("tar.zst Archives"), "*.tar.zst",
        _("tzst Archives"), "*.tzst",
        _("zst Archives"), "*.zst",
        _("tar.Z Archives"), "*.tar.Z",
        _("taZ Archives"), "*.taZ",
        _("Z Archives"), "*.Z",
        _("7z Archives"), "*.7z",
        _("rar Archives"), "*.rar",
        _("lzh Archives"), "*.lzh",
        _("ace Archives"), "*.ace",
        _("arj Archives"), "*.arj",
        NULL
    };

    archpathname = "";

    errorflag = false;
    FileDialog opendialog(this, _("Open Archive"));

    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList(patterns);
    opendialog.setDirectory(FXPath::directory(archpathname));

    // Clear list
    list->clearItems();

    if (opendialog.execute(PLACEMENT_OWNER))
    {
        archpathname = opendialog.getFilename();
        mrufiles.appendFile(archpathname);
        readArchive();
    }

    return 1;
}


// Open recent file
long XFileArchive::onCmdRecentFile(FXObject*, FXSelector, void* ptr)
{
    archpathname = (char*)ptr;
    readArchive();

    return 1;
}


// Extract archive to selected directory
long XFileArchive::onCmdExtractTo(FXObject*, FXSelector, void*)
{
    list->killSelection();
    status->setText("");

    FXString cmd;

    if (strlen(archpathname.text()) == 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("No archive opened"));
        return 0;
    }

    // Archive name
    FXString archname = FXPath::name(archpathname);

    // Extensions
    ext1 = archpathname.rafter('.', 1).lower();
    ext2 = archpathname.rafter('.', 2).lower();

    // Extract command
    if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z")
        || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
        (ext1 == "tzst"))
    {
        errorflag = false;
        cmd = "tar -axvf ";
    }
    else if (ext1 == "tar")
    {
        errorflag = false;
        cmd = "tar xvf ";
    }
    else if (ext1 == "zip" || ext1 == "jar")
    {
        errorflag = false;
        cmd = "unzip -o ";
    }
    else if (ext1 == "iso")
    {
        errorflag = false;
        cmd = "bsdtar xvf ";
    }
    else if (ext1 == "7z")
    {
        errorflag = false;
        cmd = "7z x -y ";
    }
    else if (ext1 == "rar")
    {
        errorflag = false;
        cmd = "unrar x -o+ ";
    }
    else if (ext1 == "lzh")
    {
        errorflag = false;
        cmd = "lha -xf ";
    }
    else if (ext1 == "ace")
    {
        errorflag = false;
        cmd = "unace x ";
    }
    else if (ext1 == "arj")
    {
        errorflag = false;
        cmd = "arj x -y ";
    }
    else if (errorflag == false)
    {
        errorflag = true;
        list->clearItems();
        MessageBox::error(this, BOX_OK, _("Error"), _("Unknown archive format"));
        return 0;
    }

    // File selection dialog
    FileDialog browse(this, _("Select a Destination Folder"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };
    browse.setDirectory(homedir);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_DIRECTORY);

    // Extract archive
    if (browse.execute(PLACEMENT_CURSOR))
    {
        FXString dir = browse.getFilename();

        if (xf_iswritable(dir))
        {
            FXString message = _("Extract archive to folder: ") + dir + "\n\n" +
                               _("Existing data will be overwritten...");
            MessageBox box(this, _("Extract Archive"), message, bigarchexticon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return 0;
            }

            // Wait cursor
            getApp()->beginWaitCursor();

            // Final extract command
            if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") ||
                (ext2 == "tar.z")
                || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                (ext1 == "tzst"))
            {
                cmd += xf_quote(archpathname) + " --directory " + xf_quote(dir);
            }
            else if (ext1 == "tar")
            {
                cmd += xf_quote(archpathname) + " --directory " + xf_quote(dir);
            }
            else if (ext1 == "iso")
            {
                cmd += xf_quote(archpathname) + " --directory " + xf_quote(dir);
            }
            else if (ext1 == "zip" || ext1 == "jar")
            {
                cmd += xf_quote(archpathname) + " -d " + xf_quote(dir);
            }
            else if (ext1 == "7z")
            {
                cmd += xf_quote(archpathname) + " -o" + xf_quote(dir);
            }
            else if (ext1 == "rar")
            {
                cmd += xf_quote(archpathname) + " -op " + xf_quote(dir);
            }
            else if (ext1 == "lzh")
            {
                cmd += xf_quote(archpathname) + " w=" + xf_quote(dir);
            }
            else if (ext1 == "ace")
            {
                cmd += xf_quote(archpathname) + " " + xf_quote(dir);
            }
            else if (ext1 == "arj")
            {
                cmd += xf_quote(archpathname) + " " + xf_quote(dir);
            }

            // Make and show command window
            cmdwin = new CommandWindow(this, _("Extract Archive"), cmd, 30, 80);
            cmdwin->create();
            cmdwin->setIcon(miniarchexticon);

            // The CommandWindow object will delete itself when closed!

            getApp()->endWaitCursor();
        }
        else
        {
            MessageBox::error(getApp(), BOX_OK, _("Error"), _("Can't write to %s: Permission denied"), dir.text());

            getApp()->endWaitCursor();
            status->setText("");

            return 0;
        }
    }

    return 1;
}


// Extract archive to its parent directory
long XFileArchive::onCmdExtract(FXObject*, FXSelector, void*)
{
    list->killSelection();
    status->setText("");

    FXString cmd;

    if (strlen(archpathname.text()) == 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("No archive opened"));
        return 0;
    }

    // Archive name
    FXString archname = FXPath::name(archpathname);

    // Parent directory
    FXString parentdir = FXPath::directory(archpathname);

    // Extensions
    ext1 = archpathname.rafter('.', 1).lower();
    ext2 = archpathname.rafter('.', 2).lower();

    // Extract command
    if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z")
        || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
        (ext1 == "tzst"))
    {
        errorflag = false;
        cmd = "tar -axvf ";
    }
    else if (ext1 == "tar")
    {
        errorflag = false;
        cmd = "tar xvf ";
    }
    else if (ext1 == "gz")
    {
        errorflag = false;
        cmd = "gunzip -vf ";
    }
    else if (ext1 == "bz2")
    {
        errorflag = false;
        cmd = "bunzip2 -vf ";
    }
    else if (ext1 == "zst")
    {
        errorflag = false;
        cmd = "zstd -df ";
    }
    else if (ext1 == "xz")
    {
        errorflag = false;
        cmd = "unxz -vf ";
    }
    else if (ext1 == "z")
    {
        errorflag = false;
        cmd = "uncompress -vf ";
    }
    else if (ext1 == "zip" || ext1 == "jar")
    {
        errorflag = false;
        cmd = "unzip -o ";
    }
    else if (ext1 == "iso")
    {
        errorflag = false;
        cmd = "bsdtar xvf ";
    }
    else if (ext1 == "7z")
    {
        errorflag = false;
        cmd = "7z x -y ";
    }
    else if (ext1 == "rar")
    {
        errorflag = false;
        cmd = "unrar x -o+ ";
    }
    else if (ext1 == "lzh")
    {
        errorflag = false;
        cmd = "lha -xf ";
    }
    else if (ext1 == "ace")
    {
        errorflag = false;
        cmd = "unace x ";
    }
    else if (ext1 == "arj")
    {
        errorflag = false;
        cmd = "arj x -y ";
    }
    else if (errorflag == false)
    {
        errorflag = true;
        list->clearItems();
        MessageBox::error(this, BOX_OK, _("Error"), _("Unknown archive format"));
        return 0;
    }

    FXString message = _("Extract archive to folder: ") + parentdir + "\n\n" +
                       _("Existing data will be overwritten...");
    MessageBox box(this, _("Extract Archive"), message, bigarchexticon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
    if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
    {
        return 0;
    }

    // Extract archive
    if (xf_iswritable(parentdir))
    {
        // Wait cursor
        getApp()->beginWaitCursor();

        // Final extract command
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z")
            || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
            (ext1 == "tzst"))
        {
            cmd += xf_quote(archpathname) + " --directory " + xf_quote(parentdir);
        }
        else if (ext1 == "tar")
        {
            cmd += xf_quote(archpathname) + " --directory " + xf_quote(parentdir);
        }
        else if (ext1 == "gz")
        {
            cmd += xf_quote(archpathname);
        }
        else if (ext1 == "bz2")
        {
            cmd += xf_quote(archpathname);
        }
        else if (ext1 == "zst")
        {
            cmd += xf_quote(archpathname);
        }
        else if (ext1 == "xz")
        {
            cmd += xf_quote(archpathname);
        }
        else if (ext1 == "z")
        {
            cmd += xf_quote(archpathname);
        }
        else if (ext1 == "zip" || ext1 == "jar")
        {
            cmd += xf_quote(archpathname) + " -d " + xf_quote(parentdir);
        }
        else if (ext1 == "iso")
        {
            cmd += xf_quote(archpathname) + " -d " + xf_quote(parentdir);
        }
        else if (ext1 == "7z")
        {
            cmd += xf_quote(archpathname) + " -o" + xf_quote(parentdir);
        }
        else if (ext1 == "rar")
        {
            cmd += xf_quote(archpathname) + " -op " + xf_quote(parentdir);
        }
        else if (ext1 == "lzh")
        {
            cmd += xf_quote(archpathname) + " w=" + xf_quote(parentdir);
        }
        else if (ext1 == "ace")
        {
            cmd += xf_quote(archpathname) + " " + xf_quote(parentdir);
        }
        else if (ext1 == "arj")
        {
            cmd += xf_quote(archpathname) + " " + xf_quote(parentdir);
        }

        // Make and show command window
        cmdwin = new CommandWindow(this, _("Extract Archive"), cmd, 30, 80);
        cmdwin->create();
        cmdwin->setIcon(miniarchexticon);

        // The CommandWindow object will delete itself when closed!

        getApp()->endWaitCursor();
    }
    else
    {
        MessageBox::error(getApp(), BOX_OK, _("Error"), _("Can't write to %s: Permission denied"), parentdir.text());
        return 0;
    }

    return 1;
}


// Handle single click on list item
long XFileArchive::onCmdClicked(FXObject*, FXSelector, void*)
{
    // Only for tar archives
    if ((ext2 != "tar.gz") && (ext2 != "tar.bz2") && (ext2 != "tar.xz") && (ext2 != "tar.zst") && (ext2 != "tar.z")
        && (ext1 != "tar"))
    {
        return 1;  // Avoid tree list collapse
    }

    // Single click mode
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        this->handle(this, FXSEL(SEL_COMMAND, ID_FILE_OPEN), NULL);
        return 1;
    }
    else
    {
        return 0;
    }
}


// Handle double click on list item
long XFileArchive::onCmdDoubleClicked(FXObject*, FXSelector, void*)
{
    // Only for tar archives
    if ((ext2 != "tar.gz") && (ext2 != "tar.bz2") && (ext2 != "tar.xz") && (ext2 != "tar.zst") && (ext2 != "tar.z")
        && (ext1 != "tar"))
    {
        return 1;  // Avoid tree list collapse
    }

    this->handle(this, FXSEL(SEL_COMMAND, ID_FILE_OPEN), NULL);

    return 1;
}


// Open file
long XFileArchive::onCmdFileOpen(FXObject*, FXSelector, void*)
{
    // Relative path name and file name
    FXString str = list->getCurrentItem()->getText();
    FXString relpathname = str.after(':').erase(0, 3);
    FXString filename = FXPath::name(relpathname);

    // Selected item is archive name
    if (str == FXPath::name(archpathname))
    {
        return 1;  // Avoid tree list collapse
    }

    // Selected item is a directory
    if (filename == "")
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't open a folder"));
        return 0;
    }

    // Create tmp directory if needed
    if (!xf_existfile(tmpdir))
    {
        FXbool ret = FXDir::create(tmpdir);
        if (!ret)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s"), tmpdir.text());
            return 0;
        }
    }

    // Path name of the tmp file to open
    tmppathname = tmpdir + FXPath::name(relpathname);

    // Extract command
    FXString cmd = "tar -axf " + archpathname + " " + xf_quote(relpathname) + " -O > " + xf_quote(tmppathname);

    // Wait cursor
    getApp()->beginWaitCursor();

    // Run command to extract file to tmp directory
    int ret = system(cmd.text());
    if (ret < 0)
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
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

    // If association found
    FileAssoc* association = assocdict->findFileBinding(filename.text());

    FXString cmdname;
    if (association)
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
    }
    // Or call the "Open with..." dialog
    else
    {
        getApp()->endWaitCursor();
        this->handle(this, FXSEL(SEL_COMMAND, ID_FILE_OPEN_WITH), NULL);
        return 1;
    }

    // If command exists, run it
    if (xf_existcommand(cmdname))
    {
        FXString cmd = cmdname + " " + xf_quote(tmppathname);

        FXString currentdir = FXSystem::getCurrentDirectory();

#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, currentdir, currentdir, false, "");
#else
        runcmd(cmd, currentdir, currentdir);
#endif
    }

    delete assocdict;
    getApp()->endWaitCursor();

    return 1;
}


// Open file with selected program
long XFileArchive::onCmdFileOpenWith(FXObject*, FXSelector, void*)
{
    // Relative path name and file name
    FXString str = list->getCurrentItem()->getText();
    FXString relpathname = str.after(':').erase(0, 3);
    FXString filename = FXPath::name(relpathname);

    // Selected item is archive name
    if (str == FXPath::name(archpathname))
    {
        return 1;  // Avoid tree list collapse
    }

    // Selected item is a directory
    if (filename == "")
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't open a folder"));
        return 0;
    }

    // Create tmp directory if needed
    if (!xf_existfile(tmpdir))
    {
        FXbool ret = FXDir::create(tmpdir);
        if (!ret)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't create folder %s"), tmpdir.text());
            return 0;
        }
    }

    // Path name of the tmp file to open
    tmppathname = tmpdir + FXPath::name(relpathname);

    // Extract command
    FXString cmd = "tar -axf " + archpathname + " " + xf_quote(relpathname) + " -O > " + xf_quote(tmppathname);

    // Wait cursor
    getApp()->beginWaitCursor();

    // Run command to extract file to tmp directory
    int ret = system(cmd.text());
    if (ret < 0)
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
        return 0;
    }

    FXString cmdname;
    if (opendialog == NULL)
    {
        opendialog = new HistInputDialog(this, "", _("Open selected file(s) with:"), _("Open With"), "",
                                         bigfileopenicon, false, true, HIST_INPUT_EXECUTABLE_FILE, false);
    }
    opendialog->setText(cmd);
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
        cmdname = opendialog->getText();
        if (cmdname == "")
        {
            getApp()->endWaitCursor();
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }

        cmd = cmdname + " " + xf_quote(tmppathname);

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            FXString currentdir = FXSystem::getCurrentDirectory();

#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, currentdir, currentdir, false, "");
#else
            runcmd(cmd, currentdir, currentdir);
#endif
        }
        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            this->handle(this, FXSEL(SEL_COMMAND, ID_FILE_OPEN_WITH), NULL);
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
    }

    getApp()->endWaitCursor();

    // If list has been cleared, set history size to zero
    if (opendialog->getHistorySize() == 0)
    {
        OpenNum = 0;
    }

    return 1;
}


// Extract file to archive parent directory
long XFileArchive::onCmdFileExtract(FXObject*, FXSelector, void*)
{
    // Relative path name and file name
    FXString str = list->getCurrentItem()->getText();
    FXString relpathname = str.after(':').erase(0, 3);
    FXString filename = FXPath::name(relpathname);

    // Selected item is archive name
    if (str == FXPath::name(archpathname))
    {
        return 1;  // Avoid tree list collapse
    }

    // Selected item is a directory
    if (filename == "")
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't extract a folder"));
        return 0;
    }

    // Parent directory
    FXString parentdir = FXPath::directory(archpathname);

    if (xf_iswritable(parentdir))
    {
        FXString message = _("Extract file") + FXString(" ") + filename + " " + _("to folder:") +
                           " " + parentdir + "\n\n" + _("Existing data will be overwritten...");
        MessageBox box(this, _("Extract File"), message, bigarchexticon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return 0;
        }

        // Extract command
        FXString cmd = "tar -axf " + archpathname + " " + xf_quote(relpathname) + " -O > " +
                       xf_quote(parentdir + PATHSEPSTRING + filename);

        // Run command to extract file to parent directory
        getApp()->beginWaitCursor();

        int ret = system(cmd.text());
        if (ret < 0)
        {
            getApp()->endWaitCursor();
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
            return 0;
        }

        getApp()->endWaitCursor();
    }
    else
    {
        MessageBox::error(getApp(), BOX_OK, _("Error"), _("Can't write to %s: Permission denied"), parentdir.text());
    }

    return 1;
}


// Extract file to selected directory
long XFileArchive::onCmdFileExtractTo(FXObject*, FXSelector, void*)
{
    // Relative path name and file name
    FXString str = list->getCurrentItem()->getText();
    FXString relpathname = str.after(':').erase(0, 3);
    FXString filename = FXPath::name(relpathname);

    // Selected item is archive name
    if (str == FXPath::name(archpathname))
    {
        return 1;  // Avoid tree list collapse
    }

    // Selected item is a directory
    if (filename == "")
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't extract a folder"));
        return 0;
    }

    // File selection dialog
    FileDialog browse(this, _("Select a Destination Folder"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };
    browse.setDirectory(homedir);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_DIRECTORY);

    if (browse.execute())
    {
        FXString dir = browse.getFilename();

        if (xf_iswritable(dir))
        {
            FXString message = _("Extract file") + FXString(" ") + filename + " " + _("to folder:") +
                               " " + dir + "\n\n" + _("Existing data will be overwritten...");
            MessageBox box(this, _("Extract File"), message, bigarchexticon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return 0;
            }

            // Extract command
            FXString cmd = "tar -axf " + archpathname + " " + xf_quote(relpathname) + " -O > " +
                           xf_quote(dir + PATHSEPSTRING + filename);

            // Run command to extract file to directory
            getApp()->beginWaitCursor();

            int ret = system(cmd.text());
            if (ret < 0)
            {
                getApp()->beginWaitCursor();
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), cmd.text());
                return 0;
            }

            getApp()->endWaitCursor();
        }
        else
        {
            MessageBox::error(getApp(), BOX_OK, _("Error"), _("Can't write to %s: Permission denied"), dir.text());
        }
    }

    return 1;
}


// Save configuration when quitting
void XFileArchive::saveConfig()
{
    // Write new window size and position back to registry
    getApp()->reg().writeUnsignedEntry("OPTIONS", "width", (FXuint)getWidth());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "height", (FXuint)getHeight());

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

    // Last opened archive
    getApp()->reg().writeStringEntry("OPTIONS", "archpathname", archpathname.text());

    // Toolbar status
    if (toolbar->shown())
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "showtoolbar", true);
    }
    else
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "showtoolbar", false);
    }

    // Write open history
    FXString history = "";
    for (int i = 0; i < OpenNum; i++)
    {
        history += OpenHistory[i];
        history += ":";
    }
    getApp()->reg().writeStringEntry("HISTORY", "open", history.text());

    // Write filter history
    history = "";
    for (int i = 0; i < FilterNum; i++)
    {
        history += FilterHistory[i];
        history += ":";
    }
    if (FilterNum)
    {
        getApp()->reg().writeStringEntry("HISTORY", "filter", history.text());
    }

    // Write registry options
    getApp()->reg().write();

    // Clean up tmp files
    if (xf_existfile(tmpdir))
    {
        FXString cmd = "rm -rf " + tmpdir;

        int ret = system(cmd.text());
        if (ret < 0)
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Can't remove folder %s"), tmpdir.text());
        }
    }
}


// Harvest the zombies
long XFileArchive::onSigHarvest(FXObject*, FXSelector, void*)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
    return 1;
}


// Quit application
long XFileArchive::onCmdQuit(FXObject*, FXSelector, void*)
{
    // Confirm quit
    if (opening || (cmdwin != NULL && cmdwin->shown()))
    {
        MessageBox box(this, _("Confirm Quit"), _("An operation in in progress, do you really want to quit Xfa?"),
                       bigquestionicon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_OWNER) == BOX_CLICKED_CANCEL)
        {
            return 0;
        }
    }

    // Save options
    saveConfig();

    // Exit
    getApp()->exit(EXIT_SUCCESS);
    return 1;
}


// Read archive and display its contents
int XFileArchive::readArchive(void)
{
    FXString cmd;

    // Get extensions of the archive path
    ext1 = archpathname.rafter('.', 1).lower();
    ext2 = archpathname.rafter('.', 2).lower();

    if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z")
        || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
        (ext1 == "tzst"))
    {
        if (!xf_existcommand("tar"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "tar");
            return 0;
        }

        errorflag = false;
        cmd = "tar -atvf ";
    }
    else if (ext1 == "tar")
    {
        if (!xf_existcommand("tar"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "tar");
            return 0;
        }

        errorflag = false;
        cmd = "tar -tvf ";
    }
    else if (ext1 == "gz")
    {
        if (!xf_existcommand("gunzip"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "gunzip");
            return 0;
        }

        errorflag = false;
        cmd = "gunzip -l ";
    }
    else if (ext1 == "bz2")
    {
        if (!xf_existcommand("bunzip2"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "bunzip2");
            return 0;
        }

        errorflag = false;
        cmd = "bunzip2 -tv ";
    }
    else if (ext1 == "zst")
    {
        if (!xf_existcommand("zstd"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "zstd");
            return 0;
        }

        errorflag = false;
        cmd = "zstd -t ";
    }
    else if (ext1 == "xz")
    {
        if (!xf_existcommand("unxz"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "unxz");
            return 0;
        }

        errorflag = false;
        cmd = "unxz -l ";
    }
    else if (ext1 == "z")
    {
        if (!xf_existcommand("uncompress"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "uncompress");
            return 0;
        }

        errorflag = false;
        cmd = "uncompress -l ";
    }
    else if (ext1 == "zip" || ext1 == "jar")
    {
        if (!xf_existcommand("unzip"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "unzip");
            return 0;
        }

        errorflag = false;
        cmd = "unzip -l ";
    }
    else if (ext1 == "iso")
    {
        if (!xf_existcommand("bsdtar"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "bsdtar");
            return 0;
        }

        errorflag = false;
        cmd = "bsdtar -tvf ";
    }
    else if (ext1 == "7z")
    {
        if (!xf_existcommand("7z"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "7z");
            return 0;
        }

        errorflag = false;
        cmd = "7z l ";
    }
    else if (ext1 == "rar")
    {
        if (!xf_existcommand("unrar"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "unrar");
            return 0;
        }

        errorflag = false;
        cmd = "unrar l ";
    }
    else if (ext1 == "lzh")
    {
        if (!xf_existcommand("lha"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "lha");
            return 0;
        }

        errorflag = false;
        cmd = "lha l ";
    }
    else if (ext1 == "ace")
    {
        if (!xf_existcommand("unace"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "unace");
            return 0;
        }

        errorflag = false;
        cmd = "unace l ";
    }
    else if (ext1 == "arj")
    {
        if (!xf_existcommand("arj"))
        {
            list->clearItems();
            MessageBox::error(this, BOX_OK, _("Error"), _("Unarchive program %s was not found"), "arj");
            return 0;
        }

        errorflag = false;
        cmd = "arj l ";
    }
    else if (errorflag == false)
    {
        errorflag = true;
        list->clearItems();
        MessageBox::error(this, BOX_OK, _("Error"), _("Unknown archive format"));
        return 0;
    }

    cmd += xf_quote(archpathname);

    // Clear list
    list->clearItems();

    // Set cursor
    getApp()->beginWaitCursor();

    // Set opening flag
    opening = true;

    // First item is archive name
    topmost = list->prependItem(NULL, FXPath::name(archpathname), minifoldericon, minifolderopenicon);
    list->expandTree(topmost);

    // Execute command
    execCmd(cmd);

    return 1;
}


// Execute a command and capture its output
int XFileArchive::execCmd(FXString command)
{
    // Initialize line fragment and status cursor
    linefrag = "";
    rotcur = "";

    // Open pipes to communicate with child process
    if (pipe(pipes) == -1)
    {
        return -1;
    }

    // Create child process
    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));

        getApp()->endWaitCursor();
        status->setText("");

        return -1;
    }
    if (pid == 0) // Child
    {
        char* args[4];
        int ret1 = dup2(pipes[0], STDIN_FILENO);        // Use the pipes as the new channels
        int ret2 = dup2(pipes[1], STDOUT_FILENO);       // (where stdout and stderr
        int ret3 = dup2(pipes[1], STDERR_FILENO);       // go to the same pipe!).

        if ((ret1 < 0) || (ret2 < 0) || (ret3 < 0))
        {
            int errcode = errno;
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't duplicate pipes: %s"), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't duplicate pipes"));
            }

            return -1;
        }

        args[0] = (char*)"sh";                  // Setup arguments
        args[1] = (char*)"-c";                  // to run command
        args[2] = (char*)command.text();        // in a shell in
        args[3] = NULL;                         // a new process.
        setpgid(0, 0);                          // Allows to kill the whole group
        execvp(args[0], args);                  // Start a new process which will execute the command.
        _exit(EXIT_FAILURE);                    // We'll get here only if an error occurred.
    }
    else // Parent
    {
        // Make sure we get called so we can check when child has finished
        getApp()->addChore(this, ID_WATCHPROCESS);
    }
    return 0;
}


// Watch progress of child process
long XFileArchive::onWatchProcess(FXObject*, FXSelector, void*)
{
    char buf[1048576];
    int nread;

    if ((waitpid(pid, NULL, WNOHANG) == 0))
    {
        // Child is still running, just wait
        getApp()->addChore(this, ID_WATCHPROCESS);

        // Read data from the running child (first, set I-O to non-blocking)
        int pflags;
        if ((pflags = fcntl(pipes[0], F_GETFL)) >= 0)
        {
            pflags |= O_NONBLOCK;
            if (fcntl(pipes[0], F_SETFL, pflags) >= 0)
            {
                FXString line;

                // Now read the data from the pipe
                while ((nread = read(pipes[0], buf, sizeof(buf) - 1)) > 0)
                {
                    buf[nread] = '\0';

                    // Add previous line fragment
                    line = linefrag + buf;

                    // Number of lines
                    FXuint nlines = line.contains('\n');

                    if (nlines > 0)
                    {
                        for (FXuint i = 0; i < nlines; i++)
                        {
                            FXString text = line.section('\n', i);

                            // With icon
                            if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") ||
                                (ext2 == "tar.zst") || (ext2 == "tar.z")
                                || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") ||
                                (ext1 == "taz")
                                || (ext1 == "tzst") || (ext1 == "iso"))
                            {
                                list->appendItem(topmost, text, minidocicon, minidocicon);
                            }
                            else
                            {
                                // Without icon
                                list->appendItem(topmost, text, NULL, NULL);
                            }

                            // Update status rotating cursor
                            if (rotcur == "—") // Em dash
                            {
                                rotcur = "\\";
                            }
                            else if (rotcur == "\\")
                            {
                                rotcur = "|";
                            }
                            else if (rotcur == "|")
                            {
                                rotcur = "/";
                            }
                            else
                            {
                                rotcur = "—"; // Em dash
                            }
                            status->setText(FXString(_("Please wait while opening archive...")) + " " + rotcur);
                        }

                        // Last line fragment
                        linefrag = line.after('\n', nlines);
                    }

                    if (nread < (int)(sizeof(buf) - 1))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        // Child has finished.
        // Read data from the finished child
        int pflags;
        if ((pflags = fcntl(pipes[0], F_GETFL)) >= 0)
        {
            pflags |= O_NONBLOCK;
            if (fcntl(pipes[0], F_SETFL, pflags) >= 0)
            {
                while ((nread = read(pipes[0], buf, sizeof(buf) - 1)) > 0)
                {
                    buf[nread] = '\0';

                    // Add previous line fragment
                    FXString line = linefrag + buf;

                    // Number of lines
                    FXuint nlines = line.contains('\n');

                    if (nlines > 0)
                    {
                        for (FXuint i = 0; i < nlines; i++)
                        {
                            FXString text = line.section('\n', i);

                            // With icon
                            if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") ||
                                (ext2 == "tar.zst") || (ext2 == "tar.z")
                                || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") ||
                                (ext1 == "taz")
                                || (ext1 == "tzst") || (ext1 == "iso"))
                            {
                                list->appendItem(topmost, text, minidocicon, minidocicon);
                            }
                            else
                            {
                                // Without icon
                                list->appendItem(topmost, text, NULL, NULL);
                            }

                            // Update status rotating cursor
                            if (rotcur == "—") // Em dash
                            {
                                rotcur = "\\";
                            }
                            else if (rotcur == "\\")
                            {
                                rotcur = "|";
                            }
                            else if (rotcur == "|")
                            {
                                rotcur = "/";
                            }
                            else
                            {
                                rotcur = "—"; // Em dash
                            }
                            status->setText(FXString(_("Please wait while opening archive...")) + " " + rotcur);
                        }

                        // Last line fragment
                        linefrag = line.after('\n', nlines);
                    }

                    if (nread < (int)(sizeof(buf) - 1))
                    {
                        break;
                    }
                }
            }
        }

        // Close pipes
        ::close(pipes[0]);
        ::close(pipes[1]);

        // Reset cursor and status and flag
        getApp()->endWaitCursor();
        status->setText("");
        opening = false;
    }

    return 1;
}


// Context menu
long XFileArchive::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // Popup menu only for tar archives
    if ((ext2 != "tar.gz") && (ext2 != "tar.bz2") && (ext2 != "tar.xz") && (ext2 != "tar.zst") && (ext2 != "tar.z")
        && (ext1 != "tar"))
    {
        return 0;
    }

    int x, y;
    FXuint state;

    // Item under cursor
    list->getCursorPosition(x, y, state);
    FXTreeItem* item = list->getItemAt(x, y);

    if (item != NULL)
    {
        if (item->getText() != FXPath::name(archpathname))
        {
            // Make item current
            list->setCurrentItem(item);
            list->selectItem(item);

            // Menu items
            FXMenuPane* menu = new FXMenuPane(this);

            getRoot()->getCursorPosition(x, y, state);

            new FXMenuCommand(menu, _("Open &With..."), minifileopenicon, this, XFileArchive::ID_FILE_OPEN_WITH);
            new FXMenuCommand(menu, _("&Open"), minifileopenicon, this, XFileArchive::ID_FILE_OPEN);
            new FXMenuCommand(menu, _("&Extract"), miniarchexticon, this, XFileArchive::ID_FILE_EXTRACT);
            new FXMenuCommand(menu, _("&Extract to..."), miniarchexticon, this, XFileArchive::ID_FILE_EXTRACT_TO);

            menu->create();
            menu->popup(NULL, x, y);
            getApp()->runModalWhileShown(menu);

            delete(menu);
        }
    }
    else
    {
        list->killSelection();
    }

    return 1;
}


// Display bookmark location tooltip
long XFileArchive::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    list->getCursorPosition(x, y, state);
    FXTreeItem* item = list->getItemAt(x, y);

    if (item != NULL)
    {
        FXString tip = "";  // No tooltip
        sender->handle(list, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}


// Update open menu and toolbar button
long XFileArchive::onUpdOpen(FXObject* sender, FXSelector, void*)
{
    if (opening)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    if (cmdwin != NULL)
    {
        if (cmdwin->shown())
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


// Update extract button
long XFileArchive::onUpdExtract(FXObject* sender, FXSelector, void*)
{
    if ((archpathname == "") || opening)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    if (cmdwin != NULL)
    {
        if (cmdwin->shown())
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
            status->setText(_("Please wait while extracting archive..."));
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
            if (!opening)
            {
                status->setText("");
                cmdwin = NULL;
            }
        }
    }

    return 1;
}

// Update extract to button
long XFileArchive::onUpdExtractTo(FXObject* sender, FXSelector, void*)
{
    if ((ext2 != "tar.gz") && (ext2 != "tar.bz2") && (ext2 != "tar.xz") && (ext2 != "tar.zst") && (ext2 != "tar.z"))
    {
        if (ext1 == "gz" || ext1 == "bz2" || ext1 == "xz" || ext1 == "zst" || ext1 == "z")
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
            return 1;
        }
    }

    if ((archpathname == "") || opening)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    if (cmdwin != NULL)
    {
        if (cmdwin->shown())
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
            status->setText(_("Please wait while extracting archive..."));
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
            if (!opening)
            {
                status->setText("");
                cmdwin = NULL;
            }
        }
    }

    return 1;
}


// Update close button
long XFileArchive::onUpdQuit(FXObject* sender, FXSelector, void*)
{
    if (opening)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    if (cmdwin != NULL)
    {
        if (cmdwin->shown())
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            if (!opening)
            {
                sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
            }
        }
    }

    return 1;
}


// Start application
void XFileArchive::start(FXString startarch)
{
    archpathname = startarch;

    if (archpathname != "")
    {
        FILE* fp = fopen(archpathname.text(), "r");
        if (!fp)
        {
            MessageBox::error(this, BOX_OK, _("Error Loading File"), _("Unable to open file: %s"), archpathname.text());
            archpathname = "";
        }
        else
        {
            fclose(fp);
            readArchive();
        }
    }
    else
    {
        archpathname = "";
    }
}


void XFileArchive::create()
{
    // Read the Xfe registry
    FXRegistry* reg_xfe = new FXRegistry(XFEAPPNAME, "");

    reg_xfe->read();

    // Set Xfa text font according to the Xfe registry
    FXString fontspec;
    fontspec = reg_xfe->readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    if (!fontspec.empty())
    {
        FXFont* font = new FXFont(getApp(), fontspec);
        font->create();
        list->setFont(font);
    }
    delete reg_xfe;

    // Get toolbar status
    if (getApp()->reg().readUnsignedEntry("OPTIONS", "showtoolbar", true) == false)
    {
        toolbar->hide();
    }

    // Get size
    FXuint width = getApp()->reg().readUnsignedEntry("OPTIONS", "width", DEFAULT_WINDOW_WIDTH);
    FXuint height = getApp()->reg().readUnsignedEntry("OPTIONS", "height", DEFAULT_WINDOW_HEIGHT);

    // Get position and position window
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

    // Read open history
    FXString history = getApp()->reg().readStringEntry("HISTORY", "open", "");
    FXString histent = "";
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

    // Read filter history
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

    FXMainWindow::create();

    minifoldericon->create();
    minifolderopenicon->create();
    minidocicon->create();

    // Show window
    show();

#ifdef STARTUP_NOTIFICATION
    startup_completed();
#endif
}


// Usage message
#define USAGE_MSG    _( \
            "\
\nUsage: xfa [options] [archive] \n\
\n\
    [options] can be any of the following:\n\
\n\
        -h, --help         Print (this) help screen and exit.\n\
        -v, --version      Print version information and exit.\n\
\n\
    [archive] is the path to the archive you want to open on start up.\n\
\n")



// Main function to start the program
int main(int argc, char* argv[])
{
    int i;
    FXString startarch = "";
    const char* appname = "xfa";
    const char* xfename = XFEAPPNAME;
    const char* vdrname = XFEVDRNAME;
    FXbool loadicons;
    FXString xmodifiers;

    // Get environment variables $HOME, $XDG_DATA_HOME and $XDG_CONFIG_HOME
    homedir = FXSystem::getHomeDirectory();
    if (homedir == "")
    {
        homedir = ROOTDIR;
    }
    xdgdatahome = getenv("XDG_DATA_HOME");
    if (xdgdatahome == "")
    {
        xdgdatahome = homedir + PATHSEPSTRING DATAPATH;
    }
    xdgconfighome = getenv("XDG_CONFIG_HOME");
    if (xdgconfighome == "")
    {
        xdgconfighome = homedir + PATHSEPSTRING CONFIGPATH;
    }

#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL.
    setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
    // Set the text message domain.
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
#endif

    // Parse basic arguments
    for (i = 1; i < argc; ++i)
    {
        if ((compare(argv[i], "-v") == 0) || (compare(argv[i], "--version") == 0))
        {
            fprintf(stdout, "%s version %s\n", PACKAGE, VERSION);
            exit(EXIT_SUCCESS);
        }
        else if ((compare(argv[i], "-h") == 0) || (compare(argv[i], "--help") == 0))
        {
            fprintf(stdout, USAGE_MSG);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Start archive, if any
            startarch = argv[i];
        }
    }

    // Global variable
    args = argv;

    // Make application
    FXApp* application = new FXApp(appname, vdrname);

    // Open display
    application->init(argc, argv);

    // Read the Xfe registry
    FXRegistry* reg_xfe;
    reg_xfe = new FXRegistry(xfename, vdrname);
    reg_xfe->read();

    // Compute integer and fractional scaling factors depending on the monitor resolution
    FXint res = reg_xfe->readUnsignedEntry("SETTINGS", "screenres", 100);
    scaleint = round(res / 100.0);

    // Redefine the default hand cursor depending on the integer scaling factor
    FXCursor* hand;
    if (scaleint == 1)
    {
        hand = new FXCursor(application, hand1_bits, hand1_mask_bits, hand1_width, hand1_height, hand1_x_hot,
                            hand1_y_hot);
    }
    else if (scaleint == 2)
    {
        hand = new FXCursor(application, hand2_bits, hand2_mask_bits, hand2_width, hand2_height, hand2_x_hot,
                            hand2_y_hot);
    }
    else
    {
        hand = new FXCursor(application, hand3_bits, hand3_mask_bits, hand3_width, hand3_height, hand3_x_hot,
                            hand3_y_hot);
    }
    application->setDefaultCursor(DEF_HAND_CURSOR, hand);

    // Load all application icons
    FXuint iconpathstatus;
    execpath = xf_execpath(argv[0]);
    loadicons = loadAppIcons(application, &iconpathstatus);

    // Set base and border colors (to change the default colors at first run)
    FXColor basecolor = reg_xfe->readColorEntry("SETTINGS", "basecolor", FXRGB(237, 236, 235));
    FXColor bordercolor = reg_xfe->readColorEntry("SETTINGS", "bordercolor", FXRGB(125, 125, 125));
    application->setBaseColor(basecolor);
    application->setBorderColor(bordercolor);

    // Set Xfa normal font according to the Xfe registry
    FXString fontspec;
    fontspec = reg_xfe->readStringEntry("SETTINGS", "font", DEFAULT_NORMAL_FONT);
    if (!fontspec.empty())
    {
        FXFont* normalFont = new FXFont(application, fontspec);
        normalFont->create();
        application->setNormalFont(normalFont);
    }

    // Set single click navigation according to the Xfe registry
    FXuint single_click = reg_xfe->readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    application->reg().writeUnsignedEntry("SETTINGS", "single_click", single_click);

    // Set smooth scrolling according to the Xfe registry
    FXbool smoothscroll = reg_xfe->readUnsignedEntry("SETTINGS", "smooth_scroll", true);

    // Set file list tooltip flag according to the Xfe registry
    FXbool file_tooltips = reg_xfe->readUnsignedEntry("SETTINGS", "file_tooltips", true);
    application->reg().writeUnsignedEntry("SETTINGS", "file_tooltips", file_tooltips);

    // Set relative resizing flag according to the Xfe registry
    FXbool relative_resize = reg_xfe->readUnsignedEntry("SETTINGS", "relative_resize", true);
    application->reg().writeUnsignedEntry("SETTINGS", "relative_resize", relative_resize);

    // Get value of the window position flag
    FXbool save_win_pos = reg_xfe->readUnsignedEntry("SETTINGS", "save_win_pos", false);
    application->reg().writeUnsignedEntry("SETTINGS", "save_win_pos", save_win_pos);

    // Delete the Xfe registry
    delete reg_xfe;

    // Make window
    XFileArchive* window = new XFileArchive(application);

    // Catch SIGCHLD to harvest zombie child processes
    application->addSignal(SIGCHLD, window, XFileArchive::ID_HARVEST, true);

    // Smooth scrolling
    window->setSmoothScroll(smoothscroll);

    // Create it
    application->create();

    // Icon path doesn't exist
    if (iconpathstatus == ICONPATH_NOT_FOUND)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Icon path doesn't exist, default icon path was selected.\
\n\nFrom Xfe, please check your icon path in Edit / Preferences / Appearance..."));
    }

    // Some icons not found
    if (!loadicons && iconpathstatus == ICONPATH_MISSING_ICONS)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Unable to load some icons, default icon theme was selected.\
\n\nFrom Xfe, please check your icon theme in Edit / Preferences / Appearance..."));       
    }

    // Default icon path doesn't exist
    if (iconpathstatus == DEFAULTICONPATH_NOT_FOUND)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Unable to load default icons, no icons can be shown.\
\n\nPlease check your Xfe installation..."));       
    }

    // Tooltips setup time and duration
    application->setTooltipPause(TOOLTIP_PAUSE);
    application->setTooltipTime(TOOLTIP_TIME);

    // Start
    window->start(startarch);

    // Run the application
    return application->run();
}
