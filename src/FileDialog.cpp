// File dialog. Taken from the FOX library and slightly modified.

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "FileList.h"
#include "InputDialog.h"
#include "DirHistBox.h"
#include "MessageBox.h"
#include "FileDialog.h"

#define FILELISTMASK     (ICONLIST_EXTENDEDSELECT | ICONLIST_SINGLESELECT | ICONLIST_BROWSESELECT | \
                          ICONLIST_MULTIPLESELECT)
#define FILESTYLEMASK    (ICONLIST_DETAILED | ICONLIST_MINI_ICONS | ICONLIST_BIG_ICONS | ICONLIST_ROWS | \
                          ICONLIST_COLUMNS | ICONLIST_AUTOSIZE)


// Filter history
extern char FilterHistory[FILTER_HIST_SIZE][MAX_PATTERN_SIZE];
extern int FilterNum;


// Map
FXDEFMAP(FileSelector) FileSelectorMap[] =
{
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_ACCEPT, FileSelector::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_FILEFILTER, FileSelector::onCmdFilter),
    FXMAPFUNC(SEL_DOUBLECLICKED, FileSelector::ID_FILELIST, FileSelector::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_CLICKED, FileSelector::ID_FILELIST, FileSelector::onCmdItemClicked),
    FXMAPFUNC(SEL_SELECTED, FileSelector::ID_FILELIST, FileSelector::onCmdItemSelected),
    FXMAPFUNC(SEL_DESELECTED, FileSelector::ID_FILELIST, FileSelector::onCmdItemDeselected),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_DIR_UP, FileSelector::onCmdDirUp),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_DIR_UP, FileSelector::onUpdDirUp),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_DIR_BACK, FileSelector::onCmdDirBack),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_DIR_BACK, FileSelector::onUpdDirBack),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_DIR_FORWARD, FileSelector::onCmdDirForward),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_DIR_FORWARD, FileSelector::onUpdDirForward),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_DIR_BACK_HIST, FileSelector::onCmdDirBackHist),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_DIR_BACK_HIST, FileSelector::onUpdDirBackHist),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_DIR_FORWARD_HIST, FileSelector::onCmdDirForwardHist),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_DIR_FORWARD_HIST, FileSelector::onUpdDirForwardHist),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_HOME, FileSelector::onCmdHome),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_NEWDIR, FileSelector::onCmdNewDir),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_NEWFILE, FileSelector::onCmdNewFile),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_WORK, FileSelector::onCmdWork),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, FileSelector::ID_FILELIST, FileSelector::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_POPUP_MENU, FileSelector::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, FileSelector::ID_FILTER, FileSelector::onCmdItemFilter),
    FXMAPFUNC(SEL_UPDATE, FileSelector::ID_FILTER, FileSelector::onUpdItemFilter),
    FXMAPFUNC(SEL_KEYPRESS, 0, FileSelector::onCmdKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, FileSelector::onCmdKeyRelease),
};


// Implementation
FXIMPLEMENT(FileSelector, FXPacker, FileSelectorMap, ARRAYNUMBER(FileSelectorMap))


// Default pattern
static const char allfiles[] = "All Files (*)";

