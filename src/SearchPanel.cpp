// Search panel
#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>


#include <FXPNGIcon.h>
#include <FXJPGIcon.h>
#include <FXTIFIcon.h>


#include "xfedefs.h"
#include "startupnotification.h"
#include "icons.h"
#include "File.h"
#include "FileDict.h"
#include "FileDialog.h"
#include "FileList.h"
#include "MessageBox.h"
#include "ArchInputDialog.h"
#include "HistInputDialog.h"
#include "BrowseInputDialog.h"
#include "RenameDialog.h"
#include "OverwriteBox.h"
#include "CommandWindow.h"
#include "ExecuteBox.h"
#include "XFileExplorer.h"
#include "SearchPanel.h"


// Global Variables
extern FXMainWindow* mainWindow;
extern FXString homedir;
extern FXString xdgdatahome;

// Clipboard
extern FXString clipboard;
extern FXuint clipboard_type;

// Open history
extern char OpenHistory[OPEN_HIST_SIZE][MAX_COMMAND_SIZE];
extern int OpenNum;

#if defined(linux)
extern FXStringDict* fsdevices; // Devices from fstab
#endif


// Button separator margins and height
#define SEP_SPACE     5
#define SEP_HEIGHT    20



// Map
FXDEFMAP(SearchPanel) SearchPanelMap[] =
{
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, SearchPanel::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, SearchPanel::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, SearchPanel::onClipboardRequest),
    FXMAPFUNC(SEL_CLICKED, SearchPanel::ID_FILELIST, SearchPanel::onCmdItemClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, SearchPanel::ID_FILELIST, SearchPanel::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, SearchPanel::ID_KEY_RETURN, SearchPanel::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_KEYPRESS, 0, SearchPanel::onKeyPress),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_GOTO_PARENTDIR, SearchPanel::onCmdGotoParentdir),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_OPEN_WITH, SearchPanel::onCmdOpenWith),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_OPEN, SearchPanel::onCmdOpen),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_VIEW, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_EDIT, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_COMPARE, SearchPanel::onCmdCompare),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_REFRESH, SearchPanel::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_PROPERTIES, SearchPanel::onCmdProperties),
    FXMAPFUNC(SEL_MIDDLEBUTTONPRESS, SearchPanel::ID_FILELIST, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_SELECT_ALL, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_DESELECT_ALL, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_SELECT_INVERSE, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, SearchPanel::ID_FILELIST, SearchPanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_POPUP_MENU, SearchPanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_COPY_CLIPBOARD, SearchPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_CUT_CLIPBOARD, SearchPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_COPYNAME_CLIPBOARD, SearchPanel::onCmdCopyName),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_COPYTO, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_MOVETO, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_RENAME, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_SYMLINK, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_ADD_TO_ARCH, SearchPanel::onCmdAddToArch),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_EXTRACT, SearchPanel::onCmdExtract),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_TRASH, SearchPanel::onCmdFileTrash),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_DELETE, SearchPanel::onCmdFileDelete),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_GO_SCRIPTDIR, SearchPanel::onCmdGoScriptDir),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_DIR_USAGE, SearchPanel::onCmdDirUsage),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_STATUS, SearchPanel::onUpdStatus),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_GOTO_PARENTDIR, SearchPanel::onUpdSelMult),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_COMPARE, SearchPanel::onUpdCompare),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_COPY_CLIPBOARD, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_CUT_CLIPBOARD, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_PROPERTIES, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_FILE_DELETE, SearchPanel::onUpdFileDelete),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_FILE_TRASH, SearchPanel::onUpdFileTrash),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_DIR_USAGE, SearchPanel::onUpdDirUsage),
#if defined(linux)
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_PKG_QUERY, SearchPanel::onCmdPkgQuery),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_PKG_QUERY, SearchPanel::onUpdPkgQuery),
#endif
};


// Object implementation
FXIMPLEMENT(SearchPanel, FXVerticalFrame, SearchPanelMap, ARRAYNUMBER(SearchPanelMap))



