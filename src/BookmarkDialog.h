#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include "DialogBox.h"

class XComApp;

class BookmarkDialog : public DialogBox
{
    FXDECLARE(BookmarkDialog)

protected:
    FXTextField* nameinp = NULL;
    FXLabel* msg = NULL;
    FXTextField* iconinp = NULL;
    FXButton* iconbtn = NULL;
    FXString prev_name;
    FXString prev_iconpathname;
    double scalefrac = 1.0;                  // Icon scaling factor

private:
    BookmarkDialog()
    {
    }
public:
public:

    enum
    {
        ID_BROWSE_ICON=DialogBox::ID_LAST,
        ID_ACCEPT,
        ID_CANCEL,
        ID_LAST
    };

    BookmarkDialog(FXWindow*, FXString, FXString, FXString, FXString label1 = "", FXString label2 = "",
                   FXIcon* icon = NULL,
                   FXString = FXString::null);
    virtual void create();

    FXString getName()
    {
        return nameinp->getText();
    }

    void setName(const FXString& text)
    {
        nameinp->setText(text);
    }

    FXString getIconPathname()
    {
        return iconinp->getText();
    }

    void setIconPathname(const FXString& text)
    {
        iconinp->setText(text);
    }


    void setMessage(const FXString& text)
    {
        msg->setText(text);
    }

    void selectAll()
    {
        nameinp->setSelection(0, (nameinp->getText()).length());
    }

    void CursorEnd()
    {
        nameinp->onCmdCursorEnd(0, 0, 0);
    }

    void setSelection(int pos, int len)
    {
        nameinp->setSelection(pos, len);
    }

    FXuint execute(FXuint placement);

    // Callbacks
    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onCmdAccept(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdBrowseIcon(FXObject*, FXSelector, void*);
};
#endif