// Construct
FileSelector::FileSelector(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, int x, int y, int w, int h) :
    FXPacker(p, opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    FXAccelTable* table = getShell()->getAccelTable();

    target = tgt;
    message = sel;

    // Global container
    FXVerticalFrame* cont = new FXVerticalFrame(this, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // Container for the action buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(cont, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE,
                                                       0, 0, 0, 0, 5, 5, 5, 5, 0, 0);

    // Container for the path linker
    FXHorizontalFrame* pathframe = new FXHorizontalFrame(cont, LAYOUT_FILL_X | FRAME_NONE,
                                                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // File list
    FXuint options;
    FXbool smoothscroll = getApp()->reg().readUnsignedEntry("SETTINGS", "smooth_scroll", true);
    if (smoothscroll)
    {
        options = ICONLIST_MINI_ICONS | ICONLIST_BROWSESELECT | LAYOUT_FILL_X | LAYOUT_FILL_Y;
    }
    else
    {
        options = ICONLIST_MINI_ICONS | ICONLIST_BROWSESELECT | LAYOUT_FILL_X | LAYOUT_FILL_Y | SCROLLERS_DONT_TRACK;
    }
    FXbool showthumbnails = getApp()->reg().readUnsignedEntry("FILEDIALOG", "showthumbnails", false);


    // Read file list columns order and shown status
    FXuint idCol[NMAX_COLS] = { 0 };
    FXbool colShown[FileList::ID_COL_NAME + NMAX_COLS] = { 0 };
    FXuint nbCols = 0;

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

    list = new FileList(this, cont, idCol, nbCols, this, ID_FILELIST, showthumbnails, 0, 0, options);

    // Set list colors and columns size for detailed mode
    list->setTextColor(getApp()->reg().readColorEntry("SETTINGS", "listforecolor", FXRGB(0, 0, 0)));
    list->setBackColor(getApp()->reg().readColorEntry("SETTINGS", "listbackcolor", FXRGB(255, 255, 255)));

    // Set list headers size
    for (FXuint i = 0; i < nbCols; i++)
    {
        FXuint size = 0;
        FXuint id = list->getHeaderId(i);

        switch (id)
        {
        case FileList::ID_COL_NAME:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "name_size", MIN_NAME_SIZE);
            break;

        case FileList::ID_COL_SIZE:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "size_size", 60);
            break;

        case FileList::ID_COL_TYPE:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "type_size", 100);
            break;

        case FileList::ID_COL_EXT:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "ext_size", 100);
            break;

        case FileList::ID_COL_DATE:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "date_size", 150);
            break;

        case FileList::ID_COL_USER:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "user_size", 50);
            break;

        case FileList::ID_COL_GROUP:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "group_size", 50);
            break;

        case FileList::ID_COL_PERMS:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "perms_size", 100);
            break;

        case FileList::ID_COL_LINK:
            size = getApp()->reg().readUnsignedEntry("FILEDIALOG", "link_size", 100);
            break;
        }

        list->setHeaderSize(i, size);
    }

    // Set file selector options
    FXuint liststyle = getApp()->reg().readUnsignedEntry("FILEDIALOG", "liststyle",
                                                         ICONLIST_MINI_ICONS | ICONLIST_AUTOSIZE);
    FXbool hiddenfiles = getApp()->reg().readUnsignedEntry("FILEDIALOG", "hiddenfiles", false);
    showHiddenFiles(hiddenfiles);
    setFileBoxStyle(liststyle);
    attentioncolor = getApp()->reg().readColorEntry("SETTINGS", "attentioncolor", FXRGB(255, 0, 0));

    // Entry buttons
    FXMatrix* fields = new FXMatrix(cont, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    namelabel = new FXLabel(fields, _("&File Name:"), NULL, JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    filename = new FXTextField(fields, 25, this, ID_ACCEPT, TEXTFIELD_ENTER_ONLY |
                               LAYOUT_FILL_COLUMN | LAYOUT_FILL_X | FRAME_GROOVE);
    new FXButton(fields, _("&OK"), NULL, this, ID_ACCEPT, BUTTON_DEFAULT | FRAME_GROOVE |
                 LAYOUT_FILL_X, 0, 0, 0, 0, 20, 20);
    accept = new FXButton(buttons, FXString::null, NULL, NULL, 0, LAYOUT_FIX_X | LAYOUT_FIX_Y |
                          LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, 0, 0, 0, 0, 0, 0);
    new FXLabel(fields, _("File F&ilter:"), NULL, JUSTIFY_LEFT | LAYOUT_CENTER_Y);
    FXHorizontalFrame* filterframe = new FXHorizontalFrame(fields, LAYOUT_FILL_COLUMN |
                                                           LAYOUT_FILL_X | LAYOUT_FILL_Y);
    filefilter = new ComboBox(filterframe, 10, false, false, this, ID_FILEFILTER, COMBOBOX_STATIC | LAYOUT_FILL_X);
    filefilter->setNumVisible(4);

    readonly = new FXCheckButton(filterframe, _("Read Only") + FXString(" "), NULL, 0, ICON_BEFORE_TEXT |
                                 JUSTIFY_LEFT | LAYOUT_CENTER_Y);
    cancel = new FXButton(fields, _("&Cancel"), NULL, NULL, 0, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_GROOVE |
                          LAYOUT_FILL_X, 0, 0, 0, 0, 20, 20);

    // Action buttons
    FXString key;
    FXHotKey hotkey;
    FXButton* btn;
    FXToggleButton* tglbtn;

    new FXFrame(buttons, LAYOUT_FIX_WIDTH, 0, 0, 4, 1);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_back", "Ctrl-Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Previous Folder") + PARS(key), minidirbackicon, this, ID_DIR_BACK,
                       BUTTON_TOOLBAR | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    btnbackhist = new FXArrowButton(buttons, this, ID_DIR_BACK_HIST,
                                    LAYOUT_FILL_Y | FRAME_GROOVE | ARROW_DOWN | ARROW_TOOLBAR);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_forward", "Shift-Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Next Folder") + PARS(key), minidirforwardicon, this, ID_DIR_FORWARD,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    btnforwardhist = new FXArrowButton(buttons, this, ID_DIR_FORWARD_HIST,
                                       LAYOUT_FILL_Y | FRAME_GROOVE | ARROW_DOWN | ARROW_TOOLBAR);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Parent Folder") + PARS(key), minidirupicon, this, ID_DIR_UP,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    btn = new FXButton(buttons, TAB + _("Go to Home Folder") + PARS(key), minihomeicon, this, ID_HOME,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_work", "Shift-F2");
    btn = new FXButton(buttons, TAB + _("Go to Working Folder") + PARS(key), miniworkicon, this, ID_WORK,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_folder", "F7");
    btn = new FXButton(buttons, TAB + _("New Folder") + PARS(key), minifoldernewicon, this, ID_NEWDIR,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    btn = new FXButton(buttons, TAB + _("Big Icon List") + PARS(key), minibigiconsicon, list,
                       FileList::ID_SHOW_BIG_ICONS, BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3,
                       3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    btn = new FXButton(buttons, TAB + _("Small Icon List") + PARS(key), minismalliconsicon, list,
                       FileList::ID_SHOW_MINI_ICONS, BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3,
                       3, 3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    btn = new FXButton(buttons, TAB + _("Detailed File List") + PARS(key), minidetailsicon, list,
                       FileList::ID_SHOW_DETAILS, BUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3,
                       3);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    tglbtn = new FXToggleButton(buttons, TAB + _("Show hidden files") + PARS(key),
                                TAB + _("Hide Hidden Files") + PARS(key),
                                minishowhiddenicon, minihidehiddenicon, list, FileList::ID_TOGGLE_HIDDEN,
                                TOGGLEBUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    tglbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    tglbtn = new FXToggleButton(buttons, TAB + _("Show Thumbnails") + PARS(key), TAB + _("Hide thumbnails") + PARS(key),
                                minishowthumbicon, minihidethumbicon, list, FileList::ID_TOGGLE_THUMBNAILS,
                                TOGGLEBUTTON_TOOLBAR | LAYOUT_CENTER_Y | FRAME_GROOVE, 0, 0, 0, 0, 3, 3, 3, 3);
    hotkey = xf_parseaccel(key);
    tglbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "filter", "Ctrl-D");
    btn = new FXButton(buttons, TAB + _("Filter") + PARS(key), minifiltericon, this,
                       FileSelector::ID_FILTER,
                       BUTTON_TOOLBAR | LAYOUT_LEFT | LAYOUT_CENTER_Y | ICON_BEFORE_TEXT | FRAME_NONE);

    filterlabel = new FXLabel(buttons, "", NULL, JUSTIFY_LEFT | LAYOUT_LEFT);
    closefilter = new FXButton(buttons, TAB + _("Close Filter"), miniclosefiltericon, this->list,
                               FileList::ID_CLOSE_FILTER,
                               BUTTON_TOOLBAR | LAYOUT_LEFT | LAYOUT_CENTER_Y | ICON_BEFORE_TEXT | FRAME_NONE);

    // Path text
    pathtext = new TextLabel(pathframe, 0, this, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    pathtext->setBackColor(getApp()->getBaseColor());

    // Path linker
    pathlink = new PathLinker(pathframe, list, NULL, LAYOUT_FILL_X);

    readonly->hide();
    if (table)
    {
        FXString key;
        FXHotKey hotkey;

        key = getApp()->reg().readStringEntry("KEYBINDINGS", "select_all", "Ctrl-A");
        hotkey = xf_parseaccel(key);
        table->addAccel(hotkey, list, FXSEL(SEL_COMMAND, FileList::ID_SELECT_ALL));

        key = getApp()->reg().readStringEntry("KEYBINDINGS", "deselect_all", "Ctrl-Z");
        hotkey = xf_parseaccel(key);
        table->addAccel(hotkey, list, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL));

        key = getApp()->reg().readStringEntry("KEYBINDINGS", "invert_selection", "Ctrl-I");
        hotkey = xf_parseaccel(key);
        table->addAccel(hotkey, list, FXSEL(SEL_COMMAND, FileList::ID_SELECT_INVERSE));

        key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_file", "Ctrl-N");
        hotkey = xf_parseaccel(key);
        table->addAccel(hotkey, this, FXSEL(SEL_COMMAND, ID_NEWFILE));
    }

    setSelectMode(SELECT_FILE_ANY);                             // For backward compatibility, this HAS to be the default!
    setPatternList(allfiles);
    setDirectory(FXSystem::getCurrentDirectory());              // Update file list
    pathlink->setPath(FXSystem::getCurrentDirectory());         // Update path linker
    pathtext->setText(FXSystem::getCurrentDirectory());         // Update path text

    list->setFocus();
    accept->hide();

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);

    // Change default cursor if single click navigation
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }
}


// Create X window
void FileSelector::create()
{
    // Display or hide path linker
    FXbool show_pathlink = getApp()->reg().readUnsignedEntry("SETTINGS", "show_pathlinker", true);

    if (show_pathlink)
    {
        pathtext->hide();
        pathlink->show();
    }
    else
    {
        pathlink->hide();
        pathtext->show();
    }
    FXPacker::create();
}


// Double-clicked item in file list
long FileSelector::onCmdItemDoubleClicked(FXObject*, FXSelector, void* ptr)
{
    FXlong index = (FXlong)ptr;

    if (index < 0)
    {
        return 1;
    }

    // If directory, open the directory
    if (list->isItemDirectory(index))
    {
        FXString pathname = list->getItemPathname(index);

        // Does not have access
        if (!xf_isreadexecutable(pathname))
        {
            MessageBox::error(this, BOX_OK, _("Error"), _(" Permission to: %s denied."), pathname.text());
            return 0;
        }

        setDirectory(pathname);
        pathlink->setPath(pathname);
        pathtext->setText(pathname);
        return 1;
    }

    // Only return if we wanted a file
    if ((selectmode != SELECT_FILE_DIRECTORY) && (selectmode != SELECT_FILE_MIXED))
    {
        if (list->isItemFile(index))
        {
            FXObject* tgt = accept->getTarget();
            FXSelector sel = accept->getSelector();
            if (tgt)
            {
                tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
            }
        }
    }
    return 1;
}


// Single clicked item in file list
long FileSelector::onCmdItemClicked(FXObject*, FXSelector, void* ptr)
{
    if (single_click != SINGLE_CLICK_NONE)
    {
        FXlong index = (FXlong)ptr;
        if (index < 0)
        {
            return 1;
        }

        // In detailed mode, avoid single click when mouse cursor is not over the first column
        int x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        FXbool allow = true;
        if (!(list->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
            ((x - list->getXPosition()) > list->getHeaderSize(0)))
        {
            allow = false;
        }

        // Single click with control or shift
        if (state & (CONTROLMASK | SHIFTMASK))
        {
            return 1;
        }
        // Single click without control or shift
        else
        {
            // If directory, open the directory
            if (list->isItemDirectory(index) && allow)
            {
                FXString pathname = list->getItemPathname(index);

                // Does not have access
                if (!xf_isreadexecutable(pathname))
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _(" Permission to: %s denied."), pathname.text());
                    return 0;
                }
                setDirectory(pathname);
                pathlink->setPath(pathname);
                pathtext->setText(pathname);
                return 1;
            }
            else if ((single_click == SINGLE_CLICK_DIR_FILE) && list->isItemFile(index) && allow)
            {
                FXObject* tgt = accept->getTarget();
                FXSelector sel = accept->getSelector();
                if (tgt)
                {
                    tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                }
            }
        }
    }
    return 1;
}