// Contruct
SearchPanel::SearchPanel(FXComposite* p, FXuint* ic, FXuint nc, FXuint name_size, FXuint dir_size, FXuint size_size,
                         FXuint type_size, FXuint ext_size, FXuint date_size, FXuint user_size, FXuint group_size,
                         FXuint perms_size, FXuint link_size, FXColor listbackcolor, FXColor listforecolor,
                         FXuint opts, int x, int y, int w, int h) :
    FXVerticalFrame(p, opts, x, y, w, h, 0, 0, 0, 0)
{
    // Columns id and number
    nbCols = nc;
    for (FXuint i = 0; i < nbCols; i++)
    {
        idCol[i] = ic[i];
    }

    // Global container
    FXVerticalFrame* cont = new FXVerticalFrame(this, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE,
                                                0, 0, 0, 0, 0, 0, 0, 0, 1, 1);

    // Container for the action toolbar
    FXHorizontalFrame* toolbar = new FXHorizontalFrame(cont, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0,
                                                       DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING,
                                                       DEFAULT_SPACING, 0, 0);

    // File list
    FXVerticalFrame* cont2 = new FXVerticalFrame(cont, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE,
                                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    list = new FileList(this, cont2, idCol, nbCols, this, ID_FILELIST, 0, 0, 0,
                        LAYOUT_FILL_X | LAYOUT_FILL_Y | ICONLIST_DETAILED | FILELIST_SEARCH);
    list->setTextColor(listforecolor);
    list->setBackColor(listbackcolor);

    // Set list headers size
    for (FXuint i = 0; i < nbCols; i++)
    {
        FXuint size = 0;
        FXuint id = list->getHeaderId(i);

        switch (id)
        {
        case FileList::ID_COL_NAME:
            size = name_size;
            break;

        case FileList::ID_COL_DIRNAME:
            size = dir_size;
            break;

        case FileList::ID_COL_SIZE:
            size = size_size;
            break;

        case FileList::ID_COL_TYPE:
            size = type_size;
            break;

        case FileList::ID_COL_EXT:
            size = ext_size;
            break;

        case FileList::ID_COL_DATE:
            size = date_size;
            break;

        case FileList::ID_COL_USER:
            size = user_size;
            break;

        case FileList::ID_COL_GROUP:
            size = group_size;
            break;

        case FileList::ID_COL_PERMS:
            size = perms_size;
            break;

        case FileList::ID_COL_LINK:
            size = link_size;
            break;
        }

        list->setHeaderSize(i, size);
    }

    // Set list style
    FXuint liststyle = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "liststyle",
                                                         ICONLIST_DETAILED | ICONLIST_AUTOSIZE);
    list->setListStyle(liststyle);

    // Set dirs first
    FXuint dirsfirst = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "dirs_first", 1);
    list->setDirsFirst(dirsfirst);

    // Set ignore case
    FXuint ignorecase = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "ignore_case", 1);
    list->setIgnoreCase(ignorecase);

    // Toolbar buttons
    FXHotKey hotkey;
    FXString key;

    // Refresh panel toolbar button
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    refreshbtn = new FXButton(toolbar, TAB + _("Refresh Panel") + PARS(key), minireloadicon, this,
                              SearchPanel::ID_REFRESH,
                              BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    refreshbtn->addHotKey(hotkey);

    // Goto dir toolbar button
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    gotodirbtn = new FXButton(toolbar, TAB + _("Go to Parent Folder") + PARS(key), minigotodiricon, this,
                              SearchPanel::ID_GOTO_PARENTDIR,
                              BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    gotodirbtn->addHotKey(hotkey);

    // Copy / cut / properties toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    copybtn = new FXButton(toolbar, TAB + _("Copy Selected Files to Clipboard") + PARS(key), minicopyicon, this,
                           SearchPanel::ID_COPY_CLIPBOARD,
                           BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    copybtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    cutbtn = new FXButton(toolbar, TAB + _("Cut Selected Files to Clipboard") + PARS(key), minicuticon, this,
                          SearchPanel::ID_CUT_CLIPBOARD,
                          BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    cutbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "properties", "F9");
    propbtn = new FXButton(toolbar, TAB + _("Show Properties of Selected Files") + PARS(key), miniattribicon, this,
                           SearchPanel::ID_PROPERTIES,
                           BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    propbtn->addHotKey(hotkey);

    // This button is not shown, but necessary to get the shortcut work
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_names", "Ctrl-Shift-N");
    copynamebtn = new FXButton(toolbar, TAB + _("Cop&y Names") + PARS(key), minicopyicon, this,
                               SearchPanel::ID_COPYNAME_CLIPBOARD,
                               BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    copynamebtn->addHotKey(hotkey);
    copynamebtn->hide();

    // Separator
    new FXFrame(toolbar, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
    new FXVerticalSeparator(toolbar, LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | SEPARATOR_GROOVE | LAYOUT_FIX_HEIGHT, 0, 0, 0,
                            SEP_HEIGHT);
    new FXFrame(toolbar, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);

    // Move to trash / delete toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    trashbtn = new FXButton(toolbar, TAB + _("Move Selected Files to Trash Can") + PARS(key), minideleteicon, this,
                            SearchPanel::ID_FILE_TRASH,
                            BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    trashbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    delbtn = new FXButton(toolbar, TAB + _("Delete Selected Files") + PARS(key), minideletepermicon, this,
                          SearchPanel::ID_FILE_DELETE,
                          BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    delbtn->addHotKey(hotkey);

    // Separator
    new FXFrame(toolbar, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
    new FXVerticalSeparator(toolbar, LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | SEPARATOR_GROOVE | LAYOUT_FIX_HEIGHT, 0, 0, 0,
                            SEP_HEIGHT);
    new FXFrame(toolbar, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);

    // Icon view toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    bigiconsbtn = new FXButton(toolbar, TAB + _("Big Icon List") + PARS(key), minibigiconsicon, list,
                               IconList::ID_SHOW_BIG_ICONS,
                               BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    bigiconsbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    smalliconsbtn = new FXButton(toolbar, TAB + _("Small Icon List") + PARS(key), minismalliconsicon, list,
                                 IconList::ID_SHOW_MINI_ICONS,
                                 BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    smalliconsbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    detailsbtn = new FXButton(toolbar, TAB + _("Detailed File List") + PARS(key), minidetailsicon, list,
                              IconList::ID_SHOW_DETAILS,
                              BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    detailsbtn->addHotKey(hotkey);

    // Status bar
    statusbar = new FXHorizontalFrame(cont, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 3, 3,
                                      3, 3);
    statusbar->setTarget(this);
    statusbar->setSelector(FXSEL(SEL_UPDATE, SearchPanel::ID_STATUS));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    thumbbtn = new FXToggleButton(statusbar, TAB + _("Show Thumbnails") + PARS(key),
                                  TAB + _("Hide Thumbnails") + PARS(key), minishowthumbicon, minihidethumbicon,
                                  this->list,
                                  FileList::ID_TOGGLE_THUMBNAILS,
                                  TOGGLEBUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    thumbbtn->addHotKey(hotkey);

    new FXHorizontalFrame(statusbar, LAYOUT_LEFT | JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
    statuslabel = new FXLabel(statusbar, _("Status"), NULL, JUSTIFY_LEFT | LAYOUT_LEFT | LAYOUT_FILL_X);

    corner = new FXDragCorner(statusbar);

    // File associations
    associations = NULL;
    associations = new FileDict(getApp());

    // Trashcan locations
    trashlocation = xdgdatahome + PATHSEPSTRING TRASHPATH;
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;

    // Default programs identifiers
    progs["<txtviewer>"] = TXTVIEWER;
    progs["<txteditor>"] = TXTEDITOR;
    progs["<imgviewer>"] = IMGVIEWER;
    progs["<imgeditor>"] = IMGEDITOR;
    progs["<pdfviewer>"] = PDFVIEWER;
    progs["<audioplayer>"] = AUDIOPLAYER;
    progs["<videoplayer>"] = VIDEOPLAYER;
    progs["<archiver>"] = ARCHIVER;

    // Class variable initializations
    ctrlflag = false;
    shiftf10 = false;
}


// Create window
void SearchPanel::create()
{
    // Register standard uri-list type
    urilistType = getApp()->registerDragType("text/uri-list");

    // Register special uri-list type used for Gnome, XFCE and Xfe
    xfelistType = getApp()->registerDragType("x-special/gnome-copied-files");

    // Register special uri-list type used for KDE
    kdelistType = getApp()->registerDragType("application/x-kde-cutselection");

    // Register standard UTF-8 text type used for file dialogs
    utf8Type = getApp()->registerDragType("UTF8_STRING");

    FXVerticalFrame::create();

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }
    else
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    }
}


// Clean up
SearchPanel::~SearchPanel()
{
    delete list;
    if (opendialog != NULL)
    {
        delete opendialog;
    }
    if (archdialog != NULL)
    {
        delete archdialog;
    }
    if (operationdialogsingle != NULL)
    {
        delete operationdialogsingle;
    }
    if (operationdialogrename != NULL)
    {
        delete operationdialogrename;
    }
    if (operationdialogmultiple != NULL)
    {
        delete operationdialogmultiple;
    }
    if (comparedialog != NULL)
    {
        delete comparedialog;
    }
    delete associations;
    delete statuslabel;
    delete statusbar;
}


// Keyboard press
long SearchPanel::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    int currentitem = list->getCurrentItem();

    switch (event->code)
    {
    case KEY_Escape:

        this->handle(sender, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), ptr);
        return 1;

    case KEY_KP_Enter:
    case KEY_Return:
        handle(this, FXSEL(SEL_DOUBLECLICKED, FilePanel::ID_KEY_RETURN), (void*)(FXival)(currentitem));
        return 1;

    default:

        // Pass key to IconList
        return list->onKeyPress(sender, sel, ptr);
    }

    return 0;
}


// Double Click on File Item
long SearchPanel::onCmdItemDoubleClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    // In single click mode, return if not called from return key
    if ( (FXSELID(sel) != SearchPanel::ID_KEY_RETURN) && (single_click == SINGLE_CLICK_DIR_FILE))
    {
        int x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        if ((!(list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
             ((x - list->getXPosition()) < list->getHeaderSize(0)))
            || (list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)))
        {
            return 1;
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // At most one item selected
    if (list->getNumSelectedItems() <= 1)
    {
        FXlong item = (FXlong)ptr;
        if (item > -1)
        {
#ifdef STARTUP_NOTIFICATION
            // Startup notification option and exceptions (if any)
            FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
            FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif
            // Default programs
            FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
            FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
            FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
            FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
            FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
            FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

            FXString cmd, cmdname, filename, pathname, parentdir;

            // File name and path
            filename = list->getItemFilename(item);
            pathname = list->getItemFullPathname(item);

            // If directory, open the directory
            if (list->isItemDirectory(item))
            {
                // Does not have access
                if (!xf_isreadexecutable(pathname))
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."),
                                      pathname.text());
                    getApp()->endWaitCursor();
                    return 0;
                }

                // Change directory in Xfe
                ((XFileExplorer*)mainWindow)->setDirectory(pathname);

                // Raise the Xfe window
                ((XFileExplorer*)mainWindow)->raise();
                ((XFileExplorer*)mainWindow)->setFocus();

                // Warning message when setting current folder in Xfe
                FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
                if (warn)
                {
                    MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"),
                                            _("Current folder has been set to '%s'"), pathname.text());
                }
            }
            else if (list->isItemFile(item))
            {
                // Parent directory
                parentdir = FXPath::directory(pathname);

                // Update associations dictionary
                FileDict* assocdict = new FileDict(getApp());
                FileAssoc* association = assocdict->findFileBinding(pathname.text());

                // If there is an association
                if (association)
                {
                    // Use it to open the file
                    if (association->command.section(',', 0) != "")
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

                        // If command exists, run it
                        if (xf_existcommand(cmdname))
                        {
                            cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                            runcmd(cmd, parentdir, searchdir);
#endif
                        }
                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                    // Or execute the file
                    else if (list->isItemExecutable(item))
                    {
                        execFile(pathname);
                    }
                    // Or call the "Open with..." dialog
                    else
                    {
                        getApp()->endWaitCursor();
                        handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }
                // If no association but executable
                else if (list->isItemExecutable(item))
                {
                    execFile(pathname);
                }
                // Other cases
                else
                {
                    getApp()->endWaitCursor();
                    handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }

                delete assocdict;
            }
        }
    }
    // More than one selected files
    else
    {
        handle(this, FXSEL(SEL_COMMAND, ID_OPEN), NULL);
    }

    getApp()->endWaitCursor();

    return 1;
}


// Single click on File Item
long SearchPanel::onCmdItemClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    if (single_click != SINGLE_CLICK_NONE)
    {
        // Single click with control or shift
        int x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        if (state & (CONTROLMASK | SHIFTMASK))
        {
            return 1;
        }

        // In detailed mode, avoid single click when mouse cursor is not over the first column
        FXbool allow = true;
        if (!(list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
            ((x - list->getXPosition()) > list->getHeaderSize(0)))
        {
            allow = false;
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // At most one item selected
        if (list->getNumSelectedItems() <= 1)
        {
            // Default programs
            FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
            FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
            FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
            FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
            FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
            FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

            FXString cmd, cmdname, filename, pathname, parentdir;

#ifdef STARTUP_NOTIFICATION
            // Startup notification option and exceptions (if any)
            FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
            FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

            FXlong item = (FXlong)ptr;
            if (item > -1)
            {
                // File name and path
                filename = list->getItemFilename(item);
                pathname = list->getItemFullPathname(item);

                // If directory, open the directory
                if ((single_click != SINGLE_CLICK_NONE) && list->isItemDirectory(item) && allow)
                {
                    // Does not have access
                    if (!xf_isreadexecutable(pathname))
                    {
                        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."),
                                          pathname.text());
                        getApp()->endWaitCursor();
                        return 0;
                    }

                    // Change directory in Xfe
                    ((XFileExplorer*)mainWindow)->setDirectory(pathname);

                    // Raise the Xfe window
                    ((XFileExplorer*)mainWindow)->raise();
                    ((XFileExplorer*)mainWindow)->setFocus();

                    // Warning message when setting current folder in Xfe
                    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
                    if (warn)
                    {
                        MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"),
                                                _("Current folder has been set to '%s'"), pathname.text());
                    }
                }
                // If file, use the association if any
                else if ((single_click == SINGLE_CLICK_DIR_FILE) && list->isItemFile(item) && allow)
                {
                    // Parent directory
                    parentdir = FXPath::directory(pathname);

                    // Update associations dictionary
                    FileDict* assocdict = new FileDict(getApp());
                    FileAssoc* association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to open the file
                        if (association->command.section(',', 0) != "")
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

                            // If command exists, run it
                            if (xf_existcommand(cmdname))
                            {
                                cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                                runcmd(cmd, parentdir, searchdir);
#endif
                            }
                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
                                handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }
                        // Or execute the file
                        else if (list->isItemExecutable(item))
                        {
                            execFile(pathname);
                        }
                        // Or call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                    // If no association but executable
                    else if (list->isItemExecutable(item))
                    {
                        execFile(pathname);
                    }
                    // Other cases
                    else
                    {
                        getApp()->endWaitCursor();
                        handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }

                    delete assocdict;
                }
            }
        }
        // More than one selected files
        else if ((single_click == SINGLE_CLICK_DIR_FILE) && allow)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_OPEN), NULL);
        }

        getApp()->endWaitCursor();
    }
    return 1;
}


// Execute file with an optional confirm dialog
void SearchPanel::execFile(FXString filepath)
{
    FXString cmd, cmdname, parentdir, str;

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Parent directory
    parentdir = FXPath::directory(filepath);

    // Text file
    int ret = xf_istextfile(filepath);
    if (ret == 1)
    {
        // Execution forbidden, edit the file
        FXbool no_script = getApp()->reg().readUnsignedEntry("OPTIONS", "no_script", false);
        if (no_script)
        {
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            cmd = txteditor;
            cmdname = cmd;

            // If command exists, run it
            if (xf_existcommand(cmdname))
            {
                cmd = cmdname + " " + xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                runcmd(cmd, parentdir, searchdir);
#endif
            }
            // If command does not exist, call the "Open with..." dialog
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            }
        }
        // Execution allowed, execute file with optional confirmation dialog
        else
        {
            // With confirmation dialog
            FXbool confirm_execute = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_execute", true);
            if (confirm_execute)
            {
                FXString msg;
                msg.format(_("File %s is an executable text file, what do you want to do?"), filepath.text());
                ExecuteBox* dlg = new ExecuteBox(this, _("Confirm Execute"), msg);
                FXuint answer = dlg->execute(PLACEMENT_CURSOR);
                delete dlg;

                // Execute
                if (answer == EXECBOX_CLICKED_EXECUTE)
                {
                    cmdname = FXPath::name(filepath);
                    cmd = xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                    // No startup notification in this case
                    runcmd(cmd, cmdname, parentdir, searchdir, false, "");
#else
                    runcmd(cmd, parentdir, searchdir);
#endif
                }

                // Execute in console mode
                if (answer == EXECBOX_CLICKED_CONSOLE)
                {
                    int ret = chdir(parentdir.text());
                    if (ret < 0)
                    {
                        int errcode = errno;
                        if (errcode)
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"),
                                              parentdir.text(), strerror(errcode));
                        }
                        else
                        {
                            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), parentdir.text());
                        }
                    }

                    cmdname = FXPath::name(filepath);
                    cmd = xf_quote(filepath);

                    // Make and show command window
                    // The CommandWindow object will delete itself when closed!
                    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Command Log"), cmd, 30, 80);
                    cmdwin->create();
                    cmdwin->setIcon(minirunicon);
                }

                // Edit
                if (answer == EXECBOX_CLICKED_EDIT)
                {
                    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
                    cmd = txteditor;
                    cmdname = cmd;

                    // If command exists, run it
                    if (xf_existcommand(cmdname))
                    {
                        cmd = cmdname + " " + xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
                        runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                        runcmd(cmd, parentdir, searchdir);
#endif
                    }
                    // If command does not exist, call the "Open with..." dialog
                    else
                    {
                        handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }
            }
        }
    }
    // Binary file, execute it
    else if (ret == 0)
    {
        cmdname = FXPath::name(filepath);
        cmd = xf_quote(filepath);
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
        runcmd(cmd, searchdir, searchdir);
#endif
    }
    // Broken link
    else  // ret = -1
    {
        return;
    }
}


