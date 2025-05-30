#ifndef HISTINPUTDIALOG_H
#define HISTINPUTDIALOG_H

#include "ComboBox.h"
#include "DialogBox.h"

// Browse types
enum
{
    HIST_INPUT_FILE,
    HIST_INPUT_EXECUTABLE_FILE,
    HIST_INPUT_FOLDER,
    HIST_INPUT_MIXED
};

class XComApp;

class HistComboBox : public ComboBox
{
    FXDECLARE(HistComboBox)
private:
    HistComboBox()
    {
    }

public:
    FXTextField* getTextEntry()
    {
        return field;
    }

    void CursorEnd()
    {
        field->onCmdCursorEnd(0, 0, 0);
        field->setFocus();
    }

    HistComboBox(FXComposite* p, int cols, FXbool addr = false, FXbool crlbtn = false, FXObject* tgt = NULL,
                 FXSelector sel = 0, FXuint opts = COMBOBOX_NORMAL);
    virtual void create();
};

class HistInputDialog : public DialogBox
{
    FXDECLARE(HistInputDialog)
protected:
    FXHorizontalFrame* buttons = NULL;
    FXHorizontalFrame* checkbutton = NULL;
    FXButton* accept = NULL;
    FXButton* cancel = NULL;
    HistComboBox* input = NULL;
    FXuint browsetype = 0;
    FXString initialdir;
private:
    HistInputDialog()
    {
    }
public:
    enum
    {
        ID_BROWSE_PATH=DialogBox::ID_LAST,
        ID_LAST
    };
    HistInputDialog(FXWindow*, FXString, FXString, FXString, FXString label = "", FXIcon* ic = NULL,
                    FXbool addr = false, FXbool clrbtn = false, FXuint browse = HIST_INPUT_FILE,
                    FXbool option = false, FXString = FXString::null);
    virtual void create();

    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    long onUpdAccept(FXObject*, FXSelector, void*);

    FXString getText()
    {
        return input->getText();
    }

    void setText(const FXString& text)
    {
        input->setText(text);
    }

    void CursorEnd();
    void selectAll();

    void appendItem(char* str)
    {
        input->appendItem(str);
    }

    void clearItems()
    {
        input->clearItems();
    }

    FXString getHistoryItem(int pos)
    {
        return input->getItemText(pos);
    }

    int getHistorySize()
    {
        return input->getNumItems();
    }

    void setDirectory(const FXString&);

    void sortItems()
    {
        input->setSortFunc(FXList::ascendingCase);
        input->sortItems();
    }
};
#endif