// Change in items which are selected
long FileSelector::onCmdItemSelected(FXObject*, FXSelector, void* ptr)
{
    FXlong index = (FXlong)ptr;
    FXString text, file;

    if (selectmode == SELECT_FILE_MULTIPLE)
    {
        for (int i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemFile(i) && list->isItemSelected(i))
            {
                if (!text.empty())
                {
                    text += ' ';
                }
                text += xf_quote(list->getItemFilename(i));
            }
        }
        filename->setText(text);
    }
    else if (selectmode == SELECT_FILE_MULTIPLE_ALL)
    {
        for (int i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
            {
                if (!text.empty())
                {
                    text += ' ';
                }
                text += xf_quote(list->getItemFilename(i));
            }
        }
        filename->setText(text);
    }
    else if (selectmode == SELECT_FILE_DIRECTORY)
    {
        if (list->isItemDirectory(index))
        {
            if (list->getItemFilename(index) != "..")
            {
                text = list->getItemFilename(index);
                filename->setText(text);
            }
        }
    }
    // Mode added to select both directories and files
    else if (selectmode == SELECT_FILE_MIXED)
    {
        if (list->getItemFilename(index) != "..")
        {
            text = list->getItemFilename(index);
            filename->setText(text);
        }
    }
    else
    {
        if (list->isItemFile(index))
        {
            text = list->getItemFilename(index);
            filename->setText(text);
        }
    }
    return 1;
}


// Change in items which are deselected
long FileSelector::onCmdItemDeselected(FXObject*, FXSelector, void*)
{
    FXString text, file;

    if (selectmode == SELECT_FILE_MULTIPLE)
    {
        for (int i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemFile(i) && list->isItemSelected(i))
            {
                if (!text.empty())
                {
                    text += ' ';
                }
                text += xf_quote(list->getItemFilename(i));
            }
        }
        filename->setText(text);
    }
    else if (selectmode == SELECT_FILE_MULTIPLE_ALL)
    {
        for (int i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
            {
                if (!text.empty())
                {
                    text += ' ';
                }
                text += xf_quote(list->getItemFilename(i));
            }
        }
        filename->setText(text);
    }
    return 1;
}