// Open with
long SearchPanel::onCmdOpenWith(FXObject*, FXSelector, void*)
{
    char** str = NULL;

    if (list->getNumSelectedItems() == 0)
    {
        return 0;
    }

    FXString cmd = "", cmdname;
    if (opendialog == NULL)
    {
        opendialog = new HistInputDialog(this, "", _("Open selected file(s) with:"), _("Open With"), "",
                                         bigfileopenicon, false, true, HIST_INPUT_EXECUTABLE_FILE, true, _("A&ssociate"));
    }
    opendialog->setText(cmd);

    // Dialog with history list and associate checkbox
    opendialog->CursorEnd();
    opendialog->selectAll();
    opendialog->clearItems();
    for (int i = 0; i < OpenNum; i++)
    {
        opendialog->appendItem(OpenHistory[i]);
    }
    opendialog->sortItems();
    opendialog->setDirectory(ROOTDIR);
    if (opendialog->execute())
    {
        cmd = opendialog->getText();
        if (cmd == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return 0;
        }

        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                // Handles "associate" checkbox for "open with..." dialog
                if (opendialog->getOption())
                {
                    FXString filename = list->getItemFilename(u);
                    FXString ext = filename.rafter('.', 2).lower();

                    if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") || (ext == "tar.zst") ||
                        (ext == "tar.z")) // Special cases
                    {
                    }
                    else
                    {
                        ext = FXPath::extension(filename).lower();
                    }

                    if (ext == "")
                    {
                        ext = FXPath::name(filename);
                    }

                    FileAssoc* association = list->getItemAssoc(u);

                    if (association)
                    {
                        // Update existing association
                        FXString oldfileassoc = getApp()->reg().readStringEntry("FILETYPES", ext.text(), "");
                        oldfileassoc.erase(0, oldfileassoc.section(';', 0).section(',', 0).length());
                        oldfileassoc.prepend(opendialog->getText());
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), oldfileassoc.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text()) + 1];
                        str[1] = new char[strlen(oldfileassoc.text()) + 1];
                        xf_strlcpy(str[0], ext.text(), ext.length() + 1);
                        xf_strlcpy(str[1], oldfileassoc.text(), oldfileassoc.length() + 1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                    else
                    {
                        // New association
                        FXString newcmd = opendialog->getText().append(";Document;;;;");
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), newcmd.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text()) + 1];
                        str[1] = new char[strlen(newcmd.text()) + 1];
                        xf_strlcpy(str[0], ext.text(), ext.length() + 1);
                        xf_strlcpy(str[1], newcmd.text(), newcmd.length() + 1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                }
                // End

                FXString pathname = list->getItemFullPathname(u);
                cmdname = cmd;
                cmd += " ";
                cmd = cmd + xf_quote(pathname);
            }
        }

        // Run command if it exists
        getApp()->beginWaitCursor();

#ifdef STARTUP_NOTIFICATION
        // Startup notification option and exceptions (if any)
        FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
        FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, searchdir, searchdir);
#endif
        }
        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
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

        // Restore original history order
        opendialog->clearItems();
        for (int i = 0; i < OpenNum; i++)
        {
            opendialog->appendItem(OpenHistory[i]);
        }

        // History limit reached
        if (OpenNum > OPEN_HIST_SIZE)
        {
            OpenNum--;
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

        getApp()->endWaitCursor();
    }

    // If list has been cleared, set history size to zero
    if (opendialog->getHistorySize() == 0)
    {
        OpenNum = 0;
    }

    return 1;
}


// Open single or multiple files
long SearchPanel::onCmdOpen(FXObject*, FXSelector, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

    FileAssoc* association;

    if (list->getNumSelectedItems() == 0)
    {
        getApp()->endWaitCursor();
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

    // Lists of command and items
    vector_FXString cmdlist;
    vector_FXString itemslist;

    // Ignored items
    FXString ignoreditems = "";

    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            FXString pathname = list->getItemFullPathname(u);

            // If directory, skip it
            if (xf_isdirectory(pathname))
            {
                continue;
            }

            // If association found
            association = assocdict->findFileBinding(pathname.text());
            if (association)
            {
                FXString cmd = association->command.section(',', 0);

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

                // A command exists
                if (cmd != "")
                {
                    // Check if the command is already in the list
                    FXuint j;
                    FXbool found = false;
                    for (FXuint i = 0; i < cmdlist.size(); i++)
                    {
                        if (cmdlist[i] == cmd)
                        {
                            found = true;
                            j = i;
                            break;
                        }
                    }

                    // Command not in list, then add the command to the list and add the path name to the items list
                    if (!found)
                    {
                        cmdlist.push_back(cmd);
                        itemslist.push_back(xf_quote(pathname));
                    }
                    // Command already in list, then update the items list for that command
                    else
                    {
                        itemslist[j] = itemslist[j] + " " + xf_quote(pathname);
                    }
                }
            }
            // Or execute the file
            else if (list->isItemExecutable(u))
            {
                execFile(pathname);
            }
            // Or call the "Open with..." dialog
            else
            {
                // One item, call the "Open with..." dialog
                if (list->getNumSelectedItems() == 1)
                {
                    getApp()->endWaitCursor();
                    handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }
                // More than one item, add items to the ignored list
                else
                {
                    ignoreditems += " " + FXPath::name(pathname) + "\n";
                }
            }
        }
    }

    delete assocdict;

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Open files using the associated commands
    for (FXuint i = 0; i < cmdlist.size(); i++)
    {
        FXString cmdname = cmdlist[i];

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            FXString cmd = cmdname + " " + itemslist[i];
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, searchdir, searchdir);
#endif
        }
    }

    // If there are ignored items, display a warning message
    if (ignoreditems.length() != 0)
    {
        FXString message;
        message.format(_("The following files have no association and were ignored:\n\n%s"), ignoreditems.text());
        MessageBox::warning(this, BOX_OK, _("Warning"), "%s", message.text());
    }

    getApp()->endWaitCursor();

    return 1;
}


// View/Edit files
long SearchPanel::onCmdEdit(FXObject*, FXSelector sel, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();

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
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            // Increment number of selected items
            pathname = list->getItemFullPathname(u);
            association = assocdict->findFileBinding(pathname.text());

            // If there is an association
            if (association)
            {
                // Use it to edit/view the files
                if (FXSELID(sel) == ID_EDIT) // Edit
                {
                    cmd = association->command.section(',', 2);

                    // Use a default editor if possible
                    switch (progs[cmd])
                    {
                    case TXTEDITOR:
                        cmd = txteditor;
                        break;

                    case IMGEDITOR:
                        cmd = imgeditor;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No default editor found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txteditor;
                    }
                }
                else // Any other is View
                {
                    cmd = association->command.section(',', 1);

                    // Use a default viewer if possible
                    switch (progs[cmd])
                    {
                    case TXTVIEWER:
                        cmd = txtviewer;
                        break;

                    case IMGVIEWER:
                        cmd = imgviewer;
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

                    case NONE: // No default viewer found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txtviewer;
                    }
                }
                if (cmd.text() != NULL)
                {
                    // First selected item
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

                    // List of selected items
                    itemslist += xf_quote(pathname) + " ";
                }
                else
                {
                    same = false;
                    break;
                }
            }
            // Text file
            else if (xf_istextfile(pathname) == 1)
            {
                if (FXSELID(sel) == ID_EDIT) // Edit
                {
                    cmd = txteditor;
                }
                else
                {
                    cmd = txtviewer;
                }

                // First selected item
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

                // List of selected items
                itemslist += xf_quote(pathname) + " ";
            }
            // Broken link
            else if (xf_istextfile(pathname) == -1)
            {
                return 1;
            }
            // No association
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

    // Same association for all files : execute the associated or default editor or viewer
    if (same)
    {
        cmdname = samecmd;

        // If command exists, run it
        if (xf_existcommand(cmdname))
        {
            cmd = cmdname + itemslist;
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, searchdir, searchdir);
#endif
        }
        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
        }
    }
    // Files have different associations : handle them separately
    else
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);

                // Only View / Edit regular files (not directories)
                if (xf_isfile(pathname))
                {
                    association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to edit/view the file
                        if (FXSELID(sel) == ID_EDIT) // Edit
                        {
                            cmd = association->command.section(',', 2);

                            // Use a default editor if possible
                            switch (progs[cmd])
                            {
                            case TXTEDITOR:
                                cmd = txteditor;
                                break;

                            case IMGEDITOR:
                                cmd = imgeditor;
                                break;

                            case ARCHIVER:
                                cmd = archiver;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txteditor;
                            }
                        }
                        else // Any other is View
                        {
                            cmd = association->command.section(',', 1);

                            // Use a default viewer if possible
                            switch (progs[cmd])
                            {
                            case TXTVIEWER:
                                cmd = txtviewer;
                                break;

                            case IMGVIEWER:
                                cmd = imgviewer;
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

                            case NONE: // No default viewer found
                                ;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txtviewer;
                            }
                        }
                        if (cmd.text() != NULL)
                        {
                            cmdname = cmd;

                            // If command exists, run it
                            if (xf_existcommand(cmdname))
                            {
                                cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
                                runcmd(cmd, searchdir, searchdir);
#endif
                            }
                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
                                this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }
                    }
                    // No association
                    else
                    {
                        if (FXSELID(sel) == ID_EDIT)
                        {
                            cmd = txteditor;
                        }
                        else
                        {
                            cmd = txtviewer;
                        }

                        cmdname = cmd;

                        // If command exists, run it
                        if (xf_existcommand(cmdname))
                        {
                            cmd = cmdname + " " + xf_quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
                            runcmd(cmd, searchdir, searchdir);
#endif
                        }
                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
                            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                }
            }
        }
    }

    delete assocdict;
    getApp()->endWaitCursor();

    return 1;
}


