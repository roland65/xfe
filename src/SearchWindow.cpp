// Search files dialog and panel
#include "config.h"
#include "i18n.h"

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <fx.h>
#include <fxkeys.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "icons.h"
#include "xfeutils.h"
#include "FileDialog.h"
#include "MessageBox.h"
#include "TextWindow.h"
#include "SearchPanel.h"
#include "SearchWindow.h"


// List refresh counter limits
#define REFRESH_COUNT_LIMIT    10000
#define REFRESH_COUNT_SLOW     100
#define REFRESH_COUNT_FAST     1000


// Rotating cursor refresh delay
#define ROTATING_CURSOR_REFRESH_DELAY    500

// Search history
extern int FindNum;
extern char FindHistory[FIND_HIST_SIZE][MAX_FIND_SIZE];

extern FXMainWindow* mainWindow;


// Map
FXDEFMAP(SearchWindow) SearchWindowMap[] =
{
    FXMAPFUNC(SEL_KEYPRESS, 0, SearchWindow::onKeyPress),
    FXMAPFUNC(SEL_IO_READ, SearchWindow::ID_READ_DATA, SearchWindow::onReadData),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_START, SearchWindow::onCmdStart),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_STOP, SearchWindow::onCmdStop),
    FXMAPFUNC(SEL_CLOSE, 0, SearchWindow::onCmdClose),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_CLOSE, SearchWindow::onCmdClose),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_RESET_OPTIONS, SearchWindow::onCmdResetOptions),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_BROWSE_PATH, SearchWindow::onCmdBrowsePath),
    FXMAPFUNC(SEL_VERIFY, SearchWindow::ID_PERMS, SearchWindow::onPermsVerify),
    FXMAPFUNC(SEL_COMMAND, SearchWindow::ID_MORE_OPTIONS, SearchWindow::onCmdMoreOptions),
    FXMAPFUNC(SEL_UPDATE, SearchWindow::ID_STOP, SearchWindow::onUpdStop),
    FXMAPFUNC(SEL_UPDATE, SearchWindow::ID_START, SearchWindow::onUpdStart),
    FXMAPFUNC(SEL_UPDATE, SearchWindow::ID_PERMS, SearchWindow::onUpdPerms),
    FXMAPFUNC(SEL_UPDATE, SearchWindow::ID_SIZE, SearchWindow::onUpdSize),
    FXMAPFUNC(SEL_TIMEOUT, SearchWindow::ID_ROTATING_CURSOR, SearchWindow::onRotatingCursorRefresh),
};


// Object implementation
FXIMPLEMENT(SearchWindow, FXTopWindow, SearchWindowMap, ARRAYNUMBER(SearchWindowMap))