// Hit the accept button or enter in text field
long FileSelector::onCmdAccept(FXObject*, FXSelector, void*)
{
    FXSelector sel = accept->getSelector();
    FXObject* tgt = accept->getTarget();

    // Get (first) filename
    FXString path = getFilename();

    // If filename is empty, we get the current directory
    if (path.empty())
    {
        path = list->getDirectory();
        filename->setText(path);
    }

    // Only do something if a selection was made
    if (!path.empty())
    {
        // Is directory?
        if (xf_isdirectory(path))
        {
            // In directory mode:- we got our answer!
            if ((selectmode == SELECT_FILE_DIRECTORY) || (selectmode == SELECT_FILE_MULTIPLE_ALL) ||
                (selectmode == SELECT_FILE_MIXED))
            {
                if (tgt)
                {
                    tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                }
                return 1;
            }

            // Hop over to that directory
            list->setDirectory(path);
            pathlink->setPath(list->getDirectory());
            pathtext->setText(list->getDirectory());
            filename->setText(FXString::null);
            return 1;
        }

        // Get directory part of path
        FXString dir = FXPath::directory(path);

        // In file mode, directory part of path should exist
        if (xf_isdirectory(dir))
        {
            // In any mode, existing directory part is good enough
            if (selectmode == SELECT_FILE_ANY)
            {
                if (tgt)
                {
                    tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                }
                return 1;
            }
            // In existing mode, the whole filename must exist and be a file
            else if (selectmode == SELECT_FILE_EXISTING)
            {
                if (xf_isfile(path))
                {
                    if (tgt)
                    {
                        tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                    }
                    return 1;
                }
            }
            // In multiple mode, return if all selected files exist
            else if (selectmode == SELECT_FILE_MULTIPLE)
            {
                for (int i = 0; i < list->getNumItems(); i++)
                {
                    if (list->isItemSelected(i) && list->isItemFile(i))
                    {
                        if (tgt)
                        {
                            tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                        }
                        return 1;
                    }
                }
            }
            // Multiple files and/or directories
            else
            {
                for (int i = 0; i < list->getNumItems(); i++)
                {
                    if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
                    {
                        if (tgt)
                        {
                            tgt->handle(accept, FXSEL(SEL_COMMAND, sel), (void*)1);
                        }
                        return 1;
                    }
                }
            }
        }

        // Go up to the lowest directory which still exists
        while (!FXPath::isTopDirectory(dir) && !xf_isdirectory(dir))
        {
            dir = FXPath::upLevel(dir);
        }

        // Switch as far as we could go
        list->setDirectory(dir);
        pathlink->setPath(list->getDirectory());
        pathtext->setText(list->getDirectory());

        // Put the tail end back for further editing
        if (ISPATHSEP(path[dir.length()]))
        {
            path.erase(0, dir.length() + 1);
        }
        else
        {
            path.erase(0, dir.length());
        }

        // Replace text box with new stuff
        filename->setText(path);
        filename->selectAll();
    }

    return 1;
}


// User clicked up directory button
long FileSelector::onCmdDirUp(FXObject*, FXSelector, void*)
{
    setDirectory(FXPath::upLevel(list->getDirectory()));
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());
    return 1;
}


// Can we still go up
long FileSelector::onUpdDirUp(FXObject* sender, FXSelector, void*)
{
    if (FXPath::isTopDirectory(list->getDirectory()))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL);
    }
    return 1;
}


// Directory back
long FileSelector::onCmdDirBack(FXObject*, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;
    forwardhist = list->forwardhist;

    // Get the previous directory
    item = backhist->getFirst();
    if (item)
    {
        pathname = backhist->getString(item);
    }

    // Update the history
    backhist->removeFirstItem();
    forwardhist->insertFirstItem(list->getDirectory());

    // Go to to the previous directory
    list->setDirectory(pathname, false);
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());

    return 1;
}


// Update directory back
long FileSelector::onUpdDirBack(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;

    // Gray out the button if no item in history
    if (backhist->getNumItems() == 0)
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
long FileSelector::onCmdDirForward(FXObject*, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;
    forwardhist = list->forwardhist;

    // Get the next directory
    item = forwardhist->getFirst();
    if (item)
    {
        pathname = forwardhist->getString(item);
    }

    // Update the history
    forwardhist->removeFirstItem();
    backhist->insertFirstItem(list->getDirectory());

    // Go to to the previous directory
    list->setDirectory(pathname, false);
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());

    return 1;
}


// Update directory forward
long FileSelector::onUpdDirForward(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* forwardhist;
    FXString pathname;

    // Get the filelist history
    forwardhist = list->forwardhist;

    // Gray out the button if no item in history
    if (forwardhist->getNumItems() == 0)
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
long FileSelector::onCmdDirBackHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;
    forwardhist = list->forwardhist;

    // Get all string items and display them in a list box
    int num = backhist->getNumItems();
    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = backhist->getFirst();
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = backhist->getString(item);
                dirs[i] = str;
                strlist = strlist + str + "\n";
                item = backhist->getNext(item);
            }
        }

        // Display list box
        FXWindow* owner = this->getOwner();
        int pos = DirHistBox::box(btnbackhist, DECOR_NONE, strlist, owner->getX() + 245, owner->getY() + 37);

        // If an item was selected
        if (pos != -1)
        {
            // Update back history
            if (pos == num - 1)
            {
                backhist->removeAllItems();
            }
            else
            {
                item = backhist->getItemAtPos(pos + 1);
                backhist->removeAllItemsBefore(item);
            }

            // Update forward history
            forwardhist->insertFirstItem(list->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    forwardhist->insertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            list->setDirectory(pathname, false);
            pathlink->setPath(list->getDirectory());
            pathtext->setText(list->getDirectory());
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory back
long FileSelector::onUpdDirBackHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;

    // Gray out the button if no item in history
    if (backhist->getNumItems() == 0)
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
long FileSelector::onCmdDirForwardHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = list->backhist;
    forwardhist = list->forwardhist;

    // Get all string items and display them in a list box
    int num = forwardhist->getNumItems();
    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = forwardhist->getFirst();
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = forwardhist->getString(item);
                dirs[i] = str;
                strlist = strlist + str + "\n";
                item = forwardhist->getNext(item);
            }
        }

        // Display list box
        FXWindow* owner = this->getOwner();
        int pos = DirHistBox::box(btnforwardhist, DECOR_NONE, strlist, owner->getX() + 285, owner->getY() + 37);

        // If an item was selected
        if (pos != -1)
        {
            // Update forward history
            if (pos == num - 1)
            {
                forwardhist->removeAllItems();
            }
            else
            {
                item = forwardhist->getItemAtPos(pos + 1);
                forwardhist->removeAllItemsBefore(item);
            }

            // Update back history
            backhist->insertFirstItem(list->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    backhist->insertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            list->setDirectory(pathname, false);
            pathlink->setPath(list->getDirectory());
            pathtext->setText(list->getDirectory());
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory forward
long FileSelector::onUpdDirForwardHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* forwardhist;
    FXString pathname;

    // Get the filelist history
    forwardhist = list->forwardhist;

    // Gray out the button if no item in history
    if (forwardhist->getNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Back to home directory
long FileSelector::onCmdHome(FXObject*, FXSelector, void*)
{
    setDirectory(FXSystem::getHomeDirectory());
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());
    return 1;
}


// Create new directory
long FileSelector::onCmdNewDir(FXObject*, FXSelector, void*)
{
    // Focus on current list
    list->setFocus();

    FXString dirname = "";
    FXString dirpath = list->getDirectory();
    if (dirpath != ROOTDIR)
    {
        dirpath += PATHSEPSTRING;
    }

    InputDialog* dialog = new InputDialog(this, dirname, _("Create new folder..."), _("New Folder"), "",
                                          bigfoldernewicon);
    dialog->CursorEnd();
    if (dialog->execute(PLACEMENT_CURSOR))
    {
        if (dialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Folder name is empty, operation cancelled"));
            delete dialog;
            return 0;
        }

        // Directory name contains '/'
        if (dialog->getText().contains(PATHSEPCHAR))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"),
                                _("The / character is not allowed in folder names, operation cancelled"));
            delete dialog;
            return 0;
        }

        dirname = dirpath + dialog->getText();
        if (dirname != dirpath)
        {
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
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't create folder %s: %s", dirname.text(),
                                      strerror(errcode));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't create folder %s", dirname.text());
                }
                delete dialog;
                return 0;
            }
        }
    }
    delete dialog;

    return 1;
}


