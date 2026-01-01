// Bulk rename dialog
// For replace and insert text renaming, when the number of files in the list is high, a periodic refresh
// is used instead of a GUI changed refresh
// This is done to prevent the GUI from being unresponsive when the user enters a string

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "icons.h"
#include "FileDict.h"
#include "RenameDialog.h"


// Minimum header size for lists
#ifndef MIN_HEADER_SIZE
#define MIN_HEADER_SIZE    100
#endif


// Minimum number of files for using periodic refresh
#define NUMFILES_TIMEOUT    1000

// Periodic refresh interval (ms)
#define REFRESH_TIMEOUT     1000


// Clipboard
extern FXString clipboard;
extern FXuint clipboard_type;


// Obtain message for items count
FXString getItemsMsg(FXString tgt, FXuint num)
{
    int numRenamed = 0;

    for (FXuint i = 0; i < num; i++)
    {
        FXString tgt_i = tgt.section('\n', i);

        if (tgt_i.length() > 0)
        {
            numRenamed++;
        }
    }

    FXString str = (numRenamed < 2 ? _("item to rename") : _("items to rename"));
    FXString msg = FXStringVal(numRenamed) + " " + str + _(" out of ") + FXStringVal(num) + " " + _("items");

    return msg;
}


FXDEFMAP(RenameDialog) RenameDialogMap[] =
{
    FXMAPFUNC(SEL_KEYPRESS, 0, RenameDialog::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, RenameDialog::onKeyRelease),
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, RenameDialog::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, RenameDialog::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, RenameDialog::onClipboardRequest),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_ACCEPT, RenameDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_CANCEL, RenameDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_SORT_BY_NAME, RenameDialog::onCmdSortByName),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_SORT_BY_FOLDERNAME, RenameDialog::onCmdSortByFolderName),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_SORT_BY_NEWNAME, RenameDialog::onCmdSortByNewName),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_LIST, RenameDialog::onCmdHeaderClicked),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_MODE_REPLACE, RenameDialog::onCmdMode),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_MODE_CASE, RenameDialog::onCmdMode),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_MODE_INSERT, RenameDialog::onCmdMode),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_MODE_REMOVE, RenameDialog::onCmdMode),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_NAME_ONLY, RenameDialog::onCmdNameOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_EXT_ONLY, RenameDialog::onCmdNameOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_WHOLE_NAME, RenameDialog::onCmdNameOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_IGNORE_CASE, RenameDialog::onCmdIgnoreCase),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_LOWER_CASE, RenameDialog::onCmdCaseOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_UPPER_CASE, RenameDialog::onCmdCaseOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_TITLE_CASE, RenameDialog::onCmdCaseOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_FIRST_UPPER_CASE, RenameDialog::onCmdCaseOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_INSERT_FROM_POS, RenameDialog::onCmdInsertOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_INSERT_FROM_END, RenameDialog::onCmdInsertOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_INSERT_INS, RenameDialog::onCmdInsertOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_INSERT_OVW, RenameDialog::onCmdInsertOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_REMOVE_CHARS, RenameDialog::onCmdRemoveOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_REMOVE_FROM_POS, RenameDialog::onCmdRemoveOptions),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_REMOVE_FROM_END, RenameDialog::onCmdRemoveOptions),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, RenameDialog::ID_LIST, RenameDialog::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, RenameDialog::ID_COPYNAME_CLIPBOARD, RenameDialog::onCmdCopyName),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_LIST, RenameDialog::onUpdHeader),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_MODE_REPLACE, RenameDialog::onUpdMode),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_MODE_CASE, RenameDialog::onUpdMode),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_MODE_INSERT, RenameDialog::onUpdMode),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_MODE_REMOVE, RenameDialog::onUpdMode),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_NAME_ONLY, RenameDialog::onUpdNameOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_EXT_ONLY, RenameDialog::onUpdNameOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_WHOLE_NAME, RenameDialog::onUpdNameOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_LOWER_CASE, RenameDialog::onUpdCaseOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_UPPER_CASE, RenameDialog::onUpdCaseOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_TITLE_CASE, RenameDialog::onUpdCaseOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_FIRST_UPPER_CASE, RenameDialog::onUpdCaseOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_INSERT_INS, RenameDialog::onUpdInsertOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_INSERT_OVW, RenameDialog::onUpdInsertOptions),
    FXMAPFUNC(SEL_UPDATE, RenameDialog::ID_ACCEPT, RenameDialog::onUpdAccept),
    FXMAPFUNC(SEL_CHANGED, RenameDialog::ID_INSERT_TEXT, RenameDialog::onUpdListInsert),
    FXMAPFUNC(SEL_CHANGED, RenameDialog::ID_SEARCH_TEXT, RenameDialog::onUpdListReplace),
    FXMAPFUNC(SEL_CHANGED, RenameDialog::ID_REPLACE_TEXT, RenameDialog::onUpdListReplace),
    FXMAPFUNC(SEL_TIMEOUT, RenameDialog::ID_INSERT_TEXT, RenameDialog::onTimeoutListInsert),
    FXMAPFUNC(SEL_TIMEOUT, RenameDialog::ID_SEARCH_TEXT, RenameDialog::onTimeoutListReplace),
    FXMAPFUNC(SEL_TIMEOUT, RenameDialog::ID_REPLACE_TEXT, RenameDialog::onTimeoutListReplace),
};


// Object implementation
FXIMPLEMENT(RenameDialog, DialogBox, RenameDialogMap, ARRAYNUMBER(RenameDialogMap))