// Contruct free floating dialog
SearchWindow::SearchWindow(FXApp* app, const FXString& name, int x, int y, int w, int h, int pl, int pr, int pt, int pb,
                           int hs, int vs) :
    FXTopWindow(app, name, NULL, NULL,
                DECOR_TITLE | DECOR_MINIMIZE | DECOR_MAXIMIZE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE, x, y, w,
                h, pl, pr, pt, pb, hs, vs)
{
    setIcon(minisearchicon);

    // Vertical frame
    FXVerticalFrame* frame1 = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0,
                                                  0, 0, 0, 0, 0, 0, 0);

    // Vertical frame
    searchframe = new FXVerticalFrame(frame1, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X);

    // Label and input field with history
    FXMatrix* matrix1 = new FXMatrix(searchframe, 4,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix1, _("Find files:"), NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    findfile = new HistComboBox(matrix1, 40, false, true, NULL, 0,
                                COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X | LAYOUT_FILL_COLUMN |
                                LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    findfile->setNumVisible(5);
    for (int i = 0; i < FindNum; i++)
    {
        findfile->appendItem(FindHistory[i]);
    }

    findigncase = new FXCheckButton(matrix1, _("Ignore case\tIgnore file name case") + FXString(" "), NULL, 0,
                                    JUSTIFY_NORMAL | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y);
    FXuint ignorecase = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "find_ignorecase", 0);
    findigncase->setCheck(ignorecase);

    // Hidden files
    findhidden = new FXCheckButton(matrix1, _("Hidden files\tShow hidden files and folders") + FXString(" "), NULL, 0,
                                   JUSTIFY_NORMAL | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y);
    FXuint hidden = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "find_hidden", 0);
    findhidden->setCheck(hidden);

    FXMatrix* matrix2 = new FXMatrix(searchframe, 3,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix2, _("In folder:"), NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    wheredir = new FXTextField(matrix2, 40, 0, 0,
                               LAYOUT_CENTER_Y | LAYOUT_CENTER_X | TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN |
                               LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    dirbutton = new FXButton(matrix2, _("\tIn folder..."), minifiledialogicon, this, ID_BROWSE_PATH,
                             FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    FXMatrix* matrix3 = new FXMatrix(searchframe, 3,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix3, _("Text contains:"), NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    greptext = new FXTextField(matrix3, 40, 0, 0,
                               LAYOUT_CENTER_Y | LAYOUT_CENTER_X | TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN |
                               LAYOUT_FILL_ROW | LAYOUT_FILL_X);

    grepigncase = new FXCheckButton(matrix3, _("Ignore case\tIgnore text case") + FXString(" "), NULL, 0,
                                    JUSTIFY_NORMAL | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y);
    ignorecase = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "grep_ignorecase", 0);
    grepigncase->setCheck(ignorecase);

    // Search options
    moreoptions = new FXCheckButton(searchframe, _("More options") + FXString(" "), this, ID_MORE_OPTIONS,
                                    CHECKBUTTON_PLUS | JUSTIFY_NORMAL | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y);
    moregroup = new FXGroupBox(searchframe, _("Search options"), GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_FILL_X);
    resetoptions = new FXButton(moregroup, _("Reset\tReset search options"), NULL, this, SearchWindow::ID_RESET_OPTIONS,
                                FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);

    FXbool moreopts = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "moreoptions", 0);
    if (moreopts)
    {
        moreoptions->setCheck(true);
        moregroup->show();
    }
    else
    {
        moreoptions->setCheck(false);
        moregroup->hide();
    }

    // File size
    FXMatrix* matrix4 = new FXMatrix(moregroup, 6, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | PACK_UNIFORM_WIDTH, 0, 0, 0, 0,
                                     4, 4, 4, 8, 4, 8);
    new FXLabel(matrix4, _("Min size:"), NULL, JUSTIFY_LEFT);
    minsize = new FXSpinner(matrix4, 10, this, SearchWindow::ID_SIZE, SPIN_NOMAX | JUSTIFY_LEFT);
    minsize->setTipText(_("Filter by minimum file size (kBytes)"));
    new FXLabel(matrix4, _("kB"), NULL, JUSTIFY_LEFT);

    new FXLabel(matrix4, _("Max size:"), NULL, JUSTIFY_LEFT);
    maxsize = new FXSpinner(matrix4, 10, this, SearchWindow::ID_SIZE, SPIN_NOMAX | JUSTIFY_LEFT);
    maxsize->setTipText(_("Filter by maximum file size (kBytes)"));
    new FXLabel(matrix4, _("kB"), NULL, JUSTIFY_LEFT);

    // Modification date
    new FXLabel(matrix4, _("Last modified before:"), NULL, JUSTIFY_LEFT);
    mindays = new FXSpinner(matrix4, 10, NULL, 0, SPIN_NOMAX | JUSTIFY_LEFT);
    mindays->setTipText(_("Filter by maximum modification date (days)"));
    new FXLabel(matrix4, _("Days"), NULL, JUSTIFY_LEFT);

    new FXLabel(matrix4, _("Last modified after:"), NULL, JUSTIFY_LEFT);
    maxdays = new FXSpinner(matrix4, 10, NULL, 0, SPIN_NOMAX | JUSTIFY_LEFT);
    maxdays->setTipText(_("Filter by minimum modification date (days)"));
    new FXLabel(matrix4, _("Days"), NULL, JUSTIFY_LEFT);

    // User and group
    new FXLabel(matrix4, _("User:"), NULL, JUSTIFY_LEFT);
    user = new ComboBox(matrix4, 15,false, false, NULL, 0, COMBOBOX_STATIC);
    user->setNumVisible(5);
    userbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tFilter by user name"), NULL, 0,
                                JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    new FXLabel(matrix4, _("Group:"), NULL, JUSTIFY_LEFT);
    grp = new ComboBox(matrix4, 15, false, false, NULL, 0, COMBOBOX_STATIC);
    grp->setNumVisible(5);
    grpbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tFilter by group name"), NULL, 0,
                               JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // User names (sorted in ascending order)
    struct passwd* pwde;
    while ((pwde = getpwent()))
    {
        user->appendItem(pwde->pw_name);
    }
    endpwent();
    user->setSortFunc(FXList::ascending);
    user->sortItems();

    // Group names (sorted in ascending order)
    struct group* grpe;
    while ((grpe = getgrent()))
    {
        grp->appendItem(grpe->gr_name);
    }
    endgrent();
    grp->setSortFunc(FXList::ascending);
    grp->sortItems();

    // Set user name and group name
    struct stat linfo;
    if (xf_lstat(FXSystem::getHomeDirectory().text(), &linfo) == 0)
    {
        uid = FXSystem::userName(linfo.st_uid);
        gid = FXSystem::groupName(linfo.st_gid);
        user->setText(uid);
        grp->setText(gid);
    }

    // File type
    new FXLabel(matrix4, _("File type:"), NULL, JUSTIFY_LEFT);
    type = new ComboBox(matrix4, 15, false, false, NULL, 0, COMBOBOX_STATIC);
    type->setNumVisible(5);
    type->appendItem(_("File"));
    type->appendItem(_("Folder"));
    type->appendItem(_("Link"));
    type->appendItem(_("Socket"));
    type->appendItem(_("Pipe"));
    type->setCurrentItem(0);
    typebtn = new FXCheckButton(matrix4, FXString(" ") + _("\tFilter by file type"), NULL, 0,
                                JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Permissions (in octal)
    new FXLabel(matrix4, _("Permissions:"), NULL, JUSTIFY_LEFT);
    perms = new FXTextField(matrix4, 4, this, SearchWindow::ID_PERMS,
                            TEXTFIELD_NORMAL | TEXTFIELD_INTEGER | TEXTFIELD_LIMITED | TEXTFIELD_OVERSTRIKE);
    perms->setText("0644");
    perms->setNumColumns(4);
    permsbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tFilter by permissions (octal)"), NULL, 0,
                                 JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Empty files
    new FXLabel(matrix4, _("Empty files:"), NULL, JUSTIFY_LEFT);
    emptybtn = new FXCheckButton(matrix4, FXString(" ") + _("\tEmpty files only"), NULL, 0,
                                 JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Don't follow symlinks
    new FXLabel(matrix4, "", NULL, JUSTIFY_LEFT);
    new FXLabel(matrix4, _("Don't follow symbolic links:"), NULL, JUSTIFY_LEFT);
    linkbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tSearch without following symbolic links"), NULL, 0,
                                JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Non recursive
    new FXLabel(matrix4, "", NULL, JUSTIFY_LEFT);
    new FXLabel(matrix4, _("Non recursive:"), NULL, JUSTIFY_LEFT);
    norecbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tDon't search folders recursively"), NULL, 0,
                                 JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Don't search in other file systems
    new FXLabel(matrix4, "", NULL, JUSTIFY_LEFT);
    new FXLabel(matrix4, _("Other file systems:"), NULL, JUSTIFY_LEFT);
    nofsbtn = new FXCheckButton(matrix4, FXString(" ") + _("\tSearch also in other file systems"), NULL, 0,
                                JUSTIFY_LEFT | LAYOUT_CENTER_Y);

    // Search results
    FXHorizontalFrame* frame2 = new FXHorizontalFrame(frame1, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X, 0, 0, 0, 0,
                                                      0, 0, 0, 0);
    searchresults = new FXLabel(frame2, "", NULL, LAYOUT_CENTER_Y | LAYOUT_FILL_X);

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(frame2, PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 10, 10, 5, 5);

    // Start
    startbutton = new FXButton(buttons, _("&Start\tStart Search (F3)"), NULL, this, ID_START,
                               FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Stop
    stopbutton = new FXButton(buttons, _("&Stop\tStop Search (Esc)"), NULL, this, ID_STOP,
                              FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Quit
    new FXButton(buttons, _("&Quit"), NULL, this, ID_CLOSE, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Read file list columns order and shown status
    FXbool colShown[FileList::ID_COL_NAME + NMAX_COLS] = { 0 };
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

    FXuint idCol[NMAX_COLS] = { 0 };
    FXuint nbCols = 0;
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

    // Column ids for search list
    FXuint idColSearch[NMAX_COLS + 1] = { 0 };
    FXuint nbColsSearch = nbCols + 1;

    idColSearch[0] = idCol[0];
    idColSearch[1] = FileList::ID_COL_DIRNAME;

    for (FXuint i = 1; i < nbCols; i++)
    {
        idColSearch[i + 1] = idCol[i];
    }

    // Search Panel
    FXColor listbackcolor = getApp()->reg().readColorEntry("SETTINGS", "listbackcolor", FXRGB(255, 255, 255));
    FXColor listforecolor = getApp()->reg().readColorEntry("SETTINGS", "listforecolor", FXRGB(0, 0, 0));

    searchpanel = new SearchPanel(frame1, idColSearch, nbColsSearch,
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "name_size", MIN_NAME_SIZE),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "dir_size", 150),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "size_size", 60),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "type_size", 100),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "ext_size", 100),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "date_size", 150),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "user_size", 50),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "group_size", 50),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "perms_size", 100),
                                  getApp()->reg().readUnsignedEntry("SEARCH PANEL", "link_size", 100),
                                  listbackcolor, listforecolor,
                                  LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Display thumbnails or not
    FXbool showthumbnails;
    showthumbnails = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "showthumbnails", 0);
    searchpanel->showThumbnails(showthumbnails);

    // Read and set sort function for file list
    FXString sort_func = getApp()->reg().readStringEntry("SEARCH PANEL", "sort_func", "ascendingCase");
    if (sort_func == "ascendingCase")
    {
        searchpanel->setSortFunc(FileList::ascendingCase);
    }
    else if (sort_func == "ascendingCaseMix")
    {
        searchpanel->setSortFunc(FileList::ascendingCaseMix);
    }
    else if (sort_func == "descendingCase")
    {
        searchpanel->setSortFunc(FileList::descendingCase);
    }
    else if (sort_func == "descendingCaseMix")
    {
        searchpanel->setSortFunc(FileList::descendingCaseMix);
    }
    else if (sort_func == "ascending")
    {
        searchpanel->setSortFunc(FileList::ascending);
    }
    else if (sort_func == "ascendingMix")
    {
        searchpanel->setSortFunc(FileList::ascendingMix);
    }
    else if (sort_func == "descending")
    {
        searchpanel->setSortFunc(FileList::descending);
    }
    else if (sort_func == "descendingMix")
    {
        searchpanel->setSortFunc(FileList::descendingMix);
    }
    else if (sort_func == "ascendingDirCase")
    {
        searchpanel->setSortFunc(FileList::ascendingDirCase);
    }
    else if (sort_func == "ascendingDirCaseMix")
    {
        searchpanel->setSortFunc(FileList::ascendingDirCaseMix);
    }
    else if (sort_func == "descendingDirCase")
    {
        searchpanel->setSortFunc(FileList::descendingDirCase);
    }
    else if (sort_func == "descendingDirCaseMix")
    {
        searchpanel->setSortFunc(FileList::descendingDirCaseMix);
    }
    else if (sort_func == "ascendingDir")
    {
        searchpanel->setSortFunc(FileList::ascendingDir);
    }
    else if (sort_func == "ascendingDirMix")
    {
        searchpanel->setSortFunc(FileList::ascendingDirMix);
    }
    else if (sort_func == "descendingDir")
    {
        searchpanel->setSortFunc(FileList::descendingDir);
    }
    else if (sort_func == "descendingDirMix")
    {
        searchpanel->setSortFunc(FileList::descendingDirMix);
    }
    else if (sort_func == "ascendingSize")
    {
        searchpanel->setSortFunc(FileList::ascendingSize);
    }
    else if (sort_func == "ascendingSizeMix")
    {
        searchpanel->setSortFunc(FileList::ascendingSizeMix);
    }
    else if (sort_func == "descendingSize")
    {
        searchpanel->setSortFunc(FileList::descendingSize);
    }
    else if (sort_func == "descendingSizeMix")
    {
        searchpanel->setSortFunc(FileList::descendingSizeMix);
    }
    else if (sort_func == "ascendingType")
    {
        searchpanel->setSortFunc(FileList::ascendingType);
    }
    else if (sort_func == "ascendingTypeMix")
    {
        searchpanel->setSortFunc(FileList::ascendingTypeMix);
    }
    else if (sort_func == "descendingType")
    {
        searchpanel->setSortFunc(FileList::descendingType);
    }
    else if (sort_func == "descendingTypeMix")
    {
        searchpanel->setSortFunc(FileList::descendingTypeMix);
    }
    else if (sort_func == "ascendingExt")
    {
        searchpanel->setSortFunc(FileList::ascendingExt);
    }
    else if (sort_func == "ascendingExtMix")
    {
        searchpanel->setSortFunc(FileList::ascendingExtMix);
    }
    else if (sort_func == "descendingExt")
    {
        searchpanel->setSortFunc(FileList::descendingExt);
    }
    else if (sort_func == "descendingExtMix")
    {
        searchpanel->setSortFunc(FileList::descendingExtMix);
    }
    else if (sort_func == "ascendingDate")
    {
        searchpanel->setSortFunc(FileList::ascendingDate);
    }
    else if (sort_func == "ascendingDateMix")
    {
        searchpanel->setSortFunc(FileList::ascendingDateMix);
    }
    else if (sort_func == "descendingDate")
    {
        searchpanel->setSortFunc(FileList::descendingDate);
    }
    else if (sort_func == "descendingDateMix")
    {
        searchpanel->setSortFunc(FileList::descendingDateMix);
    }
    else if (sort_func == "ascendingUser")
    {
        searchpanel->setSortFunc(FileList::ascendingUser);
    }
    else if (sort_func == "ascendingUserMix")
    {
        searchpanel->setSortFunc(FileList::ascendingUserMix);
    }
    else if (sort_func == "descendingUser")
    {
        searchpanel->setSortFunc(FileList::descendingUser);
    }
    else if (sort_func == "descendingUserMix")
    {
        searchpanel->setSortFunc(FileList::descendingUserMix);
    }
    else if (sort_func == "ascendingGroup")
    {
        searchpanel->setSortFunc(FileList::ascendingGroup);
    }
    else if (sort_func == "ascendingGroupMix")
    {
        searchpanel->setSortFunc(FileList::ascendingGroupMix);
    }
    else if (sort_func == "descendingGroup")
    {
        searchpanel->setSortFunc(FileList::descendingGroup);
    }
    else if (sort_func == "descendingGroupMix")
    {
        searchpanel->setSortFunc(FileList::descendingGroupMix);
    }
    else if (sort_func == "ascendingPerms")
    {
        searchpanel->setSortFunc(FileList::ascendingPerms);
    }
    else if (sort_func == "ascendingPermsMix")
    {
        searchpanel->setSortFunc(FileList::ascendingPermsMix);
    }
    else if (sort_func == "descendingPerms")
    {
        searchpanel->setSortFunc(FileList::descendingPerms);
    }
    else if (sort_func == "descendingPermsMix")
    {
        searchpanel->setSortFunc(FileList::descendingPermsMix);
    }
    else if (sort_func == "ascendingLink")
    {
        searchpanel->setSortFunc(FileList::ascendingLink);
    }
    else if (sort_func == "ascendingLinkMix")
    {
        searchpanel->setSortFunc(FileList::ascendingLinkMix);
    }
    else if (sort_func == "descendingLink")
    {
        searchpanel->setSortFunc(FileList::descendingLink);
    }
    else if (sort_func == "descendingLinkMix")
    {
        searchpanel->setSortFunc(FileList::descendingLinkMix);
    }

    // Add some accelerators
    FXHotKey hotkey;
    FXString key;

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "select_all", "Ctrl-A");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_SELECT_ALL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "invert_selection", "Ctrl-I");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_SELECT_INVERSE));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "deselect_all", "Ctrl-Z");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_DESELECT_ALL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rename", "F2");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_FILE_RENAME));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy_to", "F5");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_FILE_COPYTO));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to", "F6");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_FILE_MOVETO));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "symlink_to", "Ctrl-S");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_FILE_SYMLINK));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_OPEN));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view", "Shift-F4");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_VIEW));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "edit", "F4");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_EDIT));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "compare", "F8");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, searchpanel, FXSEL(SEL_COMMAND, SearchPanel::ID_COMPARE));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, SearchWindow::ID_CLOSE));

    // Warning window
    warnwindow = new TextWindow(this, _("Warnings"), 30, 80);

    // Set text font for the warning window
    FXString fontspec;
    fontspec = getApp()->reg().readStringEntry("SETTINGS", "textfont", DEFAULT_TEXT_FONT);
    if (!fontspec.empty())
    {
        FXFont* font = new FXFont(getApp(), fontspec);
        font->create();
        warnwindow->setFont(font);
    }

    // Initialize variables
    application = app;
}


