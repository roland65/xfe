// Combo Box. Taken from the FOX library and slightly modified.

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "icons.h"
#include "ComboBox.h"


#define COMBOBOX_INS_MASK   (COMBOBOX_REPLACE | COMBOBOX_INSERT_BEFORE | COMBOBOX_INSERT_AFTER | \
                             COMBOBOX_INSERT_FIRST | COMBOBOX_INSERT_LAST)
#define COMBOBOX_MASK       (COMBOBOX_STATIC | COMBOBOX_INS_MASK)


// Map
FXDEFMAP(ComboBox) ComboBoxMap[] =
{
    FXMAPFUNC(SEL_FOCUS_UP, 0, ComboBox::onFocusUp),
    FXMAPFUNC(SEL_FOCUS_DOWN, 0, ComboBox::onFocusDown),
    FXMAPFUNC(SEL_FOCUS_SELF, 0, ComboBox::onFocusSelf),
    FXMAPFUNC(SEL_UPDATE, ComboBox::ID_TEXT, ComboBox::onUpdFmText),
    FXMAPFUNC(SEL_CLICKED, ComboBox::ID_LIST, ComboBox::onListClicked),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_LIST, ComboBox::onListClicked),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS, ComboBox::ID_TEXT, ComboBox::onTextButton),
    FXMAPFUNC(SEL_MOUSEWHEEL, ComboBox::ID_TEXT, ComboBox::onMouseWheel),
    FXMAPFUNC(SEL_CHANGED, ComboBox::ID_TEXT, ComboBox::onTextChanged),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_TEXT, ComboBox::onTextCommand),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETINTVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETREALVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETSTRINGVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETINTVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETREALVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_GETSTRINGVALUE, ComboBox::onFwdToText),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, ComboBox::ID_TEXT, ComboBox::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_COPY_TEXT, ComboBox::onCmdCopyText),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_CUT_TEXT, ComboBox::onCmdCutText),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_PASTE_TEXT, ComboBox::onCmdPasteText),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_PASTE_OPEN, ComboBox::onCmdPasteOpen),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_CLEAR_TEXT, ComboBox::onCmdClearText),
    FXMAPFUNC(SEL_COMMAND, ComboBox::ID_CLEAR_LIST, ComboBox::onCmdClearList),
    FXMAPFUNC(SEL_UPDATE, ComboBox::ID_PASTE_TEXT, ComboBox::onUpdYes),
};


// Object implementation
FXIMPLEMENT(ComboBox, FXPacker, ComboBoxMap, ARRAYNUMBER(ComboBoxMap))


// Construct
ComboBox::ComboBox(FXComposite* p, int cols, FXbool addr, FXbool clrbtn, FXObject* tgt, FXSelector sel, FXuint opts,
                   int x, int y, int w, int h, int pl, int pr, int pt, int pb) :
    FXPacker(p, opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    flags |= FLAG_ENABLED;
    target = tgt;
    message = sel;
    address = addr;
    clearbutton = clrbtn;

    // !!! Hack to set options to TEXTFIELD_NORMAL instead of 0 (used by the Clearlooks theme)
    field = new FXTextField(this, cols, this, ComboBox::ID_TEXT, TEXTFIELD_NORMAL, 0, 0, 0, 0, pl, pr, pt, pb);
    // !!! End of hack

    if (options & COMBOBOX_STATIC)
    {
        field->setEditable(false);
    }
    pane = new FXPopup(this, FRAME_LINE);
    list = new FXList(pane, this, ComboBox::ID_LIST, LIST_BROWSESELECT | LIST_AUTOSELECT | LAYOUT_FILL_X |
                      LAYOUT_FILL_Y | SCROLLERS_TRACK | HSCROLLER_NEVER);

    // If clear list button
    if (clearbutton)
    {
        FXHorizontalFrame* frame = new FXHorizontalFrame(pane, JUSTIFY_LEFT | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X,
                                                         0, 0, 0, 0, 2, 2, 2, 2);
        new FXButton(frame, _("Clear\tReset history list"), minideleteicon, this, ID_CLEAR_LIST);
    }
    
    if (options & COMBOBOX_STATIC)
    {
        list->setScrollStyle(SCROLLERS_TRACK | HSCROLLING_OFF);
    }
    button = new FXMenuButton(this, FXString::null, NULL, pane, FRAME_GROOVE | MENUBUTTON_DOWN |
                              MENUBUTTON_ATTACH_RIGHT, 0, 0, 0, 0, 0, 0, 0, 0);
    button->setXOffset(border);
    button->setYOffset(border);

    flags &= ~FLAG_UPDATE; // Never GUI update
}