// Construct
RenameDialog::RenameDialog(FXWindow* win, FXString title, FXString src, FXuint num, FXbool showfld, int w, int h) :
    DialogBox(win, title, DECOR_TITLE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE, 0, 0, w, h)
{
    // Show folder name flag
    showfolder = showfld;

    // File name list
    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    list = new IconList(contents, NULL, 0, this, ID_LIST,
                        ICONLIST_STANDARD | ICONLIST_BROWSESELECT | LAYOUT_SIDE_TOP | LAYOUT_LEFT | LAYOUT_FILL_X |
                        LAYOUT_FILL_Y);

    // Number of renamed items
    new FXFrame(contents, JUSTIFY_LEFT);
    itemslbl = new FXLabel(contents, "", NULL, JUSTIFY_LEFT);
    new FXFrame(contents, JUSTIFY_LEFT);

    // Modes and name
    FXHorizontalFrame* contents1 = new FXHorizontalFrame(contents, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXGroupBox* group1 = new FXGroupBox(contents1, _("Mode"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXGroupBox* group2 = new FXGroupBox(contents1, _("Name"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXMatrix* matrix2 = new FXMatrix(group1, 3,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    replaceradio = new FXRadioButton(matrix2, _("Replace text"), this, ID_MODE_REPLACE);
    new FXLabel(matrix2, "     ", NULL, JUSTIFY_LEFT);
    caseradio = new FXRadioButton(matrix2, _("Change case"), this, ID_MODE_CASE);
    insertradio = new FXRadioButton(matrix2, _("Insert / Overwrite text"), this, ID_MODE_INSERT);
    new FXLabel(matrix2, "     ", NULL, JUSTIFY_LEFT);
    removeradio = new FXRadioButton(matrix2, _("Remove text"), this, ID_MODE_REMOVE);
    new FXLabel(matrix2, "     ", NULL, JUSTIFY_LEFT);
    FXMatrix* matrix3 = new FXMatrix(group2, 1,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    wholenameradio = new FXRadioButton(matrix3, _("Name and extension"), this, ID_WHOLE_NAME);
    nameonlyradio = new FXRadioButton(matrix3, _("Name only"), this, ID_NAME_ONLY);
    extonlyradio = new FXRadioButton(matrix3, _("Extension only"), this, ID_EXT_ONLY);

    // Replace options
    replacegroup = new FXGroupBox(contents, _("Options"),
                                  GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXMatrix* matrix4 = new FXMatrix(replacegroup, 3,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    new FXLabel(matrix4, _("Search for:"), NULL, JUSTIFY_LEFT);
    searchfld = new FXTextField(matrix4, 30, this, ID_SEARCH_TEXT,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X |
                                LAYOUT_CENTER_Y);
    searchfld->setFocus();
                                
    ignorecasechk = new FXCheckButton(matrix4, _("Ignore case"), this, ID_IGNORE_CASE);
    new FXLabel(matrix4, _("Replace with:"), NULL, JUSTIFY_LEFT);
    replacefld = new FXTextField(matrix4, 30, this, ID_REPLACE_TEXT,
                                 TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X |
                                 LAYOUT_CENTER_Y);

    // Change case options
    casegroup = new FXGroupBox(contents, _("Options"),
                               GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXMatrix* matrix5 = new FXMatrix(casegroup, 3,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    lowercaseradio = new FXRadioButton(matrix5, _("Lowercase"), this, ID_LOWER_CASE);
    new FXLabel(matrix5, "     ", NULL, JUSTIFY_LEFT);
    uppercaseradio = new FXRadioButton(matrix5, _("Uppercase"), this, ID_UPPER_CASE);
    titlecaseradio = new FXRadioButton(matrix5, _("Title case"), this, ID_TITLE_CASE);
    new FXLabel(matrix5, "     ", NULL, JUSTIFY_LEFT);
    firstuppercaseradio = new FXRadioButton(matrix5, _("First character uppercase"), this, ID_FIRST_UPPER_CASE);

    // Remove options
    removegroup = new FXGroupBox(contents, _("Options"),
                                 GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXMatrix* matrix6 = new FXMatrix(removegroup, 8,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    new FXLabel(matrix6, _("Remove:"), NULL, JUSTIFY_LEFT);
    removecharsspin = new FXSpinner(matrix6, 3, this, ID_REMOVE_CHARS, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW,
                                    0, 0, 0, 0, 2, 2, 1, 1);
    removecharsspin->setRange(0, 100);
    new FXLabel(matrix6, _("character(s)"), NULL, JUSTIFY_LEFT);
    new FXLabel(matrix6, "     ", NULL, JUSTIFY_LEFT);
    new FXLabel(matrix6, _("From position:"), NULL, JUSTIFY_LEFT);
    removefromposspin = new FXSpinner(matrix6, 3, this, ID_REMOVE_FROM_POS,
                                      JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2, 2, 1, 1);
    removefromposspin->setRange(0, 100);
    new FXLabel(matrix6, "     ", NULL, JUSTIFY_LEFT);
    removefromendchk = new FXCheckButton(matrix6, _("Starting from the end"), this, ID_REMOVE_FROM_END);

    // Insert / Overwrite options
    insertgroup = new FXGroupBox(contents, _("Options"),
                                 GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXHorizontalFrame* contents2 = new FXHorizontalFrame(insertgroup, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    insradio = new FXRadioButton(contents2, _("Insert"), this, ID_INSERT_INS);
    new FXLabel(contents2, "     ", NULL, JUSTIFY_LEFT);
    ovwradio = new FXRadioButton(contents2, _("Overwrite"), this, ID_INSERT_OVW);
    FXVerticalFrame* contents3 = new FXVerticalFrame(insertgroup, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    FXTextField* txt = new FXTextField(contents3, 80, NULL, 0, TEXTFIELD_READONLY | _TEXTFIELD_NOFRAME | JUSTIFY_LEFT);
    txt->setText(_("(Use %n, %0n, %00n, %000n, %0000n or %00000n to number)"));
    txt->setBackColor(getApp()->getBaseColor());
    FXMatrix* matrix7 = new FXMatrix(insertgroup, 5,
                                     MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    new FXLabel(matrix7, _("Text:"), NULL, ID_INSERT_TEXT, TEXTFIELD_NORMAL | JUSTIFY_LEFT);
    insertfld = new FXTextField(matrix7, 30, this, ID_INSERT_TEXT,
                                TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X |
                                LAYOUT_CENTER_Y);
    new FXLabel(matrix7, _("From position:"), NULL, JUSTIFY_LEFT);
    insertfromposspin = new FXSpinner(matrix7, 3, this, ID_INSERT_FROM_POS,
                                      JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2, 2, 1, 1);
    insertfromposspin->setRange(0, 100);
    insertfromendchk = new FXCheckButton(matrix7, _("Starting from the end"), this, ID_INSERT_FROM_END);

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(contents, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10,
                                                       5, 5);

    // Accept button
    acceptbtn = new FXButton(buttons, _("&Accept"), NULL, this, RenameDialog::ID_ACCEPT,
                             FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    acceptbtn->addHotKey(KEY_Return);

    // Cancel button
    new FXButton(buttons, _("&Cancel"), NULL, this, RenameDialog::ID_CANCEL,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Set list headers name and size
    if (showfolder)
    {
        FXuint hsize1 = getWidth() / 3 - 20;
        FXuint hsize2 = getWidth() / 3 - 20;
        FXuint hsize3 = getWidth() / 3 - 20;
        list->appendHeader(_("Name"), NULL, hsize1);
        list->appendHeader(_("Folder"), NULL, hsize2);
        list->appendHeader(_("New Name"), NULL, hsize3);
    }
    else
    {
        FXuint hsize1 = getWidth() / 2 - 20;
        FXuint hsize2 = getWidth() / 2 - 20;

        list->appendHeader(_("Name"), NULL, hsize1);
        list->appendHeader(_("New Name"), NULL, hsize2);
    }

    // Initialize sort functions
    list->setSortFunc(ascendingName);


    // File associations
    associations = NULL;
    associations = new FileDict(getApp());


    // Initializations
    numfiles = num;
    source = src;               // List of source file names
    target = "";                // List of destination file names

    mode = ID_MODE_REPLACE;
    nameoption = ID_WHOLE_NAME;
    ignorecase = false;
    caseoption = ID_LOWER_CASE;

    removechars = 0;
    removefrompos = 0;
    removefromend = false;

    insertfrompos = 0;
    insertfromend = false;
    insertoption = ID_INSERT_INS;

    prev_searchtxt = "";
    prev_replacetxt = "";
    prev_insertstr = "";
}


// Create window
void RenameDialog::create()
{
    // Register standard uri-list type
    urilistType = getApp()->registerDragType("text/uri-list");

    // Register special uri-list type used for Gnome, XFCE and Xfe
    xfelistType = getApp()->registerDragType("x-special/gnome-copied-files");

    // Register special uri-list type used for KDE
    kdelistType = getApp()->registerDragType("application/x-kde-cutselection");

    // Register standard UTF-8 text type used for file dialogs
    utf8Type = getApp()->registerDragType("UTF8_STRING");

    // Show / hide groups
    replacegroup->show();
    casegroup->hide();
    removegroup->hide();
    insertgroup->hide();

    // Create dialog box
    DialogBox::create();

    // Fill list
    getApp()->beginWaitCursor();
    list->clearItems();
    for (FXuint i = 0; i < numfiles; i++)
    {
        FXString sourcepath = source.section('\n', i);

        // Append item to the list
        appendItem(sourcepath, "");
    }
    list->sortItems();

    // Number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    // Deselect all items
    list->killSelection();
    getApp()->endWaitCursor();

    // Start timeout
    getApp()->addTimeout(this, ID_INSERT_TEXT, REFRESH_TIMEOUT);
    getApp()->addTimeout(this, ID_SEARCH_TEXT, REFRESH_TIMEOUT);
    getApp()->addTimeout(this, ID_REPLACE_TEXT, REFRESH_TIMEOUT);
}


// Delete objects
RenameDialog::~RenameDialog()
{
    // Remove timeout
    getApp()->removeTimeout(this, ID_INSERT_TEXT);
    getApp()->removeTimeout(this, ID_SEARCH_TEXT);
    getApp()->removeTimeout(this, ID_REPLACE_TEXT);

    delete list;
    delete itemslbl;
    delete acceptbtn;

    delete replaceradio;
    delete caseradio;
    delete insertradio;
    delete removeradio;
    delete nameonlyradio;
    delete extonlyradio;
    delete wholenameradio;
    delete ignorecasechk;
    delete searchfld;
    delete replacefld;
    delete lowercaseradio;
    delete uppercaseradio;
    delete titlecaseradio;
    delete firstuppercaseradio;
    delete removecharsspin;
    delete removefromendchk;
    delete removefromposspin;
    delete insradio;
    delete ovwradio;
    delete insertfromposspin;
    delete insertfromendchk;
    delete insertfld;

    delete associations;

    // Must be the latest
    delete replacegroup;
    delete casegroup;
    delete removegroup;
    delete insertgroup;
}


// Append items to the list
void RenameDialog::appendItem(FXString sourcepath, FXString targetname)
{
    FXString filename = FXPath::name(sourcepath);
    FXString dirname = FXPath::directory(sourcepath);

    // Obtain item icon
    FXbool isLink, isBrokenLink, isLinkToDir, isDir;
    FileAssoc* fileassoc;
    FXIcon* mini = NULL;
    struct stat info, linfo;

    // Only process valid file paths and paths different from the search directory
    if (xf_lstat(sourcepath.text(), &linfo) == 0)
    {
        // Get file/link info and indicate if it is a link
        isLink = S_ISLNK(linfo.st_mode);
        isBrokenLink = false;

        // Find if it is a broken link or a link to a directory
        isLinkToDir = false;
        if (isLink)
        {
            if (xf_stat(sourcepath.text(), &info) != 0)
            {
                isBrokenLink = true;
            }
            else if (S_ISDIR(info.st_mode))
            {
                isLinkToDir = true;
            }
        }

        // Find if it is a folder
        isDir = false;
        if (S_ISDIR(linfo.st_mode))
        {
            isDir = true;
        }

        // Obtain the stat info on the file itself
        if (xf_stat(sourcepath.text(), &info) != 0)
        {
            // Except in the case of a broken link
            if (isBrokenLink)
            {
                xf_lstat(sourcepath.text(), &info);
            }
        }

        // Assume no associations
        fileassoc = NULL;

        // Determine icons and type
        if (isDir)
        {
            if (!xf_isreadexecutable(sourcepath))
            {
                mini = minifolderlockedicon;
            }
            else
            {
                mini = minifoldericon;
            }
        }
        else if (S_ISCHR(info.st_mode))
        {
            mini = minichardevicon;
        }
        else if (S_ISBLK(info.st_mode))
        {
            mini = miniblockdevicon;
        }
        else if (S_ISFIFO(info.st_mode))
        {
            mini = minipipeicon;
        }
        else if (S_ISSOCK(info.st_mode))
        {
            mini = minisocketicon;
        }
        else if ((info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) && !(S_ISDIR(info.st_mode) ||
                                                                     S_ISCHR(info.st_mode) || S_ISBLK(info.st_mode) ||
                                                                     S_ISFIFO(info.st_mode) ||
                                                                     S_ISSOCK(info.st_mode)))
        {
            mini = miniexecicon;
            if (associations)
            {
                fileassoc = associations->findFileBinding(sourcepath.text());
            }
        }
        else
        {
            mini = minidocicon;
            if (associations)
            {
                fileassoc = associations->findFileBinding(sourcepath.text());
            }
        }

        // If association is found, use it
        if (fileassoc)
        {
            if (fileassoc->miniicon)
            {
                mini = fileassoc->miniicon;

                // Use IMAGE_KEEP for blending to background
                mini->setOptions(IMAGE_KEEP);

                mini->create();
            }
        }

        // Symbolic links have a specific icon
        if (isLink)
        {
            // Broken link
            if (isBrokenLink)
            {
                mini = minibrokenlinkicon;
            }
            else
            {
                if (isLinkToDir)
                {
                    mini = minifolderlinkicon;
                }
                else
                {
                    mini = minilinkicon;
                }
            }
        }
    }

    // Append item to the list
    if (showfolder)
    {
        list->appendItem(filename + TAB + dirname + TAB + targetname, NULL, mini);
    }
    else
    {
        list->appendItem(filename + TAB + targetname, NULL, mini);
    }
}


// Perform replace
FXString RenameDialog::doReplace(FXbool ignorecase, FXuint nameoption, FXString src,
                                 FXuint num, FXString searchtxt, FXString replacetxt)
{
    FXString target = "";

    // Fill list
    getApp()->beginWaitCursor();
    list->clearItems();
    for (FXuint i = 0; i < num; i++)
    {
        FXString sourcepath = src.section('\n', i);

        // Source and target names
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = sourcename;

        // Name and extension
        FXString namestr = targetname.before('.', 1);
        FXString extstr = targetname.after('.', 1);

        // Substitute string
        if (nameoption == ID_NAME_ONLY)
        {
            FXString ext = (extstr == "" ? "" : "." + extstr);

            if (ignorecase)
            {
                targetname = xf_substitutecase(namestr, searchtxt, replacetxt, true) + ext;
            }
            else
            {
                targetname = namestr.substitute(searchtxt, replacetxt, true) + ext;
            }
        }
        else if (nameoption == ID_EXT_ONLY)
        {
            if (ignorecase)
            {
                FXString str = xf_substitutecase(extstr, searchtxt, replacetxt, true);

                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
            else
            {
                FXString str = extstr.substitute(searchtxt, replacetxt, true);

                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
        }
        else // Name and extension
        {
            if (ignorecase)
            {
                targetname = xf_substitutecase(targetname, searchtxt, replacetxt, true);
            }
            else
            {
                targetname = targetname.substitute(searchtxt, replacetxt, true);
            }
        }

        // Append to the list
        if (targetname != sourcename && targetname != "")
        {
            appendItem(sourcepath, targetname);
            target += FXPath::directory(sourcepath) + PATHSEPSTRING + targetname + "\n";
        }
        else
        {
            target += FXString("") + "\n";
            appendItem(sourcepath, "");
        }
    }
    list->sortItems();

    // Deselect all items
    list->killSelection();
    getApp()->endWaitCursor();

    return target;
}


// Perform change case
FXString RenameDialog::doChangeCase(FXuint nameoption, FXuint caseoption, FXString src, FXuint num)
{
    FXString target = "";

    // Fill list
    getApp()->beginWaitCursor();
    list->clearItems();
    for (FXuint i = 0; i < num; i++)
    {
        FXString sourcepath = src.section('\n', i);

        // Source and target names
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = sourcename;

        // Name and extension
        FXString namestr = targetname.before('.', 1);
        FXString extstr = targetname.after('.', 1);

        // Change string case
        if (caseoption == ID_UPPER_CASE)
        {
            if (nameoption == ID_NAME_ONLY)
            {
                if (extstr == "")
                {
                    targetname = namestr.upper();
                }
                else
                {
                    targetname = namestr.upper() + "." + extstr;
                }
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                FXString str = extstr.upper();

                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
            else // Name and extension
            {
                targetname = targetname.upper();
            }
        }
        else if (caseoption == ID_TITLE_CASE)
        {
            if (nameoption == ID_NAME_ONLY)
            {
                int numSpaces = namestr.contains(' ');

                targetname = "";

                for (int i = 0; i <= numSpaces; i++)
                {
                    FXString str = namestr.section(' ', i);

                    // Take care of UTF-8 character
                    FXuint extent = str.extent(0);
                    FXString first = str.left(extent);
                    targetname += str.replace(0, extent, first.upper());

                    if (i < numSpaces)
                    {
                        targetname += " ";
                    }
                }
                if (extstr != "")
                {
                    targetname += "." + extstr;
                }
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                int numSpaces = extstr.contains(' ');

                targetname = "";

                for (int i = 0; i <= numSpaces; i++)
                {
                    FXString str = extstr.section(' ', i);

                    // Take care of UTF-8 character
                    FXuint extent = str.extent(0);
                    FXString first = str.left(extent);
                    targetname += str.replace(0, extent, first.upper());

                    if (i < numSpaces)
                    {
                        targetname += " ";
                    }
                }

                if (targetname == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + targetname;
                }
            }
            else // Name and extension
            {
                int numSpaces;
                numSpaces = namestr.contains(' ');

                FXString tgtname = "";

                for (int i = 0; i <= numSpaces; i++)
                {
                    FXString str = namestr.section(' ', i);

                    // Take care of UTF-8 character
                    FXuint extent = str.extent(0);
                    FXString first = str.left(extent);
                    tgtname += str.replace(0, extent, first.upper());

                    if (i < numSpaces)
                    {
                        tgtname += " ";
                    }
                }

                numSpaces = extstr.contains(' ');

                FXString tgtext = "";

                for (int i = 0; i <= numSpaces; i++)
                {
                    FXString str = extstr.section(' ', i);

                    // Take care of UTF-8 character
                    FXuint extent = str.extent(0);
                    FXString first = str.left(extent);
                    tgtext += str.replace(0, extent, first.upper());

                    if (i < numSpaces)
                    {
                        tgtext += " ";
                    }
                }

                if (tgtext == "")
                {
                    targetname = tgtname;
                }
                else
                {
                    targetname = tgtname + "." + tgtext;
                }
            }
        }
        else if (caseoption == ID_FIRST_UPPER_CASE)
        {
            if (nameoption == ID_NAME_ONLY)
            {
                FXString ext = (extstr == "" ? "" : "." + extstr);

                // Take care of UTF-8 character
                FXuint extent = namestr.extent(0);
                FXString first = namestr.left(extent);
                FXString str = namestr.replace(0, extent, first.upper());

                targetname = str + ext;
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                // Take care of UTF-8 character
                FXuint extent = extstr.extent(0);
                FXString first = extstr.left(extent);
                FXString str = extstr.replace(0, extent, first.upper());

                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
            else
            {
                // Take care of UTF-8 character
                FXuint extent = targetname.extent(0);
                FXString first = targetname.left(extent);
                targetname = targetname.replace(0, extent, first.upper());
            }
        }
        else // Lowercase
        {
            if (nameoption == ID_NAME_ONLY)
            {
                FXString ext = (extstr == "" ? "" : "." + extstr);
                targetname = namestr.lower() + ext;
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                FXString str = extstr.lower();

                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
            else // Name and extension
            {
                targetname = targetname.lower();
            }
        }

        // Append to the list
        if (targetname != sourcename && targetname != "")
        {
            appendItem(sourcepath, targetname);
            target += FXPath::directory(sourcepath) + PATHSEPSTRING + targetname + "\n";
        }
        else
        {
            target += FXString("") + "\n";
            appendItem(sourcepath, "");
        }
    }
    list->sortItems();

    // Deselect all items
    list->killSelection();
    getApp()->endWaitCursor();

    return target;
}


// Perform remove
FXString RenameDialog::doRemove(FXuint nameoption, FXuint len, FXuint pos,
                                FXbool fromend, FXString src, FXuint num)
{
    FXString target = "";

    // Fill list
    getApp()->beginWaitCursor();
    list->clearItems();
    for (FXuint i = 0; i < num; i++)
    {
        FXString sourcepath = src.section('\n', i);

        // Source and target names
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = sourcename;

        // Name and extension
        FXString namestr = targetname.before('.', 1);
        FXString extstr = targetname.after('.', 1);

        if (len > 0)
        {
            int p1, p2, rpos;

            // Name options
            if (nameoption == ID_NAME_ONLY)
            {
                if (fromend)
                {
                    rpos = FXMAX(namestr.count() - pos - len, 0);
                    p1 = namestr.offset(rpos);
                    p2 = namestr.offset(rpos + len);
                }
                else
                {
                    p1 = namestr.offset(pos);
                    p2 = namestr.offset(pos + len);
                }

                FXString ext = (extstr == "" ? "" : "." + extstr);
                targetname = namestr.erase(p1, p2 - p1) + ext;
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                if (fromend)
                {
                    rpos = FXMAX(extstr.count() - pos - len, 0);
                    p1 = extstr.offset(rpos);
                    p2 = extstr.offset(rpos + len);
                }
                else
                {
                    p1 = extstr.offset(pos);
                    p2 = extstr.offset(pos + len);
                }

                FXString str = extstr.erase(p1, p2 - p1);
                if (str == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + str;
                }
            }
            else // Name and extension
            {
                if (fromend)
                {
                    rpos = FXMAX(targetname.count() - pos - len, 0);
                    p1 = targetname.offset(rpos);
                    p2 = targetname.offset(rpos + len);
                }
                else
                {
                    p1 = targetname.offset(pos);
                    p2 = targetname.offset(pos + len);
                }

                targetname = targetname.erase(p1, p2 - p1);
            }
        }

        // Append to the list
        if (targetname != sourcename && targetname != "")
        {
            appendItem(sourcepath, targetname);
            target += FXPath::directory(sourcepath) + PATHSEPSTRING + targetname + "\n";
        }
        else
        {
            target += FXString("") + "\n";
            appendItem(sourcepath, "");
        }
    }
    list->sortItems();

    // Deselect all items
    list->killSelection();
    getApp()->endWaitCursor();

    return target;
}


// Perform insert
FXString RenameDialog::doInsert(FXuint nameoption, FXString inserttext, FXuint pos,
                                FXuint insertoption, FXbool fromend, FXString src, FXuint num)
{
    int len = inserttext.length();

    FXString target = "";

    // Fill list
    getApp()->beginWaitCursor();
    list->clearItems();
    for (FXuint i = 0; i < num; i++)
    {
        FXString sourcepath = src.section('\n', i);

        // Source and target names
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = sourcename;

        // Name and extension
        FXString namestr = targetname.before('.', 1);
        FXString extstr = targetname.after('.', 1);

        if (len > 0)
        {
            FXString text = inserttext;

            // Perform numbering
            if (text.contains("%n"))
            {
                text = text.substitute("%n", FXStringFormat("%d", i + 1), true);
            }
            if (text.contains("%0n"))
            {
                text = text.substitute("%0n", FXStringFormat("%02d", i + 1), true);
            }
            if (text.contains("%00n"))
            {
                text = text.substitute("%00n", FXStringFormat("%03d", i + 1), true);
            }
            if (text.contains("%000n"))
            {
                text = text.substitute("%000n", FXStringFormat("%04d", i + 1), true);
            }
            if (text.contains("%0000n"))
            {
                text = text.substitute("%0000n", FXStringFormat("%05d", i + 1), true);
            }
            if (text.contains("%00000n"))
            {
                text = text.substitute("%00000n", FXStringFormat("%06d", i + 1), true);
            }

            int p1, p2, rpos;

            // Name options
            if (nameoption == ID_NAME_ONLY)
            {
                if (fromend)
                {
                    rpos = FXMAX(namestr.count() - pos, 0);
                    p1 = namestr.offset(rpos);
                    p2 = namestr.offset(rpos + len);
                }
                else
                {
                    p1 = namestr.offset(pos);
                    p2 = namestr.offset(pos + len);
                }

                // Overwrite
                if (insertoption == ID_INSERT_OVW)
                {
                    namestr = namestr.erase(p1, p2 - p1);
                }

                FXString ext = (extstr == "" ? "" : "." + extstr);
                targetname = namestr.insert(p1, text) + ext;
            }
            else if (nameoption == ID_EXT_ONLY)
            {
                if (fromend)
                {
                    rpos = FXMAX(extstr.count() - pos, 0);
                    p1 = extstr.offset(rpos);
                    p2 = extstr.offset(rpos + len);
                }
                else
                {
                    p1 = extstr.offset(pos);
                    p2 = extstr.offset(pos + len);
                }

                // Overwrite
                if (insertoption == ID_INSERT_OVW)
                {
                    extstr = extstr.erase(p1, p2 - p1);
                }

                extstr = extstr.insert(p1, text);

                if (extstr == "")
                {
                    targetname = namestr;
                }
                else
                {
                    targetname = namestr + "." + extstr;
                }
            }
            else // Name and extension
            {
                if (fromend)
                {
                    rpos = FXMAX(targetname.count() - pos, 0);
                    p1 = targetname.offset(rpos);
                    p2 = targetname.offset(rpos + len);
                }
                else
                {
                    p1 = targetname.offset(pos);
                    p2 = targetname.offset(pos + len);
                }

                // Overwrite
                if (insertoption == ID_INSERT_OVW)
                {
                    targetname = targetname.erase(p1, p2 - p1);
                }

                targetname = targetname.insert(p1, text);
            }
        }

        // Append to the list
        if (targetname != sourcename && targetname != "")
        {
            appendItem(sourcepath, targetname);
            target += FXPath::directory(sourcepath) + PATHSEPSTRING + targetname + "\n";
        }
        else
        {
            target += FXString("") + "\n";
            appendItem(sourcepath, "");
        }
    }
    list->sortItems();

    // Deselect all items
    list->killSelection();
    getApp()->endWaitCursor();

    return target;
}


// Changes are accepted
long RenameDialog::onCmdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    DialogBox::onCmdAccept(sender, sel, ptr);
    return 1;
}


// Changes are cancelled
long RenameDialog::onCmdCancel(FXObject* sender, FXSelector sel, void* ptr)
{
    // Empty destination
    target = "";

    DialogBox::onCmdCancel(sender, sel, ptr);
    return 0;
}


// Compare sectioned strings (in natural order)
int RenameDialog::compareSection(const char* p, const char* q, int s)
{
    int x;

    for (x = s; x && *p; x -= (*p++ == '\t'))
    {
    }
    for (x = s; x && *q; x -= (*q++ == '\t'))
    {
    }

    return xf_comparenat((char*)p, (char*)q, false);   // Case sensitive
}


// Sort functions
int RenameDialog::ascendingName(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 0);
}


int RenameDialog::descendingName(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 0);
}


int RenameDialog::ascendingFolderName(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 0);
}


int RenameDialog::descendingFolderName(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 0);
}


int RenameDialog::ascendingNewName(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 1);
}


int RenameDialog::descendingNewName(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 1);
}


// Sort file name list by name
long RenameDialog::onCmdSortByName(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();
    list->setSortFunc((list->getSortFunc() == ascendingName) ? descendingName : ascendingName);
    list->setSortHeader(0);

    // Fill list
    list->clearItems();
    for (FXuint i = 0; i < numfiles; i++)
    {
        FXString sourcepath = source.section('\n', i);
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = FXPath::name(target.section('\n', i));

        // Append item
        appendItem(sourcepath, targetname);
    }
    list->sortItems();
    getApp()->endWaitCursor();

    return 1;
}


// Sort file name list by folder name
long RenameDialog::onCmdSortByFolderName(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();
    list->setSortFunc((list->getSortFunc() == ascendingFolderName) ? descendingFolderName : ascendingFolderName);
    list->setSortHeader(1);

    // Fill list
    list->clearItems();
    for (FXuint i = 0; i < numfiles; i++)
    {
        FXString sourcepath = source.section('\n', i);
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = FXPath::name(target.section('\n', i));

        // Append item
        appendItem(sourcepath, targetname);
    }
    list->sortItems();
    getApp()->endWaitCursor();

    return 1;
}


// Sort file name list by new name
long RenameDialog::onCmdSortByNewName(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();
    list->setSortFunc((list->getSortFunc() == ascendingNewName) ? descendingNewName : ascendingNewName);

    if (showfolder)
    {
        list->setSortHeader(2);
    }
    else
    {
        list->setSortHeader(1);
    }

    // Fill list
    list->clearItems();
    for (FXuint i = 0; i < numfiles; i++)
    {
        FXString sourcepath = source.section('\n', i);
        FXString sourcename = FXPath::name(sourcepath);
        FXString targetname = FXPath::name(target.section('\n', i));

        // Append item
        appendItem(sourcepath, targetname);
    }
    list->sortItems();
    getApp()->endWaitCursor();

    return 1;
}


// Clicked on a file name list header button
long RenameDialog::onCmdHeaderClicked(FXObject*, FXSelector, void* ptr)
{
    FXuint num = (FXuint)(FXuval)ptr;

    if (showfolder)
    {
        if (num < 3)
        {
            if (num == 0)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_SORT_BY_NAME), NULL);
            }
            else if (num == 1)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_SORT_BY_FOLDERNAME), NULL);
            }
            else if (num == 2)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_SORT_BY_NEWNAME), NULL);
            }
        }
    }
    else
    {
        if (num < 2)
        {
            if (num == 0)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_SORT_BY_NAME), NULL);
            }
            else if (num == 1)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_SORT_BY_NEWNAME), NULL);
            }
        }
    }

    return 1;
}


// Update file name list header
long RenameDialog::onUpdHeader(FXObject*, FXSelector, void*)
{
    // Update header arrow
    if (showfolder)
    {
        list->getHeader()->setArrowDir(0,
                                       (list->getSortFunc() == ascendingName) ? false : (list->getSortFunc() ==
                                                                                         descendingName) ? true :
                                       MAYBE);
        list->getHeader()->setArrowDir(1,
                                       (list->getSortFunc() == ascendingFolderName) ? false : (list->getSortFunc() ==
                                                                                               descendingFolderName) ?
                                       true : MAYBE);
        list->getHeader()->setArrowDir(2,
                                       (list->getSortFunc() == ascendingNewName) ? false : (list->getSortFunc() ==
                                                                                            descendingNewName) ? true :
                                       MAYBE);
    }
    else
    {
        list->getHeader()->setArrowDir(0,
                                       (list->getSortFunc() == ascendingName) ? false : (list->getSortFunc() ==
                                                                                         descendingName) ? true :
                                       MAYBE);
        list->getHeader()->setArrowDir(1,
                                       (list->getSortFunc() == ascendingNewName) ? false : (list->getSortFunc() ==
                                                                                            descendingNewName) ? true :
                                       MAYBE);
    }

    // Set minimum header size
    if (list->getHeaderSize(0) < MIN_HEADER_SIZE)
    {
        list->setHeaderSize(0, MIN_HEADER_SIZE);
    }
    if (list->getHeaderSize(1) < MIN_HEADER_SIZE)
    {
        list->setHeaderSize(1, MIN_HEADER_SIZE);
    }
    if (showfolder)
    {
        if (list->getHeaderSize(2) < MIN_HEADER_SIZE)
        {
            list->setHeaderSize(2, MIN_HEADER_SIZE);
        }
    }

    return 1;
}


// Execute dialog box modally
FXuint RenameDialog::execute(FXuint placement)
{
    // Execute dialog
    FXuint ret = DialogBox::execute(placement);

    return ret;
}


// Switch between the rename modes
long RenameDialog::onCmdMode(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_MODE_REPLACE:
        mode = ID_MODE_REPLACE;

        replacegroup->show();
        casegroup->hide();
        insertgroup->hide();
        removegroup->hide();

        updateListReplace();
        searchfld->setFocus();

        break;

    case ID_MODE_CASE:
        mode = ID_MODE_CASE;

        replacegroup->hide();
        casegroup->show();
        insertgroup->hide();
        removegroup->hide();

        updateListCase();

        break;

    case ID_MODE_INSERT:
        mode = ID_MODE_INSERT;

        replacegroup->hide();
        casegroup->hide();
        insertgroup->show();
        removegroup->hide();

        updateListInsert();
        insertfld->setFocus();

        break;

    case ID_MODE_REMOVE:
        mode = ID_MODE_REMOVE;

        replacegroup->hide();
        casegroup->hide();
        insertgroup->hide();
        removegroup->show();

        updateListRemove();

        break;
    }

    return 1;
}


// Switch between name options
long RenameDialog::onCmdNameOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_NAME_ONLY:
        nameoption = ID_NAME_ONLY;
        break;

    case ID_EXT_ONLY:
        nameoption = ID_EXT_ONLY;
        break;

    case ID_WHOLE_NAME:
        nameoption = ID_WHOLE_NAME;
        break;
    }

    if (mode == ID_MODE_REPLACE)
    {
        // Perform replace
        target = doReplace(ignorecase, nameoption, source, numfiles, searchfld->getText(), replacefld->getText());
    }
    else if (mode == ID_MODE_CASE)
    {
        // Perform change case
        target = doChangeCase(nameoption, caseoption, source, numfiles);
    }
    else if (mode == ID_MODE_REMOVE)
    {
        // Perform remove
        target = doRemove(nameoption, removechars, removefrompos, removefromend, source, numfiles);
    }
    else if (mode == ID_MODE_INSERT)
    {
        // Perform insert
        target = doInsert(nameoption, insertfld->getText(), insertfrompos, insertoption, insertfromend, source,
                          numfiles);
    }

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    return 1;
}