// Create window
void SearchWindow::create()
{
    // Create windows
    FXTopWindow::create();
    warnwindow->create();
}


// Clean up
SearchWindow::~SearchWindow()
{
    delete searchpanel;
    delete warnwindow;
}


// Check input for permissions in octal
long SearchWindow::onPermsVerify(FXObject* sender, FXSelector sel, void* ptr)
{
    char* str = (char*)ptr;

    for (int i = 0; i < (int)strlen(str); i++)
    {
        if (str[i] - '0' > 7)
        {
            return 1;
        }
    }

    return 0;
}


// Display or hide the more options dialog
long SearchWindow::onCmdMoreOptions(FXObject* sender, FXSelector sel, void*)
{
    if (moreoptions->getCheck())
    {
        moregroup->show();

        // Reset search options
        minsize->setValue(0);
        maxsize->setValue(0);
        mindays->setValue(0);
        maxdays->setValue(0);
        userbtn->setCheck(false);
        grpbtn->setCheck(false);
        typebtn->setCheck(false);
        permsbtn->setCheck(false);
        emptybtn->setCheck(false);
        linkbtn->setCheck(false);
        perms->setText("0644");
        type->setCurrentItem(0);
        user->setText(uid);
        grp->setText(gid);
        norecbtn->setCheck(false);
        nofsbtn->setCheck(false);
    }
    else
    {
        moregroup->hide();
    }

    // Refresh layout
    searchframe->recalc();

    return 1;
}