// Create new file
long FileSelector::onCmdNewFile(FXObject*, FXSelector, void*)
{
    FXString filename = "";

    // Focus on current list
    list->setFocus();

    FXString pathname = list->getDirectory();
    if (pathname != ROOTDIR)
    {
        pathname += PATHSEPSTRING;
    }

    InputDialog* dialog = new InputDialog(this, filename, _("Create new file..."), _("New File"), "", bignewfileicon);
    dialog->CursorEnd();

    // Accept was pressed
    if (dialog->execute(PLACEMENT_CURSOR))
    {
        if (dialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            delete dialog;
            return 0;
        }

        // File name contains '/'
        if (dialog->getText().contains(PATHSEPCHAR))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"),
                                _("The / character is not allowed in file names, operation cancelled"));
            delete dialog;
            return 0;
        }

        filename = pathname + dialog->getText();
        FILE* file;
        if (filename != pathname)
        {
            // Test some error conditions
            if (xf_existfile(filename))
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("File or folder %s already exists"), filename.text());
                delete dialog;
                return 0;
            }
            // Create the new file
            errno = 0;
            if (!(file = fopen(filename.text(), "w+")) || fclose(file))
            {
                if (errno)
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't create file %s: %s", filename.text(),
                                      strerror(errno));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't create file %s", filename.text());
                }
                delete dialog;
                return 0;
            }
            // Change the file permissions according to the current umask
            int mask;
            mask = umask(0);
            umask(mask);
            errno = 0;
            if (chmod(filename.text(), 438 & ~mask) != 0)
            {
                if (errno)
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't set permissions in %s: %s",
                                      filename.text(), strerror(errno));
                }
                else
                {
                    MessageBox::error(getApp(), BOX_OK_SU, _("Error"), "Can't set permissions in %s", filename.text());
                }
                delete dialog;
                return 0;
            }
        }
    }

    delete dialog;
    return 1;
}


// Back to current working directory
long FileSelector::onCmdWork(FXObject*, FXSelector, void*)
{
    setDirectory(FXSystem::getCurrentDirectory());
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());
    return 1;
}


// Strip pattern from text if present
FXString FileSelector::patternFromText(const FXString& pattern)
{
    int beg, end;

    end = pattern.rfind(')');     // Search from the end so we can allow ( ) in the pattern name itself
    beg = pattern.rfind('(', end - 1);
    if ((0 <= beg) && (beg < end))
    {
        return pattern.mid(beg + 1, end - beg - 1);
    }
    return pattern;
}


// Return the first extension "ext1" found in the pattern if the
// pattern is of the form "*.ext1,*.ext2,..." or the empty string
// if the pattern contains other wildcard combinations.
FXString FileSelector::extensionFromPattern(const FXString& pattern)
{
    int beg, end, c;

    beg = 0;
    if (pattern[beg] == '*')
    {
        beg++;
        if (pattern[beg] == '.')
        {
            beg++;
            end = beg;
            while ((c = pattern[end]) != '\0' && c != ',' && c != '|')
            {
                if ((c == '*') || (c == '?') || (c == '[') || (c == ']') || (c == '^') || (c == '!'))
                {
                    return FXString::null;
                }
                end++;
            }
            return pattern.mid(beg, end - beg);
        }
    }
    return FXString::null;
}


// Filter dialog
long FileSelector::onCmdItemFilter(FXObject* sender, FXSelector sel, void*)
{
    if (FilterNum == 0)
    {
        xf_strlcpy(FilterHistory[FilterNum], "*", 2);
        FilterNum++;
    }

    int i;
    FXString pat = list->getPattern();

    if (filterdialog == NULL)
    {
        filterdialog = new HistInputDialog(this, pat, _("Show:"), _("Filter"), "", bigfiltericon, false, true,
                                           HIST_INPUT_FILE, true, _("Also apply to folders"));
    }

    filterdialog->clearItems();
    filterdialog->CursorEnd();

    for (int i = 0; i < FilterNum; i++)
    {
        filterdialog->appendItem(FilterHistory[i]);
    }

    filterdialog->sortItems();
    filterdialog->setText(pat);

    // Set filter type
    if (filterdialog->execute() && ((pat = filterdialog->getText()) != ""))
    {
        // Set filter folders option
        list->setFilterFolders(filterdialog->getOption());

        // Change file list pattern
        list->setPattern(pat);

        FXbool newstr = true;
        for (i = 0; i < FilterNum; i++)
        {
            if (xf_strequal(FilterHistory[i], pat.text()))
            {
                newstr = false;
                break;
            }
        }
        // Append new string to the list bottom
        if (newstr && (FilterNum < FILTER_HIST_SIZE))
        {
            xf_strlcpy(FilterHistory[FilterNum], pat.text(), pat.length() + 1);
            FilterNum++;
        }
    }

    list->onCmdRefresh(0, 0, 0);
    list->setFocus();

    // If list has been cleared, set history size to zero
    if (filterdialog->getHistorySize() == 0)
    {
        FilterNum = 0;
    }

    return 1;
}


// Update item filter
long FileSelector::onUpdItemFilter(FXObject* sender, FXSelector sel, void* ptr)
{
    if ((list->getPattern() != "*") && (list->getPattern() != "*.*"))
    {
        closefilter->show();

        FXString str;
        if (list->getFilterFolders())
        {
            str.format(_(" - Filter: %s (files / folders)"), list->getPattern().text());
        }
        else
        {
            str.format(_(" - Filter: %s"), list->getPattern().text());
        }
        filterlabel->setText(str);
        filterlabel->setTextColor(attentioncolor);
    }
    else
    {
        closefilter->hide();
        filterlabel->setText("");
    }

    return 1;
}


// Change the pattern; change the filename to the suggested extension
long FileSelector::onCmdFilter(FXObject*, FXSelector, void* ptr)
{
    FXString pat = patternFromText((char*)ptr);

    list->setPattern(pat);
    if (selectmode == SELECT_FILE_ANY)
    {
        FXString ext = extensionFromPattern(pat);
        if (!ext.empty())
        {
            FXString name = FXPath::stripExtension(filename->getText());
            if (!name.empty())
            {
                filename->setText(name + "." + ext);
            }
        }
    }
    return 1;
}