// Create window
void ComboBox::create()
{
    FXPacker::create();
    pane->create();
}


// Detach window
void ComboBox::detach()
{
    FXPacker::detach();
    pane->detach();
}


// Destroy window
void ComboBox::destroy()
{
    pane->destroy();
    FXPacker::destroy();
}


// Enable the window
void ComboBox::enable()
{
    if (!isEnabled())
    {
        FXPacker::enable();
        field->enable();
        button->enable();
    }
}


// Disable the window
void ComboBox::disable()
{
    if (isEnabled())
    {
        FXPacker::disable();
        field->disable();
        button->disable();
    }
}


// Get default width
FXint ComboBox::getDefaultWidth()
{
    FXint ww, pw;

    ww = field->getDefaultWidth() + button->getDefaultWidth() + (border << 1);
    pw = pane->getDefaultWidth();

    return FXMAX(ww, pw);
}


// Get default height
FXint ComboBox::getDefaultHeight()
{
    FXint th, bh;

    th = field->getDefaultHeight();
    bh = button->getDefaultHeight();

    return FXMAX(th, bh) + (border << 1);
}


// Recalculate layout
void ComboBox::layout()
{
    FXint buttonWidth, textWidth, itemHeight;

    itemHeight = height - (border << 1);
    buttonWidth = button->getDefaultWidth();
    textWidth = width - buttonWidth - (border << 1);
    field->position(border, border, textWidth, itemHeight);
    button->position(border + textWidth, border, buttonWidth, itemHeight);
    pane->resize(width, pane->getDefaultHeight());
    flags &= ~FLAG_DIRTY;
}


// Forward GUI update of text field to target; but only if pane is not popped
long ComboBox::onUpdFmText(FXObject*, FXSelector, void*)
{
    return target && !isPaneShown() && target->tryHandle(this, FXSEL(SEL_UPDATE, message), NULL);
}


// Command handled in the text field
long ComboBox::onFwdToText(FXObject* sender, FXSelector sel, void* ptr)
{
    return field->handle(sender, sel, ptr);
}


// Forward clicked message from list to target
long ComboBox::onListClicked(FXObject*, FXSelector sel, void* ptr)
{
    button->handle(this, FXSEL(SEL_COMMAND, ID_UNPOST), NULL);
    if (FXSELTYPE(sel) == SEL_COMMAND)
    {
        field->setText(list->getItemText((FXint)(FXival)ptr));
        if (!(options & COMBOBOX_STATIC))
        {
            field->selectAll();    // Select if editable
        }
        if (target)
        {
            target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)getText().text());
        }
    }
    return 1;
}


// Pressed left button in text field
long ComboBox::onTextButton(FXObject*, FXSelector, void*)
{
    if (options & COMBOBOX_STATIC)
    {
        button->handle(this, FXSEL(SEL_COMMAND, ID_POST), NULL); // Post the list
        return 1;
    }
    return 0;
}


// Text has changed
long ComboBox::onTextChanged(FXObject*, FXSelector, void* ptr)
{
    return target && target->tryHandle(this, FXSEL(SEL_CHANGED, message), ptr);
}


// Text has changed
long ComboBox::onTextCommand(FXObject*, FXSelector, void* ptr)
{
    FXint index = list->getCurrentItem();

    if (!(options & COMBOBOX_STATIC))
    {
        switch (options & COMBOBOX_INS_MASK)
        {
        case COMBOBOX_REPLACE:
            if (0 <= index)
            {
                setItem(index, (FXchar*)ptr, getItemData(index));
            }
            break;
        case COMBOBOX_INSERT_BEFORE:
            if (0 <= index)
            {
                insertItem(index, (FXchar*)ptr);
            }
            break;
        case COMBOBOX_INSERT_AFTER:
            if (0 <= index)
            {
                insertItem(index + 1, (FXchar*)ptr);
            }
            break;
        case COMBOBOX_INSERT_FIRST:
            insertItem(0, (FXchar*)ptr);
            break;
        case COMBOBOX_INSERT_LAST:
            appendItem((FXchar*)ptr);
            break;
        }
    }
    return target && target->tryHandle(this, FXSEL(SEL_COMMAND, message), ptr);
}


// Bounce focus to the text field
long ComboBox::onFocusSelf(FXObject* sender, FXSelector, void* ptr)
{
    return field->handle(sender, FXSEL(SEL_FOCUS_SELF, 0), ptr);
}


