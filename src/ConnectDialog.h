
#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include "IconList.h"
#include "DialogBox.h"
#include "ComboBox.h"
#include "DirPanel.h"
#include "FilePanel.h"


class ConnectDialog : public DialogBox
{
    FXDECLARE(ConnectDialog)

protected:

    FilePanel* filepanel = NULL;
    DirPanel* dirpanel = NULL;

    HistComboBox* server = NULL;
    FXLabel* portlabel = NULL;
    FXSpinner* port = NULL;
    HistComboBox* share = NULL;
    HistComboBox* domain = NULL;
    HistComboBox* user = NULL;
    FXTextField* passwd = NULL;
    FXCheckButton* savepasswd = NULL;
    FXButton* acceptbtn = NULL;
    ComboBox* servertype = NULL;
    FXLabel* sharelabel = NULL;
    FXLabel* domainlabel = NULL;

private:
    ConnectDialog()
    {
    }

public:
    enum
    {
        ID_ACCEPT=DialogBox::ID_LAST,
        ID_CANCEL,
        ID_SERVER_TYPE,
        ID_PORT,
        ID_SERVER_NAME,
        ID_SHARE_NAME,
        ID_FOLDER_NAME,
        ID_DOMAIN_NAME,
        ID_USER_NAME,
        ID_PASSWD,
        ID_SAVE_PASSWD,
        ID_LAST
    };
    ConnectDialog(FXWindow*, FXString, FilePanel*, DirPanel*);
    virtual void create();

    virtual ~ConnectDialog();
    FXuint execute(FXuint);
    long onCmdAccept(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdServerType(FXObject*, FXSelector, void*);
    long onUpdSavePasswd(FXObject*, FXSelector, void*);
    long onUpdAccept(FXObject*, FXSelector, void*);
    long smbMount(void);
    long sftpMount(void);
};
#endif