// Compare two files
long SearchPanel::onCmdCompare(FXObject*, FXSelector sel, void*)
{
    list->setFocus();
    int num = list->getNumSelectedItems();

    // Only one or two selected items can be handled
    if ((num != 1) && (num != 2))
    {
        getApp()->endWaitCursor();
        return 0;
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    FXString filecomparator = getApp()->reg().readStringEntry("PROGS", "filecomparator", DEFAULT_FILECOMPARATOR);
    FXString pathname, cmd, cmdname, itemslist = " ";

    // One selected item
    if (num == 1)
    {
        // Get the selected item
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);
                itemslist += xf_quote(pathname) + " ";
            }
        }

        // Open a dialog to select the other item to be compared
        if (comparedialog == NULL)
        {
            comparedialog = new BrowseInputDialog(this, "", "", _("Compare"), _("With:"), bigcompareicon,
                                                  BROWSE_INPUT_FILE);
        }
        comparedialog->setIcon(bigcompareicon);
        comparedialog->setMessage(pathname);
        comparedialog->setText("");
        int rc = comparedialog->execute(PLACEMENT_CURSOR);

        // Get item path and add it to the list
        FXString str = comparedialog->getText();
        itemslist += xf_quote(str);
        if (!rc || (str == ""))
        {
            return 0;
        }
    }
    // Two selected items
    else if (num == 2)
    {
        // Get the two selected items
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);
                itemslist += xf_quote(pathname) + " ";
            }
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // If command exists, run it
    cmdname = filecomparator;
    if (xf_existcommand(cmdname))
    {
        cmd = cmdname + itemslist;
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
        runcmd(cmd, searchdir, searchdir);
#endif
    }
    // If command does not exist, issue an error message
    else
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"),
                          _("Program %s not found. Please define a file comparator program in the Preferences dialog!"),
                          cmdname.text());
    }

    getApp()->endWaitCursor();

    return 1;
}


// Force panel refresh
long SearchPanel::onCmdRefresh(FXObject* sender, FXSelector, void*)
{
    list->onCmdRefresh(0, 0, 0);

    return 1;
}


// Go to parent directory
long SearchPanel::onCmdGotoParentdir(FXObject*, FXSelector, void*)
{
    if (list->getNumSelectedItems() != 1)
    {
        return 0;
    }

    // Get selected item path name
    FXString pathname;
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            pathname = list->getItemFullPathname(u);
            break;
        }
    }

    // Parent directory name
    FXString parentdir = FXPath::directory(pathname);

    // Does not have access
    if (!xf_isreadexecutable(parentdir))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), parentdir.text());
        getApp()->endWaitCursor();
        return 0;
    }

    // Change directory in Xfe
    ((XFileExplorer*)mainWindow)->setDirectory(parentdir);

    // Raise the Xfe window
    ((XFileExplorer*)mainWindow)->raise();
    ((XFileExplorer*)mainWindow)->setFocus();

    // Warning message when setting current folder in Xfe
    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
    if (warn)
    {
        MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"),
                                _("Current folder has been set to '%s'"), parentdir.text());
    }

    return 1;
}


// File or directory properties
long SearchPanel::onCmdProperties(FXObject* sender, FXSelector, void*)
{
    int num, itm;

    // There is one selected file in the file list
    num = list->getNumSelectedItems(&itm);
    if (num == 1)
    {
        FXString filename = list->getItemFilename(itm);
        FXString pathname = FXPath::directory(list->getItemFullPathname(itm));

        PropertiesBox* attrdlg = new PropertiesBox(this, filename, pathname);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);
    }
    // There are multiple selected files in the file list
    else if (num > 1)
    {
        FXString* files = new FXString[num];
        FXString* paths = new FXString[num];

        int i = 0;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                files[i] = list->getItemText(u).section('\t', 0);
                paths[i] = FXPath::directory(list->getItemFullPathname(u));
                i++;
            }
        }

        PropertiesBox* attrdlg = new PropertiesBox(this, files, num, paths);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);
    }
    return 1;
}


// Handle item selection
long SearchPanel::onCmdSelect(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_SELECT_ALL:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_ALL), ptr);
        return 1;

    case ID_DESELECT_ALL:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), ptr);
        return 1;

    case ID_SELECT_INVERSE:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_INVERSE), ptr);
        return 1;
    }
    return 1;
}


// Set search root path
void SearchPanel::setSearchPath(FXString path)
{
    searchdir = xf_cleanpath(path);
    list->setDirectory(searchdir, false);
}


// Append an item to the file list
// Note that thumbnails are not displayed here to improve performances
long SearchPanel::appendItem(FXString& pathname)
{
    FXString filename, dirname;
    FXString grpid, usrid, perms, linkpath, mod, ext, del;
    FileAssoc* fileassoc;
    FXString filetype, lowext;
    FXIcon* big, * mini;
    time_t filemtime, filectime;
    struct stat info, linfo;
    FXbool isLink, isBrokenLink, isLinkToDir, isDir;

    // Only process valid file paths and paths different from the search directory
    if (xf_lstat(pathname.text(), &linfo) == 0)
    {
        filename = FXPath::name(pathname);
        dirname = FXPath::directory(pathname);

        // Get file/link info and indicate if it is a link
        isLink = S_ISLNK(linfo.st_mode);
        isBrokenLink = false;

        // Find if it is a broken link or a link to a directory
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
            goto end;
        }

        // File times
        filemtime = linfo.st_mtime;
        filectime = linfo.st_ctime;

        // Find if it is a folder
        isDir = false;
        if (S_ISDIR(linfo.st_mode))
        {
            isDir = true;
        }

        // User name
        usrid = FXSystem::userName(linfo.st_uid);

        // Group name
        grpid = FXSystem::groupName(linfo.st_gid);

        // Permissions (caution : we don't use the FXSystem::modeString() function because
        // it seems to be incompatible with the info.st_mode format)
        perms = xf_permissions(linfo.st_mode);

        // Mod time
        mod = FXSystem::time("%x %X", linfo.st_mtime);
        del = "";
        ext = "";

        // Obtain the extension for files only
        if (!isDir)
        {
            ext = FXPath::extension(pathname);
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
                goto end;
            }
        }

        // Assume no associations
        fileassoc = NULL;

        // Determine icons and type
        if (isDir)
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
        else if (S_ISCHR(info.st_mode))
        {
            big = bigchardevicon;
            mini = minichardevicon;
            filetype = _("Character Device");
        }
        else if (S_ISBLK(info.st_mode))
        {
            big = bigblockdevicon;
            mini = miniblockdevicon;
            filetype = _("Block Device");
        }
        else if (S_ISFIFO(info.st_mode))
        {
            big = bigpipeicon;
            mini = minipipeicon;
            filetype = _("Named Pipe");
        }
        else if (S_ISSOCK(info.st_mode))
        {
            big = bigsocketicon;
            mini = minisocketicon;
            filetype = _("Socket");
        }
        else if ((info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) && !(S_ISDIR(info.st_mode) || S_ISCHR(info.st_mode) ||
                                                                     S_ISBLK(info.st_mode) || S_ISFIFO(info.st_mode) ||
                                                                     S_ISSOCK(info.st_mode)))
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

        // Symbolic links have a specific type
        if (isBrokenLink)
        {
            filetype = _("Broken Link");
        }
        else if (isLink)
        {
            if (associations)
            {
                // Don't forget to remove trailing '/' here!
                fileassoc = associations->findFileBinding(linkpath.text());
                if (fileassoc)
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
                else
                {
                    filetype = _("Link to ") + filetype;
                }
            }
        }

        // Don't display the file size for directories
        FXString hsize;
        if (isDir)
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

#if defined(linux)
        // Devices have a specific icon
        if (fsdevices->find(pathname.text()))
        {
            filetype = _("Mount Point");

            if (xf_strequal(fsdevices->find(pathname.text()), "harddrive"))
            {
                big = bigharddriveicon;
                mini = miniharddriveicon;
            }
            else if (xf_strequal(fsdevices->find(pathname.text()), "nfsdisk"))
            {
                big = bignetdriveicon;
                mini = mininetdriveicon;
            }
            else if (xf_strequal(fsdevices->find(pathname.text()), "smbdisk"))
            {
                big = bignetdriveicon;
                mini = mininetdriveicon;
            }
            else if (xf_strequal(fsdevices->find(pathname.text()), "floppy"))
            {
                big = bigfloppyicon;
                mini = minifloppyicon;
            }
            else if (xf_strequal(fsdevices->find(pathname.text()), "cdrom"))
            {
                big = bigcdromicon;
                mini = minicdromicon;
            }
            else if (xf_strequal(fsdevices->find(pathname.text()), "zip"))
            {
                big = bigzipdiskicon;
                mini = minizipdiskicon;
            }
        }
#endif

        FXString str;
        for (FXuint i = 0; i < nbCols; i++)
        {
            switch (idCol[i])
            {
            case FileList::ID_COL_NAME:
                str += filename + "\t";
                break;

            case FileList::ID_COL_DIRNAME:
                str += dirname + "\t";
                break;

            case FileList::ID_COL_SIZE:
                str += hsize + "\t";
                break;

            case FileList::ID_COL_TYPE:
                str += filetype + "\t";
                break;

            case FileList::ID_COL_EXT:
                str += ext + "\t";
                break;

            case FileList::ID_COL_DATE:
                str += mod + "\t";
                break;

            case FileList::ID_COL_USER:
                str += usrid + "\t";
                break;

            case FileList::ID_COL_GROUP:
                str += grpid + "\t";
                break;

            case FileList::ID_COL_PERMS:
                str += perms + "\t";
                break;

            case FileList::ID_COL_LINK:
                str += linkpath + "\t";
                break;
            }
        }

        for (FXuint i = 0; i < NMAX_COLS + 1 - nbCols; i++)
        {
            str += "\t";
        }

        str += pathname;

        // Append item to the list
        list->appendItem(str, big, mini);

        // Get last item
        int count = list->getNumItems();
        FileItem* item = (FileItem*)list->getItem(count - 1);

        if (item == NULL)
        {
            fprintf(stderr, "%s::appendItem: NULL item specified.\n", getClassName());
            exit(EXIT_FAILURE);
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

        // Set item attributes
        item->size = (FXulong)linfo.st_size;
        item->assoc = fileassoc;
        item->date = filemtime;
        item->cdate = filectime;

        // And finally, don't forget to create the appended item!
        item->create();
    }
    else
    {
        return 0;
    }
end:

    return 1;
}


