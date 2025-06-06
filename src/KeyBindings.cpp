// Dialog used to modify key bindings

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "icons.h"
#include "xfeutils.h"
#include "InputDialog.h"
#include "MessageBox.h"
#include "KeyBindingsDialog.h"
#include "XFileExplorer.h"
#include "KeyBindings.h"


// Minimum header size for lists
#ifndef MIN_HEADER_SIZE
#define MIN_HEADER_SIZE    50
#endif

// Main window
extern FXMainWindow* mainWindow;


FXDEFMAP(KeyBindingsBox) KeyBindingsBoxMap[] =
{
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_ACCEPT, KeyBindingsBox::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_CANCEL, KeyBindingsBox::onCmdCancel),
    FXMAPFUNC(SEL_DOUBLECLICKED, KeyBindingsBox::ID_GLB_BINDINGS_LIST, KeyBindingsBox::onCmdDefineGlbKeyBindings),
    FXMAPFUNC(SEL_DOUBLECLICKED, KeyBindingsBox::ID_XFE_BINDINGS_LIST, KeyBindingsBox::onCmdDefineXfeKeyBindings),
    FXMAPFUNC(SEL_DOUBLECLICKED, KeyBindingsBox::ID_XFI_BINDINGS_LIST, KeyBindingsBox::onCmdDefineXfiKeyBindings),
    FXMAPFUNC(SEL_DOUBLECLICKED, KeyBindingsBox::ID_XFW_BINDINGS_LIST, KeyBindingsBox::onCmdDefineXfwKeyBindings),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_GLB_SORT_BY_ACTIONNAME, KeyBindingsBox::onCmdGlbSortByActionName),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_GLB_SORT_BY_REGISTRYKEY, KeyBindingsBox::onCmdGlbSortByRegistryKey),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_GLB_SORT_BY_KEYBINDING, KeyBindingsBox::onCmdGlbSortByKeyBinding),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFE_SORT_BY_ACTIONNAME, KeyBindingsBox::onCmdXfeSortByActionName),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFE_SORT_BY_REGISTRYKEY, KeyBindingsBox::onCmdXfeSortByRegistryKey),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFE_SORT_BY_KEYBINDING, KeyBindingsBox::onCmdXfeSortByKeyBinding),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFI_SORT_BY_ACTIONNAME, KeyBindingsBox::onCmdXfiSortByActionName),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFI_SORT_BY_REGISTRYKEY, KeyBindingsBox::onCmdXfiSortByRegistryKey),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFI_SORT_BY_KEYBINDING, KeyBindingsBox::onCmdXfiSortByKeyBinding),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFW_SORT_BY_ACTIONNAME, KeyBindingsBox::onCmdXfwSortByActionName),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFW_SORT_BY_REGISTRYKEY, KeyBindingsBox::onCmdXfwSortByRegistryKey),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFW_SORT_BY_KEYBINDING, KeyBindingsBox::onCmdXfwSortByKeyBinding),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_GLB_BINDINGS_LIST, KeyBindingsBox::onCmdGlbHeaderClicked),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFE_BINDINGS_LIST, KeyBindingsBox::onCmdXfeHeaderClicked),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFI_BINDINGS_LIST, KeyBindingsBox::onCmdXfiHeaderClicked),
    FXMAPFUNC(SEL_COMMAND, KeyBindingsBox::ID_XFW_BINDINGS_LIST, KeyBindingsBox::onCmdXfwHeaderClicked),
    FXMAPFUNC(SEL_UPDATE, KeyBindingsBox::ID_GLB_BINDINGS_LIST, KeyBindingsBox::onUpdGlbHeader),
    FXMAPFUNC(SEL_UPDATE, KeyBindingsBox::ID_XFE_BINDINGS_LIST, KeyBindingsBox::onUpdXfeHeader),
    FXMAPFUNC(SEL_UPDATE, KeyBindingsBox::ID_XFI_BINDINGS_LIST, KeyBindingsBox::onUpdXfiHeader),
    FXMAPFUNC(SEL_UPDATE, KeyBindingsBox::ID_XFW_BINDINGS_LIST, KeyBindingsBox::onUpdXfwHeader),
};


// Object implementation
FXIMPLEMENT(KeyBindingsBox, DialogBox, KeyBindingsBoxMap, ARRAYNUMBER(KeyBindingsBoxMap))