// Close window
long SearchWindow::onCmdClose(FXObject*, FXSelector, void*)
{
    xf_setwaitcursor(getApp(), END_CURSOR);

    // Clear panel items
    searchpanel->clearItems();

    // Reset search options
    minsize->setValue(0);
    maxsize->setValue(0);
    mindays->setValue(0);
    maxdays->setValue(0);
    userbtn->setCheck(false);
    grpbtn->setCheck(false);
    typebtn->setCheck(false);
    permsbtn->setCheck(false);
    emptybtn->setCheck(false);
    linkbtn->setCheck(false);
    perms->setText("0644");
    type->setCurrentItem(0);
    user->setText(uid);
    grp->setText(gid);
    norecbtn->setCheck(false);
    nofsbtn->setCheck(false);

    running = false;
    searchresults->setText("");

    if (pid != -1)
    {
        kill((-1 * pid), SIGTERM); // Kills the process group
    }
    hide();

    // Sort history
    findfile->setSortFunc(FXList::ascendingCase);
    findfile->sortItems();

    // Update history
    FindNum = 0;
    if (findfile->getNumItems() > 0)
    {
        for (int i = 0; i < findfile->getNumItems(); i++)
        {
            if (FindNum > FIND_HIST_SIZE - 1)
            {
                break;
            }
            FXString str = findfile->getItemText(i);
            xf_strlcpy(FindHistory[i], str.text(), str.length() + 1);
            FindNum++;
        }
    }

    // Set focus to main window
    mainWindow->setFocus();

    return 1;
}