// File list context menu
long SearchPanel::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // No item in list
    if (list->getNumItems() == 0)
    {
        return 0;
    }

    list->setAllowRefresh(false);

    // Check if control key was pressed
    ctrlflag = false;
    shiftf10 = false;
    if (ptr != NULL)
    {
        FXEvent* event = (FXEvent*)ptr;
        if (event->state & CONTROLMASK)
        {
            ctrlflag = true;
        }
        if (event->state & SHIFTMASK && (event->code == KEY_F10))
        {
            shiftf10 = true;
        }
    }

    // Use to select the item under cursor when right clicking
    // Only when Ctrl-Shift-F10 was not pressed
    if (!shiftf10 && (list->getNumSelectedItems() <= 1))
    {
        int x, y;
        FXuint state;
        list->getCursorPosition(x, y, state);

        int item = list->getItemAt(x, y);

        if (list->getCurrentItem() >= 0)
        {
            list->deselectItem(list->getCurrentItem());
        }
        if (item >= 0)
        {
            list->setCurrentItem(item);
            list->selectItem(item);
        }
    }

    // If control flag is set, deselect all items
    if (ctrlflag)
    {
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), ptr);
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);
    int x, y;
    FXuint state;
    getRoot()->getCursorPosition(x, y, state);

    int num, itm;
    num = list->getNumSelectedItems(&itm);

    // No selection or control key was pressed
    if ((num == 0) || ctrlflag)
    {
        // Reset the control flag
        ctrlflag = false;

        new FXMenuCheck(menu, _("Thum&bnails"), list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("B&ig Icons"), list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(menu, _("&Small Icons"), list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(menu, _("&Detailed File List"), list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("Autos&ize"), list, FileList::ID_AUTOSIZE);
        new FXMenuRadio(menu, _("&Rows"), list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(menu, _("&Columns"), list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("&Name"), list, FileList::ID_COL_NAME);
        new FXMenuRadio(menu, _("Si&ze"), list, FileList::ID_COL_SIZE);
        new FXMenuRadio(menu, _("&Type"), list, FileList::ID_COL_TYPE);
        new FXMenuRadio(menu, _("E&xtension"), list, FileList::ID_COL_EXT);
        new FXMenuRadio(menu, _("&Date"), list, FileList::ID_COL_DATE);
        new FXMenuRadio(menu, _("&User"), list, FileList::ID_COL_USER);
        new FXMenuRadio(menu, _("&Group"), list, FileList::ID_COL_GROUP);
        new FXMenuRadio(menu, _("Per&missions"), list, FileList::ID_COL_PERMS);
        new FXMenuRadio(menu, _("&Link"), list, FileList::ID_COL_LINK);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("I&gnore Case"), list, FileList::ID_SORT_CASE);
        new FXMenuCheck(menu, _("Fold&ers First"), list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(menu, _("Re&verse Order"), list, FileList::ID_SORT_REVERSE);
    }
    // Non empty selection
    else
    {
        // Submenu items
        FXMenuPane* submenu = new FXMenuPane(this);
        new FXMenuCheck(submenu, _("Thum&bnails"), list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("B&ig Icons"), list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(submenu, _("&Small Icons"), list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(submenu, _("&Detailed File List"), list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("Autos&ize"), list, FileList::ID_AUTOSIZE);
        new FXMenuRadio(submenu, _("&Rows"), list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(submenu, _("&Columns"), list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("&Name"), list, FileList::ID_COL_NAME);
        new FXMenuRadio(submenu, _("Si&ze"), list, FileList::ID_COL_SIZE);
        new FXMenuRadio(submenu, _("&Type"), list, FileList::ID_COL_TYPE);
        new FXMenuRadio(submenu, _("E&xtension"), list, FileList::ID_COL_EXT);
        new FXMenuRadio(submenu, _("&Date"), list, FileList::ID_COL_DATE);
        new FXMenuRadio(submenu, _("&User"), list, FileList::ID_COL_USER);
        new FXMenuRadio(submenu, _("&Group"), list, FileList::ID_COL_GROUP);
        new FXMenuRadio(submenu, _("Per&missions"), list, FileList::ID_COL_PERMS);
        new FXMenuRadio(submenu, _("&Link"), list, FileList::ID_COL_LINK);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("Ignore C&ase"), list, FileList::ID_SORT_CASE);
        new FXMenuCheck(submenu, _("Fold&ers First"), list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(submenu, _("Re&verse Order"), list, FileList::ID_SORT_REVERSE);
        new FXMenuCascade(menu, _("Pane&l"), NULL, submenu);
        new FXMenuSeparator(menu);


        FXbool ar = false;
        if (list->getItem(itm) && list->isItemFile(itm))
        {
            new FXMenuCommand(menu, _("Open &with..."), minifileopenicon, this, SearchPanel::ID_OPEN_WITH);
            new FXMenuCommand(menu, _("&Open"), minifileopenicon, this, SearchPanel::ID_OPEN);
            FXString name = this->list->getItemText(itm).section('\t', 0);

            // Last and before last file extensions
            FXString ext1 = name.rafter('.', 1).lower();
            FXString ext2 = name.rafter('.', 2).lower();

            // Display the extract and package menus according to the archive extensions
            if ((num == 1) &&
                ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") ||
                 (ext2 == "tar.z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("E&xtract to..."), miniarchexticon, this, SearchPanel::ID_EXTRACT);
            }
            else if ((num == 1) &&
                     ((ext1 == "gz") || (ext1 == "bz2") || (ext1 == "zst") || (ext1 == "xz") || (ext1 == "z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&Extract Here"), miniarchexticon, this, SearchPanel::ID_EXTRACT);
            }
            else if ((num == 1) && ((ext1 == "tar") || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz")
                                    || (ext1 == "tzst") || (ext1 == "taz") || (ext1 == "txz") || (ext1 == "zip") ||
                                    (ext1 == "7z")
                                    || (ext1 == "lzh") || (ext1 == "rar") || (ext1 == "ace") || (ext1 == "arj")))
            {
                ar = true;
                new FXMenuCommand(menu, _("E&xtract to..."), miniarchexticon, this, SearchPanel::ID_EXTRACT);
            }
#if defined(linux)
            else if ((num == 1) && ((ext1 == "rpm") || (ext1 == "deb")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&View"), minipackageicon, this, SearchPanel::ID_VIEW);
            }
#endif
            // Not archive nor package
            if (!ar)
            {
                new FXMenuCommand(menu, _("&View"), miniviewicon, this, SearchPanel::ID_VIEW);
                new FXMenuCommand(menu, _("&Edit"), miniediticon, this, SearchPanel::ID_EDIT);
                if (num == 1)
                {
                    new FXMenuCommand(menu, _("Com&pare..."), minicompareicon, this, SearchPanel::ID_COMPARE);
                }
                else
                {
                    new FXMenuCommand(menu, _("Com&pare"), minicompareicon, this, SearchPanel::ID_COMPARE);
                }
            }
        }
        if (!ar)
        {
            new FXMenuCommand(menu, _("&Add to Archive..."), miniarchaddicon, this, SearchPanel::ID_ADD_TO_ARCH);
        }
#if defined(linux)
        if ((num == 1) && !ar)
        {
            new FXMenuCommand(menu, _("&Packages Query "), minipackageicon, this, SearchPanel::ID_PKG_QUERY);
        }
#endif

        // Build scripts menu
        new FXMenuSeparator(menu);
        FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;
        FXMenuPane* scriptsmenu = new FXMenuPane(this);
        new FXMenuCascade(menu, _("Scripts"), minirunicon, scriptsmenu);
        readScriptDir(scriptsmenu, scriptpath);
        new FXMenuSeparator(scriptsmenu);
        new FXMenuCommand(scriptsmenu, _("&Go to Script Folder"), minigotodiricon, this, SearchPanel::ID_GO_SCRIPTDIR);

        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Go to Parent Folder"), minigotodiricon, this, SearchPanel::ID_GOTO_PARENTDIR);
        new FXMenuCommand(menu, _("&Copy"), minicopyicon, this, SearchPanel::ID_COPY_CLIPBOARD);
        new FXMenuCommand(menu, _("C&ut"), minicuticon, this, SearchPanel::ID_CUT_CLIPBOARD);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Re&name..."), minirenameicon, this, SearchPanel::ID_FILE_RENAME);
        new FXMenuCommand(menu, _("Copy &to..."), minicopyicon, this, SearchPanel::ID_FILE_COPYTO);
        new FXMenuCommand(menu, _("&Move to..."), minimoveicon, this, SearchPanel::ID_FILE_MOVETO);
        new FXMenuCommand(menu, _("Symlin&k to..."), minilinktoicon, this, SearchPanel::ID_FILE_SYMLINK);
        new FXMenuCommand(menu, _("M&ove to Trash"), minideleteicon, this, SearchPanel::ID_FILE_TRASH);
        new FXMenuCommand(menu, _("&Delete"), minideletepermicon, this, SearchPanel::ID_FILE_DELETE);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Compare &Sizes"), minicharticon, this, SearchPanel::ID_DIR_USAGE);
        if (num == 1)
        {
            new FXMenuCommand(menu, _("Cop&y Name"), minicopyicon, this, SearchPanel::ID_COPYNAME_CLIPBOARD);
        }
        else
        {
            new FXMenuCommand(menu, _("Cop&y Names"), minicopyicon, this, SearchPanel::ID_COPYNAME_CLIPBOARD);
        }
        new FXMenuCommand(menu, _("P&roperties"), miniattribicon, this, SearchPanel::ID_PROPERTIES);
    }
    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);
    list->setAllowRefresh(true);

    return 1;
}


// Read all executable file names that are located into the script directory
// Sort entries alphabetically
int SearchPanel::readScriptDir(FXMenuPane* scriptsmenu, FXString dir)
{
    DIR* dp;
    struct dirent** namelist;

    // Open directory
    if ((dp = opendir(dir.text())) == NULL)
    {
        return 0;
    }

    // Possibly add a / at the end of the directory name
    if (dir[dir.length() - 1] != '/')
    {
        dir = dir + "/";
    }

    // Read directory and sort entries alphabetically
    int n = scandir(dir.text(), &namelist, NULL, alphasort);
    if (n < 0)
    {
        perror("scandir");
    }
    else
    {
        for (int k = 0; k < n; k++)
        {
            // Avoid hidden directories and '.' and '..'
            if (namelist[k]->d_name[0] != '.')
            {
                FXString pathname = dir + namelist[k]->d_name;

                // Recurse if non empty directory
                if (xf_isdirectory(pathname))
                {
                    if (!xf_isemptydir(pathname))
                    {
                        FXMenuPane* submenu = new FXMenuPane(this);
                        new FXMenuCascade(scriptsmenu, namelist[k]->d_name, NULL, submenu);
                        readScriptDir(submenu, pathname);
                    }
                }
                // Add only executable files to the list
                else if (xf_isreadexecutable(pathname))
                {
                    new FXMenuCommand(scriptsmenu, namelist[k]->d_name + FXString("\t\t") + pathname,
                                      miniexecicon, this, FilePanel::ID_RUN_SCRIPT);
                }
            }
            free(namelist[k]);
        }
        free(namelist);
    }

    // Close directory
    (void)closedir(dp);

    return 1;
}


// Add files or directory to an archive
long SearchPanel::onCmdAddToArch(FXObject* sender, FXSelector, void*)
{
    int ret;
    FXString name, ext1, ext2, cmd, archive = "";
    File* f;

    // Enter search directory
    ret = chdir(searchdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), searchdir.text(),
                              strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), searchdir.text());
        }

        return 0;
    }

    // If only one item is selected, use its name as a starting guess for the archive name
    if (list->getNumSelectedItems() == 1)
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                name = list->getItemFilename(u);
                break;
            }
        }
        archive = name;
    }

    // Initial archive name with full path and default extension
    archive = homedir + PATHSEPSTRING + archive + ".tar.gz";

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

        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                name = FXPath::relative(searchdir, list->getItemFullPathname(u));
                cmd += " ";
                cmd = cmd + xf_quote(name);
                cmd += " ";
            }
        }

        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Create Archive"), ARCHIVE);
        f->create();

        // Create archive
        f->archive(archive, cmd);

        getApp()->endWaitCursor();
        delete f;
    }
    return 1;
}


