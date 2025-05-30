// Simple input dialog (without history)

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "icons.h"
#include "FileDialog.h"
#include "BookmarkDialog.h"


// Global variables
extern FXString execpath;


FXDEFMAP(BookmarkDialog) BookmarkDialogMap[] =
{
    FXMAPFUNC(SEL_KEYPRESS, 0, BookmarkDialog::onCmdKeyPress),
    FXMAPFUNC(SEL_COMMAND, BookmarkDialog::ID_ACCEPT, BookmarkDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, BookmarkDialog::ID_CANCEL, BookmarkDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND, BookmarkDialog::ID_BROWSE_ICON, BookmarkDialog::onCmdBrowseIcon),
};

// Object implementation
FXIMPLEMENT(BookmarkDialog, DialogBox, BookmarkDialogMap, ARRAYNUMBER(BookmarkDialogMap))

// Construct a dialog box
BookmarkDialog::BookmarkDialog(FXWindow* win, FXString inp, FXString message, FXString title, FXString label1,
                               FXString label2,
                               FXIcon* icon, FXString optiontext) :
    DialogBox(win, title, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MAXIMIZE | DECOR_CLOSE)
{
    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, PACK_UNIFORM_WIDTH | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0,
                                                       0, 0, 0, 10, 10, 5, 5);

    // Accept
    new FXButton(buttons, _("&Accept"), NULL, this, ID_ACCEPT, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Cancel
    new FXButton(buttons, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Vertical frame
    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Icon and message line
    FXMatrix* matrix = new FXMatrix(contents, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix, "", icon, LAYOUT_LEFT);
    msg = new FXLabel(matrix, "", NULL, JUSTIFY_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    msg->setText(message);
    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);  // Fill matrix

    // Label and name input field
    new FXLabel(matrix, label1, NULL, LAYOUT_RIGHT | LAYOUT_CENTER_Y | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    nameinp = new FXTextField(matrix, 40, 0, 0,
                              LAYOUT_CENTER_Y | LAYOUT_CENTER_X | TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN |
                              LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    nameinp->setText(inp);
    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);  // Fill matrix

    // Label, icon path input field and file dialog button
    new FXLabel(matrix, label2, NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_CENTER_Y);
    iconinp = new FXTextField(matrix, 40, 0, 0,
                              LAYOUT_CENTER_Y | LAYOUT_CENTER_X | TEXTFIELD_NORMAL | LAYOUT_FILL_COLUMN |
                              LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    iconinp->setText("minibookmark.png");
    iconbtn = new FXButton(matrix, _("\tSelect Icon..."), minifiledialogicon, this, ID_BROWSE_ICON,
                           FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_FILL_X | LAYOUT_CENTER_Y, 0, 0, 0, 0, 10, 10);

    // Fractional scaling factor
    FXint res = getApp()->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    scalefrac = FXMAX(1.0, res / 100.0);
}


void BookmarkDialog::create()
{
    DialogBox::create();
    nameinp->setFocus();
}


long BookmarkDialog::onCmdKeyPress(FXObject* sender, FXSelector sel, void* ptr)
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



// Execute dialog box modally
FXuint BookmarkDialog::execute(FXuint placement)
{
    // Save name and icon path name
    prev_name = nameinp->getText();
    prev_iconpathname = iconinp->getText();

    // Load icon
    if (xf_existfile(prev_iconpathname))
    {
        FXIcon* icon = xf_loadiconfile(getApp(), FXPath::directory(prev_iconpathname), FXPath::name(prev_iconpathname),
                                    scalefrac, getApp()->getBaseColor());
        iconbtn->setIcon(icon);
    }
    else
    {
        iconbtn->setIcon(minifiledialogicon);  // Default icon
    }

    // Execute dialog
    FXuint ret = DialogBox::execute(placement);

    return ret;
}



// Changes are cancelled
long BookmarkDialog::onCmdCancel(FXObject* sender, FXSelector sel, void* ptr)
{
    // Restore name and icon path name
    nameinp->setText(prev_name);
    iconinp->setText(prev_iconpathname);

    DialogBox::onCmdCancel(sender, sel, ptr);
    return 0;
}


// Changes are accepted
long BookmarkDialog::onCmdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    DialogBox::onCmdAccept(sender, sel, ptr);
    return 1;
}


long BookmarkDialog::onCmdBrowseIcon(FXObject* sender, FXSelector sel, void* ptr)
{
    FXString defaulticonpath = xf_realpath(FXPath::directory(execpath) + "/../share/xfe/icons/default-theme");
    FXString iconpath = xf_realpath(getApp()->reg().readStringEntry("SETTINGS", "iconpath", defaulticonpath.text()));
    const char* patterns[] =
    {
        _("PNG Images"), "*.png",
        _("GIF Images"), "*.gif",
        _("BMP Images"), "*.bmp",
        _("All Files"), "*", NULL
    };
    FileDialog browseIcon(this, _("Select an Icon File"));

    if (iconinp->getText() != "minibookmark.png")
    {
        browseIcon.setFilename(iconpath + PATHSEPSTRING + iconinp->getText());
    }
    else
    {
        browseIcon.setFilename(iconpath + PATHSEPSTRING);
    }

    browseIcon.setPatternList(patterns);
    browseIcon.setSelectMode(SELECT_FILE_EXISTING);

    // Save thumbnails state and force show
    FXbool showthumb = browseIcon.shownThumbnails();
    browseIcon.showThumbnails(true);

    if (browseIcon.execute())
    {
        FXString path;

        path = browseIcon.getFilename();
        if (!xf_existfile(path))
        {
            return 0;
        }

        // Load icon
        iconinp->setText(path);
        FXIcon* icon = xf_loadiconfile(getApp(), path, iconinp->getText(), scalefrac, getApp()->getBaseColor());
        iconbtn->setIcon(icon);
    }

    // Restore image thumbnail state
    browseIcon.showThumbnails(showthumb); // Show image thumbnails

    return 1;
}