// Start the file search
long SearchWindow::onCmdStart(FXObject*, FXSelector, void*)
{
    // Set the search pattern from user input
    // Add '*' before and after the string if not present
    FXString searchpattern;

    if (findfile->getText() == "")
    {
        searchpattern = "*";
    }
    else if (findfile->getText().contains('*'))
    {
        searchpattern = findfile->getText();
    }
    else
    {
        searchpattern = "*" + findfile->getText() + "*";
    }

    // Set search path in search panel
    searchpanel->setSearchPath(wheredir->getText());

    // Compose the find and grep command according to the selected options

    // Don't follow symlinks
    if (linkbtn->getCheck())
    {
        searchcommand = "find -P " + xf_quote(wheredir->getText());
    }
    else
    {
        searchcommand = "find -L " + xf_quote(wheredir->getText());
    }

    // Ignore case
    FXString nameoption;
    if (findigncase->getCheck())
    {
        nameoption = " -iname \"";
    }
    else
    {
        nameoption = " -name \"";
    }

    // Min file size
    if (minsize->getValue() > 0)
    {
        searchcommand += " -size +" + FXStringVal(minsize->getValue()) + "k";
    }

    // Max file size
    if (maxsize->getValue() > 0)
    {
        searchcommand += " -size -" + FXStringVal(maxsize->getValue()) + "k";
    }

    // Date modified before
    if (mindays->getValue() > 0)
    {
        searchcommand += " -mtime -" + FXStringVal(mindays->getValue());
    }

    // Date modified after
    if (maxdays->getValue() > 0)
    {
        searchcommand += " -mtime +" + FXStringVal(maxdays->getValue());
    }

    // User
    if (userbtn->getCheck())
    {
        searchcommand += " -user " + user->getText();
    }

    // Group
    if (grpbtn->getCheck())
    {
        searchcommand += " -group " + grp->getText();
    }

    // Non recursive
    if (norecbtn->getCheck())
    {
        searchcommand += " -maxdepth 1 ";
    }

    // Search other file systems
    if (!nofsbtn->getCheck())
    {
        searchcommand += " -mount ";
    }

    // File type
    if (typebtn->getCheck())
    {
        if (type->getCurrentItem() == 0)
        {
            searchcommand += " -type f";
        }
        else if (type->getCurrentItem() == 1)
        {
            searchcommand += " -type d";
        }
        else if (type->getCurrentItem() == 2)
        {
            searchcommand += " -type l";
        }
        else if (type->getCurrentItem() == 3)
        {
            searchcommand += " -type s";
        }
        else if (type->getCurrentItem() == 4)
        {
            searchcommand += " -type p";
        }
    }

    // Permissions
    if (permsbtn->getCheck())
    {
        searchcommand += " -perm " + perms->getText();
    }

    // Empty files
    if (emptybtn->getCheck())
    {
        searchcommand += " -empty";
    }

    // Hidden files
    if (!findhidden->getCheck())
    {
        searchcommand += " \\( ! -regex '.*/\\..*' \\)";
    }

    // Without grep command
    if (greptext->getText() == "")
    {
        searchcommand += nameoption + searchpattern + "\" -print";
    }
    // With grep command
    else
    {
        searchcommand += nameoption + searchpattern + "\" -exec grep -q -s ";

        // Ignore case
        if (grepigncase->getCheck())
        {
            searchcommand += "-i ";
        }

        searchcommand += "\"" + greptext->getText() + "\" '{}' \\; -print";
    }

    // Clear all file list items
    searchpanel->clearItems();

    // Don't use standard cursor wait function
    xf_setwaitcursor(getApp(), BEGIN_CURSOR);

    // Search message
    searchresults->setText(FXString(">>>> ") + _("Search started - Please wait...") + FXString(" <<<<"));

    // Running flag and rotating cursor
    running = true;
    rotcur = "-";

    // Execute command
    count = 0;
    strprev = "";
    warnwindow->setText("");

    getApp()->addTimeout(this, SearchWindow::ID_ROTATING_CURSOR, ROTATING_CURSOR_REFRESH_DELAY);

    execCmd(searchcommand.text());

    return 1;
}


