// Preferences dialog box

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "icons.h"
#include "xfedefs.h"
#include "xfeutils.h"
#include "FileDialog.h"
#include "FontDialog.h"
#include "XFileExplorer.h"
#include "MessageBox.h"
#include "KeyBindings.h"
#include "Preferences.h"


FXbool Theme::operator !=(const Theme& t)
{
    for (int i = 0; i < NUM_COLORS; i++)
    {
        if (color[i] != t.color[i])
        {
            return true;
        }
    }
    return false;
}


// Main window
extern FXMainWindow* mainWindow;
extern FXString execpath;


// Create hilite color from given color for gradient controls
static FXColor makeHiliteColorGradient(FXColor color)
{
    FXuint r, g, b;

    r = FXREDVAL(color);
    g = FXGREENVAL(color);
    b = FXBLUEVAL(color);

    r = (FXuint)(FXMIN(1.2 * r, 255));
    g = (FXuint)(FXMIN(1.2 * g, 255));
    b = (FXuint)(FXMIN(1.2 * b, 255));

    return FXRGB(r, g, b);
}


// Create shadow color from given color for gradient controls
static FXColor makeShadowColorGradient(FXColor color)
{
    FXuint r, g, b;

    r = FXREDVAL(color);
    g = FXGREENVAL(color);
    b = FXBLUEVAL(color);

    r = (FXuint)(0.7 * r);
    g = (FXuint)(0.7 * g);
    b = (FXuint)(0.7 * b);

    return FXRGB(r, g, b);
}


// Get header name from the column id
FXString getHeaderName(FXuint id)
{
    FXString name;

    switch (id)
    {
    case FileList::ID_COL_NAME:
        name = _("Name");
        break;

    case FileList::ID_COL_SIZE:
        name = _("Size");
        break;

    case FileList::ID_COL_TYPE:
        name = _("Type");
        break;

    case FileList::ID_COL_EXT:
        name = _("Extension");
        break;

    case FileList::ID_COL_DATE:
        name = _("Modified Date");
        break;

    case FileList::ID_COL_USER:
        name = _("User");
        break;

    case FileList::ID_COL_GROUP:
        name = _("Group");
        break;

    case FileList::ID_COL_PERMS:
        name = _("Permissions");
        break;

    case FileList::ID_COL_LINK:
        name = _("Link");
        break;
    }
    return name;
}


// Map
FXDEFMAP(PreferencesBox) PreferencesMap[] =
{
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_ACCEPT, PreferencesBox::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_CANCEL, PreferencesBox::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_TXTEDIT, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_TXTVIEW, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_IMGVIEW, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_ARCHIVER, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_PDFVIEW, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_VIDEOPLAY, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_AUDIOPLAY, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_XTERM, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_MOUNTCMD, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_UMOUNTCMD, PreferencesBox::onCmdBrowse),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_COLOR, PreferencesBox::onCmdColor),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_THEME_SAVEAS, PreferencesBox::onCmdThemeSaveAs),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_THEME_RENAME, PreferencesBox::onCmdThemeRename),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_THEME_REMOVE, PreferencesBox::onCmdThemeRemove),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, PreferencesBox::ID_THEME, PreferencesBox::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_NORMALFONT, PreferencesBox::onCmdNormalFont),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_TEXTFONT, PreferencesBox::onCmdTextFont),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_THEME, PreferencesBox::onCmdTheme),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_BROWSE_ICON_PATH, PreferencesBox::onCmdBrowsePath),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_START_HOMEDIR, PreferencesBox::onCmdStartDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_START_CURRENTDIR, PreferencesBox::onCmdStartDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_START_LASTDIR, PreferencesBox::onCmdStartDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_NEWTAB_HOMEDIR, PreferencesBox::onCmdNewTabDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_NEWTAB_CURRENTDIR, PreferencesBox::onCmdNewTabDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_NEWTAB_ROOTDIR, PreferencesBox::onCmdNewTabDir),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_PKEXEC_CMD, PreferencesBox::onCmdSuMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_SU_CMD, PreferencesBox::onCmdSuMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_SUDO_CMD, PreferencesBox::onCmdSuMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_WHEELADJUST, PreferencesBox::onCmdWheelAdjust),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_SCROLLBARSIZE, PreferencesBox::onCmdScrollBarSize),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_MOUNT_TIMEOUT, PreferencesBox::onCmdMountTimeout),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_CHANGE_KEYBINDINGS, PreferencesBox::onCmdChangeKeyBindings),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_RESTORE_KEYBINDINGS, PreferencesBox::onCmdRestoreKeyBindings),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_COPY_SUFFIX_BEFORE, PreferencesBox::onCmdCopySuffixPos),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_COPY_SUFFIX_AFTER, PreferencesBox::onCmdCopySuffixPos),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_DIRPANEL_PLACES_TREE, PreferencesBox::onCmdDirPanelMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_DIRPANEL_PLACES, PreferencesBox::onCmdDirPanelMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_DIRPANEL_TREE, PreferencesBox::onCmdDirPanelMode),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_LISTCOL_UP, PreferencesBox::onCmdColUp),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_LISTCOL_DOWN, PreferencesBox::onCmdColDown),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_VIEW_COL, PreferencesBox::onCmdViewCol),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_DIRPANEL_PLACES_STARTUP, PreferencesBox::onCmdDirPanelStartup),
    FXMAPFUNC(SEL_COMMAND, PreferencesBox::ID_DIRPANEL_TREE_STARTUP, PreferencesBox::onCmdDirPanelStartup),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_LISTCOL_UP, PreferencesBox::onUpdListColUpDown),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_LISTCOL_DOWN, PreferencesBox::onUpdListColUpDown),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_COLOR, PreferencesBox::onUpdColor),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_WHEELADJUST, PreferencesBox::onUpdWheelAdjust),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_SCROLLBARSIZE, PreferencesBox::onUpdScrollBarSize),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_MOUNT_TIMEOUT, PreferencesBox::onUpdMountTimeout),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_SINGLE_CLICK_FILEOPEN, PreferencesBox::onUpdSingleClickFileopen),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_EXEC_TEXT_FILES, PreferencesBox::onUpdExecTextFiles),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_CONFIRM_TRASH, PreferencesBox::onUpdTrash),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_TRASH_BYPASS, PreferencesBox::onUpdTrash),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_CONFIRM_DEL_EMPTYDIR, PreferencesBox::onUpdConfirmDelEmptyDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_PKEXEC_CMD, PreferencesBox::onUpdSuMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_SU_CMD, PreferencesBox::onUpdSuMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_SUDO_CMD, PreferencesBox::onUpdSuMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_START_HOMEDIR, PreferencesBox::onUpdStartDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_START_CURRENTDIR, PreferencesBox::onUpdStartDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_START_LASTDIR, PreferencesBox::onUpdStartDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_NEWTAB_HOMEDIR, PreferencesBox::onUpdNewTabDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_NEWTAB_CURRENTDIR, PreferencesBox::onUpdNewTabDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_NEWTAB_ROOTDIR, PreferencesBox::onUpdNewTabDir),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_COPY_SUFFIX_BEFORE, PreferencesBox::onUpdCopySuffixPos),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_COPY_SUFFIX_AFTER, PreferencesBox::onUpdCopySuffixPos),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_DIRPANEL_PLACES_TREE, PreferencesBox::onUpdDirPanelMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_DIRPANEL_PLACES, PreferencesBox::onUpdDirPanelMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_DIRPANEL_TREE, PreferencesBox::onUpdDirPanelMode),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_DIRPANEL_PLACES_STARTUP, PreferencesBox::onUpdDirPanelStartup),
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_DIRPANEL_TREE_STARTUP, PreferencesBox::onUpdDirPanelStartup),
#if defined(linux) && defined(XFE_AUTOMOUNTER)
    FXMAPFUNC(SEL_UPDATE, PreferencesBox::ID_AUTO_OPEN, PreferencesBox::onUpdAutoOpen),
#endif
    FXMAPFUNC(SEL_QUERY_TIP, PreferencesBox::ID_THEME, PreferencesBox::onQueryTip),
};

// Object implementation
FXIMPLEMENT(PreferencesBox, DialogBox, PreferencesMap, ARRAYNUMBER(PreferencesMap))