// Construct
KeyBindingsBox::KeyBindingsBox(FXWindow* win, FXStringDict* glbbindings, FXStringDict* xfebindings,
                               FXStringDict* xfibindings, FXStringDict* xfwbindings) :
    DialogBox(win, _("Key Bindings"), DECOR_TITLE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE, 0, 0, 800, 600)
{
    glbBindingsDict = glbbindings;
    xfeBindingsDict = xfebindings;
    xfiBindingsDict = xfibindings;
    xfwBindingsDict = xfwbindings;

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10, 5,
                                                       5);

    // Contents
    FXHorizontalFrame* contents = new FXHorizontalFrame(this,
                                                        LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y |
                                                        PACK_UNIFORM_WIDTH);

    // Accept button
    FXButton* ok = new FXButton(buttons, _("&Accept"), NULL, this, KeyBindingsBox::ID_ACCEPT,
                                FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    ok->addHotKey(KEY_Return);

    // Cancel button
    new FXButton(buttons, _("&Cancel"), NULL, this, KeyBindingsBox::ID_CANCEL,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

    // Tab book
    FXTabBook* tabbook = new FXTabBook(contents, NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_RIGHT);

    // First tab is global key bindings
    new FXTabItem(tabbook, _("&Global Key Bindings"), NULL);
    FXVerticalFrame* frame1 = new FXVerticalFrame(tabbook, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                                  0, 0, 0, 0, 0, 0, 20, 20);
    new FXLabel(frame1, FXString::null, NULL, JUSTIFY_LEFT); // For spacing
    FXGroupBox* groupbox1 = new FXGroupBox(frame1, FXString::null,
                                           LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(groupbox1,
                _("These key bindings are common to all Xfe applications.\nDouble click on an item to modify the selected key binding..."),
                NULL, LAYOUT_LEFT | JUSTIFY_LEFT, 0, 0, 0, 0, 0, 0, 0, 20);
    glbBindingsList = new IconList(groupbox1, NULL, 0, this, ID_GLB_BINDINGS_LIST,
                                   ICONLIST_STANDARD | HSCROLLER_NEVER | ICONLIST_BROWSESELECT |
                                   LAYOUT_SIDE_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Second tab is Xfe key bindings
    new FXTabItem(tabbook, _("Xf&e Key Bindings"), NULL);
    FXVerticalFrame* frame2 = new FXVerticalFrame(tabbook, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                                  0, 0, 0, 0, 0, 0, 20, 20);
    new FXLabel(frame2, FXString::null, NULL, JUSTIFY_LEFT); // For spacing
    FXGroupBox* groupbox2 = new FXGroupBox(frame2, FXString::null,
                                           LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(groupbox2,
                _("These key bindings are specific to the X File Explorer application.\nDouble click on an item to modify the selected key binding..."),
                NULL, LAYOUT_LEFT | JUSTIFY_LEFT, 0, 0, 0, 0, 0, 0, 0, 20);
    xfeBindingsList = new IconList(groupbox2, NULL, 0, this, ID_XFE_BINDINGS_LIST,
                                   ICONLIST_STANDARD | HSCROLLER_NEVER | ICONLIST_BROWSESELECT |
                                   LAYOUT_SIDE_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Third tab is Xfi key bindings
    new FXTabItem(tabbook, _("Xf&i Key Bindings"), NULL);
    FXVerticalFrame* frame3 = new FXVerticalFrame(tabbook, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                                  0, 0, 0, 0, 0, 0, 20, 20);
    new FXLabel(frame3, FXString::null, NULL, JUSTIFY_LEFT); // For spacing
    FXGroupBox* groupbox3 = new FXGroupBox(frame3, FXString::null,
                                           LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(groupbox3,
                _("These key bindings are specific to the X File Image application.\nDouble click on an item to modify the selected key binding..."),
                NULL, LAYOUT_LEFT | JUSTIFY_LEFT, 0, 0, 0, 0, 0, 0, 0, 20);
    xfiBindingsList = new IconList(groupbox3, NULL, 0, this, ID_XFI_BINDINGS_LIST,
                                   ICONLIST_STANDARD | HSCROLLER_NEVER | ICONLIST_BROWSESELECT |
                                   LAYOUT_SIDE_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Fourth tab is Xfw key bindings
    new FXTabItem(tabbook, _("Xf&w Key Bindings"), NULL);
    FXVerticalFrame* frame4 = new FXVerticalFrame(tabbook, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y,
                                                  0, 0, 0, 0, 0, 0, 20, 20);
    new FXLabel(frame4, FXString::null, NULL, JUSTIFY_LEFT); // For spacing
    FXGroupBox* groupbox4 = new FXGroupBox(frame4, FXString::null,
                                           LAYOUT_SIDE_TOP | FRAME_GROOVE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(groupbox4,
                _("These key bindings are specific to the X File Write application.\nDouble click on an item to modify the selected key binding..."),
                NULL, LAYOUT_LEFT | JUSTIFY_LEFT, 0, 0, 0, 0, 0, 0, 0, 20);
    xfwBindingsList = new IconList(groupbox4, NULL, 0, this, ID_XFW_BINDINGS_LIST,
                                   ICONLIST_STANDARD | HSCROLLER_NEVER | ICONLIST_BROWSESELECT |
                                   LAYOUT_SIDE_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Set list headers name and size
    FXuint hsize1 = getWidth() / 2 - 50;
    FXuint hsize2 = getWidth() / 4;

    glbBindingsList->appendHeader(_("Action Name"), NULL, hsize1);
    glbBindingsList->appendHeader(_("Registry Key"), NULL, hsize2);
    glbBindingsList->appendHeader(_("Key Binding"), NULL, hsize2);

    xfeBindingsList->appendHeader(_("Action Name"), NULL, hsize1);
    xfeBindingsList->appendHeader(_("Registry Key"), NULL, hsize2);
    xfeBindingsList->appendHeader(_("Key Binding"), NULL, hsize2);

    xfiBindingsList->appendHeader(_("Action Name"), NULL, hsize1);
    xfiBindingsList->appendHeader(_("Registry Key"), NULL, hsize2);
    xfiBindingsList->appendHeader(_("Key Binding"), NULL, hsize2);

    xfwBindingsList->appendHeader(_("Action Name"), NULL, hsize1);
    xfwBindingsList->appendHeader(_("Registry Key"), NULL, hsize2);
    xfwBindingsList->appendHeader(_("Key Binding"), NULL, hsize2);

    // Initialize sort functions
    glbBindingsList->setSortFunc(ascendingActionName);
    xfeBindingsList->setSortFunc(ascendingActionName);
    xfiBindingsList->setSortFunc(ascendingActionName);
    xfwBindingsList->setSortFunc(ascendingActionName);

    // Initialize initial binding dicts
    glbBindingsDict_prev = new FXStringDict();
    xfeBindingsDict_prev = new FXStringDict();
    xfiBindingsDict_prev = new FXStringDict();
    xfwBindingsDict_prev = new FXStringDict();

    // Changed flag
    changed = false;
}


// Create window
void KeyBindingsBox::create()
{
    DialogBox::create();

    int i;
    FXString str, data, action, keybinding;

    // Fullfill the four lists
    glbBindingsList->clearItems();
    for (i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
    {
        data = glbBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + glbBindingsDict->key(i) + TAB + keybinding;
        glbBindingsList->appendItem(str);
    }
    glbBindingsList->sortItems();

    xfeBindingsList->clearItems();
    for (i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
    {
        data = xfeBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfeBindingsDict->key(i) + TAB + keybinding;
        xfeBindingsList->appendItem(str);
    }
    xfeBindingsList->sortItems();

    xfiBindingsList->clearItems();
    for (i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
    {
        data = xfiBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfiBindingsDict->key(i) + TAB + keybinding;
        xfiBindingsList->appendItem(str);
    }
    xfiBindingsList->sortItems();

    xfwBindingsList->clearItems();
    for (i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
    {
        data = xfwBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfwBindingsDict->key(i) + TAB + keybinding;
        xfwBindingsList->appendItem(str);
    }
    xfwBindingsList->sortItems();

    // Deselect all items
    glbBindingsList->killSelection();
    xfeBindingsList->killSelection();
    xfiBindingsList->killSelection();
    xfwBindingsList->killSelection();
}


// Delete objects
KeyBindingsBox::~KeyBindingsBox()
{
    delete glbBindingsList;
    delete xfeBindingsList;
    delete xfiBindingsList;
    delete xfwBindingsList;
    delete glbBindingsDict_prev;
    delete xfeBindingsDict_prev;
    delete xfiBindingsDict_prev;
    delete xfwBindingsDict_prev;
}


// Changes are accepted
long KeyBindingsBox::onCmdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    // If some key binding was modified
    if (changed)
    {
        // Write keybindings to the registry
        FXString data, regkey, keybinding;
        for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
        {
            regkey = glbBindingsDict->key(i);
            data = glbBindingsDict->data(i);
            keybinding = data.after('\t');
            getApp()->reg().writeStringEntry("KEYBINDINGS", regkey.text(), keybinding.text());
        }
        for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
        {
            regkey = xfeBindingsDict->key(i);
            data = xfeBindingsDict->data(i);
            keybinding = data.after('\t');
            getApp()->reg().writeStringEntry("KEYBINDINGS", regkey.text(), keybinding.text());
        }
        for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
        {
            regkey = xfiBindingsDict->key(i);
            data = xfiBindingsDict->data(i);
            keybinding = data.after('\t');
            getApp()->reg().writeStringEntry("KEYBINDINGS", regkey.text(), keybinding.text());
        }
        for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
        {
            regkey = xfwBindingsDict->key(i);
            data = xfwBindingsDict->data(i);
            keybinding = data.after('\t');
            getApp()->reg().writeStringEntry("KEYBINDINGS", regkey.text(), keybinding.text());
        }

        // Update the registry
        getApp()->reg().write();

        // Reinit the changed flag
        changed = false;

        // Ask the user if he wants to restart Xfe
        if (BOX_CLICKED_CANCEL != MessageBox::question(this, BOX_OK_CANCEL, _("Restart"),
                                  _("Key bindings will be changed after restart.\nRestart X File Explorer now?")))
        {
            mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_RESTART), NULL);
        }
    }
    DialogBox::onCmdAccept(sender, sel, ptr);
    return 1;
}


// Changes are cancelled
long KeyBindingsBox::onCmdCancel(FXObject* sender, FXSelector sel, void* ptr)
{
    // Restore initial (i.e. before modification) binding dicts
    FXString data, regkey;

    for (int i = glbBindingsDict_prev->first(); i < glbBindingsDict_prev->size(); i = glbBindingsDict_prev->next(i))
    {
        regkey = glbBindingsDict_prev->key(i);
        data = glbBindingsDict_prev->data(i);
        glbBindingsDict->replace(regkey.text(), data.text());
    }
    for (int i = xfeBindingsDict_prev->first(); i < xfeBindingsDict_prev->size(); i = xfeBindingsDict_prev->next(i))
    {
        regkey = xfeBindingsDict_prev->key(i);
        data = xfeBindingsDict_prev->data(i);
        xfeBindingsDict->replace(regkey.text(), data.text());
    }
    for (int i = xfiBindingsDict_prev->first(); i < xfiBindingsDict_prev->size(); i = xfiBindingsDict_prev->next(i))
    {
        regkey = xfiBindingsDict_prev->key(i);
        data = xfiBindingsDict_prev->data(i);
        xfiBindingsDict->replace(regkey.text(), data.text());
    }
    for (int i = xfwBindingsDict_prev->first(); i < xfwBindingsDict_prev->size(); i = xfwBindingsDict_prev->next(i))
    {
        regkey = xfwBindingsDict_prev->key(i);
        data = xfwBindingsDict_prev->data(i);
        xfwBindingsDict->replace(regkey.text(), data.text());
    }

    // Reinit the changed flag
    changed = false;

    DialogBox::onCmdCancel(sender, sel, ptr);
    return 0;
}


// Compare sectioned strings
int KeyBindingsBox::compareSection(const char* p, const char* q, int s)
{
    int c1, c2, x;

    for (x = s; x && *p; x -= (*p++ == '\t'))
    {
    }
    for (x = s; x && *q; x -= (*q++ == '\t'))
    {
    }
    do
    {
        c1 = (FXuchar)(*p++);
        c2 = (FXuchar)(*q++);
    }
    while ('\t' < c1 && (c1 == c2));

    return c1 - c2;
}


// Sort functions
int KeyBindingsBox::ascendingActionName(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 0);
}


int KeyBindingsBox::descendingActionName(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 0);
}


int KeyBindingsBox::ascendingRegistryKey(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 1);
}


int KeyBindingsBox::descendingRegistryKey(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 1);
}


int KeyBindingsBox::ascendingKeyBinding(const IconItem* a, const IconItem* b)
{
    return compareSection(a->getText().text(), b->getText().text(), 2);
}


int KeyBindingsBox::descendingKeyBinding(const IconItem* a, const IconItem* b)
{
    return compareSection(b->getText().text(), a->getText().text(), 2);
}


// Sort global list by action name
long KeyBindingsBox::onCmdGlbSortByActionName(FXObject*, FXSelector, void*)
{
    glbBindingsList->setSortFunc((glbBindingsList->getSortFunc() ==
                                  ascendingActionName) ? descendingActionName : ascendingActionName);
    glbBindingsList->setSortHeader(0);
    glbBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
    {
        data = glbBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + glbBindingsDict->key(i) + TAB + keybinding;
        glbBindingsList->appendItem(str);
    }
    glbBindingsList->sortItems();

    return 1;
}


// Sort global list by registry key name
long KeyBindingsBox::onCmdGlbSortByRegistryKey(FXObject*, FXSelector, void*)
{
    glbBindingsList->setSortFunc((glbBindingsList->getSortFunc() ==
                                  ascendingRegistryKey) ? descendingRegistryKey : ascendingRegistryKey);
    glbBindingsList->setSortHeader(1);
    glbBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
    {
        data = glbBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + glbBindingsDict->key(i) + TAB + keybinding;
        glbBindingsList->appendItem(str);
    }
    glbBindingsList->sortItems();

    return 1;
}


// Sort global list by key binding
long KeyBindingsBox::onCmdGlbSortByKeyBinding(FXObject*, FXSelector, void*)
{
    glbBindingsList->setSortFunc((glbBindingsList->getSortFunc() ==
                                  ascendingKeyBinding) ? descendingKeyBinding : ascendingKeyBinding);
    glbBindingsList->setSortHeader(2);
    glbBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
    {
        data = glbBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + glbBindingsDict->key(i) + TAB + keybinding;
        glbBindingsList->appendItem(str);
    }
    glbBindingsList->sortItems();

    return 1;
}


// Sort Xfe list by action name
long KeyBindingsBox::onCmdXfeSortByActionName(FXObject*, FXSelector, void*)
{
    xfeBindingsList->setSortFunc((xfeBindingsList->getSortFunc() ==
                                  ascendingActionName) ? descendingActionName : ascendingActionName);
    xfeBindingsList->setSortHeader(0);
    xfeBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
    {
        data = xfeBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfeBindingsDict->key(i) + TAB + keybinding;
        xfeBindingsList->appendItem(str);
    }
    xfeBindingsList->sortItems();

    return 1;
}


// Sort Xfe list by registry key name
long KeyBindingsBox::onCmdXfeSortByRegistryKey(FXObject*, FXSelector, void*)
{
    xfeBindingsList->setSortFunc((xfeBindingsList->getSortFunc() ==
                                  ascendingRegistryKey) ? descendingRegistryKey : ascendingRegistryKey);
    xfeBindingsList->setSortHeader(1);
    xfeBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
    {
        data = xfeBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfeBindingsDict->key(i) + TAB + keybinding;
        xfeBindingsList->appendItem(str);
    }
    xfeBindingsList->sortItems();

    return 1;
}


// Sort Xfe list by key binding
long KeyBindingsBox::onCmdXfeSortByKeyBinding(FXObject*, FXSelector, void*)
{
    xfeBindingsList->setSortFunc((xfeBindingsList->getSortFunc() ==
                                  ascendingKeyBinding) ? descendingKeyBinding : ascendingKeyBinding);
    xfeBindingsList->setSortHeader(2);
    xfeBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
    {
        data = xfeBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfeBindingsDict->key(i) + TAB + keybinding;
        xfeBindingsList->appendItem(str);
    }
    xfeBindingsList->sortItems();

    return 1;
}


// Sort Xfi list by action name
long KeyBindingsBox::onCmdXfiSortByActionName(FXObject*, FXSelector, void*)
{
    xfiBindingsList->setSortFunc((xfiBindingsList->getSortFunc() ==
                                  ascendingActionName) ? descendingActionName : ascendingActionName);
    xfiBindingsList->setSortHeader(0);
    xfiBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
    {
        data = xfiBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfiBindingsDict->key(i) + TAB + keybinding;
        xfiBindingsList->appendItem(str);
    }
    xfiBindingsList->sortItems();

    return 1;
}


// Sort Xfi list by registry key name
long KeyBindingsBox::onCmdXfiSortByRegistryKey(FXObject*, FXSelector, void*)
{
    xfiBindingsList->setSortFunc((xfiBindingsList->getSortFunc() ==
                                  ascendingRegistryKey) ? descendingRegistryKey : ascendingRegistryKey);
    xfiBindingsList->setSortHeader(1);
    xfiBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
    {
        data = xfiBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfiBindingsDict->key(i) + TAB + keybinding;
        xfiBindingsList->appendItem(str);
    }
    xfiBindingsList->sortItems();

    return 1;
}


// Sort Xfi list by key binding
long KeyBindingsBox::onCmdXfiSortByKeyBinding(FXObject*, FXSelector, void*)
{
    xfiBindingsList->setSortFunc((xfiBindingsList->getSortFunc() ==
                                  ascendingKeyBinding) ? descendingKeyBinding : ascendingKeyBinding);
    xfiBindingsList->setSortHeader(2);
    xfiBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
    {
        data = xfiBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfiBindingsDict->key(i) + TAB + keybinding;
        xfiBindingsList->appendItem(str);
    }
    xfiBindingsList->sortItems();

    return 1;
}


// Sort Xfw list by action name
long KeyBindingsBox::onCmdXfwSortByActionName(FXObject*, FXSelector, void*)
{
    xfwBindingsList->setSortFunc((xfwBindingsList->getSortFunc() ==
                                  ascendingActionName) ? descendingActionName : ascendingActionName);
    xfwBindingsList->setSortHeader(0);
    xfwBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
    {
        data = xfwBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfwBindingsDict->key(i) + TAB + keybinding;
        xfwBindingsList->appendItem(str);
    }
    xfwBindingsList->sortItems();

    return 1;
}


// Sort Xfw list by registry key name
long KeyBindingsBox::onCmdXfwSortByRegistryKey(FXObject*, FXSelector, void*)
{
    xfwBindingsList->setSortFunc((xfwBindingsList->getSortFunc() ==
                                  ascendingRegistryKey) ? descendingRegistryKey : ascendingRegistryKey);
    xfwBindingsList->setSortHeader(1);
    xfwBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
    {
        data = xfwBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfwBindingsDict->key(i) + TAB + keybinding;
        xfwBindingsList->appendItem(str);
    }
    xfwBindingsList->sortItems();

    return 1;
}


// Sort Xfw list by key binding
long KeyBindingsBox::onCmdXfwSortByKeyBinding(FXObject*, FXSelector, void*)
{
    xfwBindingsList->setSortFunc((xfwBindingsList->getSortFunc() ==
                                  ascendingKeyBinding) ? descendingKeyBinding : ascendingKeyBinding);
    xfwBindingsList->setSortHeader(2);
    xfwBindingsList->clearItems();
    FXString str, data, action, keybinding;
    for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
    {
        data = xfwBindingsDict->data(i);
        action = data.before('\t');
        keybinding = data.after('\t');
        str = action + TAB + xfwBindingsDict->key(i) + TAB + keybinding;
        xfwBindingsList->appendItem(str);
    }
    xfwBindingsList->sortItems();

    return 1;
}


// Clicked on a global list header button
long KeyBindingsBox::onCmdGlbHeaderClicked(FXObject*, FXSelector, void* ptr)
{
    FXuint num = (FXuint)(FXuval)ptr;

    if (num < 3)
    {
        if (num == 0)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_GLB_SORT_BY_ACTIONNAME), NULL);
        }
        else if (num == 1)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_GLB_SORT_BY_REGISTRYKEY), NULL);
        }
        else if (num == 2)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_GLB_SORT_BY_KEYBINDING), NULL);
        }
    }

    return 1;
}