// Execute the search command and capture its output
int SearchWindow::execCmd(FXString command)
{
    // Open pipes to communicate with child process
    if ((pipe(in) == -1) || (pipe(out) == -1))
    {
        return -1;
    }

    // Create child process
    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, _("Error: Fork failed: %s\n"), strerror(errno));
        return -1;
    }
    application->addInput(out[0], INPUT_READ, this, ID_READ_DATA);

    if (pid == 0)
    {
        // Here, we are running as the child process!
        char* args[4];
        ::close(out[0]);
        int ret1 = dup2(out[1], STDOUT_FILENO);
        int ret2 = dup2(out[1], STDERR_FILENO);
        ::close(in[1]);
        int ret3 = dup2(in[0], STDIN_FILENO);

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
        args[3] = NULL;                         // a new process

        setpgid(0, 0);                          // Allows to kill the whole group
        execvp(args[0], args);                  // Start a new process which will execute the command
        _exit(EXIT_FAILURE);                    // We'll get here only if an error occurred
    }
    else
    {
        // Here, we are running as the parent process!
        ::close(out[1]);
        ::close(in[0]);
    }

    return 1;
}


// Read data from pipe
long SearchWindow::onReadData(FXObject*, FXSelector, void*)
{
    char buf[4096];
    int nread, status, beg, end;
    FXString strbuf, pathname;

    if (running)
    {
        // Stop refreshing the file list
        searchpanel->setAllowRefresh(false);

        nread = read(out[0], buf, 4095);
        if (nread > 0)
        {
            buf[nread] = '\0';

            // Caution : string strprev is initialized in onCmdStart()
            strbuf = strprev + buf;

            // Loop over lines in strbuf
            for (beg = 0; beg < strbuf.length(); beg = end + 1)
            {
                // If line not complete, the fragment will be used in the next line
                if ((end = strbuf.find("\n", beg)) < 0)
                {
                    end = strbuf.length();
                    strprev = strbuf.mid(beg, end - beg);
                    break;
                }
                else
                {
                    strprev = "";
                }

                pathname = strbuf.mid(beg, end - beg);

                // Append item to the file panel
                // Only process valid file paths and paths different from the search directory
                if ((pathname != searchcommand) && (pathname != wheredir->getText()))
                {
                    if (pathname.find(PATHSEPSTRING) == 0)
                    {
                        if (searchpanel->appendItem(pathname))
                        {
                            count++;
                        }

                        // Refresh file list (two speeds depending on the number of items)
                        FXuint cnt = (count <
                                      REFRESH_COUNT_LIMIT ? count % REFRESH_COUNT_SLOW : count % REFRESH_COUNT_FAST);
                        if (cnt == 0)
                        {
                            searchpanel->setCurrentItem(count - 1);
                            searchpanel->setStatusText(FXStringVal(count) + _(" items"));
                            getApp()->forceRefresh();
                            getApp()->repaint();
                        }
                    }
                    else
                    {
                        if (!warnwindow->shown())
                        {
                            warnwindow->show(PLACEMENT_OWNER);
                        }

                        pathname = pathname + "\n";

                        warnwindow->appendText(pathname.text());
                        warnwindow->scrollToLastLine();
                    }
                }
            }
        }
        // Nothing to read
        else if (nread == 0)
        {
            waitpid(pid, &status, 0);
            application->removeInput(out[0], INPUT_READ);

            searchresults->setText(FXString(">>>> ") + _("Search results") + FXString(" <<<<"));

            running = false;

            // Don't use standard cursor wait function
            xf_setwaitcursor(getApp(), END_CURSOR);

            // Update item count
            searchpanel->setStatusText(FXStringVal(count) + _(" items"));
            getApp()->repaint();

            // Force file list refresh
            searchpanel->setSearchPath(wheredir->getText());  // To autosize, if enabled
            searchpanel->setAllowRefresh(true);
            searchpanel->onCmdRefresh(0, 0, 0);
        }
        // Input / Output error
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Input / Output error"));
            application->removeInput(out[0], INPUT_READ);
            exit(EXIT_FAILURE);
        }
    }

    return 1;
}