// Extract archive
long SearchPanel::onCmdExtract(FXObject*, FXSelector, void*)
{
    FXString name, ext1, ext2, cmd, dir;
    File* f;

    // File selection dialog
    FileDialog browse(this, _("Select a Destination Folder"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };

    browse.setDirectory(homedir);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_DIRECTORY);

    int item;
    list->getNumSelectedItems(&item);
    if (list->getItem(item))
    {
        // Path
        FXString path = FXPath::directory(list->getItemFullPathname(item));

        // Archive name and extensions
        name = list->getItemText(item).text();
        ext1 = name.section('\t', 0).rafter('.', 1).lower();
        ext2 = name.section('\t', 0).rafter('.', 2).lower();
        name = xf_quote(path + PATHSEPSTRING + name.section('\t', 0));

        // Handle different archive formats
        if ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.zst") || (ext2 == "tar.z"))
        {
            cmd = "tar -axvf ";
        }
        else if ((ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "txz") || (ext1 == "taz") ||
                 (ext1 == "tzst"))
        {
            cmd = "tar -axvf ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -xvf ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gunzip -v ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bunzip2 -v ";
        }
        else if (ext1 == "zst")
        {
            cmd = "zstd -df ";
        }
        else if (ext1 == "xz")
        {
            cmd = "unxz -v ";
        }
        else if (ext1 == "z")
        {
            cmd = "uncompress -v ";
        }
        else if (ext1 == "zip")
        {
            cmd = "unzip -o ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z x -y ";
        }
        else if (ext1 == "rar")
        {
            cmd = "unrar x -o+ ";
        }
        else if (ext1 == "lzh")
        {
            cmd = "lha -xf ";
        }
        else if (ext1 == "ace")
        {
            cmd = "unace x ";
        }
        else if (ext1 == "arj")
        {
            cmd = "arj x -y ";
        }
        else
        {
            cmd = "tar -axvf ";
        }

        // Final extract command
        cmd += name + " ";

        // Extract archive
        if (browse.execute())
        {
            dir = browse.getFilename();

            if (xf_iswritable(dir))
            {
                // Wait cursor
                getApp()->beginWaitCursor();

                // File object
                f = new File(this, _("Extract Archive"), EXTRACT);
                f->create();

                // Extract archive
                f->extract(name, dir, cmd);

                getApp()->endWaitCursor();
                delete f;
            }
            else
            {
                MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"),
                                  dir.text());
            }
        }
    }

    return 1;
}


// Directory usage on file selection
long SearchPanel::onCmdDirUsage(FXObject* sender, FXSelector, void*)
{
    FXString pathname, command, itemslist = " ";
    FXString cmd1 = "/usr/bin/du --apparent-size -k -s ";
    FXString cmd2 =
        " 2> /dev/null | /usr/bin/sort -rn | /usr/bin/cut -f2 | /usr/bin/xargs -d '\n' /usr/bin/du --apparent-size --total --si -s 2> /dev/null";

    // Construct selected files list
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            pathname = list->getItemFullPathname(u);

            // List of selected items
            itemslist += xf_quote(pathname) + " ";
        }
    }

    // Command to be executed
    command = cmd1 + itemslist + cmd2;

    // Make and show command window
    CommandWindow* cmdwin = new CommandWindow(getApp(), _("Sizes of Selected Items"), command, 25, 50);
    cmdwin->create();
    cmdwin->setIcon(minicharticon);

    return 1;
}


// Trash files from the file list
long SearchPanel::onCmdFileTrash(FXObject*, FXSelector, void*)
{
    int firstitem = 0;
    File* f = NULL;

    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    // If we don't have permission to write to the trash directory
    if (!xf_iswritable(trashfileslocation))
    {
        MessageBox::error(getApp(), BOX_OK_SU, _("Error"), _("Can't write to trash location %s: Permission denied"),
                          trashfileslocation.text());
        return 0;
    }

    // Items number in the file list
    int num = list->getNumSelectedItems();
    if (num < 1)
    {
        return 0;
    }

    if (confirm_trash)
    {
        FXString message;
        if (num == 1)
        {
            FXString pathname;
            for (int u = 0; u < list->getNumItems(); u++)
            {
                if (list->isItemSelected(u))
                {
                    pathname = list->getItemFullPathname(u);
                }
            }
            if (xf_isdirectory(pathname))
            {
                message.format(_("Move folder %s to trash can?"), pathname.text());
            }
            else
            {
                message.format(_("Move file %s to trash can?"), pathname.text());
            }
        }
        else
        {
            message.format(_("Move %s selected items to trash can?"), FXStringVal(num).text());
        }

        MessageBox box(this, _("Confirm Trash"), message, bigdeleteicon, BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return 0;
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();

    // File object
    f = new File(this, _("Move to Trash"), DELETE, num);
    f->create();
    list->setAllowRefresh(false);

    // Overwrite initialisations
    FXbool overwrite = false;
    FXbool overwrite_all = false;
    FXbool skip_all = false;

    // Delete selected files
    FXString filename, pathname;
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            // Get index of first selected item
            if (firstitem == 0)
            {
                firstitem = u;
            }

            // Get file name and path
            filename = list->getItemFilename(u);
            pathname = list->getItemFullPathname(u);

            // File could have already been trashed above in the tree
            if (!xf_existfile(pathname))
            {
                continue;
            }

            // If we don't have permission to write to the file
            if (!xf_iswritable(pathname))
            {
                // Overwrite dialog if necessary
                if (!(overwrite_all | skip_all))
                {
                    f->hideProgressDialog();
                    FXString msg;
                    msg.format(_("File %s is write-protected, move it anyway to trash can?"), pathname.text());

                    if (num == 1)
                    {
                        OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg, OVWBOX_SINGLE_FILE);
                        FXuint answer = dlg->execute(PLACEMENT_OWNER);
                        delete dlg;
                        if (answer == 1)
                        {
                            overwrite = true;
                        }
                        else
                        {
                            goto end;
                        }
                    }
                    else
                    {
                        OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg);
                        FXuint answer = dlg->execute(PLACEMENT_OWNER);
                        delete dlg;
                        switch (answer)
                        {
                        // Cancel
                        case 0:
                            goto end;
                            break;

                        // Overwrite
                        case 1:
                            overwrite = true;
                            break;

                        // Overwrite all
                        case 2:
                            overwrite_all = true;
                            break;

                        // Skip
                        case 3:
                            overwrite = false;
                            break;

                        // Skip all
                        case 4:
                            skip_all = true;
                            break;
                        }
                    }
                }
                if ((overwrite | overwrite_all) && !skip_all)
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
                        MessageBox::error(this, BOX_OK, _("Error"),
                                          _("An error has occurred during the move to trash operation!"));
                        break;
                    }
                }
                f->showProgressDialog();
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
                    MessageBox::error(this, BOX_OK, _("Error"),
                                      _("An error has occurred during the move to trash operation!"));
                    break;
                }

                // If action is cancelled in progress dialog
                if (f->isCancelled())
                {
                    f->hideProgressDialog();
                    MessageBox::error(this, BOX_OK, _("Warning"), _("Move to trash file operation cancelled!"));
                    break;
                }
            }
        }
    }
end:
    getApp()->endWaitCursor();
    delete f;

    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return 1;
}


// Definitively delete files from the file list or the tree list (no trash can)
long SearchPanel::onCmdFileDelete(FXObject*, FXSelector, void*)
{
    int firstitem = 0;
    File* f = NULL;

    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);
    FXbool confirm_del_emptydir = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete_emptydir", true);


    // Items number in the file list
    int num = list->getNumSelectedItems();

    if (num == 0)
    {
        return 0;
    }

    // If exist selected files, use them
    if (num >= 1)
    {
        if (confirm_del)
        {
            FXString message;
            if (num == 1)
            {
                FXString pathname;
                for (int u = 0; u < list->getNumItems(); u++)
                {
                    if (list->isItemSelected(u))
                    {
                        pathname = list->getItemFullPathname(u);
                    }
                }
                if (xf_isdirectory(pathname))
                {
                    message.format(_("Definitively delete folder %s ?"), pathname.text());
                }
                else
                {
                    message.format(_("Definitively delete file %s ?"), pathname.text());
                }
            }
            else
            {
                message.format(_("Definitively delete %s selected items?"), FXStringVal(num).text());
            }
            MessageBox box(this, _("Confirm Delete"), message, bigdeletepermicon,
                           BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return 0;
            }
        }
        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Delete files"), DELETE, num);
        f->create();
        list->setAllowRefresh(false);

        // Overwrite initialisations
        FXbool overwrite = false;
        FXbool overwrite_all = false;
        FXbool skip_all = false;
        FXbool ask_del_empty = true;
        FXbool skip_all_del_emptydir = false;

        // Delete selected files
        FXString filename, pathname;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                // Get index of first selected item
                if (firstitem == 0)
                {
                    firstitem = u;
                }

                // Get file name and path
                filename = list->getItemFilename(u);
                pathname = list->getItemFullPathname(u);

                // File could have already been deleted above in the tree
                if (!xf_existfile(pathname))
                {
                    continue;
                }

                // Confirm empty directory deletion
                if (confirm_del & confirm_del_emptydir & ask_del_empty)
                {
                    if ((xf_isemptydir(pathname) == 0) && !xf_islink(pathname))
                    {
                        if (skip_all_del_emptydir)
                        {
                            continue;
                        }

                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("Folder %s is not empty, delete it anyway?"), pathname.text());
                        OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
                        FXuint answer = dlg->execute(PLACEMENT_OWNER);
                        delete dlg;
                        switch (answer)
                        {
                        // Cancel
                        case 0:
                            goto end;
                            break;

                        // Yes
                        case 1:
                            break;

                        // Yes for all
                        case 2:
                            ask_del_empty = false;
                            break;

                        // Skip
                        case 3:
                            continue;
                            break;

                        // Skip all
                        case 4:
                            skip_all_del_emptydir = true;
                            continue;
                            break;
                        }
                        f->showProgressDialog();
                    }
                }

                // If we don't have permission to write to the file
                if (!xf_iswritable(pathname))
                {
                    // Overwrite dialog if necessary
                    if (!(overwrite_all | skip_all))
                    {
                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("File %s is write-protected, delete it anyway?"), pathname.text());

                        if (num == 1)
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg, OVWBOX_SINGLE_FILE);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            if (answer == 1)
                            {
                                overwrite = true;
                            }
                            else
                            {
                                goto end;
                            }
                        }
                        else
                        {
                            OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
                            FXuint answer = dlg->execute(PLACEMENT_OWNER);
                            delete dlg;
                            switch (answer)
                            {
                            // Cancel
                            case 0:
                                goto end;
                                break;

                            // Yes
                            case 1:
                                overwrite = true;
                                break;

                            // Yes for all
                            case 2:
                                overwrite_all = true;
                                break;

                            // Skip
                            case 3:
                                overwrite = false;
                                break;

                            // Skip all
                            case 4:
                                skip_all = true;
                                break;
                            }
                        }
                    }
                    if ((overwrite | overwrite_all) && !skip_all)
                    {
                        // Definitively remove file or folder
                        f->remove(pathname);
                    }
                    f->showProgressDialog();
                }
                // If we have permission to write
                else
                {
                    // Definitively remove file or folder
                    f->remove(pathname);

                    // If is located at trash location, try to also remove the corresponding trashinfo file if it exists
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (pathname.left(trashfileslocation.length()) == trashfileslocation))
                    {
                        FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + filename + ".trashinfo";
                        unlink(trashinfopathname.text());
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Delete file operation cancelled!"));
                        break;
                    }
                }
            }
        }
