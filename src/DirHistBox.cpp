// Display a history list box and allows the user to select a string
// This is based on FXChoiceBox

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "xfedefs.h"
#include "DirHistBox.h"


#define VISIBLE_LINES    10


// Map
FXDEFMAP(DirHistBox) DirHistBoxMap[] =
{
    FXMAPFUNC(SEL_KEYPRESS, 0, DirHistBox::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, DirHistBox::onKeyRelease),
    FXMAPFUNC(SEL_FOCUSOUT, 0, DirHistBox::onCmdClose),
    FXMAPFUNC(SEL_COMMAND, DirHistBox::ID_CLOSE, DirHistBox::onCmdClose),
    FXMAPFUNC(SEL_CLICKED, DirHistBox::ID_LIST, DirHistBox::onCmdClicked),
    FXMAPFUNC(SEL_QUERY_TIP, DirHistBox::ID_LIST, DirHistBox::onQueryTip),
};


// Object implementation
FXIMPLEMENT(DirHistBox, DialogBox, DirHistBoxMap, ARRAYNUMBER(DirHistBoxMap))


// Construct list box with given caption, icon, message text, and with choices from array of strings
DirHistBox::DirHistBox(FXWindow* owner, const char** choices, FXuint opts, int x, int y, int w, int h) :
    DialogBox(owner, "", opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    int n;
    FXHorizontalFrame* hor = new FXHorizontalFrame(this, FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X |
                                                   LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    list = new FXList(hor, this, ID_LIST, LIST_BROWSESELECT | LAYOUT_FILL_Y | LAYOUT_FILL_X | HSCROLLING_OFF);
    list->setBackColor(this->getBackColor());
    n = list->fillItems(choices);
    list->setNumVisible(FXMIN(n, VISIBLE_LINES));
}


// Construct list box with given caption, icon, message text, and with choices from newline separated strings
DirHistBox::DirHistBox(FXWindow* owner, const FXString& choices, FXuint opts, int x, int y, int w, int h) :
    DialogBox(owner, "", opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    int n;
    FXHorizontalFrame* hor = new FXHorizontalFrame(this, FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    list = new FXList(hor, this, ID_LIST, LIST_BROWSESELECT | LAYOUT_FILL_Y | LAYOUT_FILL_X | HSCROLLING_OFF);
    list->setBackColor(this->getBackColor());
    n = list->fillItems(choices);
    list->setNumVisible(FXMIN(n, VISIBLE_LINES));
}


// Select item when click in list
long DirHistBox::onCmdClicked(FXObject*, FXSelector, void*)
{
    // Set item current except when called from up/down key event
    if (!updownkey)
    {
        getApp()->stopModal(this, list->getCurrentItem());
        hide();
    }
    return 1;
}


// Close dialog
long DirHistBox::onCmdClose(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this, -1);
    hide();
    return 1;
}


// Destroy list box
DirHistBox::~DirHistBox()
{
    list = (FXList*)-1L;
}


// Show a modal list dialog
int DirHistBox::box(FXWindow* owner, FXuint opts, const char** choices, int x, int y, int w, int h)
{
    DirHistBox box(owner, choices, opts, x, y, w, h);

    return box.execute(PLACEMENT_DEFAULT);
}


// Show a modal list dialog
int DirHistBox::box(FXWindow* owner, FXuint opts, const FXString& choices, int x, int y, int w, int h)
{
    DirHistBox box(owner, choices, opts, x, y, w, h);

    return box.execute(PLACEMENT_DEFAULT);
}


// Keyboard press
// Handle escape to close the dialog, up/down key to navigate
// and return key to enter directory
long DirHistBox::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    switch (event->code)
    {
    case KEY_Escape:
        handle(this, FXSEL(SEL_COMMAND, ID_CLOSE), NULL);
        return 1;

    case KEY_Up:
    case KEY_Down:

        // Set up/down key event flag
        updownkey = true;

        list->onKeyPress(sender, sel, ptr);
        return 1;

    case KEY_Return:
    case KEY_KP_Enter:
    case KEY_space:
        handle(this, FXSEL(SEL_CLICKED, ID_LIST), NULL);
        return 1;

    default:
        FXTopWindow::onKeyPress(sender, sel, ptr);
        return 1;
    }

    return 0;
}


// Keyboard release; handle escape to close the dialog
long DirHistBox::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr)
{
    // Reset up/down key event flag
    updownkey = false;

    if (FXTopWindow::onKeyRelease(sender, sel, ptr))
    {
        return 1;
    }
    if (((FXEvent*)ptr)->code == KEY_Escape)
    {
        return 1;
    }

    return 0;
}


// Display tooltip
long DirHistBox::onQueryTip(FXObject* sender, FXSelector, void* ptr)
{
    int x, y;
    FXuint state;

    list->getCursorPosition(x, y, state);
    int index = list->getItemAt(x, y);

    if (index != -1)
    {
        FXString tip = "";  // No tooltip
        sender->handle(list, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

        return 1;
    }

    return 0;
}
