#ifndef ARCHINPUTDIALOG_H
#define ARCHINPUTDIALOG_H

#include "DialogBox.h"

class XComApp;

class ArchInputDialog : public DialogBox
{
    FXDECLARE(ArchInputDialog)
protected:
    FXTextField* input = NULL;
    FXPopup* popup = NULL;
    FXOptionMenu* optionmenu = NULL;
    FXOption* option_tgz = NULL;
    FXOption* option_zip = NULL;
    FXOption* option_7zip = NULL;
    FXOption* option_rar = NULL;
    FXOption* option_tbz2 = NULL;
    FXOption* option_txz = NULL;
    FXOption* option_tzst = NULL;
    FXOption* option_tar = NULL;
    FXOption* option_taz = NULL;
    FXOption* option_gz = NULL;
    FXOption* option_bz2 = NULL;
    FXOption* option_xz = NULL;
    FXOption* option_zst = NULL;
    FXOption* option_z = NULL;

private:
    ArchInputDialog()
    {
    }
public:
    enum
    {
        ID_BROWSE_PATH=DialogBox::ID_LAST,
        ID_FORMAT_TAR_GZ,
        ID_FORMAT_ZIP,
        ID_FORMAT_7ZIP,
        ID_FORMAT_RAR,
        ID_FORMAT_TAR_BZ2,
        ID_FORMAT_TAR_XZ,
        ID_FORMAT_TAR_ZST,
        ID_FORMAT_TAR,
        ID_FORMAT_TAR_Z,
        ID_FORMAT_GZ,
        ID_FORMAT_BZ2,
        ID_FORMAT_XZ,
        ID_FORMAT_ZST,
        ID_FORMAT_Z,
        ID_LAST
    };
    ArchInputDialog(FXWindow*, FXString);
    virtual void create();

    virtual ~ArchInputDialog();
    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    long onCmdOption(FXObject*, FXSelector, void*);
    long onUpdOption(FXObject*, FXSelector, void*);
    FXString getText()
    {
        return input->getText();
    }

    void setText(const FXString& text)
    {
        input->setText(text);
    }

    void selectAll()
    {
        input->setSelection(0, (input->getText()).length());
    }

    void CursorEnd()
    {
        input->onCmdCursorEnd(0, 0, 0);
    }
};
#endif