// Toggle ignore case
long RenameDialog::onCmdIgnoreCase(FXObject* sender, FXSelector sel, void* ptr)
{
    ignorecase = ignorecasechk->getCheck();

    FXString searchtxt = searchfld->getText();
    FXString replacetxt = replacefld->getText();

    // Perform replace
    target = doReplace(ignorecase, nameoption, source, numfiles, searchtxt, replacetxt);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    return 1;
}


// Switch between case options
long RenameDialog::onCmdCaseOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_LOWER_CASE:
        caseoption = ID_LOWER_CASE;
        break;

    case ID_UPPER_CASE:
        caseoption = ID_UPPER_CASE;
        break;

    case ID_TITLE_CASE:
        caseoption = ID_TITLE_CASE;
        break;

    case ID_FIRST_UPPER_CASE:
        caseoption = ID_FIRST_UPPER_CASE;
        break;
    }

    // Perform change case
    target = doChangeCase(nameoption, caseoption, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    return 1;
}


// Process remove options
long RenameDialog::onCmdRemoveOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    removechars = removecharsspin->getValue();
    removefrompos = removefromposspin->getValue();
    removefromend = removefromendchk->getCheck();

    // Perform remove
    target = doRemove(nameoption, removechars, removefrompos, removefromend, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    return 1;
}


