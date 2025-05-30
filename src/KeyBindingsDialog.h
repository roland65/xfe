#ifndef KEYBINDINGSDIALOG_H
#define KEYBINDINGSDIALOG_H

#include "DialogBox.h"

class XComApp;

class KeyBindingsDialog : public DialogBox
{
    FXDECLARE(KeyBindingsDialog)
protected:
    FXLabel* keylabel = NULL;
private:
    KeyBindingsDialog()
    {
    }
public:
    KeyBindingsDialog(FXWindow*, FXString, FXString, FXString, FXIcon* icon = NULL);
    virtual void create();

    long onCmdKeyPress(FXObject*, FXSelector, void*);
    FXString getKey()
    {
        return keylabel->getText();
    }
};
#endif