end:
        getApp()->endWaitCursor();
        delete f;
    }

    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return 1;
}


// We now really do have the clipboard, keep clipboard content
long SearchPanel::onClipboardGained(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardGained(sender, sel, ptr);
    return 1;
}


// We lost the clipboard
long SearchPanel::onClipboardLost(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardLost(sender, sel, ptr);
    return 1;
}


// Somebody wants our clipboard content
long SearchPanel::onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr)
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


// Copy or cut to clipboard
long SearchPanel::onCmdCopyCut(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard
    clipboard.clear();

    // Clipboard type
    if (FXSELID(sel) == ID_CUT_CLIPBOARD)
    {
        clipboard_type = CUT_CLIPBOARD;
    }
    else
    {
        clipboard_type = COPY_CLIPBOARD;
    }

    // Items number in the file list
    int num = list->getNumSelectedItems();

    if (num == 0)
    {
        return 0;
    }

    // If exist selected files, use them
    if (num >= 1)
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                FXString pathname = list->getItemFullPathname(u);
                clipboard += FXURL::encode(::xf_filetouri(pathname)) + "\n";
            }
        }
    }

    // Remove the last \n of the list, for compatibility with some file managers (e.g. nautilus 2.30.1)
    clipboard.erase(clipboard.length() - 1);

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


// Copy file names (without path) to clipboard
long SearchPanel::onCmdCopyName(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard and set clipboard type
    clipboard.clear();
    clipboard_type = COPYNAME_CLIPBOARD;

    // Items number in the file list
    int num = list->getNumSelectedItems();

    if (num == 0)
    {
        return 0;
    }
    // If exist selected files, use them
    else if (num >= 1)
    {
        // Construct the uri list of files and fill the clipboard with it
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                FXString name = list->getItemText(u).text();
                name = name.section('\t', 0);
                clipboard += FXURL::encode(::xf_filetouri(name)) + "\n";
            }
        }
    }

    // Remove the last \n of the list
    clipboard.erase(clipboard.length() - 1);

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


// Copy/Move/Rename/Symlink file(s)
long SearchPanel::onCmdFileMan(FXObject* sender, FXSelector sel, void*)
{
    int num;
    FXString src, targetdir, target, name, source;

    // Confirmation dialog?
    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);

    // Number of selected items
    num = list->getNumSelectedItems();

    // If no item, return
    if (num <= 0)
    {
        return 0;
    }

    // Obtain the list of source files
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            src += list->getItemFullPathname(u) + "\n";
        }
    }

    // Name and directory of the first source file
    source = src.section('\n', 0);
    name = FXPath::name(source);
    FXString dir = FXPath::directory(source);

    // Initialise target dir name
x:
    targetdir = homedir;
    if (targetdir != ROOTDIR)
    {
        target = targetdir + PATHSEPSTRING;
    }
    else
    {
        target = targetdir;
    }

    // Target dir for the rename command
    if (FXSELID(sel) == ID_FILE_RENAME)
    {
        targetdir = dir;
    }

    // Configure the command, title, message, etc.
    FXIcon* icon = NULL;
    FXString command, title, message;
    if (FXSELID(sel) == ID_FILE_RENAME)
    {
        command = "rename";
        title = _("Rename");
        icon = bigmoveicon;
        if (num == 1)
        {
            message = _("Rename ");
            message += name;
            target = name;
        }
    }
    if (FXSELID(sel) == ID_FILE_COPYTO)
    {
        command = "copy";
        title = _("Copy");
        icon = bigcopyicon;
        if (num == 1)
        {
            message = _("Copy ");
            message += source;
        }
        else
        {
            message.format(_("Copy %s items"), FXStringVal(num).text());
        }
    }
    if (FXSELID(sel) == ID_FILE_MOVETO)
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
            message.format(_("Move %s items"), FXStringVal(num).text());
        }
    }
    if (FXSELID(sel) == ID_FILE_SYMLINK)
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
            message.format(_("Symlink %s items"), FXStringVal(num).text());
        }
    }

    // File operation dialog, if needed
    if (ask_before_copy || (source == target) || (FXSELID(sel) == ID_FILE_COPYTO) || (FXSELID(sel) == ID_FILE_MOVETO) ||
        (FXSELID(sel) == ID_FILE_RENAME) || (FXSELID(sel) == ID_FILE_SYMLINK))
    {
        if (num == 1)
        {
            if (FXSELID(sel) == ID_FILE_RENAME)
            {
                if (operationdialogrename == NULL)
                {
                    operationdialogrename = new InputDialog(this, "", "", title, _("To:"), icon);
                }
                operationdialogrename->setTitle(title);
                operationdialogrename->setIcon(icon);
                operationdialogrename->setMessage(message);
                operationdialogrename->setText(target);

                if (xf_isdirectory(source)) // directory
                {
                    operationdialogrename->selectAll();
                }
                else
                {
                    int pos = target.rfind('.');
                    if (pos <= 0)
                    {
                        operationdialogrename->selectAll(); // no extension or dot file
                    }
                    else
                    {
                        operationdialogrename->setSelection(0, pos);
                    }
                }

                int rc = operationdialogrename->execute(PLACEMENT_CURSOR);
                target = operationdialogrename->getText();

                // Target name contains '/'
                if (target.contains(PATHSEPCHAR))
                {
                    MessageBox::error(this, BOX_OK, _("Error"),
                                      _("Character '/' is not allowed in file or folder names, operation cancelled"));
                    return 0;
                }

                if (!rc)
                {
                    return 0;
                }
            }
            else
            {
                if (operationdialogsingle == NULL)
                {
                    operationdialogsingle = new BrowseInputDialog(this, "", "", title, _("To:"), icon,
                                                                  BROWSE_INPUT_MIXED);
                }
                operationdialogsingle->setTitle(title);
                operationdialogsingle->setIcon(icon);
                operationdialogsingle->setMessage(message);
                operationdialogsingle->setText(target);


                // Select file name without path
                if (FXSELID(sel) == ID_FILE_SYMLINK)
                {
                    int pos = target.rfind(PATHSEPSTRING);
                    if (pos >= 0)
                    {
                        operationdialogsingle->setSelection(pos + 1, target.length());
                    }
                }

                operationdialogsingle->setDirectory(targetdir);
                int rc = operationdialogsingle->execute(PLACEMENT_CURSOR);
                target = operationdialogsingle->getText();
                if (!rc)
                {
                    return 0;
                }
            }
        }
        else // Multiple sources
        {
            if (FXSELID(sel) != ID_FILE_RENAME) // No such dialog for multiple rename
            {
                if (operationdialogmultiple == NULL)
                {
                    operationdialogmultiple = new BrowseInputDialog(this, "", "", title, _("To folder:"), icon,
                                                                    BROWSE_INPUT_FOLDER);
                }
                operationdialogmultiple->setTitle(title);
                operationdialogmultiple->setIcon(icon);
                operationdialogmultiple->setMessage(message);
                operationdialogmultiple->setText(target);
                operationdialogmultiple->CursorEnd();
                operationdialogmultiple->setDirectory(targetdir);
                int rc = operationdialogmultiple->execute(PLACEMENT_CURSOR);
                target = operationdialogmultiple->getText();
                if (!rc)
                {
                    return 0;
                }
            }
        }
    }

    // Nothing entered
    if (target == "")
    {
        MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
        return 0;
    }

    // Except for rename, an absolute path is required
    if ((FXSELID(sel) != ID_FILE_RENAME) && !ISPATHSEP(target[0]))
    {
        MessageBox::warning(this, BOX_OK, _("Warning"), _("You must enter an absolute path!"));
        goto x;
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

    FXString hsourcesize;
    FXulong sourcesize, tstart = 0;

    // One source
    File* f = NULL;
    int ret;
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
            f = new File(this, _("Copy files"), COPY, num);
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
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
            }
        }
        else if (command == "rename")
        {
            f = new File(this, _("Rename Files"), RENAME, num);
            f->create();
            ret = f->rename(source, target);

            // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
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
            f = new File(this, _("Move Files"), MOVE, num);
            f->create();

            // Get total source size and start time
            hsourcesize = f->sourcesize(source, &sourcesize);
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
                FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) + ".trashinfo";
                unlink(trashinfopathname.text());
            }

            // An error has occurred
            if ((ret == 0) && !f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move file operation!"));
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
            }
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK, num);
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
    else if (num > 1)
    {
        // Wait cursor
        getApp()->beginWaitCursor();

        // File object
        if (command == "copy")
        {
            f = new File(this, _("Copy Files"), COPY, num);
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
            f = new File(this, _("Move Files"), MOVE, num);
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
            f = new File(this, _("Symlink"), SYMLINK, num);
            f->create();
        }
        else if (command == "rename")
        {
            getApp()->endWaitCursor();

            // Remove folders from the list
            FXString srcfiles = "";
            int numfiles = 0;
            FXbool warn = true;

            // Loop on the multiple files
            for (int i = 0; i < num; i++)
            {
                source = src.section('\n', i);

                if (xf_isdirectory(source))
                {
                    if (warn)
                    {
                        MessageBox::information(this, BOX_OK, _("Information"),
                        _("Multiple renaming of folders is not supported in Search Panel\n\n=> Folders will be deselected"));
                        warn = false;
                    }

                    continue;
                }
                else
                {
                    srcfiles += source + "\n";
                    numfiles++;
                }
            }

            // Read Rename Dialog geometry
            FXuint width = getApp()->reg().readUnsignedEntry("OPTIONS", "rename_width", 800);
            FXuint height = getApp()->reg().readUnsignedEntry("OPTIONS", "rename_height", 600);

            // Rename dialog
            RenameDialog* renamedialog = new RenameDialog(this, title, srcfiles, numfiles, true, width, height); // Show folder names
            renamedialog->execute(PLACEMENT_OWNER);

            // Rename files
            FXString tgt = renamedialog->getTarget();
            if (tgt != "")
            {
                getApp()->beginWaitCursor();

                f = new File(this, _("Rename Files"), RENAME, num);
                f->create();

                // Loop on the multiple files
                for (int i = 0; i < numfiles; i++)
                {
                    // Individual source and target files
                    source = srcfiles.section('\n', i);
                    target = tgt.section('\n', i);

                    if (target != "" && target != source)
                    {
                        ret = f->rename(source, target);

                        // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
                        // Do it silently and don't report any error if it fails
                        FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                        if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
                        {
                            FXString trashinfopathname = trashinfolocation + PATHSEPSTRING + FXPath::name(source) +
                                                         ".trashinfo";
                            unlink(trashinfopathname.text());
                        }

                        // If action is cancelled in progress dialog
                        if (f->isCancelled())
                        {
                            MessageBox::error(this, BOX_OK, _("Warning"), _("Rename file operation cancelled!"));
                            break;
                        }
                        // An known error has occurred
                        if (ret == -1)
                        {
                            break;
                        }
                        // An unknown error has occurred
                        if (ret == 0)
                        {
                            MessageBox::error(this, BOX_OK, _("Error"),
                                              _("An error has occurred during the rename file operation!"));
                            break;
                        }
                    }
                }

                // Deselect all items
                list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), NULL);

                getApp()->endWaitCursor();
                delete f;
            }

            // Write Rename Dialog geometry
            getApp()->reg().writeUnsignedEntry("OPTIONS", "rename_width", (FXuint)renamedialog->getWidth());
            getApp()->reg().writeUnsignedEntry("OPTIONS", "rename_height", (FXuint)renamedialog->getHeight());
            getApp()->reg().write();

            delete renamedialog;
            return 1;
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }

        list->setAllowRefresh(false);

        // Loop on the multiple files
        for (int i = 0; i < num; i++)
        {
            // Individual source file
            source = src.section('\n', i);

            // File could have already been moved above in the tree
            if (!xf_existfile(source))
            {
                continue;
            }

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

out:
        getApp()->endWaitCursor();
        delete f;
    }

    // Force list refresh
    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return 1;
}