// Clicked on a Xfe list header button
long KeyBindingsBox::onCmdXfeHeaderClicked(FXObject*, FXSelector, void* ptr)
{
    FXuint num = (FXuint)(FXuval)ptr;

    if (num < 3)
    {
        if (num == 0)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFE_SORT_BY_ACTIONNAME), NULL);
        }
        else if (num == 1)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFE_SORT_BY_REGISTRYKEY), NULL);
        }
        else if (num == 2)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFE_SORT_BY_KEYBINDING), NULL);
        }
    }

    return 1;
}


// Clicked on a Xfi list header button
long KeyBindingsBox::onCmdXfiHeaderClicked(FXObject*, FXSelector, void* ptr)
{
    FXuint num = (FXuint)(FXuval)ptr;

    if (num < 3)
    {
        if (num == 0)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFI_SORT_BY_ACTIONNAME), NULL);
        }
        else if (num == 1)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFI_SORT_BY_REGISTRYKEY), NULL);
        }
        else if (num == 2)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFI_SORT_BY_KEYBINDING), NULL);
        }
    }

    return 1;
}


// Clicked on a Xfw list header button
long KeyBindingsBox::onCmdXfwHeaderClicked(FXObject*, FXSelector, void* ptr)
{
    FXuint num = (FXuint)(FXuval)ptr;

    if (num < 3)
    {
        if (num == 0)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFW_SORT_BY_ACTIONNAME), NULL);
        }
        else if (num == 1)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFW_SORT_BY_REGISTRYKEY), NULL);
        }
        else if (num == 2)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_XFW_SORT_BY_KEYBINDING), NULL);
        }
    }

    return 1;
}