// Process insert options
long RenameDialog::onCmdInsertOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_INSERT_INS:
        insertoption = ID_INSERT_INS;
        break;

    case ID_INSERT_OVW:
        insertoption = ID_INSERT_OVW;
        break;
    }

    // Perform insert
    insertfrompos = insertfromposspin->getValue();
    insertfromend = insertfromendchk->getCheck();
    target = doInsert(nameoption, insertfld->getText(), insertfrompos, insertoption, insertfromend, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));

    return 1;
}


// Update mode radio buttons
long RenameDialog::onUpdMode(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_MODE_REPLACE:
        sender->handle(this, (mode == ID_MODE_REPLACE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_MODE_CASE:
        sender->handle(this, (mode == ID_MODE_CASE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_MODE_INSERT:
        sender->handle(this, (mode == ID_MODE_INSERT ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_MODE_REMOVE:
        sender->handle(this, (mode == ID_MODE_REMOVE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;
    }

    return 1;
}


// Update name radio buttons
long RenameDialog::onUpdNameOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_NAME_ONLY:
        sender->handle(this,
                       (nameoption == ID_NAME_ONLY ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_EXT_ONLY:
        sender->handle(this,
                       (nameoption == ID_EXT_ONLY ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_WHOLE_NAME:
        sender->handle(this,
                       (nameoption == ID_WHOLE_NAME ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;
    }

    return 1;
}


// Update case options radio buttons
long RenameDialog::onUpdCaseOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_LOWER_CASE:
        sender->handle(this,
                       (caseoption == ID_LOWER_CASE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_UPPER_CASE:
        sender->handle(this,
                       (caseoption == ID_UPPER_CASE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_TITLE_CASE:
        sender->handle(this,
                       (caseoption == ID_TITLE_CASE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_FIRST_UPPER_CASE:
        sender->handle(this,
                       (caseoption == ID_FIRST_UPPER_CASE ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND,
                                                                                                 ID_UNCHECK)), NULL);
        break;
    }

    return 1;
}



// Update insert options radio buttons
long RenameDialog::onUpdInsertOptions(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_INSERT_INS:
        sender->handle(this,
                       (insertoption == ID_INSERT_INS ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;

    case ID_INSERT_OVW:
        sender->handle(this,
                       (insertoption == ID_INSERT_OVW ? FXSEL(SEL_COMMAND, ID_CHECK) : FXSEL(SEL_COMMAND, ID_UNCHECK)),
                       NULL);
        break;
    }

    return 1;
}


// Update accept button
long RenameDialog::onUpdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    // Check if target is empty or contains only numfiles '\n' characters
    if (target == "" || target.count() == (int)numfiles)
    {
        acceptbtn->disable();
    }
    else
    {
        acceptbtn->enable();
    }

    return 1;
}


// Force update file list in replace mode
void RenameDialog::updateListReplace(void)
{
    // Perform replace
    target = doReplace(ignorecase, nameoption, source, numfiles, searchfld->getText(), replacefld->getText());

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));
}


// Update file list in change case mode
void RenameDialog::updateListCase(void)
{
    // Perform change case
    target = doChangeCase(nameoption, caseoption, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));
}


// Update file list in remove mode
void RenameDialog::updateListRemove(void)
{
    // Perform remove
    target = doRemove(nameoption, removechars, removefrompos, removefromend, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));
}


// Update file list in insert mode
void RenameDialog::updateListInsert(void)
{
    // Perform insert
    target = doInsert(nameoption, insertfld->getText(), insertfrompos, insertoption, insertfromend, source, numfiles);

    // Update number of renamed items
    itemslbl->setText(getItemsMsg(target, numfiles));
}


// Update file list in replace mode
// This is done when GUI has changed
long RenameDialog::onUpdListReplace(FXObject* sender, FXSelector sel, void* ptr)
{
    if (numfiles <= NUMFILES_TIMEOUT)
    {
        FXString searchtxt = searchfld->getText();
        FXString replacetxt = replacefld->getText();

        if (searchtxt != prev_searchtxt || replacetxt != prev_replacetxt)
        {
            // Perform replace
            target = doReplace(ignorecase, nameoption, source, numfiles, searchtxt, replacetxt);

            // Update number of renamed items
            itemslbl->setText(getItemsMsg(target, numfiles));

            // Update previous strings
            prev_searchtxt = searchtxt;
            prev_replacetxt = replacetxt;
        }
    }

    return 1;
}


// Update file list in insert mode
// This is done when GUI has changed
long RenameDialog::onUpdListInsert(FXObject* sender, FXSelector sel, void* ptr)
{
    if (numfiles <= NUMFILES_TIMEOUT)
    {
        FXString insertstr = insertfld->getText();

        if (insertstr != prev_insertstr)
        {
            // Perform insert
            target = doInsert(nameoption, insertstr, insertfrompos, insertoption, insertfromend, source, numfiles);

            // Update number of renamed items
            itemslbl->setText(getItemsMsg(target, numfiles));

            // Update previous string
            prev_insertstr = insertstr;
        }
    }

    return 1;
}


// Update file list in replace mode when number of files is high
// This is done every time timeout is due
long RenameDialog::onTimeoutListReplace(FXObject* sender, FXSelector sel, void* ptr)
{
    if (numfiles > NUMFILES_TIMEOUT)
    {
        FXString searchtxt = searchfld->getText();
        FXString replacetxt = replacefld->getText();

        if (searchtxt != prev_searchtxt || replacetxt != prev_replacetxt)
        {
            // Perform replace
            target = doReplace(ignorecase, nameoption, source, numfiles, searchtxt, replacetxt);

            // Update number of renamed items
            itemslbl->setText(getItemsMsg(target, numfiles));

            // Update previous strings
            prev_searchtxt = searchtxt;
            prev_replacetxt = replacetxt;
        }

        // Reset timeout
        getApp()->addTimeout(this, ID_SEARCH_TEXT, REFRESH_TIMEOUT);
        getApp()->addTimeout(this, ID_REPLACE_TEXT, REFRESH_TIMEOUT);
    }

    return 1;
}


// Update file list in insert mode
// This is done every time timeout is due
long RenameDialog::onTimeoutListInsert(FXObject* sender, FXSelector sel, void* ptr)
{
    if (numfiles > NUMFILES_TIMEOUT)
    {
        FXString insertstr = insertfld->getText();

        if (insertstr != prev_insertstr)
        {
            // Perform insert
            target = doInsert(nameoption, insertstr, insertfrompos, insertoption, insertfromend, source, numfiles);

            // Update number of renamed items
            itemslbl->setText(getItemsMsg(target, numfiles));

            // Update previous string
            prev_insertstr = insertstr;
        }

        // Reset timeout
        getApp()->addTimeout(this, ID_INSERT_TEXT, REFRESH_TIMEOUT);
    }

    return 1;
}

// Context menu
long RenameDialog::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    int x, y;
    FXuint state;

    list->getCursorPosition(x, y, state);

    int item = list->getItemAt(x, y);

    if (item >= 0)
    {
        list->setCurrentItem(item);
        list->selectItem(item);

        // Menu items
        FXMenuPane* menu = new FXMenuPane(this);

        getRoot()->getCursorPosition(x, y, state);

        new FXMenuCommand(menu, _("Cop&y name"), minicopyicon, this, RenameDialog::ID_COPYNAME_CLIPBOARD);

        menu->create();
        menu->popup(NULL, x, y);
        getApp()->runModalWhileShown(menu);

        delete(menu);
    }
    else
    {
        list->killSelection();
    }

    return 1;
}


// Copy file name (without path) to clipboard
long RenameDialog::onCmdCopyName(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard and set clipboard type
    clipboard.clear();
    clipboard_type = COPYNAME_CLIPBOARD;

    // Get selected item
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            FXString str = list->getItemText(u);
            FXString name = str.section('\t', 0);
            clipboard += FXURL::encode(::xf_filetouri(name));
        }
    }

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


// We now really do have the clipboard, keep clipboard content
long RenameDialog::onClipboardGained(FXObject* sender, FXSelector sel, void* ptr)
{
    DialogBox::onClipboardGained(sender, sel, ptr);
    return 1;
}


// We lost the clipboard, free clipboard content
long RenameDialog::onClipboardLost(FXObject* sender, FXSelector sel, void* ptr)
{
    DialogBox::onClipboardLost(sender, sel, ptr);
    return 1;
}


// Somebody wants our clipboard content
long RenameDialog::onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuchar* data;
    FXuint len;

    // Perhaps the target wants to supply its own data for the clipboard
    if (DialogBox::onClipboardRequest(sender, sel, ptr))
    {
        return 1;
    }

    // Clipboard target is utf8Type (to paste file pathes as text to other applications)
    if (event->target == utf8Type)
    {
        if (!clipboard.empty())
        {
            int beg = 0, end = 0;
            FXString str = "";
            FXString pathname, url;

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
            str = str.mid(0, end);

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


// Keyboard press; handle escape and return to close the dialog
long RenameDialog::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    switch (event->code)
    {
    case KEY_Escape:
        handle(this, FXSEL(SEL_COMMAND, ID_CANCEL), NULL);
        return 1;

    case KEY_KP_Enter:
    case KEY_Return:

        // Check if target is empty or contains only numfiles '\n' characters
        if (target == "" || target.count() == (int)numfiles)
        {
            return 0;
        }
        else
        {
            handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);
            return 1;
        }

    default:
        FXTopWindow::onKeyPress(sender, sel, ptr);
        return 1;
    }

    return 0;
}

// Keyboard release; handle escape to close the dialog
long RenameDialog::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr)
{
    if (((FXEvent*)ptr)->code == KEY_Escape)
    {
        return 1;
    }

    if (FXTopWindow::onKeyRelease(sender, sel, ptr))
    {
        return 1;
    }

    return 0;
}