// Select upper item
long ComboBox::onFocusUp(FXObject*, FXSelector, void*)
{
    if (isEnabled())
    {
        FXint index = getCurrentItem();
        if (index < 0)
        {
            index = getNumItems() - 1;
        }
        else if (0 < index)
        {
            index--;
        }
        if (0 <= index && index < getNumItems())
        {
            setCurrentItem(index, TRUE);
        }
        return 1;
    }
    return 0;
}


// Select lower item
long ComboBox::onFocusDown(FXObject*, FXSelector, void*)
{
    if (isEnabled())
    {
        FXint index = getCurrentItem();
        if (index < 0)
        {
            index = 0;
        }
        else if (index < getNumItems() - 1)
        {
            index++;
        }
        if (0 <= index && index < getNumItems())
        {
            setCurrentItem(index, TRUE);
        }
        return 1;
    }
    return 0;
}


// Mouse wheel
long ComboBox::onMouseWheel(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    if (isEnabled())
    {
        FXint index = getCurrentItem();
        if (event->code < 0)
        {
            if (index < 0)
            {
                index = 0;
            }
            else if (index < getNumItems() - 1)
            {
                index++;
            }
        }
        else if (event->code > 0)
        {
            if (index < 0)
            {
                index = getNumItems() - 1;
            }
            else if (0 < index)
            {
                index--;
            }
        }
        if (0 <= index && index < getNumItems())
        {
            setCurrentItem(index, TRUE);
        }
        return 1;
    }
    return 0;
}


// Return true if editable
FXbool ComboBox::isEditable() const
{
    return field->isEditable();
}


// Set widget is editable or not
void ComboBox::setEditable(FXbool edit)
{
    field->setEditable(edit);
}


// Set text
void ComboBox::setText(const FXString& text)
{
    field->setText(text);
}


// Obtain text
FXString ComboBox::getText() const
{
    return field->getText();
}


// Set number of text columns
void ComboBox::setNumColumns(FXint cols)
{
    field->setNumColumns(cols);
}


// Get number of text columns
FXint ComboBox::getNumColumns() const
{
    return field->getNumColumns();
}


// Get number of items
FXint ComboBox::getNumItems() const
{
    return list->getNumItems();
}


// Get number of visible items
FXint ComboBox::getNumVisible() const
{
    return list->getNumVisible();
}


// Set number of visible items
void ComboBox::setNumVisible(FXint nvis)
{
    list->setNumVisible(nvis);
}


// Is item current
FXbool ComboBox::isItemCurrent(FXint index) const
{
    return list->isItemCurrent(index);
}


// Change current item
void ComboBox::setCurrentItem(FXint index, FXbool notify)
{
    FXint current = list->getCurrentItem();

    if (current != index)
    {
        list->setCurrentItem(index);
        list->makeItemVisible(index);
        if (0 <= index)
        {
            setText(list->getItemText(index));
        }
        else
        {
            setText(FXString::null);
        }
        if (notify && target)
        {
            target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)getText().text());
        }
    }
}


// Get current item
FXint ComboBox::getCurrentItem() const
{
    return list->getCurrentItem();
}


// Retrieve item
FXString ComboBox::getItem(FXint index) const
{
    return list->getItem(index)->getText();
}


// Replace text of item at index
FXint ComboBox::setItem(FXint index, const FXString& text, void* ptr)
{
    if (index < 0 || list->getNumItems() <= index)
    {
        fxerror("%s::setItem: index out of range.\n", getClassName());
    }
    list->setItem(index, text, NULL, ptr);
    if (isItemCurrent(index))
    {
        field->setText(text);
    }
    recalc();
    return index;
}


// Fill list by appending items from array of strings
FXint ComboBox::fillItems(const FXchar** strings)
{
    FXint numberofitems = list->getNumItems();
    FXint n = list->fillItems(strings);

    if (numberofitems <= list->getCurrentItem())
    {
        field->setText(list->getItemText(list->getCurrentItem()));
    }
    recalc();
    return n;
}


// Fill list by appending items from newline separated strings
FXint ComboBox::fillItems(const FXString& strings)
{
    FXint numberofitems = list->getNumItems();
    FXint n = list->fillItems(strings);

    if (numberofitems <= list->getCurrentItem())
    {
        field->setText(list->getItemText(list->getCurrentItem()));
    }
    recalc();
    return n;
}


// Insert item at index
FXint ComboBox::insertItem(FXint index, const FXString& text, void* ptr)
{
    if (index < 0 || list->getNumItems() < index)
    {
        fxerror("%s::insertItem: index out of range.\n", getClassName());
    }
    list->insertItem(index, text, NULL, ptr);
    if (isItemCurrent(index))
    {
        field->setText(text);
    }
    recalc();
    return index;
}