// Set directory
void FileSelector::setDirectory(const FXString& path)
{
    FXString abspath = FXPath::absolute(path);

    list->setDirectory(abspath);
    pathlink->setPath(list->getDirectory());
    pathtext->setText(list->getDirectory());

    if (selectmode != SELECT_FILE_ANY)
    {
        filename->setText(FXString::null);
    }
}


// Get directory
FXString FileSelector::getDirectory() const
{
    return list->getDirectory();
}


// Set file name
void FileSelector::setFilename(const FXString& path)
{
    FXString abspath = FXPath::absolute(path);

    list->setCurrentFile(abspath);
    pathlink->setPath(FXPath::directory(abspath));
    pathtext->setText(FXPath::directory(abspath));
    filename->setText(FXPath::name(abspath));
}


// Get complete path + filename
FXString FileSelector::getFilename() const
{
    int i;

    if (selectmode == SELECT_FILE_MULTIPLE_ALL)
    {
        for (i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
            {
                return FXPath::absolute(list->getDirectory(), list->getItemFilename(i));
            }
        }
    }
    else if (selectmode == SELECT_FILE_MULTIPLE)
    {
        for (i = 0; i < list->getNumItems(); i++)
        {
            if (list->isItemSelected(i) && list->isItemFile(i))
            {
                return FXPath::absolute(list->getDirectory(), list->getItemFilename(i));
            }
        }
    }
    else
    {
        if (!filename->getText().empty())
        {
            return FXPath::absolute(list->getDirectory(), filename->getText());
        }
    }
    return FXString::null;
}


// Return empty-string terminated list of selected file names, or NULL
FXString* FileSelector::getFilenames() const
{
    FXString* files = NULL;
    int i, n;

    if (list->getNumItems())
    {
        if (selectmode == SELECT_FILE_MULTIPLE_ALL)
        {
            for (i = n = 0; i < list->getNumItems(); i++)
            {
                if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
                {
                    n++;
                }
            }
            if (n > 0)
            {
                // Limit the number of files to open
                n = FXMAX(n, MAX_OPEN_FILES);

                files = new FXString [n + 1];
                for (i = n = 0; i < list->getNumItems(); i++)
                {
                    if (list->isItemSelected(i) && (list->getItemFilename(i) != ".."))
                    {
                        files[n++] = list->getItemPathname(i);
                    }
                }
                files[n] = FXString::null;
            }
        }
        else
        {
            for (i = n = 0; i < list->getNumItems(); i++)
            {
                if (list->isItemSelected(i) && list->isItemFile(i))
                {
                    n++;
                }
            }
            if (n > 0)
            {
                // Limit the number of files to open
                n = FXMAX(n, MAX_OPEN_FILES);

                files = new FXString [n + 1];
                for (i = n = 0; i < list->getNumItems(); i++)
                {
                    if (list->isItemSelected(i) && list->isItemFile(i))
                    {
                        files[n++] = list->getItemPathname(i);
                    }
                }
                files[n] = FXString::null;
            }
        }
    }

    return files;
}


// Set bunch of patterns
void FileSelector::setPatternList(const char** ptrns)
{
    filefilter->clearItems();
    if (ptrns)
    {
        while (ptrns[0] && ptrns[1])
        {
            filefilter->appendItem(FXStringFormat("%s (%s)", ptrns[0], ptrns[1]));
            ptrns += 2;
        }
    }
    if (!filefilter->getNumItems())
    {
        filefilter->appendItem(allfiles);
    }
    setCurrentPattern(0);
}


// Change patterns, each pattern separated by newline
void FileSelector::setPatternList(const FXString& patterns)
{
    FXString pat;
    int i;

    filefilter->clearItems();
    for (i = 0; !(pat = patterns.section('\n', i)).empty(); i++)
    {
        filefilter->appendItem(pat);
    }
    if (!filefilter->getNumItems())
    {
        filefilter->appendItem(allfiles);
    }
    setCurrentPattern(0);
}


// Return list of patterns
FXString FileSelector::getPatternList() const
{
    FXString pat;
    int i;

    for (i = 0; i < filefilter->getNumItems(); i++)
    {
        if (!pat.empty())
        {
            pat += '\n';
        }
        pat += filefilter->getItemText(i);
    }
    return pat;
}


// Set current filter pattern
void FileSelector::setPattern(const FXString& ptrn)
{
    filefilter->setText(ptrn);
    list->setPattern(ptrn);
}


// Get current filter pattern
FXString FileSelector::getPattern() const
{
    return list->getPattern();
}