// Kill process when clicking on the stop button
long SearchWindow::onCmdStop(FXObject*, FXSelector, void*)
{
    if (running)
    {
        xf_setwaitcursor(getApp(), END_CURSOR);
        running = false;
        searchresults->setText(FXString(">>>> ") + _("Search stopped...") + FXString(" <<<<"));

        if (pid != -1)
        {
            kill((-1 * pid), SIGTERM); // Kills the process group
        }
        application->removeInput(out[0], INPUT_READ);
        searchpanel->setAllowRefresh(true);


        getApp()->removeTimeout(this, SearchWindow::ID_ROTATING_CURSOR);
    }

    return 1;
}


// Update stop button and other UI buttons
long SearchWindow::onUpdStop(FXObject*, FXSelector, void*)
{
    if (running)
    {
        // Enable stop button while searching files
        stopbutton->enable();

        // Disable all other buttons
        searchpanel->disableButtons();
        findfile->disable();
        findigncase->disable();
        findhidden->disable();
        wheredir->disable();
        dirbutton->disable();
        greptext->disable();
        grepigncase->disable();
        moreoptions->disable();
        resetoptions->disable();
        minsize->disable();
        maxsize->disable();
        mindays->disable();
        maxdays->disable();
        userbtn->disable();
        grpbtn->disable();
        typebtn->disable();
        permsbtn->disable();
        emptybtn->disable();
        linkbtn->disable();
        perms->disable();
        type->disable();
        user->disable();
        grp->disable();
        norecbtn->disable();
        nofsbtn->disable();
    }
    else
    {
        // Disable stop button while not searching files
        stopbutton->disable();

        // Enable all other buttons
        searchpanel->enableButtons();
        findfile->enable();
        findigncase->enable();
        findhidden->enable();
        wheredir->enable();
        dirbutton->enable();
        greptext->enable();
        grepigncase->enable();
        moreoptions->enable();
        resetoptions->enable();
        minsize->enable();
        maxsize->enable();
        mindays->enable();
        maxdays->enable();
        userbtn->enable();
        grpbtn->enable();
        typebtn->enable();
        permsbtn->enable();
        emptybtn->enable();
        linkbtn->enable();
        perms->enable();
        type->enable();
        user->enable();
        grp->enable();
        norecbtn->enable();
        nofsbtn->enable();
    }

    return 1;
}