// Construct
PreferencesBox::PreferencesBox(FXWindow* win, FXColor listbackcolor, FXColor listforecolor, FXColor highlightcolor,
                               FXColor pbarcolor, FXColor attentioncolor, FXColor scrollbarcolor) :
    DialogBox(win, _("Preferences"), DECOR_TITLE | DECOR_BORDER | DECOR_MAXIMIZE | DECOR_STRETCHABLE | DECOR_CLOSE)
{
    // Current theme
    currTheme.name = _("Current Theme");
    currTheme.tip = _("Applied theme, right click to save");
    currTheme.color[0] = getApp()->getBaseColor();
    currTheme.color[1] = getApp()->getBorderColor();
    currTheme.color[2] = getApp()->getBackColor();
    currTheme.color[3] = getApp()->getForeColor();
    currTheme.color[4] = getApp()->getSelbackColor();
    currTheme.color[5] = getApp()->getSelforeColor();
    currTheme.color[6] = listbackcolor;
    currTheme.color[7] = listforecolor;
    currTheme.color[8] = highlightcolor;
    currTheme.color[9] = pbarcolor;
    currTheme.color[10] = attentioncolor;
    currTheme.color[11] = scrollbarcolor;

    Themes.push_back(currTheme);

    // Other default themes
    Theme theme;
    
    theme = Theme("Xfe", _("The default theme"), FXRGB(237, 236, 235), FXRGB(125, 125, 125), FXRGB(255, 255, 255),
                      FXRGB(0, 0, 0),
                      FXRGB(10, 36, 106), FXRGB(255, 255, 255), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(10, 36, 106), FXRGB(255, 0, 0), FXRGB(237, 236, 235));
    Themes.push_back(theme);
                      
    theme = Theme("Clearlooks", _("A light grey theme with white background"), FXRGB(237, 236, 235),
                      FXRGB(125, 125, 125), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(139, 175, 220), FXRGB(255, 255, 255), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(121, 153, 192), FXRGB(255, 0, 0), FXRGB(149, 178, 215));
    Themes.push_back(theme);

    theme = Theme("Dark", _("A dark theme"), FXRGB(52, 52, 52), FXRGB(105, 105, 105), FXRGB(36, 36, 36),
                      FXRGB(215, 215, 215),
                      FXRGB(49, 86, 124), FXRGB(255, 255, 255), FXRGB(36, 36, 36), FXRGB(215, 215, 215),
                      FXRGB(69, 69, 69), FXRGB(49, 86, 124), FXRGB(255, 0, 0), FXRGB(72, 72, 72));
    Themes.push_back(theme);

    theme = Theme("Blue Bird", _("A blue grey theme with white background"), FXRGB(165, 178, 198),
                      FXRGB(115, 115, 115), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(49, 101, 156), FXRGB(255, 255, 255), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(49, 101, 156), FXRGB(255, 0, 0), FXRGB(68, 106, 146));
    Themes.push_back(theme);

    theme = Theme("Slate Blue", _("A light blue theme with grey background"), FXRGB(156, 186, 205),
                      FXRGB(125, 125, 125), FXRGB(197, 194, 197), FXRGB(0, 0, 0),
                      FXRGB(82, 129, 148), FXRGB(255, 255, 255), FXRGB(197, 194, 197), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(82, 129, 148), FXRGB(255, 0, 0), FXRGB(156, 186, 205));
    Themes.push_back(theme);

    theme = Theme("GNOME", _("A theme with GNOME colors"), FXRGB(220, 218, 213), FXRGB(125, 125, 125),
                      FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(75, 105, 131), FXRGB(255, 255, 255), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(75, 105, 131), FXRGB(255, 0, 0), FXRGB(134, 171, 217));
    Themes.push_back(theme);

    theme = Theme("KDE", _("A theme with KDE colors"), FXRGB(238, 238, 230), FXRGB(125, 125, 125),
                      FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(255, 222, 118), FXRGB(0, 0, 0), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(255, 222, 118), FXRGB(255, 0, 0), FXRGB(238, 238, 230));
    Themes.push_back(theme);

    theme = Theme("XFCE", _("A theme with XFCE colors"), FXRGB(238, 238, 238), FXRGB(125, 125, 125),
                      FXRGB(238, 238, 238), FXRGB(0, 0, 0),
                      FXRGB(99, 119, 146), FXRGB(255, 255, 255), FXRGB(255, 255, 255), FXRGB(0, 0, 0),
                      FXRGB(238, 238, 238), FXRGB(99, 119, 146), FXRGB(255, 0, 0), FXRGB(238, 238, 238));
    Themes.push_back(theme);

    // Custom themes
    if (getApp()->reg().readStringEntry("THEMES", "theme0", NULL))
    {
        FXuint i = 0;
        while (1)
        {
            FXString regkey = "theme" + FXStringVal(i);
            FXString data = getApp()->reg().readStringEntry("THEMES", regkey.text(), NULL);

            if (data != "")
            {
                FXString name = data.section(';', 0);
                FXString tip = data.section(';', 1);
                if (tip == "")
                {
                    tip = _("Custom theme, right click to rename or remove");
                }
                FXColor color0 = FXUIntVal(data.section(';', 2));
                FXColor color1 = FXUIntVal(data.section(';', 3));
                FXColor color2 = FXUIntVal(data.section(';', 4));
                FXColor color3 = FXUIntVal(data.section(';', 5));
                FXColor color4 = FXUIntVal(data.section(';', 6));
                FXColor color5 = FXUIntVal(data.section(';', 7));
                FXColor color6 = FXUIntVal(data.section(';', 8));
                FXColor color7 = FXUIntVal(data.section(';', 9));
                FXColor color8 = FXUIntVal(data.section(';', 10));
                FXColor color9 = FXUIntVal(data.section(';', 11));
                FXColor color10 = FXUIntVal(data.section(';', 12));
                FXColor color11 = FXUIntVal(data.section(';', 13));

                Theme theme = Theme(name.text(), tip.text(), color0, color1, color2, color3, color4,
                                    color5, color6, color7, color8, color9, color10, color11);
                Themes.push_back(theme);

                i++;
            }
            else
            {
                break;
            }
        }
    }

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10, 5, 5);

    // Contents
    FXHorizontalFrame* contents = new FXHorizontalFrame(this,
                                                        LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                                        PACK_UNIFORM_WIDTH);

    // Accept
    FXButton* ok = new FXButton(buttons, _("&Accept"), NULL, this, PreferencesBox::ID_ACCEPT,
                                BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0,
                                0, 0, 20, 20);
    ok->addHotKey(KEY_Return);
    ok->setFocus();

    // Cancel
    new FXButton(buttons, _("&Cancel"), NULL, this, PreferencesBox::ID_CANCEL,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Switcher
    FXTabBook* tabbook = new FXTabBook(contents, NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_RIGHT | FRAME_NONE);

    // First tab - General options
    new FXTabItem(tabbook, _("&General"), NULL);
    FXVerticalFrame* general = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);

    FXGroupBox* group = new FXGroupBox(general, _("Starting"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE |
                                       LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXRadioButton(group, _("Start in home folder") + FXString(" "), this, PreferencesBox::ID_START_HOMEDIR);
    new FXRadioButton(group, _("Start in current folder") + FXString(" "), this, PreferencesBox::ID_START_CURRENTDIR);
    new FXRadioButton(group, _("Start in last visited folder") + FXString(" "), this, PreferencesBox::ID_START_LASTDIR);

    newtabdirmode = getApp()->reg().readUnsignedEntry("OPTIONS", "newtabdir_mode", NEWTAB_HOMEDIR) + ID_NEWTAB_HOMEDIR;
    oldnewtabdirmode = newtabdirmode;
    newtabdirtarget.connect(newtabdirmode);

    group = new FXGroupBox(general, _("General Options"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    trashcan = new FXCheckButton(group, _("Use trash can for file deletion (safe delete)") + FXString(" "));
    trashbypass = new FXCheckButton(group, _("Include a command to bypass the trash can (permanent delete)") + FXString(" "),
                                    this, ID_TRASH_BYPASS);
    autosave = new FXCheckButton(group, _("Auto save layout") + FXString(" "));
    savewinpos = new FXCheckButton(group, _("Save window position") + FXString(" "));
    diropen = new FXCheckButton(group, _("Single click folder open") + FXString(" "));
    fileopen = new FXCheckButton(group, _("Single click file open") + FXString(" "), this, ID_SINGLE_CLICK_FILEOPEN);
    filetooltips = new FXCheckButton(group, _("Display tooltips in file and folder lists") + FXString(" "), this,
                                     ID_FILE_TOOLTIPS);
    relativeresize = new FXCheckButton(group, _("Relative resizing of file lists") + FXString(" "), this,
                                       ID_RELATIVE_RESIZE);
    showpathlink = new FXCheckButton(group, _("Display a path linker above file lists") + FXString(" "), this,
                                     ID_SHOW_PATHLINK);
#ifdef STARTUP_NOTIFICATION
    usesn = new FXCheckButton(group, _("Notify when applications start up") + FXString(" "));
#endif
    noscript = new FXCheckButton(group, _("Don't execute script files") + FXString(" "));

    FXMatrix* matrix = new FXMatrix(group, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);
    FXVerticalFrame* vframe = new FXVerticalFrame(matrix,
                                                  LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                                  LAYOUT_LEFT);
    new FXLabel(vframe, _("Folders panel view:"), NULL, JUSTIFY_LEFT);
    dirpanel_places_tree = new FXRadioButton(vframe, _("Places and tree"), this, ID_DIRPANEL_PLACES_TREE);
    dirpanel_places = new FXRadioButton(vframe, _("Places"), this, ID_DIRPANEL_PLACES);
    dirpanel_tree = new FXRadioButton(vframe, _("Tree"), this, ID_DIRPANEL_TREE);
    dirpanel_mode = getApp()->reg().readUnsignedEntry("OPTIONS", "dirpanel_mode", 0);

    if (dirpanel_mode == 0)
    {
        dirpanel_places_tree->setCheck(true);
        dirpanel_places->setCheck(false);
        dirpanel_tree->setCheck(false);
    }
    else if (dirpanel_mode == 1)
    {
        dirpanel_places_tree->setCheck(false);
        dirpanel_places->setCheck(true);
        dirpanel_tree->setCheck(false);
    }
    else // dirpanel_mode == 2
    {
        dirpanel_places_tree->setCheck(false);
        dirpanel_places->setCheck(false);
        dirpanel_tree->setCheck(true);
    }

    new FXLabel(matrix, "     ");
    vframe = new FXVerticalFrame(matrix, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_LEFT);
    dirpanel_startup = new FXLabel(vframe, _("Displayed list at startup:"), NULL, JUSTIFY_LEFT);
    dirpanel_placesstartup = new FXRadioButton(vframe, _("Places"), this, ID_DIRPANEL_PLACES_STARTUP);
    dirpanel_treestartup = new FXRadioButton(vframe, _("Tree"), this, ID_DIRPANEL_TREE_STARTUP);
    dirpanel_list_startup = getApp()->reg().readUnsignedEntry("OPTIONS", "dirpanel_list_startup", 0);
    if (dirpanel_list_startup == 0)
    {
        dirpanel_placesstartup->setCheck(true);
        dirpanel_treestartup->setCheck(false);
    }
    else
    {
        dirpanel_placesstartup->setCheck(false);
        dirpanel_treestartup->setCheck(true);
    }

    matrix = new FXMatrix(group, 2,
                          MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix,
                _("Date format used in file and folder lists:\n(Type 'man strftime' in a terminal for help on the format)"),
                NULL, JUSTIFY_LEFT);
    timeformat = new FXTextField(matrix, 15, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    oldtimeformat = getApp()->reg().readStringEntry("SETTINGS", "time_format", DEFAULT_TIME_FORMAT);
    timeformat->setText(oldtimeformat);

    // Spacing
    new FXFrame(matrix, JUSTIFY_LEFT);
    new FXFrame(matrix, JUSTIFY_LEFT);

    new FXLabel(matrix, _("Suffix used for file copying in the same folder"), NULL, JUSTIFY_LEFT);
    copysuffix = new FXTextField(matrix, 15, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    oldcopysuffix = getApp()->reg().readStringEntry("OPTIONS", "copysuffix", DEFAULT_COPY_SUFFIX);
    copysuffix->setText(oldcopysuffix);

    vframe = new FXVerticalFrame(matrix, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    FXRadioButton* copysuffix_before = new FXRadioButton(vframe,
                                                         _("Suffix added before file extension (ex: file (copy).txt)"),
                                                         this, ID_COPY_SUFFIX_BEFORE);
    FXRadioButton* copysuffix_after = new FXRadioButton(vframe,
                                                        _("Suffix added after file extension (ex: file.txt.copy)"),
                                                        this, ID_COPY_SUFFIX_AFTER);
    copysuffix_pos = getApp()->reg().readUnsignedEntry("OPTIONS", "copysuffix_pos", 0);
    if (copysuffix_pos == 0)
    {
        copysuffix_before->setCheck(true);
        copysuffix_after->setCheck(false);
    }
    else
    {
        copysuffix_before->setCheck(false);
        copysuffix_after->setCheck(true);
    }

    // Second tab - Dialogs
    new FXTabItem(tabbook, _("&Dialogs"), NULL);
    FXVerticalFrame* dialogs = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    group = new FXGroupBox(dialogs, _("Confirmations"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    ask = new FXCheckButton(group, _("Confirm copy/move/rename/symlink") + FXString(" "));
    dnd = new FXCheckButton(group, _("Confirm drag and drop") + FXString(" "));
    trashmv = new FXCheckButton(group, _("Confirm move to trash/restore from trash") + FXString(" "), this,
                                ID_CONFIRM_TRASH);
    del = new FXCheckButton(group, _("Confirm delete") + FXString(" "));
    del_emptydir = new FXCheckButton(group, _("Confirm delete non empty folders") + FXString(" "), this,
                                     ID_CONFIRM_DEL_EMPTYDIR);
    overwrite = new FXCheckButton(group, _("Confirm overwrite") + FXString(" "));
    exec = new FXCheckButton(group, _("Confirm execute text files") + FXString(" "), this, ID_EXEC_TEXT_FILES);
    properties = new FXCheckButton(group, _("Confirm change properties") + FXString(" "));

    group = new FXGroupBox(dialogs, _("Warnings"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    folder_warning = new FXCheckButton(group, _("Warn when setting current folder in search window") + FXString(" "));

#if defined(linux)

    matrix = new FXMatrix(group, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);
    mountwarn = new FXCheckButton(matrix, _("Warn when mount points are not responding") + FXString(" "));
    FXbool mount_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_warn", true);
    mountwarn->setCheck(mount_warn);
    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    new FXLabel(matrix, _("Delay before warning (seconds):"), NULL,
                JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    FXSpinner* spin = new FXSpinner(matrix, 3, this, PreferencesBox::ID_MOUNT_TIMEOUT,
                                    JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2, 2, 1, 1);
    spin->setRange(1, 300);

    FXuint value = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_timeout", MOUNT_TIMEOUT);
    spin->setValue(value);

    show_mount = new FXCheckButton(group, _("Display mount / unmount success messages") + FXString(" "));
    FXbool mount_messages = getApp()->reg().readUnsignedEntry("OPTIONS", "mount_messages", true);
    show_mount->setCheck(mount_messages);

#endif

    preserve_date_warning = new FXCheckButton(group, _("Warn when date preservation failed") + FXString(" "));
    root_warning = new FXCheckButton(group, _("Warn if running as root") + FXString(" "));

    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);
    trashmv->setCheck(confirm_trash);
    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);
    del->setCheck(confirm_del);
    FXbool confirm_properties = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_properties", true);
    properties->setCheck(confirm_properties);
    FXbool confirm_del_emptydir = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete_emptydir", true);
    del_emptydir->setCheck(confirm_del_emptydir);
    FXbool confirm_overwrite = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_overwrite", true);
    overwrite->setCheck(confirm_overwrite);
    FXbool confirm_exec = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_execute", true);
    exec->setCheck(confirm_exec);
    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);
    ask->setCheck(ask_before_copy);
    FXbool confirm_dnd = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_drag_and_drop", true);
    dnd->setCheck(confirm_dnd);

    FXbool folder_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folderwarn", true);
    folder_warning->setCheck(folder_warn);

    FXbool preserve_date_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "preserve_date_warn", true);
    preserve_date_warning->setCheck(preserve_date_warn);

    FXbool root_warn = getApp()->reg().readUnsignedEntry("OPTIONS", "root_warn", true);

    // User is non root
    if (getuid())
    {
        root_warning->disable();
    }
    else
    {
        root_warning->setCheck(root_warn);
    }

    // Third tab - Settings
    new FXTabItem(tabbook, _("&Settings"), NULL);
    FXVerticalFrame* settings = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);

    startdirmode = getApp()->reg().readUnsignedEntry("OPTIONS", "startdir_mode", START_HOMEDIR) + ID_START_HOMEDIR;
    oldstartdirmode = startdirmode;
    startdirtarget.connect(startdirmode);

    group = new FXGroupBox(settings, _("Tabs"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    matrix = new FXMatrix(group, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);

    new FXRadioButton(matrix, _("Open new tab in home folder") + FXString(" "), this, PreferencesBox::ID_NEWTAB_HOMEDIR);
    alwaysshowtabbar = new FXCheckButton(matrix, _("Always show tab bar") + FXString(" "), this, ID_SHOW_TABBAR);
    new FXRadioButton(matrix, _("Open new tab in current folder") + FXString(" "), this, PreferencesBox::ID_NEWTAB_CURRENTDIR);
    restoretabs = new FXCheckButton(matrix, _("Restore tabs on startup") + FXString(" "), this, ID_RESTORE_TABS);
    new FXRadioButton(matrix, _("Open new tab in root folder") + FXString(" "), this, PreferencesBox::ID_NEWTAB_ROOTDIR);

    group = new FXGroupBox(settings, _("Scrolling"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    matrix = new FXMatrix(group, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);
    scroll = new FXCheckButton(matrix, _("Smooth scrolling in file lists and text windows") + FXString(" "));
    FXbool smoothscroll = getApp()->reg().readUnsignedEntry("SETTINGS", "smooth_scroll", true);
    scroll->setCheck(smoothscroll);
    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    new FXLabel(matrix, _("Mouse scrolling speed:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    FXSpinner* spinner = new FXSpinner(matrix, 3, this, PreferencesBox::ID_WHEELADJUST,
                            JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2, 2, 1, 1);
    spinner->setRange(1, 100);
    new FXLabel(matrix, _("Scrollbar width:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    spinner = new FXSpinner(matrix, 3, this, PreferencesBox::ID_SCROLLBARSIZE,
                            JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2, 2, 1, 1);
    spinner->setRange(1, 100);

    group = new FXGroupBox(settings, _("Thumbnails"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    matrix = new FXMatrix(group, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXHorizontalFrame* hframe = new FXHorizontalFrame(matrix,
                                                      LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(hframe, _("Small thumbnails size:") + FXString("  "), NULL,
                JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    minithumb_spin = new FXSpinner(hframe, 3, this, ID_MINITHUMB_SIZE, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW,
                                   0, 0, 0, 0, 2, 2, 1, 1);
    minithumb_spin->setRange(16, 512);
    minithumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "minithumb_size", 24);
    minithumb_spin->setValue(minithumb_size);
    new FXLabel(matrix, "     ", NULL, JUSTIFY_LEFT);
    hframe = new FXHorizontalFrame(matrix, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(hframe, _("Big thumbnails size:") + FXString("  "), NULL,
                JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    bigthumb_spin = new FXSpinner(hframe, 4, this, ID_BIGTHUMB_SIZE, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0,
                                  0, 0, 0, 2, 2, 1, 1);
    bigthumb_spin->setRange(32, 1024);
    bigthumb_size = getApp()->reg().readUnsignedEntry("SETTINGS", "bigthumb_size", 64);
    bigthumb_spin->setValue(bigthumb_size);

#if defined(linux) && defined(XFE_AUTOMOUNTER)
    automountgroup = new FXGroupBox(settings, _("Automount"),
                                    GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(automountgroup, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    automountlabel = new FXLabel(vframe, _("(Only enable automounting if your system does not have this feature)"), NULL, JUSTIFY_LEFT);
    matrix = new FXMatrix(vframe, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);
    automountbutton = new FXCheckButton(matrix, _("Enable automounting") + FXString(" "));
    FXbool automount = getApp()->reg().readUnsignedEntry("OPTIONS", "automount", false);
    automountbutton->setCheck(automount);
    FXbool automount_open = getApp()->reg().readUnsignedEntry("OPTIONS", "automount_open", false);
    autoopenbutton = new FXCheckButton(matrix, _("Open new Xfe window when mounting") + FXString(" "), this, ID_AUTO_OPEN);
    autoopenbutton->setCheck(automount_open);

#endif
 
    rootgroup = new FXGroupBox(settings, _("Root Mode"),
                               GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(rootgroup, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    rootmode = new FXCheckButton(vframe, _("Allow root mode") + FXString(" "));
    FXRadioButton* pkexecbutton = new FXRadioButton(vframe,
                                                    _("Authentication using pkexec (requires the pkexec package)") +
                                                    FXString(" "), this, ID_PKEXEC_CMD);
    FXRadioButton* sudobutton = new FXRadioButton(vframe,
                                                  _("Authentication using sudo (uses user password)") + FXString(" "),
                                                  this, ID_SUDO_CMD);
    matrix = new FXMatrix(vframe, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    sudolabel = new FXLabel(matrix, _("sudo command:"), NULL, JUSTIFY_LEFT);
    sudocmd = new FXTextField(matrix, 40, NULL, 0, TEXTFIELD_NORMAL);
    FXRadioButton* subutton = new FXRadioButton(vframe,
                                                _("Authentication using su (uses root password)") + FXString(" "), this,
                                                ID_SU_CMD);
    matrix = new FXMatrix(vframe, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    sulabel = new FXLabel(matrix, _("su command:"), NULL, JUSTIFY_LEFT);
    sucmd = new FXTextField(matrix, 40, NULL, 0, TEXTFIELD_NORMAL);

    FXString sudo_cmd = getApp()->reg().readStringEntry("OPTIONS", "sudo_cmd", DEFAULT_SUDO_CMD);
    FXString su_cmd = getApp()->reg().readStringEntry("OPTIONS", "su_cmd", DEFAULT_SU_CMD);
    sudocmd->setText(sudo_cmd);
    sucmd->setText(su_cmd);

    FXbool root_mode = getApp()->reg().readUnsignedEntry("OPTIONS", "root_mode", true);
    rootmode->setCheck(root_mode);

    // User is root
    if (getuid() == 0)
    {
#if defined(linux) && defined(XFE_AUTOMOUNTER)
        automountgroup->disable();
        automountlabel->disable();
        automountbutton->disable();
        autoopenbutton->disable();
#endif
        rootgroup->disable();
        rootmode->disable();
        pkexecbutton->disable();
        sudobutton->disable();
        subutton->disable();
        sulabel->disable();
        sucmd->hide();
        sudolabel->disable();
        sudocmd->hide();
    }
    root_auth = getApp()->reg().readUnsignedEntry("OPTIONS", "root_auth", 0);

    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
    trashcan->setCheck(use_trash_can);

    if (trashcan->getCheck())
    {
        FXbool use_trash_bypass = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_bypass", false);
        trashbypass->setCheck(use_trash_bypass);
    }
    else
    {
        trashbypass->disable();
    }

    FXbool auto_save_layout = getApp()->reg().readUnsignedEntry("OPTIONS", "auto_save_layout", true);
    autosave->setCheck(auto_save_layout);

    FXbool save_win_pos = getApp()->reg().readUnsignedEntry("SETTINGS", "save_win_pos", false);
    savewinpos->setCheck(save_win_pos);

    // Single click navigation
    FXuint single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR)
    {
        diropen->setCheck(true);
        fileopen->setCheck(false);
    }
    else if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        diropen->setCheck(true);
        fileopen->setCheck(true);
    }
    else
    {
        diropen->setCheck(false);
        fileopen->setCheck(false);
    }

    // File tooltips
    FXbool file_tooltips = getApp()->reg().readUnsignedEntry("SETTINGS", "file_tooltips", true);
    filetooltips->setCheck(file_tooltips);

    // Relative resizing
    FXbool relative_resize = getApp()->reg().readUnsignedEntry("SETTINGS", "relative_resize", true);
    relativeresize->setCheck(relative_resize);

    // Always show tab bar
    FXbool always_show_tabbar = getApp()->reg().readUnsignedEntry("OPTIONS", "always_show_tabbar", true);
    alwaysshowtabbar->setCheck(always_show_tabbar);

    // Restore tabs
    FXbool restore_tabs = getApp()->reg().readUnsignedEntry("OPTIONS", "restore_tabs", false);
    restoretabs->setCheck(restore_tabs);

    // Display path linker
    FXbool show_pathlink = getApp()->reg().readUnsignedEntry("SETTINGS", "showpathlinker", true);
    showpathlink->setCheck(show_pathlink);

#ifdef STARTUP_NOTIFICATION
    FXbool use_sn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    usesn->setCheck(use_sn);
#endif

    // Do not run script files
    FXbool no_script = getApp()->reg().readUnsignedEntry("OPTIONS", "no_script", false);
    noscript->setCheck(no_script);

    // Fourth tab - Columns order
    new FXTabItem(tabbook, _("File &List"), NULL);
    FXVerticalFrame* columns = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    group = new FXGroupBox(columns, _("Displayed Columns"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    FXVerticalFrame* frame = new FXVerticalFrame(group, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X);
    new FXLabel(frame, _("(Name column is always displayed)"), NULL, JUSTIFY_LEFT);
    matrix = new FXMatrix(frame, 4, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXCheckButton* namecol = new FXCheckButton(matrix, _("Name") + FXString("       ")); // Always at first position
    sizecol = new FXCheckButton(matrix, _("Size") + FXString("       "), this, ID_VIEW_COL);
    typecol = new FXCheckButton(matrix, _("Type") + FXString("       "), this, ID_VIEW_COL);
    extcol = new FXCheckButton(matrix, _("Extension") + FXString("       "), this, ID_VIEW_COL);
    datecol = new FXCheckButton(matrix, _("Modified Date") + FXString("       "), this, ID_VIEW_COL);
    usercol = new FXCheckButton(matrix, _("User") + FXString("       "), this, ID_VIEW_COL);
    groupcol = new FXCheckButton(matrix, _("Group") + FXString("       "), this, ID_VIEW_COL);
    permscol = new FXCheckButton(matrix, _("Permissions") + FXString("       "), this, ID_VIEW_COL);
    linkcol = new FXCheckButton(matrix, _("Link") + FXString("       "), this, ID_VIEW_COL);

    // Read file list columns order and show status
    idCol[0] = FileList::ID_COL_NAME; // Name is always at position 0
    namecol->setCheck(true);
    namecol->disable();

    FXuint i = FileList::ID_COL_NAME;
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_name", 1);
    namecol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);
    sizecol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);
    typecol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);
    extcol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);
    datecol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);
    usercol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);
    groupcol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);
    permscol->setCheck(colShown[i++]);
    colShown[i] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);
    linkcol->setCheck(colShown[i++]);

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

    group = new FXGroupBox(columns, _("Column Order"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    frame = new FXVerticalFrame(group, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X);
    new FXLabel(frame, _("(Name column is always first)"), NULL, JUSTIFY_LEFT);
    hframe = new FXHorizontalFrame(frame, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    listcol = new FXList(hframe, this, ID_LISTCOL,
                         LAYOUT_FILL_X | LAYOUT_FILL_Y | SCROLLERS_DONT_TRACK | FRAME_NONE | LIST_BROWSESELECT);
    listcol->setNumVisible(NMAX_COLS);
    for (FXuint i = 0; i < nbCols; i++)
    {
        listcol->appendItem(getHeaderName(idCol[i]));
    }
    listcol->disableItem(0);
    if (nbCols > 1)
    {
        listcol->selectItem(1);
    }

    frame = new FXVerticalFrame(hframe, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_Y);
    listcol_up = new FXButton(frame, _("\tMove up"), miniupicon, this, ID_LISTCOL_UP,
                              FRAME_GROOVE | LAYOUT_LEFT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 10, 10);
    listcol_down = new FXButton(frame, _("\tMove down"), minidownicon, this, ID_LISTCOL_DOWN,
                                FRAME_GROOVE | LAYOUT_LEFT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 10, 10);

    // Fifth tab - Programs
    new FXTabItem(tabbook, _("&Programs"), NULL);
    FXVerticalFrame* programs = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    group = new FXGroupBox(programs, _("Default Programs"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    matrix = new FXMatrix(group, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Text viewer:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    txtviewer = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_TXTVIEW,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldtxtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
    txtviewer->setText(oldtxtviewer);

    new FXLabel(matrix, _("Text editor:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    txteditor = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_TXTEDIT,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldtxteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
    txteditor->setText(oldtxteditor);

    new FXLabel(matrix, _("File / folder comparator:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    filecomparator = new FXTextField(matrix, 30, NULL, 0,
                                     TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_TXTEDIT,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldfilecomparator = getApp()->reg().readStringEntry("PROGS", "filecomparator", DEFAULT_FILECOMPARATOR);
    filecomparator->setText(oldfilecomparator);

    new FXLabel(matrix, _("Image editor:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    imgeditor = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_IMGVIEW,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldimgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
    imgeditor->setText(oldimgeditor);

    new FXLabel(matrix, _("Image viewer:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    imgviewer = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_IMGVIEW,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldimgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
    imgviewer->setText(oldimgviewer);

    new FXLabel(matrix, _("Archiver:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    archiver = new FXTextField(matrix, 30, NULL, 0,
                               TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_ARCHIVER,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldarchiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);
    archiver->setText(oldarchiver);

    new FXLabel(matrix, _("Pdf viewer:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    pdfviewer = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_PDFVIEW,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldpdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
    pdfviewer->setText(oldpdfviewer);

    new FXLabel(matrix, _("Audio player:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    audioplayer = new FXTextField(matrix, 30, NULL, 0,
                                  TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_AUDIOPLAY,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldaudioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
    audioplayer->setText(oldaudioplayer);

    new FXLabel(matrix, _("Video player:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    videoplayer = new FXTextField(matrix, 30, NULL, 0,
                                  TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_VIDEOPLAY,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldvideoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
    videoplayer->setText(oldvideoplayer);

    new FXLabel(matrix, _("Terminal:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    xterm = new FXTextField(matrix, 30, NULL, 0,
                            TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_XTERM,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldxterm = getApp()->reg().readStringEntry("PROGS", "xterm", DEFAULT_TERMINAL);
    xterm->setText(oldxterm);

    group = new FXGroupBox(programs, _("Volume Management"),
                           GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    matrix = new FXMatrix(group, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Mount:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    mountcmd = new FXTextField(matrix, 30, NULL, 0,
                               TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_MOUNTCMD,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldmountcmd = getApp()->reg().readStringEntry("PROGS", "mount", DEFAULT_MOUNTCMD);
    mountcmd->setText(oldmountcmd);

    new FXLabel(matrix, _("Unmount:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    umountcmd = new FXTextField(matrix, 30, NULL, 0,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix, _("\tSelect file..."), minifiledialogicon, this, ID_BROWSE_UMOUNTCMD,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    oldumountcmd = getApp()->reg().readStringEntry("PROGS", "unmount", DEFAULT_UMOUNTCMD);
    umountcmd->setText(oldumountcmd);

    // Sixth tab - Appearance
    new FXTabItem(tabbook, _("&Appearance"), NULL);
    FXVerticalFrame* visual = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    FXGroupBox* themes = new FXGroupBox(visual, _("Color Theme"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    vframe = new FXVerticalFrame(themes, FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    new FXLabel(vframe, _("(Right click on the current theme to save it as a custom theme. Right click \n\
on a custom theme to rename or remove it.)"), NULL, JUSTIFY_LEFT, 0, 0, 0, 0, 0, 0, 10, 10);

    FXPacker* pack = new FXPacker(vframe, FRAME_NONE | LAYOUT_FILL_Y | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0);
    themesList = new FXList(pack, this, ID_THEME, LIST_BROWSESELECT | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    themesList->setNumVisible(7);
    for (FXuint i = 0; i < Themes.size(); i++)
    {
        themesList->appendItem(Themes[i].name);
    }
    themesList->setCurrentItem(0);

    FXGroupBox* colors = new FXGroupBox(visual, _("Custom Colors"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X);
    FXMatrix* matrix3 = new FXMatrix(colors, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                     0, 0, 0, 0, 0);
    colorsBox = new ComboBox(matrix3, NUM_COLORS, false, false, NULL, 0,
                             COMBOBOX_STATIC | LAYOUT_FILL_X | LAYOUT_SIDE_RIGHT | LAYOUT_CENTER_Y);
    colorsBox->setNumVisible(NUM_COLORS);
    cwell = new ColorWell(matrix3, FXRGB(0, 0, 0), this, ID_COLOR, LAYOUT_FILL_X | LAYOUT_FILL_COLUMN | LAYOUT_FILL_Y,
                          0, 0, 0, 0, 10, 10, 0, 0);
    cwell->setTipText(_("Double click to customize the color"));

    colorsBox->appendItem(_("Base color"));
    colorsBox->appendItem(_("Border color"));
    colorsBox->appendItem(_("Background color"));
    colorsBox->appendItem(_("Text color"));
    colorsBox->appendItem(_("Selection background color"));
    colorsBox->appendItem(_("Selection text color"));
    colorsBox->appendItem(_("File list background color"));
    colorsBox->appendItem(_("File list text color"));
    colorsBox->appendItem(_("File list highlight color"));
    colorsBox->appendItem(_("Progress bar color"));
    colorsBox->appendItem(_("Attention color"));
    colorsBox->appendItem(_("Scrollbar color"));
    colorsBox->setCurrentItem(0);

    // Monitor resolution
    FXGroupBox* ui = new FXGroupBox(visual, _("Screen Resolution"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X);
    matrix = new FXMatrix(ui, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP);
    new FXLabel(matrix, _("DPI:") + FXString(" "), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    spindpi = new FXSpinner(matrix, 4, this, ID_UIDPI, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2,
                            2, 1, 1);
    spindpi->setRange(60, 800);
    uidpi = getApp()->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    spindpi->setValue(uidpi);

    // Find iconpath from the Xfe registry settings or set it to default
    FXGroupBox* group2 = new FXGroupBox(visual, _("Icon Theme Path"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X);
    FXMatrix* matrix2 = new FXMatrix(group2, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    iconpath = new FXTextField(matrix2, 40, NULL, 0,
                               TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(matrix2, _("\tSelect path..."), minifiledialogicon, this, ID_BROWSE_ICON_PATH,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    FXString defaulticonpath = xf_realpath(FXPath::directory(execpath) + "/../share/xfe/icons/default-theme");
    oldiconpath = xf_realpath(getApp()->reg().readStringEntry("SETTINGS", "iconpath", defaulticonpath.text()));
    iconpath->setText(oldiconpath);

    // Seventh tab - Fonts
    new FXTabItem(tabbook, _("&Fonts"), NULL);
    FXVerticalFrame* fonts = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    FXGroupBox* fgroup = new FXGroupBox(fonts, _("Fonts"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXMatrix* fmatrix = new FXMatrix(fgroup, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(fmatrix, _("Normal font:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    normalfont = new FXTextField(fmatrix, 30, NULL, 0,
                                 TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(fmatrix, _(" Select..."), NULL, this, ID_NORMALFONT, FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y); //,0,0,0,0,20,20);
    oldnormalfont = getApp()->reg().readStringEntry("SETTINGS", "font", DEFAULT_NORMAL_FONT);
    normalfont->setText(oldnormalfont);

    new FXLabel(fmatrix, _("Text font:"), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    textfont = new FXTextField(fmatrix, 30, NULL, 0,
                               TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    new FXButton(fmatrix, _(" Select..."), NULL, this, ID_TEXTFONT, FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y); //0,0,0,0,20,20);
    oldtextfont = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    textfont->setText(oldtextfont);

    // Eighth tab - Key bindings
    new FXTabItem(tabbook, _("&Key Bindings"), NULL);
    FXVerticalFrame* keybindings = new FXVerticalFrame(tabbook, FRAME_NONE, 0, 0, 0, 0, 0, 0, 10, 0);
    FXGroupBox* kbgroup = new FXGroupBox(keybindings, _("Key Bindings"),
                                         GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXPacker* kbpack = new FXPacker(kbgroup, LAYOUT_FILL_X);
    new FXButton(kbpack, _("&Modify key bindings..."), minikeybindingsicon, this, ID_CHANGE_KEYBINDINGS,
                 FRAME_GROOVE | ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);    //,0,0,0,0,20,20);
    new FXButton(kbpack, _("&Restore default key bindings..."), minireloadicon, this, ID_RESTORE_KEYBINDINGS,
                 FRAME_GROOVE | ICON_BEFORE_TEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);    //,0,0,0,0,20,20);

    // Reset restarted flag
    getApp()->reg().writeUnsignedEntry("SETTINGS", "restarted", false);
}


long PreferencesBox::onUpdColor(FXObject* sender, FXSelector sel, void* ptr)
{
    ColorWell* cwell = (ColorWell*)sender;
    int i = colorsBox->getCurrentItem();

    cwell->setRGBA(currTheme.color[i]);

    return 1;
}


long PreferencesBox::onCmdColor(FXObject* sender, FXSelector sel, void* ptr)
{
    ColorWell* cwell = (ColorWell*)sender;
    int i = colorsBox->getCurrentItem();

    currTheme.color[i] = cwell->getRGBA();

    return 1;
}


// Context menu on themes list
long PreferencesBox::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    int x, y;
    FXuint state;

    themesList->getCursorPosition(x, y, state);
    int item = themesList->getItemAt(x, y);

    // Current theme
    if (item == 0)
    {
        themesList->setCurrentItem(item);
        themesList->selectItem(item);

        // Menu items
        FXMenuPane* menu = new FXMenuPane(this);

        getRoot()->getCursorPosition(x, y, state);

        new FXMenuCommand(menu, _("&Save As..."), minisaveasicon, this, PreferencesBox::ID_THEME_SAVEAS);

        menu->create();
        menu->popup(NULL, x, y);
        getApp()->runModalWhileShown(menu);

        delete(menu);
    }
    else if (item >= NUM_DEFAULT_THEMES && item < (int)Themes.size())
    {
        themesList->setCurrentItem(item);
        themesList->selectItem(item);

        // Menu items
        FXMenuPane* menu = new FXMenuPane(this);

        getRoot()->getCursorPosition(x, y, state);

        new FXMenuCommand(menu, _("Re&name..."), minirenameicon, this, PreferencesBox::ID_THEME_RENAME);
        new FXMenuCommand(menu, _("&Remove"), minideleteicon, this, PreferencesBox::ID_THEME_REMOVE);

        menu->create();
        menu->popup(NULL, x, y);
        getApp()->runModalWhileShown(menu);

        delete(menu);
        
    }
    else
    {
        return 0;
    }

    return 1;
}


// Save current theme
long PreferencesBox::onCmdThemeSaveAs(FXObject* sender, FXSelector sel, void* ptr)
{
    // Dialog
    InputDialog* savecurrentdialog = new InputDialog(this, "", _("Enter theme name:"), _("Save current theme"), "",
                                                     bigsaveasicon, false);
    savecurrentdialog->setText("");

    // Accept was pressed
    if (savecurrentdialog->execute(PLACEMENT_CURSOR))
    {
        if (savecurrentdialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Theme name is empty, operation cancelled"));
            return 0;
        }

        // Theme name
        FXString name = savecurrentdialog->getText();
        
        // Theme name already exists
        FXbool exist = false;
        for (FXuint n = 0; n < Themes.size(); n++)
        {
            if (Themes[n].name == name)
            {
                exist = true;
                break;
            }
        }
        if (exist)
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Theme name already exists, operation cancelled"));
            return 0;            
        }
   
        // Add theme
        Theme theme = Theme(name.text(), "", currTheme.color[0], currTheme.color[1], currTheme.color[2],
                            currTheme.color[3], currTheme.color[4], currTheme.color[5], currTheme.color[6],
                            currTheme.color[7], currTheme.color[8], currTheme.color[9], currTheme.color[10],
                            currTheme.color[11]);
        Themes.push_back(theme);
        themesList->appendItem(theme.name);
    }

    // Cancel was pressed
    else
    {
        return 0;
    }

    return 1;
}


// Rename theme
long PreferencesBox::onCmdThemeRename(FXObject* sender, FXSelector sel, void* ptr)
{
    int index = themesList->getCurrentItem();
    
    if (index > 0 && index < (int)Themes.size())
    {
        // Dialog
        InputDialog* renamedialog = new InputDialog(this, "", _("Enter theme name:"), _("Rename theme"), "",
                                                    bigrenameicon, false);
        renamedialog->setText(Themes[index].name);

        // Accept was pressed
        if (renamedialog->execute(PLACEMENT_CURSOR))
        {
            if (renamedialog->getText() == "")
            {
                MessageBox::warning(this, BOX_OK, _("Warning"), _("Theme name is empty, operation cancelled"));
                return 0;
            }

            // New theme name
            FXString name = renamedialog->getText();

            // Theme name already exists
            FXbool exist = false;
            for (FXuint n = 0; n < Themes.size(); n++)
            {
                if (Themes[n].name == name)
                {
                    exist = true;
                    break;
                }
            }
            if (exist)
            {
                MessageBox::warning(this, BOX_OK, _("Warning"), _("Theme name already exists, operation cancelled"));
                return 0;            
            }

            // Rename theme
            Themes[index].name = name;           
            themesList->setItemText(index, name);
        }

        // Cancel was pressed
        else
        {
            return 0;
        }      
    }
    else
    {
        return 0;
    }

    return 1;
}


// Remove theme
long PreferencesBox::onCmdThemeRemove(FXObject* sender, FXSelector sel, void* ptr)
{
    int index = themesList->getCurrentItem();
    
    if (index > 0 && index < (int)Themes.size())
    {
        // Remove theme
        Themes.erase(Themes.begin() + index);
        themesList->clearItems();
        for (FXuint i = 0; i < Themes.size(); i++)
        {
            themesList->appendItem(Themes[i].name);
        }
        themesList->setCurrentItem(FXMAX(index - 1, 0));
    }
    else
    {
        return 0;
    }

    return 1;
}


// Select theme
long PreferencesBox::onCmdTheme(FXObject* sender, FXSelector sel, void* ptr)
{
    currTheme = Themes[themesList->getCurrentItem()];
    return 1;
}


// Browse icons path
long PreferencesBox::onCmdBrowsePath(FXObject* o, FXSelector s, void* p)
{
    FileDialog browse(this, _("Select an icon theme folder or an icon file"));

    browse.setSelectMode(SELECT_FILE_MIXED);

    FXString path = iconpath->getText();
    if (path[0] == '~')
    {
        path = FXSystem::getHomeDirectory() + path.after('~');
    }

    browse.setDirectory(path);
    if (browse.execute())
    {
        FXString path = browse.getFilename();
        if (xf_isfile(path))
        {
            iconpath->setText(FXPath::directory(path).text());
        }
        else
        {
            iconpath->setText(path);
        }
    }
    return 1;
}


// Browse file system
long PreferencesBox::onCmdBrowse(FXObject* sender, FXSelector sel, void* ptr)
{
    FileDialog browse(this, _("Select an executable file"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };

    browse.setFilename(ROOTDIR);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_EXISTING);
    if (browse.execute())
    {
        FXString path = browse.getFilename();

        switch (FXSELID(sel))
        {
        case ID_BROWSE_TXTVIEW:
            txtviewer->setText(path);
            break;

        case ID_BROWSE_TXTEDIT:
            txteditor->setText(path);
            break;

        case ID_BROWSE_FILECOMP:
            filecomparator->setText(path);
            break;

        case ID_BROWSE_IMGVIEW:
            imgviewer->setText(path);
            break;

        case ID_BROWSE_ARCHIVER:
            archiver->setText(path);
            break;

        case ID_BROWSE_PDFVIEW:
            pdfviewer->setText(path);
            break;

        case ID_BROWSE_AUDIOPLAY:
            audioplayer->setText(path);
            break;

        case ID_BROWSE_VIDEOPLAY:
            videoplayer->setText(path);
            break;

        case ID_BROWSE_XTERM:
            xterm->setText(path);
            break;

        case ID_BROWSE_MOUNTCMD:
            mountcmd->setText(path);
            break;

        case ID_BROWSE_UMOUNTCMD:
            umountcmd->setText(path);
            break;
        }
    }
    return 1;
}


// Change normal font
long PreferencesBox::onCmdNormalFont(FXObject*, FXSelector, void*)
{
    FontDialog fontdlg(this, _("Change Normal Font"), DECOR_BORDER | DECOR_TITLE);
    FXFontDesc fontdesc;
    FXString fontspec;

    fontspec = getApp()->reg().readStringEntry("SETTINGS", "font", DEFAULT_NORMAL_FONT);
    FXFont* nfont = new FXFont(getApp(), fontspec);
    nfont->create();
    nfont->getFontDesc(fontdesc);

    fontdlg.setFontSelection(fontdesc);
    if (fontdlg.execute())
    {
        fontdlg.getFontSelection(fontdesc);
        nfont->setFontDesc(fontdesc);
        fontspec = nfont->getFont();
        normalfont->setText(fontspec);
    }

    delete nfont;
    return 1;
}


// Change text font
long PreferencesBox::onCmdTextFont(FXObject*, FXSelector, void*)
{
    FontDialog fontdlg(this, _("Change Text Font"), DECOR_BORDER | DECOR_TITLE);
    FXFontDesc fontdesc;
    FXString fontspec;

    fontspec = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    FXFont* tfont = new FXFont(getApp(), fontspec);
    tfont->create();
    tfont->getFontDesc(fontdesc);
    fontdlg.setFontSelection(fontdesc);
    if (fontdlg.execute())
    {
        fontdlg.getFontSelection(fontdesc);
        tfont->setFontDesc(fontdesc);
        fontspec = tfont->getFont();
        textfont->setText(fontspec);
    }

    delete tfont;
    return 1;
}


// Change key bindings
long PreferencesBox::onCmdChangeKeyBindings(FXObject*, FXSelector, void*)
{
    FXString key, str;

    // String dictionary used to store global key bindings
    if (glbBindingsDict == NULL)
    {
        glbBindingsDict = new FXStringDict();
    }

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_back", "Ctrl-Backspace");
    str = _("Go to previous folder") + TAB + key;
    glbBindingsDict->insert("go_back", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_forward", "Shift-Backspace");
    str = _("Go to next folder") + TAB + key;
    glbBindingsDict->insert("go_forward", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    str = _("Go to parent folder") + TAB + key;
    glbBindingsDict->insert("go_up", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    str = _("Go to home folder") + TAB + key;
    glbBindingsDict->insert("go_home", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_file", "Ctrl-N");
    str = _("Create new file") + TAB + key;
    glbBindingsDict->insert("new_file", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_folder", "F7");
    str = _("Create new folder") + TAB + key;
    glbBindingsDict->insert("new_folder", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    str = _("Copy to clipboard") + TAB + key;
    glbBindingsDict->insert("copy", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    str = _("Cut to clipboard") + TAB + key;
    glbBindingsDict->insert("cut", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "paste", "Ctrl-V");
    str = _("Paste from clipboard") + TAB + key;
    glbBindingsDict->insert("paste", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    str = _("Big icon list") + TAB + key;
    glbBindingsDict->insert("big_icons", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    str = _("Small icon list") + TAB + key;
    glbBindingsDict->insert("small_icons", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    str = _("Detailed file list") + TAB + key;
    glbBindingsDict->insert("detailed_file_list", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    str = _("Open file") + TAB + key;
    glbBindingsDict->insert("open", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "quit", "Ctrl-Q");
    str = _("Quit application") + TAB + key;
    glbBindingsDict->insert("quit", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "select_all", "Ctrl-A");
    str = _("Select all") + TAB + key;
    glbBindingsDict->insert("select_all", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "deselect_all", "Ctrl-Z");
    str = _("Deselect all") + TAB + key;
    glbBindingsDict->insert("deselect_all", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "invert_selection", "Ctrl-I");
    str = _("Invert selection") + TAB + key;
    glbBindingsDict->insert("invert_selection", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "help", "F1");
    str = _("Display help") + TAB + key;
    glbBindingsDict->insert("help", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    str = _("Toggle display hidden files") + TAB + key;
    glbBindingsDict->insert("hidden_files", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    str = _("Toggle display thumbnails") + TAB + key;
    glbBindingsDict->insert("thumbnails", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_work", "Shift-F2");
    str = _("Go to working folder") + TAB + key;
    glbBindingsDict->insert("go_work", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    str = _("Close window") + TAB + key;
    glbBindingsDict->insert("close", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "print", "Ctrl-P");
    str = _("Print file") + TAB + key;
    glbBindingsDict->insert("print", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "search", "Ctrl-F");
    str = _("Search") + TAB + key;
    glbBindingsDict->insert("search", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "search_prev", "Ctrl-Shift-G");
    str = _("Search previous") + TAB + key;
    glbBindingsDict->insert("search_prev", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "search_next", "Ctrl-G");
    str = _("Search next") + TAB + key;
    glbBindingsDict->insert("search_next", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    str = _("Vertical panels") + TAB + key;
    glbBindingsDict->insert("vert_panels", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");
    str = _("Horizontal panels") + TAB + key;
    glbBindingsDict->insert("horz_panels", str.text());

    // Key bindings specific to X File Explorer (Xfe)
    if (xfeBindingsDict == NULL)
    {
        xfeBindingsDict = new FXStringDict();
    }

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    str = _("Refresh panels") + TAB + key;
    xfeBindingsDict->insert("refresh", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_symlink", "Ctrl-J");
    str = _("Create new symbolic link") + TAB + key;
    xfeBindingsDict->insert("new_symlink", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "properties", "F9");
    str = _("File properties") + TAB + key;
    xfeBindingsDict->insert("properties", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "dirpanel_places", "Ctrl-F9");
    str = _("Show places") + TAB + key;
    xfeBindingsDict->insert("dirpanel_places", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "dirpanel_tree", "Ctrl-F10");
    str = _("Show tree") + TAB + key;
    xfeBindingsDict->insert("dirpanel_tree", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    str = _("Move files to trash") + TAB + key;
    xfeBindingsDict->insert("move_to_trash", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "restore_from_trash", "Alt-Del");
    str = _("Restore files from trash") + TAB + key;
    xfeBindingsDict->insert("restore_from_trash", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    str = _("Delete files") + TAB + key;
    xfeBindingsDict->insert("delete", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_names", "Ctrl-Shift-N");
    str = _("Copy file names to clipboard") + TAB + key;
    xfeBindingsDict->insert("copy_names", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_window", "F3");
    str = _("Create new window") + TAB + key;
    xfeBindingsDict->insert("new_window", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_root_window", "Shift-F3");
    str = _("Create new root window") + TAB + key;
    xfeBindingsDict->insert("new_root_window", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "execute_command", "Ctrl-E");
    str = _("Execute command") + TAB + key;
    xfeBindingsDict->insert("execute_command", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "terminal", "Ctrl-T");
    str = _("Launch terminal") + TAB + key;
    xfeBindingsDict->insert("terminal", str.text());

#if defined(linux)
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "connect_to_server", "Shift-F5");
    str = _("Connect to server (Linux only)") + TAB + key;
    xfeBindingsDict->insert("connect_to_server", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mount", "Ctrl-M");
    str = _("Mount file system (Linux only)") + TAB + key;
    xfeBindingsDict->insert("mount", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "unmount", "Ctrl-U");
    str = _("Unmount file system (Linux only)") + TAB + key;
    xfeBindingsDict->insert("unmount", str.text());
#endif
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "one_panel", "Ctrl-F1");
    str = _("One panel mode") + TAB + key;
    xfeBindingsDict->insert("one_panel", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_panel", "Ctrl-F2");
    str = _("Folders and panel mode") + TAB + key;
    xfeBindingsDict->insert("tree_panel", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "two_panels", "Ctrl-F3");
    str = _("Two panels mode") + TAB + key;
    xfeBindingsDict->insert("two_panels", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "tree_two_panels", "Ctrl-F4");
    str = _("Folders and two panels mode") + TAB + key;
    xfeBindingsDict->insert("tree_two_panels", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "clear_address", "Ctrl-L");
    str = _("Clear address bar") + TAB + key;
    xfeBindingsDict->insert("clear_address", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rename", "F2");
    str = _("Rename files") + TAB + key;
    xfeBindingsDict->insert("rename", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_to", "F5");
    str = _("Copy files to location") + TAB + key;
    xfeBindingsDict->insert("copy_to", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to", "F6");
    str = _("Move files to location") + TAB + key;
    xfeBindingsDict->insert("move_to", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "symlink_to", "Ctrl-S");
    str = _("Symlink files to location") + TAB + key;
    xfeBindingsDict->insert("symlink_to", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "add_bookmark", "Ctrl-B");
    str = _("Add bookmark") + TAB + key;
    xfeBindingsDict->insert("add_bookmark", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "synchronize_panels", "Ctrl-Y");
    str = _("Synchronize panels") + TAB + key;
    xfeBindingsDict->insert("synchronize_panels", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "switch_panels", "Ctrl-K");
    str = _("Switch panels") + TAB + key;
    xfeBindingsDict->insert("switch_panels", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_to_trash", "Ctrl-F8");
    str = _("Go to trash can") + TAB + key;
    xfeBindingsDict->insert("go_to_trash", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "empty_trash_can", "Ctrl-Del");
    str = _("Empty trash can") + TAB + key;
    xfeBindingsDict->insert("empty_trash_can", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view", "Shift-F4");
    str = _("View") + TAB + key;
    xfeBindingsDict->insert("view", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "edit", "F4");
    str = _("Edit") + TAB + key;
    xfeBindingsDict->insert("edit", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "compare", "F8");
    str = _("Compare") + TAB + key;
    xfeBindingsDict->insert("compare", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_dirs", "Ctrl-F5");
    str = _("Toggle display hidden folders") + TAB + key;
    xfeBindingsDict->insert("hidden_dirs", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "filter", "Ctrl-D");
    str = _("Filter files") + TAB + key;
    xfeBindingsDict->insert("filter", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_tab", "Shift-F1");
    str = _("New tab") + TAB + key;
    xfeBindingsDict->insert("new_tab", str.text());

    // Key bindings specific to X File Image (Xfi)
    if (xfiBindingsDict == NULL)
    {
        xfiBindingsDict = new FXStringDict();
    }

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_prev", "Ctrl-J");
    str = _("View previous image") + TAB + key;
    xfiBindingsDict->insert("view_prev", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_next", "Ctrl-K");
    str = _("View next image") + TAB + key;
    xfiBindingsDict->insert("view_next", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_win", "Ctrl-F");
    str = _("Zoom to fit window") + TAB + key;
    xfiBindingsDict->insert("zoom_win", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_100", "Ctrl-I");
    str = _("Zoom image to 100%") + TAB + key;
    xfiBindingsDict->insert("zoom_100", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_win", "Ctrl-F");
    str = _("Zoom to fit window") + TAB + key;
    xfiBindingsDict->insert("zoom_win", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_left", "Ctrl-L");
    str = _("Rotate image to left") + TAB + key;
    xfiBindingsDict->insert("rotate_left", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_right", "Ctrl-R");
    str = _("Rotate image to right") + TAB + key;
    xfiBindingsDict->insert("rotate_right", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_horizontally", "Ctrl-Shift-H");
    str = _("Mirror image horizontally") + TAB + key;
    xfiBindingsDict->insert("mirror_horizontally", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_vertically", "Ctrl-Shift-V");
    str = _("Mirror image vertically") + TAB + key;
    xfiBindingsDict->insert("mirror_vertically", str.text());

    // Key bindings specific to X File Write (Xfw)
    if (xfwBindingsDict == NULL)
    {
        xfwBindingsDict = new FXStringDict();
    }

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "new", "Ctrl-N");
    str = _("Create new document") + TAB + key;
    xfwBindingsDict->insert("new", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "save", "Ctrl-S");
    str = _("Save changes to file") + TAB + key;
    xfwBindingsDict->insert("save", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "goto_line", "Ctrl-L");
    str = _("Goto line") + TAB + key;
    xfwBindingsDict->insert("goto_line", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "undo", "Ctrl-Z");
    str = _("Undo last change") + TAB + key;
    xfwBindingsDict->insert("undo", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "redo", "Ctrl-Y");
    str = _("Redo last change") + TAB + key;
    xfwBindingsDict->insert("redo", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "replace", "Ctrl-R");
    str = _("Replace string") + TAB + key;
    xfwBindingsDict->insert("replace", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "word_wrap", "Ctrl-K");
    str = _("Toggle word wrap mode") + TAB + key;
    xfwBindingsDict->insert("word_wrap", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "line_numbers", "Ctrl-T");
    str = _("Toggle line numbers mode") + TAB + key;
    xfwBindingsDict->insert("line_numbers", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "lower_case", "Ctrl-U");
    str = _("Toggle lower case mode") + TAB + key;
    xfwBindingsDict->insert("lower_case", str.text());

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "upper_case", "Ctrl-Shift-U");
    str = _("Toggle upper case mode") + TAB + key;
    xfwBindingsDict->insert("upper_case", str.text());

    // Display the key bindings dialog box
    if (bindingsbox == NULL)
    {
        bindingsbox = new KeyBindingsBox(this, glbBindingsDict, xfeBindingsDict, xfiBindingsDict, xfwBindingsDict);
    }

    bindingsbox->execute(PLACEMENT_OWNER);

    return 1;
}


// Restore default key bindings
long PreferencesBox::onCmdRestoreKeyBindings(FXObject*, FXSelector, void*)
{
    // Confirmation message
    FXString message =
        _("Do you really want to restore the default key bindings?\n\nAll your customizations will be lost!");
    MessageBox box(this, _("Restore default key bindings"), message, bigkeybindingsicon,
                   BOX_OK_CANCEL | DECOR_TITLE | DECOR_BORDER);

    if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
    {
        return 0;
    }

    // Write default key bindings to the registry

    // Global key bindings
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_back", "Ctrl-Backspace");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_forward", "Shift-Backspace");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_up", "Backspace");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_file", "Ctrl-N");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_folder", "F7");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "paste", "Ctrl-V");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "big_icons", "F10");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "small_icons", "F11");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "quit", "Ctrl-Q");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "select_all", "Ctrl-A");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "deselect_all", "Ctrl-Z");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "invert_selection", "Ctrl-I");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "help", "F1");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_work", "Shift-F2");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "print", "Ctrl-P");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "search", "Ctrl-F");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "search_prev", "Ctrl-Shift-G");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "search_next", "Ctrl-G");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");

    // Key bindings specific to X File Explorer (Xfe)
    getApp()->reg().writeStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_symlink", "Ctrl-J");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "properties", "F9");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "dirpanel_places", "Ctrl-F9");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "dirpanel_tree", "Ctrl-F10");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "restore_from_trash", "Alt-Del");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_window", "F3");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_root_window", "Shift-F3");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "execute_command", "Ctrl-E");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "terminal", "Ctrl-T");
#if defined(linux)
    getApp()->reg().writeStringEntry("KEYBINDINGS", "connect_to_server", "Shift-F5");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "mount", "Ctrl-M");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "unmount", "Ctrl-U");
#endif
    getApp()->reg().writeStringEntry("KEYBINDINGS", "one_panel", "Ctrl-F1");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "tree_panel", "Ctrl-F2");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "two_panels", "Ctrl-F3");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "tree_two_panels", "Ctrl-F4");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "clear_address", "Ctrl-L");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "rename", "F2");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "copy_to", "F5");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "copy_names", "Ctrl-Shift-N");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "move_to", "F6");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "symlink_to", "Ctrl-S");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "add_bookmark", "Ctrl-B");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "synchronize_panels", "Ctrl-Y");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "switch_panels", "Ctrl-K");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "go_to_trash", "Ctrl-F8");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "empty_trash_can", "Ctrl-Del");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "view", "Shift-F4");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "edit", "F4");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "compare", "F8");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "hidden_dirs", "Ctrl-F5");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "filter", "Ctrl-D");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new_tab", "Shift-F1");

    // Key bindings specific to X File Image (Xfi)
    getApp()->reg().writeStringEntry("KEYBINDINGS", "zoom_100", "Ctrl-I");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "zoom_win", "Ctrl-F");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "view_prev", "Ctrl-J");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "view_next", "Ctrl-K");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "rotate_left", "Ctrl-L");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "rotate_right", "Ctrl-R");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "mirror_horizontally", "Ctrl-Shift-H");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "mirror_vertically", "Ctrl-Shift-V");

    // Key bindings specific to X File Write (Xfw)
    getApp()->reg().writeStringEntry("KEYBINDINGS", "new", "Ctrl-N");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "save", "Ctrl-S");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "goto_line", "Ctrl-L");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "undo", "Ctrl-Z");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "redo", "Ctrl-Y");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "replace", "Ctrl-R");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "word_wrap", "Ctrl-K");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "line_numbers", "Ctrl-T");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "lower_case", "Ctrl-U");
    getApp()->reg().writeStringEntry("KEYBINDINGS", "upper_case", "Ctrl-Shift-U");

    // Finally, update the registry
    getApp()->reg().write();

    // Ask the user if he wants to restart Xfe
    if (BOX_CLICKED_CANCEL != MessageBox::question(this, BOX_OK_CANCEL, _("Restart"),
                              _("Key bindings will be changed after restart.\nRestart X File Explorer now?")))
    {
        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_RESTART), NULL);
    }

    return 1;
}


// Accept button pressed
long PreferencesBox::onCmdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    FXbool restart_theme = false;
    FXbool restart_smoothscroll = false;
    FXbool restart_scrollbarsize = false;
    FXbool restart_showpathlink = false;
    FXbool restart_normalfont = false;
    FXbool restart_textfont = false;
    FXbool restart_uidpi = false;
    FXbool restart_dirpanel_mode = false;
    FXbool restart_columns = false;
    FXbool restart_diropen = false;
    FXbool restart_fileopen = false;
    FXbool restart_relativeresize = false;
    FXbool restart_alwaysshowtabbar = false;
    FXbool restart_filetooltips = false;
    FXbool restart_automount = false;

    oldtimeformat = timeformat->getText();
    oldcopysuffix = copysuffix->getText();

    if (iconpath->getText() == "")
    {
        iconpath->setText(oldiconpath);
    }

    // Icon path has changed
    if (oldiconpath != iconpath->getText())
    {
        getApp()->reg().writeStringEntry("SETTINGS", "iconpath", iconpath->getText().text());
        getApp()->reg().write();
        restart_theme = true;
    }

    // Normal font has changed
    if (oldnormalfont != normalfont->getText())
    {
        getApp()->reg().writeStringEntry("SETTINGS", "font", normalfont->getText().text());
        getApp()->reg().write();
        restart_normalfont = true;
    }

    // Text font has changed
    if (oldtextfont != textfont->getText())
    {
        getApp()->reg().writeStringEntry("SETTINGS", "textfont", textfont->getText().text());
        getApp()->reg().write();
        restart_textfont = true;
    }

    // Note: code below is for compatibility with pre 1.40 Xfe versions
    // To be removed in the future!

    // Text viewer has changed
    if (oldtxtviewer != txtviewer->getText())
    {
        // Update the txtviewer string
        FXString newtxtviewer = txtviewer->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "txtviewer", newtxtviewer.text());

        // Update each filetype where the old txtviewer was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old txtviewer string with the new one
            if (value.contains(oldtxtviewer))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace only the view string, if needed
                if (view == oldtxtviewer)
                {
                    view = "<txtviewer>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Text editor has changed
    if (oldtxteditor != txteditor->getText())
    {
        // Update the txteditor string
        FXString newtxteditor = txteditor->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "txteditor", newtxteditor.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old txteditor was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old txteditor string with the new one
            if (value.contains(oldtxteditor))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace only the open and edit strings, if needed
                if (open == oldtxteditor)
                {
                    open = "<txteditor>";
                }
                if (edit == oldtxteditor)
                {
                    edit = "<txteditor>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // File comparator has changed
    if (oldfilecomparator != filecomparator->getText())
    {
        // Update the filecomparator string
        FXString newfilecomparator = filecomparator->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "filecomparator", newfilecomparator.text());
    }

    // Image editor has changed
    if (oldimgeditor != imgeditor->getText())
    {
        // Update the imgeditor string
        FXString newimgeditor = imgeditor->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "imgeditor", newimgeditor.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old imgeditor was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old imgeditor string with the new one
            if (value.contains(oldimgeditor))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace only the open and edit strings, if needed
                if (open == oldimgeditor)
                {
                    open = "<imgeditor>";
                }
                if (edit == oldimgeditor)
                {
                    edit = "<imgeditor>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Image viewer has changed
    if (oldimgviewer != imgviewer->getText())
    {
        // Update the imgviewer string
        FXString newimgviewer = imgviewer->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "imgviewer", newimgviewer.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old imgviewer was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old imgviewer string with the new one
            if (value.contains(oldimgviewer))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace the open and view string, if needed
                if (open == oldimgviewer)
                {
                    open = "<imgviewer>";
                }
                if (view == oldimgviewer)
                {
                    view = "<imgviewer>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Archiver has changed
    if (oldarchiver != archiver->getText())
    {
        // Update the archiver string
        FXString newarchiver = archiver->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "archiver", newarchiver.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old archiver was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old archiver string with the new one
            if (value.contains(oldarchiver))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace the open, view and edit strings, if needed
                if (open == oldarchiver)
                {
                    open = "<archiver>";
                }
                if (view == oldarchiver)
                {
                    view = "<archiver>";
                }
                if (edit == oldarchiver)
                {
                    edit = "<archiver>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // PDF viewer has changed
    if (oldpdfviewer != pdfviewer->getText())
    {
        // Update the PDF viewer string
        FXString newpdfviewer = pdfviewer->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "pdfviewer", newpdfviewer.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old PDF viewer was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old PDF viewer string with the new one
            if (value.contains(oldpdfviewer))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace the open, view and edit strings, if needed
                if (open == oldpdfviewer)
                {
                    open = "<pdfviewer>";
                }
                if (view == oldpdfviewer)
                {
                    view = "<pdfviewer>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Audio player has changed
    if (oldaudioplayer != audioplayer->getText())
    {
        // Update the audio player string
        FXString newaudioplayer = audioplayer->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "audioplayer", newaudioplayer.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old audio player was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old audio player string with the new one
            if (value.contains(oldaudioplayer))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace the open, view and edit strings, if needed
                if (open == oldaudioplayer)
                {
                    open = "<audioplayer>";
                }
                if (view == oldaudioplayer)
                {
                    view = "<audioplayer>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Video player has changed
    if (oldvideoplayer != videoplayer->getText())
    {
        // Update the video player string
        FXString newvideoplayer = videoplayer->getText().text();
        getApp()->reg().writeStringEntry("PROGS", "videoplayer", newvideoplayer.text());

        // Note: code below is for compatibility with pre 1.40 Xfe versions
        // To be removed in the future!

        // Update each filetype where the old video player was used
        FXStringDict* strdict = getApp()->reg().find("FILETYPES");
        FileDict* assoc = new FileDict(getApp());

        FXString key, value, newvalue;
        FXString strtmp, open, view, edit, command;
        for (int i = strdict->first(); i < strdict->size(); i = strdict->next(i))
        {
            // Read key and value of each filetype
            key = strdict->key(i);
            value = strdict->data(i);

            // Replace the old video player string with the new one
            if (value.contains(oldvideoplayer))
            {
                // Obtain the open, view, edit and command strings
                strtmp = value.before(';', 1);
                command = value.after(';', 1);
                open = strtmp.section(',', 0);
                view = strtmp.section(',', 1);
                edit = strtmp.section(',', 2);

                // Replace the open, view and edit strings, if needed
                if (open == oldvideoplayer)
                {
                    open = "<videoplayer>";
                }
                if (view == oldvideoplayer)
                {
                    view = "<videoplayer>";
                }

                // Replace with the new value
                value = open + "," + view + "," + edit + ";" + command;
                assoc->replace(key.text(), value.text());
            }
        }
        delete assoc;
    }

    // Terminal has changed
    if (oldxterm != xterm->getText())
    {
        getApp()->reg().writeStringEntry("PROGS", "xterm", xterm->getText().text());
    }

    // Mount command has changed
    if (oldmountcmd != mountcmd->getText())
    {
        getApp()->reg().writeStringEntry("PROGS", "mount", mountcmd->getText().text());
    }

    // Unmount command has changed
    if (oldumountcmd != umountcmd->getText())
    {
        getApp()->reg().writeStringEntry("PROGS", "unmount", umountcmd->getText().text());
    }

    getApp()->reg().writeUnsignedEntry("OPTIONS", "auto_save_layout", autosave->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "save_win_pos", savewinpos->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "use_trash_can", trashcan->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "use_trash_bypass", trashbypass->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "ask_before_copy", ask->getCheck());
    getApp()->reg().writeStringEntry("SETTINGS", "time_format", timeformat->getText().text());
    getApp()->reg().writeStringEntry("OPTIONS", "copysuffix", copysuffix->getText().text());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "copysuffix_pos", copysuffix_pos);
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_trash", trashmv->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_delete", del->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_properties", properties->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_delete_emptydir", del_emptydir->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_overwrite", overwrite->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_execute", exec->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "confirm_drag_and_drop", dnd->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "folder_warn", folder_warning->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "preserve_date_warn", preserve_date_warning->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "startdir_mode", startdirmode - ID_START_HOMEDIR);
    getApp()->reg().writeUnsignedEntry("OPTIONS", "newtabdir_mode", newtabdirmode - ID_NEWTAB_HOMEDIR);
    getApp()->reg().writeUnsignedEntry("OPTIONS", "root_warn", root_warning->getCheck());
#if defined(linux) && defined(XFE_AUTOMOUNTER)
    getApp()->reg().writeUnsignedEntry("OPTIONS", "automount", automountbutton->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "automount_open", autoopenbutton->getCheck());
#endif
    getApp()->reg().writeUnsignedEntry("OPTIONS", "root_mode", rootmode->getCheck());
    getApp()->reg().writeStringEntry("OPTIONS", "sudo_cmd", sudocmd->getText().text());
    getApp()->reg().writeStringEntry("OPTIONS", "su_cmd", sucmd->getText().text());
#ifdef STARTUP_NOTIFICATION
    getApp()->reg().writeUnsignedEntry("OPTIONS", "use_startup_notification", usesn->getCheck());
#endif
    getApp()->reg().writeUnsignedEntry("OPTIONS", "no_script", noscript->getCheck());
#if defined(linux)
    getApp()->reg().writeUnsignedEntry("OPTIONS", "mount_warn", mountwarn->getCheck());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "mount_messages", show_mount->getCheck());
#endif
    getApp()->reg().writeUnsignedEntry("OPTIONS", "dirpanel_mode", dirpanel_mode);
    getApp()->reg().writeUnsignedEntry("OPTIONS", "dirpanel_list_startup", dirpanel_list_startup);

    // Smooth scrolling
    getApp()->reg().writeUnsignedEntry("SETTINGS", "smooth_scroll", scroll->getCheck());
    if (scroll->getCheck() != smoothscroll_prev)
    {
        getApp()->reg().write();
        restart_smoothscroll = true;
    }

    // Scrollbar size
    if (getApp()->getScrollBarSize() != scrollbarsize_prev)
    {
        getApp()->reg().write();
        restart_scrollbarsize = true;
    }

    // Define hilite and shadow from basecolor
    getApp()->reg().writeColorEntry("SETTINGS", "hilitecolor", makeHiliteColorGradient(currTheme.color[0]));
    getApp()->reg().writeColorEntry("SETTINGS", "shadowcolor", makeShadowColorGradient(currTheme.color[0]));

    getApp()->reg().write();

    // Single click navigation
    FXuint single_click;
    if (diropen->getCheck() && fileopen->getCheck())
    {
        single_click = SINGLE_CLICK_DIR_FILE;
    }
    else if (diropen->getCheck() && !fileopen->getCheck())
    {
        single_click = SINGLE_CLICK_DIR;
    }
    else
    {
        single_click = SINGLE_CLICK_NONE;
    }

    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        ((XFileExplorer*)mainWindow)->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }
    else
    {
        ((XFileExplorer*)mainWindow)->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    }
    getApp()->reg().writeUnsignedEntry("SETTINGS", "single_click", (FXuint)single_click);
    if (diropen_prev != diropen->getCheck())
    {
        restart_diropen = true;
    }
    if (fileopen_prev != fileopen->getCheck())
    {
        restart_fileopen = true;
    }

    // File tooltips
    getApp()->reg().writeUnsignedEntry("SETTINGS", "file_tooltips", (FXuint)filetooltips->getCheck());
    if (filetooltips_prev != filetooltips->getCheck())
    {
        restart_filetooltips = true;
    }

    // Relative resize
    getApp()->reg().writeUnsignedEntry("SETTINGS", "relative_resize", (FXuint)relativeresize->getCheck());
    if (relativeresize_prev != relativeresize->getCheck())
    {
        restart_relativeresize = true;
    }

    // Always show tab bar
    getApp()->reg().writeUnsignedEntry("OPTIONS", "always_show_tabbar", (FXuint)alwaysshowtabbar->getCheck());
    if (alwaysshowtabbar_prev != alwaysshowtabbar->getCheck())
    {
        restart_alwaysshowtabbar = true;
    }

    // Restore tabs
    getApp()->reg().writeUnsignedEntry("OPTIONS", "restore_tabs", (FXuint)restoretabs->getCheck());

    // Path linker
    FXbool show_pathlink = showpathlink->getCheck();
    getApp()->reg().writeUnsignedEntry("SETTINGS", "showpathlinker", show_pathlink);
    if (show_pathlink != show_pathlink_prev)
    {
        getApp()->reg().write();
        restart_showpathlink = true;
    }

    // Theme has changed
    if (currTheme != Themes[0])
    {
        getApp()->reg().writeColorEntry("SETTINGS", "basecolor", currTheme.color[0]);
        getApp()->reg().writeColorEntry("SETTINGS", "bordercolor", currTheme.color[1]);
        getApp()->reg().writeColorEntry("SETTINGS", "backcolor", currTheme.color[2]);
        getApp()->reg().writeColorEntry("SETTINGS", "forecolor", currTheme.color[3]);
        getApp()->reg().writeColorEntry("SETTINGS", "selbackcolor", currTheme.color[4]);
        getApp()->reg().writeColorEntry("SETTINGS", "selforecolor", currTheme.color[5]);
        getApp()->reg().writeColorEntry("SETTINGS", "listbackcolor", currTheme.color[6]);
        getApp()->reg().writeColorEntry("SETTINGS", "listforecolor", currTheme.color[7]);
        getApp()->reg().writeColorEntry("SETTINGS", "highlightcolor", currTheme.color[8]);
        getApp()->reg().writeColorEntry("SETTINGS", "pbarcolor", currTheme.color[9]);
        getApp()->reg().writeColorEntry("SETTINGS", "attentioncolor", currTheme.color[10]);
        getApp()->reg().writeColorEntry("SETTINGS", "scrollbarcolor", currTheme.color[11]);

        // Define hilite and shadow colors from basecolor
        getApp()->reg().writeColorEntry("SETTINGS", "hilitecolor", makeHiliteColorGradient(currTheme.color[0]));
        getApp()->reg().writeColorEntry("SETTINGS", "shadowcolor", makeShadowColorGradient(currTheme.color[0]));

        getApp()->reg().write();
        restart_theme = true;
    }
    
    // Save custom themes
    if (Themes.size() > NUM_DEFAULT_THEMES)
    {
        // Delete themes section
        getApp()->reg().read();
        getApp()->reg().deleteSection("THEMES");
        
        // Add custom themes to registry
        for (FXuint i = NUM_DEFAULT_THEMES; i < Themes.size(); i++)
        {
            FXString regkey = "theme" + FXStringVal(i - NUM_DEFAULT_THEMES);          
            FXString str = Themes[i].name + ";" + Themes[i].tip + ";"
                           + FXStringVal(Themes[i].color[0]) + ";"
                           + FXStringVal(Themes[i].color[1]) + ";"
                           + FXStringVal(Themes[i].color[2]) + ";"
                           + FXStringVal(Themes[i].color[3]) + ";"
                           + FXStringVal(Themes[i].color[4]) + ";"
                           + FXStringVal(Themes[i].color[5]) + ";"
                           + FXStringVal(Themes[i].color[6]) + ";"
                           + FXStringVal(Themes[i].color[7]) + ";"
                           + FXStringVal(Themes[i].color[8]) + ";"
                           + FXStringVal(Themes[i].color[9]) + ";"
                           + FXStringVal(Themes[i].color[10]) + ";"
                           + FXStringVal(Themes[i].color[11]) + ";";
            
            getApp()->reg().writeStringEntry("THEMES", regkey.text(), str.text());
        }

        // Update registry
        getApp()->reg().write();       
    }

    // Thumbnails size
    bigthumb_size = bigthumb_spin->getValue();
    if (bigthumb_size != bigthumb_size_prev)
    {
        getApp()->reg().writeUnsignedEntry("SETTINGS", "bigthumb_size", bigthumb_size);
    }
    minithumb_size = minithumb_spin->getValue();
    if (minithumb_size != minithumb_size_prev)
    {
        getApp()->reg().writeUnsignedEntry("SETTINGS", "minithumb_size", minithumb_size);
    }

    // UI DPI
    uidpi = spindpi->getValue();
    if (uidpi != uidpi_prev)
    {
        getApp()->reg().writeUnsignedEntry("SETTINGS", "screenres", uidpi);
        getApp()->reg().write();
        restart_uidpi = true;
    }

    if (dirpanel_mode != dirpanel_mode_prev)
    {
        restart_dirpanel_mode = true;
    }

    // Columns shown status
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_name", 1); // Name is always shown
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_size", sizecol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_type", typecol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_ext", extcol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_date", datecol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_user", usercol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_group", groupcol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_perms", permscol->getCheck());
    getApp()->reg().writeUnsignedEntry("SETTINGS", "view_col_link", linkcol->getCheck());

    // Columns order
    for (FXuint i = 0; i < NMAX_COLS; i++)
    {
        FXString strid = "id_col_" + FXStringVal(i);
        if (i < nbCols)
        {
            getApp()->reg().writeUnsignedEntry("SETTINGS", strid.text(), idCol[i]);
        }
        else
        {
            getApp()->reg().writeUnsignedEntry("SETTINGS", strid.text(), 0);
        }
    }

    // Columns view have changed
    if (sizecol->getCheck() != sizecol_prev || typecol->getCheck() != typecol_prev ||
        extcol->getCheck() != extcol_prev || datecol->getCheck() != datecol_prev ||
        usercol->getCheck() != usercol_prev || groupcol->getCheck() != groupcol_prev ||
        permscol->getCheck() != permscol_prev || linkcol->getCheck() != linkcol_prev)
    {
        ((XFileExplorer*)mainWindow)->getLeftPanel()->setSortFunc(FileList::ascendingCase);
        ((XFileExplorer*)mainWindow)->getRightPanel()->setSortFunc(FileList::ascendingCase);

        if (((XFileExplorer*)mainWindow)->getSearchWindow() != NULL)
        {
            ((XFileExplorer*)mainWindow)->getSearchWindow()->setSortFunc(FileList::ascendingCase);
        }
        else
        {
            getApp()->reg().writeStringEntry("SEARCH PANEL", "sort_func", "ascendingCase");
        }

        restart_columns = true;
    }

    // Columns number has changed
    if (nbCols != nbCols_prev)
    {
        restart_columns = true;
    }
    // Columns order have changed
    else
    {
        FXbool changed = false;
        for (FXuint i = 0; i < nbCols; i++)
        {
            if (idCol[i] != idCol_prev[i])
            {
                changed = true;
                break;
            }
        }
        if (changed)
        {
            restart_columns = true;
        }
    }

#if defined(linux) && defined(XFE_AUTOMOUNTER)
    if (automountbutton->getCheck() != automount_prev || autoopenbutton->getCheck() != autoopen_prev)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "restart_automount", true);
        restart_automount = true;
    }
#endif

    // Restart application if necessary
    if (restart_smoothscroll | restart_scrollbarsize | restart_theme | restart_showpathlink |
        restart_normalfont | restart_textfont | restart_uidpi | restart_dirpanel_mode |
        restart_columns | restart_filetooltips | restart_relativeresize | restart_alwaysshowtabbar |
        restart_diropen | restart_fileopen | restart_automount)
    {
        if (BOX_CLICKED_CANCEL != MessageBox::question(this, BOX_OK_CANCEL, _("Restart"),
                                 _("Preferences will be changed after restart.\nRestart X File Explorer now?")))
        {
            // Set restarted flag
            getApp()->reg().writeUnsignedEntry("SETTINGS", "restarted", true);

            // Restart Xfe
            mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_RESTART), NULL);
        }
    }

    // Update the registry
    getApp()->reg().write();

    // Refresh panels
    mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_REFRESH), NULL);

    DialogBox::onCmdAccept(sender, sel, ptr);

    return 1;
}


// Cancel button pressed
long PreferencesBox::onCmdCancel(FXObject* sender, FXSelector sel, void* ptr)
{
    // Reset preferences to their previous values

    // First tab - General options
    trashcan->setCheck(trashcan_prev);
    trashbypass->setCheck(trashbypass_prev);
    autosave->setCheck(autosave_prev);
    savewinpos->setCheck(savewinpos_prev);
    diropen->setCheck(diropen_prev);
    fileopen->setCheck(fileopen_prev);
    filetooltips->setCheck(filetooltips_prev);
    relativeresize->setCheck(relativeresize_prev);
    alwaysshowtabbar->setCheck(alwaysshowtabbar_prev);
    restoretabs->setCheck(restoretabs_prev);
    showpathlink->setCheck(show_pathlink_prev);
    dirpanel_mode = dirpanel_mode_prev;
    dirpanel_list_startup = dirpanel_list_startup_prev;
    copysuffix_pos = copysuffix_pos_prev;

#ifdef STARTUP_NOTIFICATION
    usesn->setCheck(usesn_prev);
#endif
    noscript->setCheck(noscript_prev);
    timeformat->setText(oldtimeformat);
    copysuffix->setText(oldcopysuffix);

    // Second tab - Modes
    startdirmode = oldstartdirmode;
    newtabdirmode = oldnewtabdirmode;
    scroll->setCheck(smoothscroll_prev);
    getApp()->setWheelLines(wheellines_prev);
    getApp()->setScrollBarSize(scrollbarsize_prev);
    bigthumb_spin->setValue(bigthumb_size_prev);
    minithumb_spin->setValue(minithumb_size_prev);
#if defined(linux) && defined(XFE_AUTOMOUNTER)
    automountbutton->setCheck(automount_prev);
    autoopenbutton->setCheck(autoopen_prev);
#endif
    rootmode->setCheck(rootmode_prev);
    root_auth = root_auth_prev;
    getApp()->reg().writeUnsignedEntry("OPTIONS", "root_auth", root_auth);
    sudocmd->setText(sudocmd_prev);
    sucmd->setText(sucmd_prev);

    // Third tab - Dialogs
    ask->setCheck(ask_prev);
    dnd->setCheck(dnd_prev);
    trashmv->setCheck(trashmv_prev);
    del->setCheck(del_prev);
    properties->setCheck(properties_prev);
    del_emptydir->setCheck(del_emptydir_prev);
    overwrite->setCheck(overwrite_prev);
    exec->setCheck(exec_prev);
#if defined(linux)
    mountwarn->setCheck(mountwarn_prev);
    show_mount->setCheck(show_mount_prev);
#endif
    folder_warning->setCheck(folder_warning_prev);
    preserve_date_warning->setCheck(preserve_date_warning_prev);
    root_warning->setCheck(root_warning_prev);

    // Fourth tab - Columns
    sizecol->setCheck(sizecol_prev);
    typecol->setCheck(typecol_prev);
    extcol->setCheck(extcol_prev);
    datecol->setCheck(datecol_prev);
    usercol->setCheck(usercol_prev);
    groupcol->setCheck(groupcol_prev);
    permscol->setCheck(permscol_prev);
    linkcol->setCheck(linkcol_prev);
    nbCols = nbCols_prev;
    for (FXuint i = 0; i < nbCols_prev; i++)
    {
        idCol[i] = idCol_prev[i];
    }
    listcol->clearItems();
    for (FXuint i = 0; i < nbCols; i++)
    {
        listcol->appendItem(getHeaderName(idCol[i]));
    }
    listcol->disableItem(0);
    if (nbCols > 1)
    {
        listcol->selectItem(1);
    }

    // Fifth tab - Programs
    txtviewer->setText(oldtxtviewer);
    txteditor->setText(oldtxteditor);
    filecomparator->setText(oldfilecomparator);
    imgeditor->setText(oldimgeditor);
    imgviewer->setText(oldimgviewer);
    archiver->setText(oldarchiver);
    pdfviewer->setText(oldpdfviewer);
    audioplayer->setText(oldaudioplayer);
    videoplayer->setText(oldvideoplayer);
    xterm->setText(oldxterm);
    mountcmd->setText(oldmountcmd);
    umountcmd->setText(oldumountcmd);

    // Sixth tab - Visual
    themesList->setCurrentItem(themelist_prev);
    currTheme = currTheme_prev;
    iconpath->setText(oldiconpath);
    spindpi->setValue(uidpi_prev);
    uidpi = uidpi_prev;

    // Seventh tab - Fonts
    normalfont->setText(oldnormalfont);
    textfont->setText(oldtextfont);

    // Finally, update the registry (really necessary?)
    getApp()->reg().write();

    DialogBox::onCmdCancel(sender, sel, ptr);
    return 1;
}


// Execute dialog box modally
FXuint PreferencesBox::execute(FXuint placement)
{
    // Save current preferences to restore them if cancel is pressed

    // First tab - Options
    trashcan_prev = trashcan->getCheck();
    trashbypass_prev = trashbypass->getCheck();
    autosave_prev = autosave->getCheck();
    savewinpos_prev = savewinpos->getCheck();
    diropen_prev = diropen->getCheck();
    fileopen_prev = fileopen->getCheck();
    filetooltips_prev = filetooltips->getCheck();
    relativeresize_prev = relativeresize->getCheck();
    alwaysshowtabbar_prev = alwaysshowtabbar->getCheck();
    restoretabs_prev = restoretabs->getCheck();
    show_pathlink_prev = showpathlink->getCheck();
#ifdef STARTUP_NOTIFICATION
    usesn_prev = usesn->getCheck();
#endif
    noscript_prev = noscript->getCheck();

    copysuffix_pos_prev = copysuffix_pos;
    dirpanel_mode_prev = dirpanel_mode;
    dirpanel_list_startup_prev = dirpanel_list_startup;

    // Second tab - Modes
    wheellines_prev = getApp()->getWheelLines();
    scrollbarsize_prev = getApp()->getScrollBarSize();
    root_auth_prev = root_auth;
    smoothscroll_prev = scroll->getCheck();
#if defined(linux) && defined(XFE_AUTOMOUNTER)
    automount_prev = automountbutton->getCheck();
    autoopen_prev = autoopenbutton->getCheck();
#endif
    rootmode_prev = rootmode->getCheck();
    sudocmd_prev = sudocmd->getText();
    sucmd_prev = sucmd->getText();
    uidpi_prev = uidpi;
    bigthumb_size_prev = bigthumb_size;
    minithumb_size_prev = minithumb_size;

    // Third tab - Dialogs
    ask_prev = ask->getCheck();
    dnd_prev = dnd->getCheck();
    trashmv_prev = trashmv->getCheck();
    del_prev = del->getCheck();
    properties_prev = properties->getCheck();
    del_emptydir_prev = del_emptydir->getCheck();
    overwrite_prev = overwrite->getCheck();
    exec_prev = exec->getCheck();
#if defined(linux)
    mountwarn_prev = mountwarn->getCheck();
    show_mount_prev = show_mount->getCheck();
#endif
    folder_warning_prev = folder_warning->getCheck();
    preserve_date_warning_prev = preserve_date_warning->getCheck();
    root_warning_prev = root_warning->getCheck();

    // Fourth tab - Columns
    sizecol_prev = sizecol->getCheck();
    typecol_prev = typecol->getCheck();
    extcol_prev = extcol->getCheck();
    datecol_prev = datecol->getCheck();
    usercol_prev = usercol->getCheck();
    groupcol_prev = groupcol->getCheck();
    permscol_prev = permscol->getCheck();
    linkcol_prev = linkcol->getCheck();
    nbCols_prev = nbCols;
    for (FXuint i = 0; i < nbCols_prev; i++)
    {
        idCol_prev[i] = idCol[i];
    }

    // Fifth tab - Programs
    oldtxtviewer = txtviewer->getText();
    oldtxteditor = txteditor->getText();
    oldfilecomparator = filecomparator->getText();
    oldimgeditor = imgeditor->getText();
    oldimgviewer = imgviewer->getText();
    oldarchiver = archiver->getText();
    oldpdfviewer = pdfviewer->getText();
    oldaudioplayer = audioplayer->getText();
    oldvideoplayer = videoplayer->getText();
    oldxterm = xterm->getText();
    oldmountcmd = mountcmd->getText();
    oldumountcmd = umountcmd->getText();

    // Sixth tab - Visual
    themelist_prev = themesList->getCurrentItem();
    currTheme_prev = currTheme;
    oldiconpath = iconpath->getText();

    // Seventh tab - Fonts
    oldnormalfont = normalfont->getText();
    oldtextfont = textfont->getText();

    create();
    show(placement);
    getApp()->refresh();
    return getApp()->runModalFor(this);
}


// Update buttons related to the trash can option item
long PreferencesBox::onUpdTrash(FXObject* sender, FXSelector, void*)
{
    if (trashcan->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Update the confirm delete empty directories option item
long PreferencesBox::onUpdConfirmDelEmptyDir(FXObject* sender, FXSelector, void*)
{
    if (del->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Set root mode
long PreferencesBox::onCmdSuMode(FXObject*, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_PKEXEC_CMD)
    {
        root_auth = 0;
    }
    else if (FXSELID(sel) == ID_SUDO_CMD)
    {
        root_auth = 1;
    }
    else if (FXSELID(sel) == ID_SU_CMD)
    {
        root_auth = 2;
    }
    else
    {
        // Should not happen
    }

    getApp()->reg().writeUnsignedEntry("OPTIONS", "root_auth", root_auth);
    getApp()->reg().write();

    return 1;
}


// Update root mode radio button
long PreferencesBox::onUpdSuMode(FXObject* sender, FXSelector sel, void*)
{
    if (!rootmode->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL);
        sudolabel->disable();
        sudocmd->hide();
        sulabel->disable();
        sucmd->hide();
    }
    else
    {
        // Non root user
        if (getuid())
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL);

            if (root_auth == 0)
            {
                sudolabel->disable();
                sudocmd->hide();
                sulabel->disable();
                sucmd->hide();
            }
            else if (root_auth == 1)
            {
                sudolabel->enable();
                sudocmd->show();
                sulabel->disable();
                sucmd->hide();
            }
            else if (root_auth == 2)
            {
                sudolabel->disable();
                sudocmd->hide();
                sulabel->enable();
                sucmd->show();
            }
            else
            {
                // Should not happen
            }
        }

        FXSelector updatemessage = FXSEL(SEL_COMMAND, ID_UNCHECK);

        if (FXSELID(sel) == ID_PKEXEC_CMD)
        {
            if (root_auth == 0)
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_CHECK);
            }
            else
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_UNCHECK);
            }
        }
        else if (FXSELID(sel) == ID_SUDO_CMD)
        {
            if (root_auth == 1)
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_CHECK);
            }
            else
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_UNCHECK);
            }
        }
        else if (FXSELID(sel) == ID_SU_CMD)
        {
            if (root_auth == 2)
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_CHECK);
            }
            else
            {
                updatemessage = FXSEL(SEL_COMMAND, ID_UNCHECK);
            }
        }
        else
        {
            // Should not happen
        }

        sender->handle(this, updatemessage, NULL);
    }
    return 1;
}


#if defined(linux) && defined(XFE_AUTOMOUNTER)

// Update auto open window when mounting
long PreferencesBox::onUpdAutoOpen(FXObject* sender, FXSelector, void*)
{
    if (automountbutton->getCheck())
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


// Set scroll wheel lines (Mathew Robertson <mathew@optushome.com.au>)
long PreferencesBox::onCmdWheelAdjust(FXObject* sender, FXSelector, void*)
{
    FXuint value;

    sender->handle(this, FXSEL(SEL_COMMAND, ID_GETINTVALUE), (void*)&value);
    getApp()->setWheelLines(value);
    getApp()->reg().write();
    return 1;
}


// Update the wheel lines button
long PreferencesBox::onUpdWheelAdjust(FXObject* sender, FXSelector, void*)
{
    FXuint value = getApp()->getWheelLines();

    sender->handle(this, FXSEL(SEL_COMMAND, ID_SETINTVALUE), (void*)&value);
    return 1;
}


// Set scrollbar size
long PreferencesBox::onCmdScrollBarSize(FXObject* sender, FXSelector, void*)
{
    FXuint value;

    sender->handle(this, FXSEL(SEL_COMMAND, ID_GETINTVALUE), (void*)&value);
    getApp()->setScrollBarSize(value);
    getApp()->reg().write();
    return 1;
}


// Update the scrollbar size button
long PreferencesBox::onUpdScrollBarSize(FXObject* sender, FXSelector, void*)
{
    FXuint value = getApp()->getScrollBarSize();

    sender->handle(this, FXSEL(SEL_COMMAND, ID_SETINTVALUE), (void*)&value);
    return 1;
}


// Set mount timeout
long PreferencesBox::onCmdMountTimeout(FXObject* sender, FXSelector, void*)
{
    FXuint value;

    sender->handle(this, FXSEL(SEL_COMMAND, ID_GETINTVALUE), (void*)&value);

    getApp()->reg().writeUnsignedEntry("OPTIONS", "mount_timeout", value);

    getApp()->reg().write();

    return 1;
}


// Update mount timeout button
long PreferencesBox::onUpdMountTimeout(FXObject* sender, FXSelector, void*)
{
    if (mountwarn->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Update single click file open button
long PreferencesBox::onUpdSingleClickFileopen(FXObject* sender, FXSelector, void*)
{
    if (diropen->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        fileopen->setCheck(false);
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Update exec text files button
long PreferencesBox::onUpdExecTextFiles(FXObject* sender, FXSelector, void*)
{
    if (!noscript->getCheck())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Start directory mode
long PreferencesBox::onCmdStartDir(FXObject*, FXSelector sel, void*)
{
    startdirmode = FXSELID(sel);
    return 1;
}


// Update start directory mode radio buttons
long PreferencesBox::onUpdStartDir(FXObject* sender, FXSelector sel, void*)
{
    sender->handle(this, (FXSELID(sel) == startdirmode) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                   (void*)&startdirmode);
    return 1;
}


// New tab directory mode
long PreferencesBox::onCmdNewTabDir(FXObject*, FXSelector sel, void*)
{
    newtabdirmode = FXSELID(sel);
    return 1;
}


// Update new tab directory mode radio buttons
long PreferencesBox::onUpdNewTabDir(FXObject* sender, FXSelector sel, void*)
{
    sender->handle(this, (FXSELID(sel) == newtabdirmode) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                   (void*)&newtabdirmode);
    return 1;
}


// Set copy suffix position
long PreferencesBox::onCmdCopySuffixPos(FXObject*, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_COPY_SUFFIX_BEFORE)
    {
        copysuffix_pos = 0;
    }
    else if (FXSELID(sel) == ID_COPY_SUFFIX_AFTER)
    {
        copysuffix_pos = 1;
    }
    else
    {
        // Should not happen
    }

    return 1;
}


// Update copy suffix position radio button
long PreferencesBox::onUpdCopySuffixPos(FXObject* sender, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_COPY_SUFFIX_BEFORE)
    {
        sender->handle(this, (copysuffix_pos == 0) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    if (FXSELID(sel) == ID_COPY_SUFFIX_AFTER)
    {
        sender->handle(this, (copysuffix_pos == 1) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    return 1;
}



// Set directory panel mode
long PreferencesBox::onCmdDirPanelMode(FXObject*, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_DIRPANEL_PLACES_TREE)
    {
        dirpanel_mode = 0;
    }
    else if (FXSELID(sel) == ID_DIRPANEL_PLACES)
    {
        dirpanel_mode = 1;
    }
    else // ID_DIRPANEL_TREE
    {
        dirpanel_mode = 2;
    }

    return 1;
}


// Update radio buttons
long PreferencesBox::onUpdDirPanelMode(FXObject* sender, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_DIRPANEL_PLACES_TREE)
    {
        sender->handle(this, (dirpanel_mode == 0) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    if (FXSELID(sel) == ID_DIRPANEL_PLACES)
    {
        sender->handle(this, (dirpanel_mode == 1) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    if (FXSELID(sel) == ID_DIRPANEL_TREE)
    {
        sender->handle(this, (dirpanel_mode == 2) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    return 1;
}


// Set directory panel list to open at startup (places or tree)
long PreferencesBox::onCmdDirPanelStartup(FXObject*, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_DIRPANEL_PLACES_STARTUP)
    {
        dirpanel_list_startup = 0;
    }
    else if (FXSELID(sel) == ID_DIRPANEL_TREE_STARTUP)
    {
        dirpanel_list_startup = 1;
    }
    else
    {
        // Should not happen
    }

    return 1;
}


// Update radio buttons
long PreferencesBox::onUpdDirPanelStartup(FXObject* sender, FXSelector sel, void*)
{
    if (FXSELID(sel) == ID_DIRPANEL_PLACES_STARTUP)
    {
        sender->handle(this,
                       (dirpanel_list_startup == 0) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    if (FXSELID(sel) == ID_DIRPANEL_TREE_STARTUP)
    {
        sender->handle(this,
                       (dirpanel_list_startup == 1) ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK),
                       NULL);
    }

    if (dirpanel_places_tree->getCheck())
    {
        dirpanel_startup->enable();
        dirpanel_placesstartup->enable();
        dirpanel_treestartup->enable();
    }
    else
    {
        dirpanel_startup->disable();
        dirpanel_placesstartup->disable();
        dirpanel_treestartup->disable();
    }

    return 1;
}


// Move column up
long PreferencesBox::onCmdColUp(FXObject*, FXSelector sel, void*)
{
    // Index of selected item
    FXuint index = 0;

    for (int i = 0; i < (int)nbCols; i++)
    {
        if (listcol->isItemSelected(i))
        {
            index = i;
            break;
        }
    }

    // Move item up
    if (index > 1 && index < nbCols)
    {
        FXuint tmp = idCol[index - 1];
        idCol[index - 1] = idCol[index];
        idCol[index] = tmp;

        // Update columns list
        listcol->clearItems();

        for (FXuint i = 0; i < nbCols; i++)
        {
            listcol->appendItem(getHeaderName(idCol[i]));
        }

        // Select moved item
        listcol->selectItem(index - 1);

        // Disable first item
        listcol->disableItem(0);
    }


    return 1;
}


// Move column down
long PreferencesBox::onCmdColDown(FXObject*, FXSelector sel, void*)
{
    // Index of selected item
    FXuint index = 0;

    for (int i = 0; i < (int)nbCols; i++)
    {
        if (listcol->isItemSelected(i))
        {
            index = i;
            break;
        }
    }

    // Move item down
    if (index > 0 && index < nbCols - 1)
    {
        FXuint tmp = idCol[index + 1];
        idCol[index + 1] = idCol[index];
        idCol[index] = tmp;

        // Update columns list
        listcol->clearItems();

        for (FXuint i = 0; i < nbCols; i++)
        {
            listcol->appendItem(getHeaderName(idCol[i]));
        }

        // Select moved item
        listcol->selectItem(index + 1);

        // Disable first item
        listcol->disableItem(0);
    }

    return 1;
}


// Update columns list
long PreferencesBox::onCmdViewCol(FXObject* sender, FXSelector sel, void*)
{
    idCol[0] = FileList::ID_COL_NAME;

    FXuint i = 1;
    if (sizecol->getCheck())
    {
        idCol[i] = FileList::ID_COL_SIZE;
        i++;
    }
    if (typecol->getCheck())
    {
        idCol[i] = FileList::ID_COL_TYPE;
        i++;
    }
    if (extcol->getCheck())
    {
        idCol[i] = FileList::ID_COL_EXT;
        i++;
    }
    if (datecol->getCheck())
    {
        idCol[i] = FileList::ID_COL_DATE;
        i++;
    }
    if (usercol->getCheck())
    {
        idCol[i] = FileList::ID_COL_USER;
        i++;
    }
    if (groupcol->getCheck())
    {
        idCol[i] = FileList::ID_COL_GROUP;
        i++;
    }
    if (permscol->getCheck())
    {
        idCol[i] = FileList::ID_COL_PERMS;
        i++;
    }
    if (linkcol->getCheck())
    {
        idCol[i] = FileList::ID_COL_LINK;
        i++;
    }

    // Update number of columns
    nbCols = i;

    // Update columns list
    listcol->clearItems();
    for (FXuint i = 0; i < nbCols; i++)
    {
        listcol->appendItem(getHeaderName(idCol[i]));
    }

    // Disable first item and select second item
    listcol->disableItem(0);
    if (nbCols > 1)
    {
        listcol->selectItem(1);
    }

    return 1;
}


// Update columns up / down buttons
long PreferencesBox::onUpdListColUpDown(FXObject* sender, FXSelector sel, void*)
{
    if (nbCols > 1)
    {
        if (listcol->isItemSelected(1))
        {
            listcol_up->disable();
        }
        else
        {
            listcol_up->enable();
        }

        if (listcol->isItemSelected(nbCols - 1))
        {
            listcol_down->disable();
        }
        else
        {
            listcol_down->enable();
        }
    }
    else
    {
        listcol_up->disable();
        listcol_down->disable();
    }

    return 1;
}


// Display themes list tooltip
long PreferencesBox::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    themesList->getCursorPosition(x, y, state);
    int index = themesList->getItemAt(x, y);

    if (index != -1)
    {
        FXString tip = Themes[index].tip;
        sender->handle(themesList, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}