// Append item
FXint ComboBox::appendItem(const FXString& text, void* ptr)
{
    FXint index = list->appendItem(text, NULL, ptr);

    if (isItemCurrent(getNumItems() - 1))
    {
        field->setText(text);
    }
    recalc();
    return index;
}


// Prepend item
FXint ComboBox::prependItem(const FXString& text, void* ptr)
{
    FXint index = list->prependItem(text, NULL, ptr);

    if (isItemCurrent(0))
    {
        field->setText(text);
    }
    recalc();
    return index;
}


// Move item from oldindex to newindex
FXint ComboBox::moveItem(FXint newindex, FXint oldindex)
{
    if (newindex < 0 || list->getNumItems() <= newindex || oldindex < 0 || list->getNumItems() <= oldindex)
    {
        fxerror("%s::moveItem: index out of range.\n", getClassName());
    }
    FXint current = list->getCurrentItem();
    list->moveItem(newindex, oldindex);
    if (current != list->getCurrentItem())
    {
        current = list->getCurrentItem();
        if (0 <= current)
        {
            field->setText(list->getItemText(current));
        }
        else
        {
            field->setText(" ");
        }
    }
    recalc();
    return newindex;
}


// Remove given item
void ComboBox::removeItem(FXint index)
{
    FXint current = list->getCurrentItem();

    list->removeItem(index);
    if (index == current)
    {
        current = list->getCurrentItem();
        if (0 <= current)
        {
            field->setText(list->getItemText(current));
        }
        else
        {
            field->setText(FXString::null);
        }
    }
    recalc();
}


// Remove all items
void ComboBox::clearItems()
{
    field->setText(FXString::null);
    list->clearItems();
    recalc();
}


// Get item by name
FXint ComboBox::findItem(const FXString& text, FXint start, FXuint flgs) const
{
    return list->findItem(text, start, flgs);
}


// Get item by data
FXint ComboBox::findItemByData(const void* ptr, FXint start, FXuint flgs) const
{
    return list->findItemByData(ptr, start, flgs);
}


// Set item text
void ComboBox::setItemText(FXint index, const FXString& txt)
{
    if (isItemCurrent(index))
    {
        setText(txt);
    }
    list->setItemText(index, txt);
    recalc();
}


// Get item text
FXString ComboBox::getItemText(FXint index) const
{
    return list->getItemText(index);
}


// Set item data
void ComboBox::setItemData(FXint index, void* ptr) const
{
    list->setItemData(index, ptr);
}


// Get item data
void* ComboBox::getItemData(FXint index) const
{
    return list->getItemData(index);
}


// Select text
void ComboBox::selectAll(void)
{
    field->selectAll();
}


// Is the pane shown
FXbool ComboBox::isPaneShown() const
{
    return pane->shown();
}


// Set font
void ComboBox::setFont(FXFont* fnt)
{
    if (!fnt)
    {
        fxerror("%s::setFont: NULL font specified.\n", getClassName());
    }
    field->setFont(fnt);
    list->setFont(fnt);
    recalc();
}


// Obtain font
FXFont* ComboBox::getFont() const
{
    return field->getFont();
}


// Change combobox style
void ComboBox::setComboStyle(FXuint mode)
{
    FXuint opts = (options & ~COMBOBOX_MASK) | (mode & COMBOBOX_MASK);

    if (opts != options)
    {
        options = opts;
        if (options & COMBOBOX_STATIC)
        {
            field->setEditable(FALSE);                                  // Non-editable
            list->setScrollStyle(SCROLLERS_TRACK | HSCROLLING_OFF);     // No scrolling
        }
        else
        {
            field->setEditable(TRUE);                                           // Editable
            list->setScrollStyle(SCROLLERS_TRACK | HSCROLLER_NEVER);            // Scrollable, but no scrollbar
        }
        recalc();
    }
}


// Get combobox style
FXuint ComboBox::getComboStyle() const
{
    return options & COMBOBOX_MASK;
}


// Set text justify style
void ComboBox::setJustify(FXuint style)
{
    field->setJustify(style);
}


// Get text justify style
FXuint ComboBox::getJustify() const
{
    return field->getJustify();
}


// Set window background color
void ComboBox::setBackColor(FXColor clr)
{
    field->setBackColor(clr);
    list->setBackColor(clr);
}


// Get background color
FXColor ComboBox::getBackColor() const
{
    return field->getBackColor();
}


// Set text color
void ComboBox::setTextColor(FXColor clr)
{
    field->setTextColor(clr);
    list->setTextColor(clr);
}


// Return text color
FXColor ComboBox::getTextColor() const
{
    return field->getTextColor();
}


