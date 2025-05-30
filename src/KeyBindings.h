#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "DialogBox.h"
#include "IconList.h"


class KeyBindingsBox : public DialogBox
{
    FXDECLARE(KeyBindingsBox)
protected:
    IconList* glbBindingsList = NULL;
    IconList* xfeBindingsList = NULL;
    IconList* xfiBindingsList = NULL;
    IconList* xfwBindingsList = NULL;
    FXStringDict* glbBindingsDict = NULL;
    FXStringDict* xfeBindingsDict = NULL;
    FXStringDict* xfiBindingsDict = NULL;
    FXStringDict* xfwBindingsDict = NULL;
    FXStringDict* glbBindingsDict_prev = NULL;
    FXStringDict* xfeBindingsDict_prev = NULL;
    FXStringDict* xfiBindingsDict_prev = NULL;
    FXStringDict* xfwBindingsDict_prev = NULL;
    FXbool changed = false;

private:
    KeyBindingsBox()
    {
    }
public:
    enum
    {
        ID_ACCEPT=DialogBox::ID_LAST,
        ID_CANCEL,
        ID_GLB_BINDINGS_LIST,
        ID_XFE_BINDINGS_LIST,
        ID_XFI_BINDINGS_LIST,
        ID_XFW_BINDINGS_LIST,
        ID_GLB_SORT_BY_ACTIONNAME,
        ID_GLB_SORT_BY_REGISTRYKEY,
        ID_GLB_SORT_BY_KEYBINDING,
        ID_XFE_SORT_BY_ACTIONNAME,
        ID_XFE_SORT_BY_REGISTRYKEY,
        ID_XFE_SORT_BY_KEYBINDING,
        ID_XFI_SORT_BY_ACTIONNAME,
        ID_XFI_SORT_BY_REGISTRYKEY,
        ID_XFI_SORT_BY_KEYBINDING,
        ID_XFW_SORT_BY_ACTIONNAME,
        ID_XFW_SORT_BY_REGISTRYKEY,
        ID_XFW_SORT_BY_KEYBINDING,
        ID_LAST
    };
    KeyBindingsBox(FXWindow*, FXStringDict*, FXStringDict*, FXStringDict*, FXStringDict*);
    virtual void create();

    virtual ~KeyBindingsBox();
    FXuint execute(FXuint);
    long onCmdAccept(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdDefineGlbKeyBindings(FXObject*, FXSelector, void*);
    long onCmdDefineXfeKeyBindings(FXObject*, FXSelector, void*);
    long onCmdDefineXfiKeyBindings(FXObject*, FXSelector, void*);
    long onCmdDefineXfwKeyBindings(FXObject*, FXSelector, void*);
    long onCmdGlbSortByActionName(FXObject*, FXSelector, void*);
    long onCmdGlbSortByRegistryKey(FXObject*, FXSelector, void*);
    long onCmdGlbSortByKeyBinding(FXObject*, FXSelector, void*);
    long onCmdXfeSortByActionName(FXObject*, FXSelector, void*);
    long onCmdXfeSortByRegistryKey(FXObject*, FXSelector, void*);
    long onCmdXfeSortByKeyBinding(FXObject*, FXSelector, void*);
    long onCmdXfiSortByActionName(FXObject*, FXSelector, void*);
    long onCmdXfiSortByRegistryKey(FXObject*, FXSelector, void*);
    long onCmdXfiSortByKeyBinding(FXObject*, FXSelector, void*);
    long onCmdXfwSortByActionName(FXObject*, FXSelector, void*);
    long onCmdXfwSortByRegistryKey(FXObject*, FXSelector, void*);
    long onCmdXfwSortByKeyBinding(FXObject*, FXSelector, void*);
    long onCmdGlbHeaderClicked(FXObject*, FXSelector, void*);
    long onCmdXfeHeaderClicked(FXObject*, FXSelector, void*);
    long onCmdXfiHeaderClicked(FXObject*, FXSelector, void*);
    long onCmdXfwHeaderClicked(FXObject*, FXSelector, void*);
    long onUpdGlbHeader(FXObject*, FXSelector, void*);
    long onUpdXfeHeader(FXObject*, FXSelector, void*);
    long onUpdXfiHeader(FXObject*, FXSelector, void*);
    long onUpdXfwHeader(FXObject*, FXSelector, void*);
public:
    static int compareSection(const char*, const char*, int);
    static int ascendingActionName(const IconItem*, const IconItem*);
    static int descendingActionName(const IconItem*, const IconItem*);
    static int ascendingRegistryKey(const IconItem*, const IconItem*);
    static int descendingRegistryKey(const IconItem*, const IconItem*);
    static int ascendingKeyBinding(const IconItem*, const IconItem*);
    static int descendingKeyBinding(const IconItem*, const IconItem*);
};
#endif