// Update start button
long SearchWindow::onUpdStart(FXObject*, FXSelector, void*)
{
    if (running)
    {
        startbutton->disable();
    }
    else
    {
        startbutton->enable();

        getApp()->removeTimeout(this, SearchWindow::ID_ROTATING_CURSOR);
    }

    return 1;
}


// Show window such that the cursor is in it
void SearchWindow::show(FXuint placement)
{
    // Clear all file list items
    searchpanel->clearItems();

    // Sort history
    findfile->setSortFunc(FXList::ascendingCase);
    findfile->sortItems();

    // Set focus on the find search field and select all chars
    findfile->selectAll();
    findfile->CursorEnd();

    // Empty find text at first call
    if (firstcall)
    {
        findfile->setText("");
        firstcall = false;
    }

    // Pop the window
    FXTopWindow::show(placement);
}


// Keyboard press
long SearchWindow::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    if (event->code == KEY_Escape)
    {
        // Kill process
        handle(this, FXSEL(SEL_COMMAND, ID_STOP), NULL);

        // Deselect files if any
        searchpanel->handle(sender, FXSEL(SEL_COMMAND, SearchPanel::ID_DESELECT_ALL), ptr);

        return 1;
    }
    else if ((event->code == KEY_F3) || ((searchpanel->hasFocus() == false) && (event->code == KEY_Return)))
    {
        // Start process
        handle(this, FXSEL(SEL_COMMAND, ID_START), NULL);
        return 1;
    }
    // Shift-F10 or menu was pressed : open popup menu
    else if ((event->state & SHIFTMASK && event->code == KEY_F10) || event->code == KEY_Menu)
    {
        searchpanel->handle(sender, FXSEL(SEL_COMMAND, SearchPanel::ID_POPUP_MENU), ptr);
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


// Execute dialog box modally
FXuint SearchWindow::execute(FXuint placement)
{
    create();
    show(placement);
    getApp()->refresh();
    return getApp()->runModalFor(this);
}


// Browse the file system
long SearchWindow::onCmdBrowsePath(FXObject* sender, FXSelector sel, void* ptr)
{
    FileDialog browse(this, _("Select path"), 0, 0, 0, 650, 480);

    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };

    browse.setFilename(wheredir->getText());
    browse.setPatternList(patterns);

    // Browse files in mixed mode
    browse.setSelectMode(SELECT_FILE_DIRECTORY);
    if (browse.execute())
    {
        FXString path = browse.getFilename();
        wheredir->setText(path);
    }
    return 1;
}


// Update permissions string
long SearchWindow::onUpdPerms(FXObject* sender, FXSelector, void*)
{
    int len = perms->getText().length();

    if (len < 4)
    {
        FXString str;
        if (len == 0)
        {
            perms->setText("0644");
        }
        else if (len == 1)
        {
            str = "000" + perms->getText();
        }
        else if (len == 2)
        {
            str = "00" + perms->getText();
        }
        else
        {
            str = "0" + perms->getText();
        }

        perms->setText(str);
    }

    return 1;
}


// Update size buttons
long SearchWindow::onUpdSize(FXObject* sender, FXSelector, void*)
{
    // Disable or enable size buttons
    if (emptybtn->getCheck())
    {
        minsize->setValue(0);
        maxsize->setValue(0);
        minsize->disable();
        maxsize->disable();
    }
    else
    {
        minsize->enable();
        maxsize->enable();
    }

    return 1;
}


// Reset search options
long SearchWindow::onCmdResetOptions(FXObject* sender, FXSelector sel, void*)
{
    minsize->setValue(0);
    maxsize->setValue(0);
    mindays->setValue(0);
    maxdays->setValue(0);
    userbtn->setCheck(false);
    grpbtn->setCheck(false);
    typebtn->setCheck(false);
    permsbtn->setCheck(false);
    emptybtn->setCheck(false);
    linkbtn->setCheck(false);
    perms->setText("0644");
    type->setCurrentItem(0);
    user->setText(uid);
    grp->setText(gid);
    norecbtn->setCheck(false);
    nofsbtn->setCheck(false);

    return 1;
}


// Refresh rotating cursor
long SearchWindow::onRotatingCursorRefresh(FXObject*, FXSelector, void*)
{
    FXString msg = _("Search started - Please wait...");
    FXString gt = ">>>> ";
    FXString lt = " <<<<";


    // Show a rotating cursor
    if (rotcur == "—") // Em dash
    {
        searchresults->setText(gt + msg + FXString(" \\") + lt);
        rotcur = "\\";
    }
    else if (rotcur == "\\")
    {
        searchresults->setText(gt + msg + FXString(" |") + lt);
        rotcur = "|";
    }
    else if (rotcur == "|")
    {
        searchresults->setText(gt + msg + FXString(" /") + lt);
        rotcur = "/";
    }
    else
    {
        searchresults->setText(gt + msg + FXString(" —") + lt);
        rotcur = "—"; // Em dash
    }

    getApp()->addTimeout(this, SearchWindow::ID_ROTATING_CURSOR, ROTATING_CURSOR_REFRESH_DELAY);

    return 1;
}