// Set select background color
void ComboBox::setSelBackColor(FXColor clr)
{
    field->setSelBackColor(clr);
    list->setSelBackColor(clr);
}


// Return selected background color
FXColor ComboBox::getSelBackColor() const
{
    return field->getSelBackColor();
}


// Set selected text color
void ComboBox::setSelTextColor(FXColor clr)
{
    field->setSelTextColor(clr);
    list->setSelTextColor(clr);
}


// Return selected text color
FXColor ComboBox::getSelTextColor() const
{
    return field->getSelTextColor();
}


// Sort items using current sort function
void ComboBox::sortItems()
{
    list->sortItems();
}


// Return sort function
FXListSortFunc ComboBox::getSortFunc() const
{
    return list->getSortFunc();
}


// Change sort function
void ComboBox::setSortFunc(FXListSortFunc func)
{
    list->setSortFunc(func);
}


// Set help text
void ComboBox::setHelpText(const FXString& txt)
{
    field->setHelpText(txt);
}


// Get help text
const FXString& ComboBox::getHelpText() const
{
    return field->getHelpText();
}


// Set tip text
void ComboBox::setTipText(const FXString& txt)
{
    field->setTipText(txt);
}


// Get tip text
const FXString& ComboBox::getTipText() const
{
    return field->getTipText();
}


// Save object to stream
void ComboBox::save(FXStream& store) const
{
    FXPacker::save(store);
    store << field;
    store << button;
    store << list;
    store << pane;
}


// Load object from stream
void ComboBox::load(FXStream& store)
{
    FXPacker::load(store);
    store >> field;
    store >> button;
    store >> list;
    store >> pane;
}


// Delete it
ComboBox::~ComboBox()
{
    delete pane;
    pane = (FXPopup*)-1L;
    field = (FXTextField*)-1L;
    button = (FXMenuButton*)-1L;
    list = (FXList*)-1L;
}


// Popup menu on text field
long ComboBox::onCmdPopupMenu(FXObject*, FXSelector, void*)
{
    int x, y;
    FXuint state;

    list->getCursorPosition(x, y, state);

    // Right mouse popup
    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu, _("&Copy"), minicopyicon, this, ID_COPY_TEXT);
    new FXMenuCommand(popupmenu, _("C&ut"), minicuticon, this, ID_CUT_TEXT);
    new FXMenuCommand(popupmenu, _("&Paste"), minipasteicon, this, ID_PASTE_TEXT);
    
    // For address bar, add a menu to paste and open directory
    if (address)
    {
        new FXMenuCommand(popupmenu, _("Paste and &Open"), minigotodiricon, this, ID_PASTE_OPEN);        
    }
    new FXMenuCommand(popupmenu, _("C&lear"), minicloseicon, this, ID_CLEAR_TEXT);

    getRoot()->getCursorPosition(x, y, state);

    popupmenu->create();
    popupmenu->popup(NULL, x, y);

    return 1;
}


// Copy text
long ComboBox::onCmdCopyText(FXObject*, FXSelector, void*)
{
    field->selectAll();
    field->handle(this, FXSEL(SEL_COMMAND, FXTextField::ID_COPY_SEL), NULL);

    return 1;
}


// Cut text
long ComboBox::onCmdCutText(FXObject*, FXSelector, void*)
{
    field->selectAll();
    field->handle(this, FXSEL(SEL_COMMAND, FXTextField::ID_CUT_SEL), NULL);

    return 1;
}


// Paste text
long ComboBox::onCmdPasteText(FXObject*, FXSelector, void*)
{
    field->selectAll();
    field->handle(this, FXSEL(SEL_COMMAND, FXTextField::ID_PASTE_SEL), NULL);

    return 1;
}


// Paste and open directory
long ComboBox::onCmdPasteOpen(FXObject*, FXSelector, void*)
{
    field->selectAll();
    field->handle(this, FXSEL(SEL_COMMAND, FXTextField::ID_PASTE_SEL), NULL);
    
    // Send return key to FXTextField object
    FXEvent event;
    event.code = KEY_Return;   
    field->handle(this, FXSEL(SEL_KEYPRESS, 0), &event);

    return 1;
}


// Clear text
long ComboBox::onCmdClearText(FXObject*, FXSelector, void*)
{
    field->selectAll();
    field->handle(this, FXSEL(SEL_COMMAND, FXTextField::ID_DELETE_ALL), NULL);

    return 1;
}


// Clear list
long ComboBox::onCmdClearList(FXObject*, FXSelector, void*)
{
    clearItems();
    return 1;
}