// Set current file pattern from the list
void FileSelector::setCurrentPattern(int patno)
{
    if ((FXuint)patno >= (FXuint)filefilter->getNumItems())
    {
        fprintf(stderr, "%s::setCurrentPattern: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    filefilter->setCurrentItem(patno);
    list->setPattern(patternFromText(filefilter->getItemText(patno)));
}


// Return current pattern
int FileSelector::getCurrentPattern() const
{
    return filefilter->getCurrentItem();
}


// Change pattern for pattern number patno
void FileSelector::setPatternText(int patno, const FXString& text)
{
    if ((FXuint)patno >= (FXuint)filefilter->getNumItems())
    {
        fprintf(stderr, "%s::setPatternText: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    filefilter->setItemText(patno, text);
    if (patno == filefilter->getCurrentItem())
    {
        setPattern(patternFromText(text));
    }
}


// Return pattern text of pattern patno
FXString FileSelector::getPatternText(int patno) const
{
    if ((FXuint)patno >= (FXuint)filefilter->getNumItems())
    {
        fprintf(stderr, "%s::getPatternText: index out of range.\n", getClassName());
        exit(EXIT_FAILURE);
    }
    return filefilter->getItemText(patno);
}


// Change space for item
void FileSelector::setItemSpace(int s)
{
    list->setItemSpace(s);
}


// Get space for item
int FileSelector::getItemSpace() const
{
    return list->getItemSpace();
}


// Change File List style
void FileSelector::setFileBoxStyle(FXuint style)
{
    list->setListStyle((list->getListStyle() & ~FILESTYLEMASK) | (style & FILESTYLEMASK));
}


// Return File List style
FXuint FileSelector::getFileBoxStyle() const
{
    return list->getListStyle() & FILESTYLEMASK;
}


// Change file selection mode
void FileSelector::setSelectMode(FXuint mode)
{
    switch (mode)
    {
    case SELECT_FILE_EXISTING:
        list->showOnlyDirectories(false);
        list->setListStyle((list->getListStyle() & ~FILELISTMASK) | ICONLIST_BROWSESELECT);
        break;

    case SELECT_FILE_MULTIPLE:
    case SELECT_FILE_MULTIPLE_ALL:
        list->showOnlyDirectories(false);
        list->setListStyle((list->getListStyle() & ~FILELISTMASK) | ICONLIST_EXTENDEDSELECT);
        break;

    case SELECT_FILE_DIRECTORY:
        list->showOnlyDirectories(true);
        list->setListStyle((list->getListStyle() & ~FILELISTMASK) | ICONLIST_BROWSESELECT);
        setNameLabel(_("Folder Name:"));
        filefilter->disable();
        break;

    case SELECT_FILE_MIXED:
        list->setListStyle((list->getListStyle() & ~FILELISTMASK) | ICONLIST_BROWSESELECT);
        break;

    default:
        list->showOnlyDirectories(false);
        list->setListStyle((list->getListStyle() & ~FILELISTMASK) | ICONLIST_BROWSESELECT);
        break;
    }
    selectmode = mode;
}


// Show readonly button
void FileSelector::showReadOnly(FXbool show)
{
    show ? readonly->show() : readonly->hide();
}


// Return true if readonly is shown
FXbool FileSelector::shownReadOnly() const
{
    return readonly->shown();
}


// Set initial state of readonly button
void FileSelector::setReadOnly(FXbool state)
{
    readonly->setCheck(state);
}


// Get readonly state
FXbool FileSelector::getReadOnly() const
{
    return readonly->getCheck();
}


// Return true if hidden files are displayed
FXbool FileSelector::shownHiddenFiles() const
{
    return list->shownHiddenFiles();
}


// Return true if thumbnails are displayed
FXbool FileSelector::shownThumbnails() const
{
    return list->shownThumbnails();
}


// Change show hidden files mode
void FileSelector::showHiddenFiles(FXbool shown)
{
    list->showHiddenFiles(shown);
}


// Change show thumbnails files mode
void FileSelector::showThumbnails(FXbool shown)
{
    list->showThumbnails(shown);
}


// Cleanup; icons must be explicitly deleted
FileSelector::~FileSelector()
{
    // Write options to the registry
    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "name_size", list->getHeaderSize(0)); // Name is at position 0

    FXbool isShown;
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "size_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_SIZE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "type_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_TYPE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "ext_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_EXT)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "date_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_DATE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "user_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_USER)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "group_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_GROUP)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "perms_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_PERMS)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("FILEDIALOG", "link_size",
                                           (FXuint)list->getHeaderSize((FXuint)list->getHeaderIndex(FileList::ID_COL_LINK)));
    }

    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "liststyle", getFileBoxStyle());
    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "hiddenfiles", shownHiddenFiles());
    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "showthumbnails", shownThumbnails());
    getApp()->reg().write();

    FXAccelTable* table = getShell()->getAccelTable();
    if (table)
    {
        table->removeAccel(MKUINT(KEY_BackSpace, 0));
        table->removeAccel(MKUINT(KEY_h, CONTROLMASK));
        table->removeAccel(MKUINT(KEY_w, CONTROLMASK));
        table->removeAccel(MKUINT(KEY_a, CONTROLMASK));
        table->removeAccel(MKUINT(KEY_i, CONTROLMASK));
    }
    delete list;
    delete pathlink;
    delete pathtext;
    delete namelabel;
    delete filename;
    delete filefilter;
    delete readonly;
    delete accept;
    delete cancel;
    delete btnbackhist;
    delete btnforwardhist;
    delete filterdialog;
    delete filterlabel;
    delete closefilter;
}


// File selector context menu
long FileSelector::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // Popup menu pane
    FXMenuPane menu(this);
    int x, y;
    FXuint state;

    getRoot()->getCursorPosition(x, y, state);

    new FXMenuCommand(&menu, _("Go Ho&me"), minihomeicon, this, ID_HOME);
    new FXMenuCommand(&menu, _("Go &Work"), miniworkicon, this, ID_WORK);
    new FXMenuCommand(&menu, _("New &File..."), mininewfileicon, this, ID_NEWFILE);
    new FXMenuCommand(&menu, _("New F&older..."), minifoldernewicon, this, ID_NEWDIR);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("&Hidden Files"), list, FileList::ID_TOGGLE_HIDDEN);
    new FXMenuCheck(&menu, _("Thum&bnails"), list, FileList::ID_TOGGLE_THUMBNAILS);
    new FXMenuSeparator(&menu);
    new FXMenuRadio(&menu, _("B&ig Icons"), list, IconList::ID_SHOW_BIG_ICONS);
    new FXMenuRadio(&menu, _("&Small Icons"), list, IconList::ID_SHOW_MINI_ICONS);
    new FXMenuRadio(&menu, _("&Detailed File List"), list, IconList::ID_SHOW_DETAILS);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("Autos&ize"), list, FileList::ID_AUTOSIZE);
    new FXMenuRadio(&menu, _("&Rows"), list, FileList::ID_ARRANGE_BY_ROWS);
    new FXMenuRadio(&menu, _("&Columns"), list, FileList::ID_ARRANGE_BY_COLUMNS);
    new FXMenuSeparator(&menu);
    new FXMenuRadio(&menu, _("&Name"), list, FileList::ID_COL_NAME);
    new FXMenuRadio(&menu, _("Si&ze"), list, FileList::ID_COL_SIZE);
    new FXMenuRadio(&menu, _("&Type"), list, FileList::ID_COL_TYPE);
    new FXMenuRadio(&menu, _("E&xtension"), list, FileList::ID_COL_EXT);
    new FXMenuRadio(&menu, _("&Date"), list, FileList::ID_COL_DATE);
    new FXMenuRadio(&menu, _("&User"), list, FileList::ID_COL_USER);
    new FXMenuRadio(&menu, _("&Group"), list, FileList::ID_COL_GROUP);
    new FXMenuRadio(&menu, _("&Permissions"), list, FileList::ID_COL_PERMS);
    new FXMenuRadio(&menu, _("&Link"), list, FileList::ID_COL_LINK);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("Ignore C&ase"), list, FileList::ID_SORT_CASE);
    new FXMenuCheck(&menu, _("Fold&ers First"), list, FileList::ID_DIRS_FIRST);
    new FXMenuCheck(&menu, _("Re&verse Order"), list, FileList::ID_SORT_REVERSE);

    menu.create();
    menu.popup(NULL, x, y);
    getApp()->runModalWhileShown(&menu);
    return 1;
}

// If Shift-F10 or Menu is pressed, opens the popup menu
long FileSelector::onCmdKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    // Shift-F10 or Menu was pressed : open popup menu
    if ((event->state & SHIFTMASK && event->code == KEY_F10) || event->code == KEY_Menu)
    {
        this->handle(sender, FXSEL(SEL_COMMAND, FileSelector::ID_POPUP_MENU), ptr);
        return 1;
    }
    // Any other key was pressed : handle the pressed key in the usual way
    else
    {
        if (this->onKeyPress(sender, sel, ptr))
        {
            return 1;
        }
    }

    return 0;
}