// Update global list header
long KeyBindingsBox::onUpdGlbHeader(FXObject*, FXSelector, void*)
{
    // Update header arrow
    glbBindingsList->getHeader()->setArrowDir(0,
                                              (glbBindingsList->getSortFunc() ==
                                               ascendingActionName) ? false : (glbBindingsList->getSortFunc() ==
                                                                               descendingActionName) ? true : MAYBE);
    glbBindingsList->getHeader()->setArrowDir(1,
                                              (glbBindingsList->getSortFunc() ==
                                               ascendingRegistryKey) ? false : (glbBindingsList->getSortFunc() ==
                                                                                descendingRegistryKey) ? true : MAYBE);
    glbBindingsList->getHeader()->setArrowDir(2,
                                              (glbBindingsList->getSortFunc() ==
                                               ascendingKeyBinding) ? false : (glbBindingsList->getSortFunc() ==
                                                                               descendingKeyBinding) ? true : MAYBE);

    // Set minimum header size
    if (glbBindingsList->getHeaderSize(0) < MIN_HEADER_SIZE)
    {
        glbBindingsList->setHeaderSize(0, MIN_HEADER_SIZE);
    }
    if (glbBindingsList->getHeaderSize(1) < MIN_HEADER_SIZE)
    {
        glbBindingsList->setHeaderSize(1, MIN_HEADER_SIZE);
    }
    if (glbBindingsList->getHeaderSize(2) < MIN_HEADER_SIZE)
    {
        glbBindingsList->setHeaderSize(2, MIN_HEADER_SIZE);
    }

    return 1;
}


