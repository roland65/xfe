// Input dialog with history list and an optional check box

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "FileDialog.h"
#include "HistInputDialog.h"


// Global variables
extern FXString homedir;


// Object implementation
FXIMPLEMENT(HistComboBox, ComboBox, NULL, 0)

HistComboBox::HistComboBox(FXComposite* p, int cols, FXbool addr, FXbool clrbtn, FXObject* tgt, FXSelector sel, FXuint opts) :
    ComboBox(p, cols, addr, clrbtn, tgt, sel, opts)
{
}


void HistComboBox::create()
{
    ComboBox::create();
    setFocus();
}


FXDEFMAP(HistInputDialog) HistInputDialogMap[] =
{
    FXMAPFUNC(SEL_KEYPRESS, 0, HistInputDialog::onCmdKeyPress),
    FXMAPFUNC(SEL_COMMAND, HistInputDialog::ID_BROWSE_PATH, HistInputDialog::onCmdBrowsePath),
    FXMAPFUNC(SEL_UPDATE, HistInputDialog::ID_ACCEPT, HistInputDialog::onUpdAccept),
};


// Object implementation
FXIMPLEMENT(HistInputDialog, DialogBox, HistInputDialogMap, ARRAYNUMBER(HistInputDialogMap))

// Construct a dialog box with an optional check box
HistInputDialog::HistInputDialog(FXWindow* w, FXString inp, FXString message, FXString title, FXString label,
                                 FXIcon* ic, FXbool addr, FXbool clrbtn, FXuint browse, FXbool option, FXString optiontext) :
    DialogBox(w, title, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MAXIMIZE | DECOR_CLOSE)
{
    // Browse type flag
    browsetype = browse;

    // Buttons
    buttons = new FXHorizontalFrame(this, PACK_UNIFORM_WIDTH | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10,
                                    5, 5);

    // Accept
    accept = new FXButton(buttons, _("&Accept"), NULL, this, ID_ACCEPT, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Cancel
    cancel = new FXButton(buttons, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Optional check box
    checkbutton = new FXHorizontalFrame(this, JUSTIFY_RIGHT | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10, 0,
                                        0);

    if (option)
    {
        new FXCheckButton(checkbutton, optiontext + FXString(" "), this, ID_TOGGLE_OPTION);
    }

    // Vertical frame
    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Icon and first line
    FXMatrix* matrix1 = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix1, "", ic, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    new FXLabel(matrix1, message, NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW | FRAME_NONE);

    // Label and input field (combo box)
    FXMatrix* matrix2 = new FXMatrix(contents, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix2, label, NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    input = new HistComboBox(matrix2, 40, addr, clrbtn, NULL, 0, COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X |
                             LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    input->setNumVisible(8);
    input->setText(inp);
    new FXButton(matrix2, _("\tSelect Destination..."), minifiledialogicon, this, ID_BROWSE_PATH, FRAME_GROOVE |
                 LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    if (!xf_isutf8(message.text(), message.length()))
    {
        new FXLabel(contents, _("=> Warning: file name is not UTF-8 encoded!"), NULL, LAYOUT_LEFT |
                    LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    }

    // Initial directory for browsing
    initialdir = homedir;
}


void HistInputDialog::create()
{
    DialogBox::create();
    input->setFocus();
}


void HistInputDialog::CursorEnd()
{
    input->CursorEnd();
    input->setFocus();
}


void HistInputDialog::selectAll()
{
    input->onFwdToText(this, FXSEL(SEL_FOCUSIN, 0), NULL);
    input->onFwdToText(this, FXSEL(SEL_COMMAND, FXTextField::ID_SELECT_ALL), NULL);
}


long HistInputDialog::onCmdKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    switch (event->code)
    {
    case KEY_Escape:
        handle(this, FXSEL(SEL_COMMAND, ID_CANCEL), NULL);
        return 1;

    case KEY_KP_Enter:
    case KEY_Return:
        handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);
        return 1;

    default:
        FXTopWindow::onKeyPress(sender, sel, ptr);
        return 1;
    }
    return 0;
}


long HistInputDialog::onCmdBrowsePath(FXObject* sender, FXSelector sel, void* ptr)
{
    FXString title;

    if (browsetype == HIST_INPUT_FOLDER)
    {
        title = _("Select a Destination Folder");
    }
    else if (browsetype == HIST_INPUT_FILE)
    {
        title = _("Select a File");
    }
    else if (browsetype == HIST_INPUT_EXECUTABLE_FILE)
    {
        title = _("Select an Executable File");
    }
    else
    {
        title = _("Select a File or a Destination Folder");
    }

    // File dialog
    FileDialog browse(this, title);

    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };
    browse.setDirectory(initialdir);
    browse.setPatternList(patterns);

    // Browse files in directory, or existing, or mixed mode depending on the flag
    if (browsetype == HIST_INPUT_FOLDER)
    {
        browse.setSelectMode(SELECT_FILE_DIRECTORY);
    }
    else if (browsetype == HIST_INPUT_FILE)
    {
        browse.setSelectMode(SELECT_FILE_EXISTING);
    }
    else if (browsetype == HIST_INPUT_EXECUTABLE_FILE)
    {
        browse.setSelectMode(SELECT_FILE_EXISTING);
    }
    else
    {
        browse.setSelectMode(SELECT_FILE_MIXED);
    }
    if (browse.execute())
    {
        FXString path = browse.getFilename();
        input->setText(path);
    }

    return 1;
}


// Set initial directory
void HistInputDialog::setDirectory(const FXString& path)
{
    initialdir = path;
}


// Update accept button
long HistInputDialog::onUpdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    if (input->getText() == "")
    {
        accept->disable();
    }
    else
    {
        accept->enable();
    }
    
    return 1;    
}