long FileSelector::onCmdKeyRelease(FXObject* sender, FXSelector sel, void* ptr)
{
    if (this->onKeyRelease(sender, sel, ptr))
    {
        return 1;
    }

    return 0;
}


// Object implementation
FXIMPLEMENT(FileDialog, DialogBox, NULL, 0)


// File Dialog object
FileDialog::FileDialog(FXWindow* owner, const FXString& name, FXuint opts, int x, int y, int w, int h) :
    DialogBox(owner, name, opts | DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MAXIMIZE | DECOR_CLOSE,
              x, y, w, h, 0, 0, 0, 0, 4, 4)
{
    list = new FileSelector(this, NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    list->acceptButton()->setTarget(this);
    list->acceptButton()->setSelector(DialogBox::ID_ACCEPT);
    list->cancelButton()->setTarget(this);
    list->cancelButton()->setSelector(DialogBox::ID_CANCEL);

    // Set file dialog options
    FXuint width = getApp()->reg().readUnsignedEntry("FILEDIALOG", "width", 640);
    FXuint height = getApp()->reg().readUnsignedEntry("FILEDIALOG", "height", 480);
    setWidth(width);
    setHeight(height);
}


// Set file name
void FileDialog::setFilename(const FXString& path)
{
    list->setFilename(path);
}


// Get filename, if any
FXString FileDialog::getFilename() const
{
    return list->getFilename();
}


// Return empty-string terminated list of selected file names,
FXString* FileDialog::getFilenames() const
{
    return list->getFilenames();
}


// Set pattern
void FileDialog::setPattern(const FXString& ptrn)
{
    list->setPattern(ptrn);
}


// Get pattern
FXString FileDialog::getPattern() const
{
    return list->getPattern();
}


// Change patterns, each pattern separated by newline
void FileDialog::setPatternList(const FXString& patterns)
{
    list->setPatternList(patterns);
}


// Return list of patterns
FXString FileDialog::getPatternList() const
{
    return list->getPatternList();
}


// Set directory
void FileDialog::setDirectory(const FXString& path)
{
    list->setDirectory(path);
}


// Get directory
FXString FileDialog::getDirectory() const
{
    return list->getDirectory();
}


// Set current file pattern from the list
void FileDialog::setCurrentPattern(int n)
{
    list->setCurrentPattern(n);
}


// Return current pattern
int FileDialog::getCurrentPattern() const
{
    return list->getCurrentPattern();
}


FXString FileDialog::getPatternText(int patno) const
{
    return list->getPatternText(patno);
}


void FileDialog::setPatternText(int patno, const FXString& text)
{
    list->setPatternText(patno, text);
}


// Set list of patterns (DEPRECATED)
void FileDialog::setPatternList(const char** ptrns)
{
    list->setPatternList(ptrns);
}


// Change space for item
void FileDialog::setItemSpace(int s)
{
    list->setItemSpace(s);
}


// Get space for item
int FileDialog::getItemSpace() const
{
    return list->getItemSpace();
}


// Change File List style
void FileDialog::setFileBoxStyle(FXuint style)
{
    list->setFileBoxStyle(style);
}


// Return File List style
FXuint FileDialog::getFileBoxStyle() const
{
    return list->getFileBoxStyle();
}


// Change file selection mode
void FileDialog::setSelectMode(FXuint mode)
{
    list->setSelectMode(mode);
}


// Return file selection mode
FXuint FileDialog::getSelectMode() const
{
    return list->getSelectMode();
}


// Show readonly button
void FileDialog::showReadOnly(FXbool show)
{
    list->showReadOnly(show);
}


// Return true if readonly is shown
FXbool FileDialog::shownReadOnly() const
{
    return list->shownReadOnly();
}


// Set initial state of readonly button
void FileDialog::setReadOnly(FXbool state)
{
    list->setReadOnly(state);
}


// Get readonly state
FXbool FileDialog::getReadOnly() const
{
    return list->getReadOnly();
}


// Return true if thumbnails are displayed
FXbool FileDialog::shownThumbnails() const
{
    return list->shownThumbnails();
}


// Return true if hidden files are displayed
FXbool FileDialog::shownHiddenFiles() const
{
    return list->shownHiddenFiles();
}


// Change show hidden files mode
void FileDialog::showHiddenFiles(FXbool shown)
{
    list->showHiddenFiles(shown);
}


// Change show thumbnails files mode
void FileDialog::showThumbnails(FXbool shown)
{
    list->showThumbnails(shown);
}


// Cleanup
FileDialog::~FileDialog()
{
    // Write options to the registry
    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "width", getWidth());
    getApp()->reg().writeUnsignedEntry("FILEDIALOG", "height", getHeight());
    getApp()->reg().write();

    delete list;
}


// Open existing filename
FXString FileDialog::getOpenFilename(FXWindow* owner, const FXString& caption, const FXString& path,
                                     const FXString& patterns, int initial)
{
    FileDialog opendialog(owner, caption);
    FXString filename;

    opendialog.setSelectMode(SELECT_FILE_EXISTING);
    opendialog.setPatternList(patterns);
    opendialog.setCurrentPattern(initial);
    opendialog.setFilename(path);
    if (opendialog.execute())
    {
        filename = opendialog.getFilename();
        if (xf_isfile(filename))
        {
            return filename;
        }
    }
    return FXString::null;
}


// Save to filename
FXString FileDialog::getSaveFilename(FXWindow* owner, const FXString& caption, const FXString& path,
                                     const FXString& patterns, int initial)
{
    FileDialog savedialog(owner, caption);

    savedialog.setSelectMode(SELECT_FILE_ANY);
    savedialog.setPatternList(patterns);
    savedialog.setCurrentPattern(initial);
    savedialog.setFilename(path);
    if (savedialog.execute())
    {
        return savedialog.getFilename();
    }
    return FXString::null;
}


// Open multiple existing files
FXString* FileDialog::getOpenFilenames(FXWindow* owner, const FXString& caption, const FXString& path,
                                       const FXString& patterns, int initial)
{
    FileDialog opendialog(owner, caption);

    opendialog.setSelectMode(SELECT_FILE_MULTIPLE);
    opendialog.setPatternList(patterns);
    opendialog.setCurrentPattern(initial);
    opendialog.setFilename(path);
    if (opendialog.execute())
    {
        return opendialog.getFilenames();
    }
    return NULL;
}


// Open existing directory name
FXString FileDialog::getOpenDirectory(FXWindow* owner, const FXString& caption, const FXString& path)
{
    FileDialog dirdialog(owner, caption);
    FXString dirname;

    dirdialog.setSelectMode(SELECT_FILE_DIRECTORY);
    dirdialog.setFilename(path);
    if (dirdialog.execute())
    {
        dirname = dirdialog.getFilename();
        if (xf_isdirectory(dirname))
        {
            return dirname;
        }
    }
    return FXString::null;
}