// Update Xfe list header
long KeyBindingsBox::onUpdXfeHeader(FXObject*, FXSelector, void*)
{
    // Update header arrow
    xfeBindingsList->getHeader()->setArrowDir(0,
                                              (xfeBindingsList->getSortFunc() ==
                                               ascendingActionName) ? false : (xfeBindingsList->getSortFunc() ==
                                                                               descendingActionName) ? true : MAYBE);
    xfeBindingsList->getHeader()->setArrowDir(1,
                                              (xfeBindingsList->getSortFunc() ==
                                               ascendingRegistryKey) ? false : (xfeBindingsList->getSortFunc() ==
                                                                                descendingRegistryKey) ? true : MAYBE);
    xfeBindingsList->getHeader()->setArrowDir(2,
                                              (xfeBindingsList->getSortFunc() ==
                                               ascendingKeyBinding) ? false : (xfeBindingsList->getSortFunc() ==
                                                                               descendingKeyBinding) ? true : MAYBE);

    // Set minimum header size
    if (xfeBindingsList->getHeaderSize(0) < MIN_HEADER_SIZE)
    {
        xfeBindingsList->setHeaderSize(0, MIN_HEADER_SIZE);
    }
    if (xfeBindingsList->getHeaderSize(1) < MIN_HEADER_SIZE)
    {
        xfeBindingsList->setHeaderSize(1, MIN_HEADER_SIZE);
    }
    if (xfeBindingsList->getHeaderSize(2) < MIN_HEADER_SIZE)
    {
        xfeBindingsList->setHeaderSize(2, MIN_HEADER_SIZE);
    }

    return 1;
}