// Go to script directory
long SearchPanel::onCmdGoScriptDir(FXObject* sender, FXSelector sel, void*)
{
    FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;

    if (!xf_existfile(scriptpath))
    {
        // Create the script directory according to the umask
        int mask = umask(0);
        umask(mask);
        errno = 0;
        int ret = xf_mkpath(scriptpath.text(), 511 & ~mask);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s: %s"), scriptpath.text(),
                                  strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s"), scriptpath.text());
            }

            return 0;
        }
    }

    // Change directory in Xfe
    ((XFileExplorer*)mainWindow)->setDirectory(scriptpath);

    // Raise the Xfe window
    ((XFileExplorer*)mainWindow)->raise();
    ((XFileExplorer*)mainWindow)->setFocus();

    return 1;
}


// Clear file list and reset panel status
void SearchPanel::clearItems(void)
{
    statuslabel->setText(_("0 item"));
    list->clearItems();
}


// Update the status bar
long SearchPanel::onUpdStatus(FXObject* sender, FXSelector, void*)
{
    // Update the status bar
    int item = -1;
    FXString status, linkto;
    char size[64];
    FXulong sz = 0;

    FXString hsize = _("0 bytes");
    FXString path = list->getDirectory();
    int num = list->getNumSelectedItems();

    item = list->getCurrentItem();

    // Several items selected
    if (num > 1)
    {
        int nbdirs = 0;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u) && !list->isItemDirectory(u))
            {
                sz += list->getItemFileSize(u);
#if __WORDSIZE == 64
                snprintf(size, sizeof(size), "%lu", sz);
#else
                snprintf(size, sizeof(size), "%llu", sz);
#endif
                hsize = xf_humansize(size);
            }

            if (list->isItemSelected(u) && list->isItemDirectory(u))
            {
                nbdirs++;
            }
        }

        int nbfiles = num - nbdirs;
        if (nbdirs <= 1 && nbfiles <= 1)
        {
            status.format(_("%s in %s selected items (%s folder, %s file)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else if (nbdirs <= 1 && nbfiles > 1)
        {
            status.format(_("%s in %s selected items (%s folder, %s files)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else if (nbdirs > 1 && nbfiles <= 1)
        {
            status.format(_("%s in %s selected items (%s folders, %s file)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
        else
        {
            status.format(_("%s in %s selected items (%s folders, %s files)"), hsize.text(), FXStringVal(num).text(),
                          FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
        }
    }
    else
    {
        // Nothing selected
        if ((num == 0) || (item < 0))
        {
            num = list->getNumItems();
            if (num == 1)
            {
                status = _("1 item");
            }
            else
            {
                int nbdirs = 0;
                for (int u = 0; u < num; u++)
                {
                    if (list->isItemDirectory(u))
                    {
                        nbdirs++;
                    }
                }

                int nbfiles = num - nbdirs;
                status.format(_("%s items (%s folders, %s files)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(),
                              FXStringVal(nbfiles).text());
                if (nbdirs <= 1 && nbfiles <= 1)
                {
                    status.format(_("%s items (%s folder, %s file)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text());
                }
                else if (nbdirs <= 1 && nbfiles > 1)
                {
                    status.format(_("%s items (%s folder, %s files)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text());
                }
                else if (nbdirs > 1 && nbfiles <= 1)
                {
                    status.format(_("%s items (%s folders, %s file)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text());
                }
                else
                {
                    status.format(_("%s items (%s folders, %s files)"), FXStringVal(num).text(),
                                  FXStringVal(nbdirs).text(),
                                  FXStringVal(nbfiles).text());
                }
            }
        }

        // One item selected
        else
        {
            // Item text
            FXString str = list->getItemText(item);

            // Status text
            for (FXuint i = 2; i < nbCols - 1; i++)  // Skip name and directory
            {
                FXString txt = str.section('\t', i);
                if (txt.length() != 0)
                {
                    status += txt + " | ";
                }
            }
            status += str.section('\t', nbCols - 1);
        }
    }

    statuslabel->setText(status);

    return 1;
}


// Update the status of the menu items that should be disabled
// when the number of selected items is not one
long SearchPanel::onUpdSelMult(FXObject* sender, FXSelector sel, void*)
{
    int num;

    num = list->getNumSelectedItems();

    if (num == 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update the file compare menu item
long SearchPanel::onUpdCompare(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is enabled only when two files are selected
    int num;

    num = list->getNumSelectedItems();

    if ((num == 1) || (num == 2))
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
long SearchPanel::onUpdMenu(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is disabled when nothing is selected
    int num;

    num = list->getNumSelectedItems();

    if (num == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    return 1;
}


// Update file delete menu item and toolbar button
long SearchPanel::onUpdFileDelete(FXObject* sender, FXSelector sel, void*)
{
    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    FXbool use_trash_bypass = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_bypass", false);

    if ((!use_trash_can) | use_trash_bypass)
    {
        int num = list->getNumSelectedItems();

        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update move to trash menu item and toolbar button
long SearchPanel::onUpdFileTrash(FXObject* sender, FXSelector sel, void*)
{
    // Disable move to trash menu if we are in trash can
    // or if the trash can directory is selected

    FXbool trashenable = true;
    FXString trashparentdir = trashlocation.rbefore('/');
    FXString curdir = list->getDirectory();

    if (curdir.left(trashlocation.length()) == trashlocation)
    {
        trashenable = false;
    }

    if (curdir == trashparentdir)
    {
        FXString pathname;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemPathname(u);
                if (pathname == trashlocation)
                {
                    trashenable = false;
                }
            }
        }
    }

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    if (use_trash_can && trashenable)
    {
        int num = list->getNumSelectedItems();

        if (num == 0)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update directory usage menu item
long SearchPanel::onUpdDirUsage(FXObject* sender, FXSelector, void*)
{
    // Menu item is enabled only when at least two items are selected
    int num, item;

    num = list->getNumSelectedItems(&item);
    if (num > 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


#if defined(linux)

// Query packages data base
long SearchPanel::onCmdPkgQuery(FXObject* sender, FXSelector sel, void*)
{
    FXString cmd;

    // Name of the current selected file
    FXString file = list->getCurrentFile();

    // Read package format
    FXuint pkg_format = getApp()->reg().readUnsignedEntry("SETTINGS", "package_format", DEB_PKG);

    // Command to perform
    if (pkg_format == DEB_PKG)
    {
        cmd = "dpkg -S " + xf_quote(file);
    }
    else if (pkg_format == RPM_PKG)
    {
        cmd = "rpm -qf " + xf_quote(file);
    }
    else
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("No compatible package manager (rpm or dpkg) found!"));
        return 0;
    }

    // Query command
    cmd += " 2>&1";

    // Wait cursor
    getApp()->beginWaitCursor();

    // Perform the command
    FILE* pcmd = popen(cmd.text(), "r");
    if (!pcmd)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Failed command: %s"), cmd.text());
        return 0;
    }

    // Get command output
    char text[10000] = { 0 };
    FXString buf;
    while (fgets(text, sizeof(text), pcmd))
    {
        buf += text;
    }
    snprintf(text, sizeof(text), "%s", buf.text());

    // Close the stream and display error message if any
    if ((pclose(pcmd) == -1) && (errno != ECHILD)) // ECHILD can be set if the child was caught by sigHarvest
    {
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), "%s", text);
        return 0;
    }
    getApp()->endWaitCursor();

    // Get package name, or detect when the file isn't in a package
    FXString str = text;
    if (pkg_format == DEB_PKG)                                  // DEB based distribution
    {
        int idx = str.find(" ");                                // Split output at first whitespace
        FXString pkgname = str.left(idx - 1);                   // Remove trailing colon
        FXString fname = str.right(str.length() - idx);
        fname.trim();                                           // Remove leading space and trailing newline
        if (xf_strequal(fname.text(), file.text()))             // No other word than the file name
        {
            str = pkgname.text();
        }
        else
        {
            str = "";
        }
    }
    if (pkg_format == RPM_PKG)                  // RPM based distribution
    {
        if (str.find(' ') != -1)                // Space character exists in the string
        {
            str = "";
        }
    }

    // Display the related output message
    FXString message;
    if (str == "")
    {
        message.format(_("File %s does not belong to any package."), file.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }
    else
    {
        message.format(_("File %s belongs to the package: %s"), file.text(), str.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }

    return 1;
}


// Update the package query menu
long SearchPanel::onUpdPkgQuery(FXObject* sender, FXSelector sel, void*)
{
    // Menu item is disabled when multiple selection
    // or when unique selection and the selected item is a directory

    int num;

    num = list->getNumSelectedItems();

    if (num > 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else // num=1
    {
        int item = list->getCurrentItem();
        if ((item >= 0) && list->isItemDirectory(item))
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


#endif