// Update Xfi list header
long KeyBindingsBox::onUpdXfiHeader(FXObject*, FXSelector, void*)
{
    // Update header arrow
    xfiBindingsList->getHeader()->setArrowDir(0,
                                              (xfiBindingsList->getSortFunc() ==
                                               ascendingActionName) ? false : (xfiBindingsList->getSortFunc() ==
                                                                               descendingActionName) ? true : MAYBE);
    xfiBindingsList->getHeader()->setArrowDir(1,
                                              (xfiBindingsList->getSortFunc() ==
                                               ascendingRegistryKey) ? false : (xfiBindingsList->getSortFunc() ==
                                                                                descendingRegistryKey) ? true : MAYBE);
    xfiBindingsList->getHeader()->setArrowDir(2,
                                              (xfiBindingsList->getSortFunc() ==
                                               ascendingKeyBinding) ? false : (xfiBindingsList->getSortFunc() ==
                                                                               descendingKeyBinding) ? true : MAYBE);

    // Set minimum header size
    if (xfiBindingsList->getHeaderSize(0) < MIN_HEADER_SIZE)
    {
        xfiBindingsList->setHeaderSize(0, MIN_HEADER_SIZE);
    }
    if (xfiBindingsList->getHeaderSize(1) < MIN_HEADER_SIZE)
    {
        xfiBindingsList->setHeaderSize(1, MIN_HEADER_SIZE);
    }
    if (xfiBindingsList->getHeaderSize(2) < MIN_HEADER_SIZE)
    {
        xfiBindingsList->setHeaderSize(2, MIN_HEADER_SIZE);
    }

    return 1;
}


// Update Xfw list header
long KeyBindingsBox::onUpdXfwHeader(FXObject*, FXSelector, void*)
{
    // Update header arrow
    xfwBindingsList->getHeader()->setArrowDir(0,
                                              (xfwBindingsList->getSortFunc() ==
                                               ascendingActionName) ? false : (xfwBindingsList->getSortFunc() ==
                                                                               descendingActionName) ? true : MAYBE);
    xfwBindingsList->getHeader()->setArrowDir(1,
                                              (xfwBindingsList->getSortFunc() ==
                                               ascendingRegistryKey) ? false : (xfwBindingsList->getSortFunc() ==
                                                                                descendingRegistryKey) ? true : MAYBE);
    xfwBindingsList->getHeader()->setArrowDir(2,
                                              (xfwBindingsList->getSortFunc() ==
                                               ascendingKeyBinding) ? false : (xfwBindingsList->getSortFunc() ==
                                                                               descendingKeyBinding) ? true : MAYBE);

    // Set minimum header size
    if (xfwBindingsList->getHeaderSize(0) < MIN_HEADER_SIZE)
    {
        xfwBindingsList->setHeaderSize(0, MIN_HEADER_SIZE);
    }
    if (xfwBindingsList->getHeaderSize(1) < MIN_HEADER_SIZE)
    {
        xfwBindingsList->setHeaderSize(1, MIN_HEADER_SIZE);
    }
    if (xfwBindingsList->getHeaderSize(2) < MIN_HEADER_SIZE)
    {
        xfwBindingsList->setHeaderSize(2, MIN_HEADER_SIZE);
    }

    return 1;
}


// Double clicked on an item in the global list
long KeyBindingsBox::onCmdDefineGlbKeyBindings(FXObject*, FXSelector, void*)
{
    // Get selected item string
    FXString itemtext = "";
    int index = -1;

    for (int u = 0; u < glbBindingsList->getNumItems(); u++)
    {
        if (glbBindingsList->isItemSelected(u))
        {
            itemtext = glbBindingsList->getItemText(u);
            index = u;
        }
    }
    if (index < 0) // Should not happen
    {
        return 0;
    }

    // Decompose item text
    FXString data = itemtext.rbefore('\t');
    FXString key = itemtext.rafter('\t');
    FXString action = data.before('\t');
    FXString regkey = data.after('\t');

    // Input dialog
    FXString message;
    message.format(_("Press the combination of keys you want to use for the action: %s"), action.text());
    message = message + "\n" + _("[Press space to disable the key binding for this action]");
    KeyBindingsDialog* kbdialog = new KeyBindingsDialog(this, key, message, _("Modify Key Binding"),
                                                        bigkeybindingsicon);

    // Accept was pressed
    if (kbdialog->execute(PLACEMENT_CURSOR))
    {
        // Convert the entered string into a valid key binding string
        FXString newkey = kbdialog->getKey();
        if (newkey == "Space")
        {
            newkey = "";
        }

        // Check if the new key binding is not already used elsewhere
        if (newkey != "")
        {
            FXString dictdata, dictkey;
            FXbool exist_in_glb = false, exist_in_xfe = false, exist_in_xfi = false, exist_in_xfw = false;
            for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
            {
                dictdata = glbBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_glb = true;
                    break;
                }
            }
            if (exist_in_glb)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the global section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
            {
                dictdata = xfeBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfe = true;
                    break;
                }
            }
            if (exist_in_xfe)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfe section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
            {
                dictdata = xfiBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfi = true;
                    break;
                }
            }
            if (exist_in_xfi)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfi section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
            {
                dictdata = xfwBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfw = true;
                    break;
                }
            }
            if (exist_in_xfw)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfw section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
        }

        // Modify the item text
        itemtext = data + TAB + newkey;
        glbBindingsList->setItemText(index, itemtext);

        // Update dictionary
        FXString str = action + TAB + newkey;
        glbBindingsDict->replace(regkey.text(), str.text());

        changed = true;
    }
    // Cancel was pressed
    else
    {
        delete kbdialog;
        return 0;
    }

    delete kbdialog;
    return 1;
}


// Double clicked on an item in the Xfe list
long KeyBindingsBox::onCmdDefineXfeKeyBindings(FXObject*, FXSelector, void*)
{
    // Get selected item string
    FXString itemtext = "";
    int index = -1;

    for (int u = 0; u < xfeBindingsList->getNumItems(); u++)
    {
        if (xfeBindingsList->isItemSelected(u))
        {
            itemtext = xfeBindingsList->getItemText(u);
            index = u;
        }
    }
    if (index < 0) // Should not happen
    {
        return 0;
    }

    // Decompose item text
    FXString data = itemtext.rbefore('\t');
    FXString key = itemtext.rafter('\t');
    FXString action = data.before('\t');
    FXString regkey = data.after('\t');

    // Input dialog
    FXString message;
    message.format(_("Press the combination of keys you want to use for the action: %s"), action.text());
    message = message + "\n" + _("[Press space to disable the key binding for this action]");
    KeyBindingsDialog* kbdialog = new KeyBindingsDialog(this, key, message, _("Modify Key Binding"),
                                                        bigkeybindingsicon);

    // Accept was pressed
    if (kbdialog->execute(PLACEMENT_CURSOR))
    {
        // Convert the entered string into a valid key binding string
        FXString newkey = kbdialog->getKey();
        if (newkey == "Space")
        {
            newkey = "";
        }

        // Check if the new key binding is not already used elsewhere
        if (newkey != "")
        {
            FXString dictdata, dictkey;
            FXbool exist_in_glb = false, exist_in_xfe = false;
            for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
            {
                dictdata = glbBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_glb = true;
                    break;
                }
            }
            if (exist_in_glb)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the global section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
            {
                dictdata = xfeBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfe = true;
                    break;
                }
            }
            if (exist_in_xfe)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfe section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
        }

        // Modify the item text
        itemtext = data + TAB + newkey;
        xfeBindingsList->setItemText(index, itemtext);

        // Update dictionary
        FXString str = action + TAB + newkey;
        xfeBindingsDict->replace(regkey.text(), str.text());

        changed = true;
    }
    // Cancel was pressed
    else
    {
        delete kbdialog;
        return 0;
    }

    delete kbdialog;
    return 1;
}


// Double clicked on an item in the Xfi list
long KeyBindingsBox::onCmdDefineXfiKeyBindings(FXObject*, FXSelector, void*)
{
    // Get selected item string
    FXString itemtext = "";
    int index = -1;

    for (int u = 0; u < xfiBindingsList->getNumItems(); u++)
    {
        if (xfiBindingsList->isItemSelected(u))
        {
            itemtext = xfiBindingsList->getItemText(u);
            index = u;
        }
    }
    if (index < 0) // Should not happen
    {
        return 0;
    }

    // Decompose item text
    FXString data = itemtext.rbefore('\t');
    FXString key = itemtext.rafter('\t');
    FXString action = data.before('\t');
    FXString regkey = data.after('\t');

    // Input dialog
    FXString message;
    message.format(_("Press the combination of keys you want to use for the action: %s"), action.text());
    message = message + "\n" + _("[Press space to disable the key binding for this action]");
    KeyBindingsDialog* kbdialog = new KeyBindingsDialog(this, key, message, _("Modify Key Binding"),
                                                        bigkeybindingsicon);

    // Accept was pressed
    if (kbdialog->execute(PLACEMENT_CURSOR))
    {
        // Convert the entered string into a valid key binding string
        FXString newkey = kbdialog->getKey();
        if (newkey == "Space")
        {
            newkey = "";
        }

        // Check if the new key binding is not already used elsewhere
        if (newkey != "")
        {
            FXString dictdata, dictkey;
            FXbool exist_in_glb = false, exist_in_xfi = false;
            for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
            {
                dictdata = glbBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_glb = true;
                    break;
                }
            }
            if (exist_in_glb)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the global section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
            {
                dictdata = xfiBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfi = true;
                    break;
                }
            }
            if (exist_in_xfi)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfi section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
        }

        // Modify the item text
        itemtext = data + TAB + newkey;
        xfiBindingsList->setItemText(index, itemtext);

        // Update dictionary
        FXString str = action + TAB + newkey;
        xfiBindingsDict->replace(regkey.text(), str.text());

        changed = true;
    }
    // Cancel was pressed
    else
    {
        delete kbdialog;
        return 0;
    }

    delete kbdialog;
    return 1;
}


// Double clicked on an item in the Xfw list
long KeyBindingsBox::onCmdDefineXfwKeyBindings(FXObject*, FXSelector, void*)
{
    // Get selected item string
    FXString itemtext = "";
    int index = -1;

    for (int u = 0; u < xfwBindingsList->getNumItems(); u++)
    {
        if (xfwBindingsList->isItemSelected(u))
        {
            itemtext = xfwBindingsList->getItemText(u);
            index = u;
        }
    }
    if (index < 0) // Should not happen
    {
        return 0;
    }

    // Decompose item text
    FXString data = itemtext.rbefore('\t');
    FXString key = itemtext.rafter('\t');
    FXString action = data.before('\t');
    FXString regkey = data.after('\t');

    // Input dialog
    FXString message;
    message.format(_("Press the combination of keys you want to use for the action: %s"), action.text());
    message = message + "\n" + _("[Press space to disable the key binding for this action]");
    KeyBindingsDialog* kbdialog = new KeyBindingsDialog(this, key, message, _("Modify Key Binding"),
                                                        bigkeybindingsicon);

    // Accept was pressed
    if (kbdialog->execute(PLACEMENT_CURSOR))
    {
        // Convert the entered string into a valid key binding string
        FXString newkey = kbdialog->getKey();
        if (newkey == "Space")
        {
            newkey = "";
        }

        // Check if the new key binding is not already used elsewhere
        if (newkey != "")
        {
            FXString dictdata, dictkey;
            FXbool exist_in_glb = false, exist_in_xfw = false;
            for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
            {
                dictdata = glbBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_glb = true;
                    break;
                }
            }
            if (exist_in_glb)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the global section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
            for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
            {
                dictdata = xfwBindingsDict->data(i);
                dictkey = dictdata.after('\t');
                if (dictkey == newkey)
                {
                    exist_in_xfw = true;
                    break;
                }
            }
            if (exist_in_xfw)
            {
                MessageBox::error(this, BOX_OK, _("Error"),
                                  _("The key binding %s is already used in the Xfw section.\n\
You should erase the existing key binding before assigning it again."),
                                  newkey.text());

                delete kbdialog;
                return 0;
            }
        }

        // Modify the item text
        itemtext = data + TAB + newkey;
        xfwBindingsList->setItemText(index, itemtext);

        // Update dictionary
        FXString str = action + TAB + newkey;
        xfwBindingsDict->replace(regkey.text(), str.text());

        changed = true;
    }
    // Cancel was pressed
    else
    {
        delete kbdialog;
        return 0;
    }

    delete kbdialog;
    return 1;
}


// Execute dialog box modally
FXuint KeyBindingsBox::execute(FXuint placement)
{
    // Save binding dicts for cancel purpose
    FXString data, regkey;

    for (int i = glbBindingsDict->first(); i < glbBindingsDict->size(); i = glbBindingsDict->next(i))
    {
        regkey = glbBindingsDict->key(i);
        data = glbBindingsDict->data(i);
        glbBindingsDict_prev->replace(regkey.text(), data.text());
    }
    for (int i = xfeBindingsDict->first(); i < xfeBindingsDict->size(); i = xfeBindingsDict->next(i))
    {
        regkey = xfeBindingsDict->key(i);
        data = xfeBindingsDict->data(i);
        xfeBindingsDict_prev->replace(regkey.text(), data.text());
    }
    for (int i = xfiBindingsDict->first(); i < xfiBindingsDict->size(); i = xfiBindingsDict->next(i))
    {
        regkey = xfiBindingsDict->key(i);
        data = xfiBindingsDict->data(i);
        xfiBindingsDict_prev->replace(regkey.text(), data.text());
    }
    for (int i = xfwBindingsDict->first(); i < xfwBindingsDict->size(); i = xfwBindingsDict->next(i))
    {
        regkey = xfwBindingsDict->key(i);
        data = xfwBindingsDict->data(i);
        xfwBindingsDict_prev->replace(regkey.text(), data.text());
    }

    // Execute dialog
    FXuint ret = DialogBox::execute(placement);
    return ret;
}
