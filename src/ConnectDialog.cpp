#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "icons.h"
#include "MessageBox.h"
#include "ConnectDialog.h"


// Server history
extern char ServerHistory[SERVER_HIST_SIZE][MAX_SERVER_SIZE];
extern int ServerNum;

// Share history
extern char ShareHistory[SHARE_HIST_SIZE][MAX_SHARE_SIZE];
extern int ShareNum;

// Domain history
extern char DomainHistory[DOMAIN_HIST_SIZE][MAX_DOMAIN_SIZE];
extern int DomainNum;

// User history
extern char UserHistory[USER_HIST_SIZE][MAX_USER_SIZE];
extern int UserNum;


FXDEFMAP(ConnectDialog) ConnectDialogMap[] =
{
    FXMAPFUNC(SEL_COMMAND, ConnectDialog::ID_ACCEPT, ConnectDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND, ConnectDialog::ID_CANCEL, ConnectDialog::onCmdCancel),
    FXMAPFUNC(SEL_UPDATE, ConnectDialog::ID_SAVE_PASSWD, ConnectDialog::onUpdSavePasswd),
    FXMAPFUNC(SEL_COMMAND, ConnectDialog::ID_SERVER_TYPE, ConnectDialog::onCmdServerType),
    FXMAPFUNC(SEL_UPDATE, ConnectDialog::ID_ACCEPT, ConnectDialog::onUpdAccept),
};


// Object implementation
FXIMPLEMENT(ConnectDialog, DialogBox, ConnectDialogMap, ARRAYNUMBER(ConnectDialogMap))

// Construct
ConnectDialog::ConnectDialog(FXWindow* win, FXString title, FilePanel* fpanel, DirPanel* dpanel) :
    DialogBox(win, title, DECOR_TITLE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE)
{
    // File and dir panels
    filepanel = fpanel;
    dirpanel = dpanel;

    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXGroupBox* group = new FXGroupBox(contents, _("Server Details"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXVerticalFrame* vframe = new FXVerticalFrame(group, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXMatrix* matrix = new FXMatrix(vframe, 4, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);

    new FXLabel(matrix, _("Server:") + FXString("     "), NULL, JUSTIFY_LEFT);

    server = new HistComboBox(matrix, 30, false, true, this, ID_SERVER_NAME,
                                COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X | LAYOUT_FILL_COLUMN |
                                LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    server->setNumVisible(5);
    for (int i = 0; i < ServerNum; i++)
    {
        server->appendItem(ServerHistory[i]);
    }

    portlabel = new FXLabel(matrix, FXString("    ") + _("PORT:") + FXString(" "), NULL, JUSTIFY_LEFT | LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW);
    port = new FXSpinner(matrix, 5, this, ID_PORT, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_FILL_ROW, 0, 0, 0, 0, 2,
                            2, 1, 1);
    port->setRange(0, 65535);
    port->setValue(0);
    portlabel->disable();
    port->disable();
    port->setTextColor(getApp()->getBorderColor());

    new FXLabel(matrix, _("Type:"), NULL, JUSTIFY_LEFT);

    servertype = new ComboBox(matrix, 2, false, false, this, ID_SERVER_TYPE, COMBOBOX_STATIC | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);
    servertype->setNumVisible(2);
    servertype->appendItem(_("Windows share"));
    servertype->appendItem(_("SSH"));
    servertype->setCurrentItem(0);

    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT);
    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT);

    sharelabel = new FXLabel(matrix, _("Share:"), NULL, JUSTIFY_LEFT);

    share = new HistComboBox(matrix, 30, false, true, this, ID_SHARE_NAME,
                                COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X | LAYOUT_FILL_COLUMN |
                                LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    share->setNumVisible(5);
    for (int i = 0; i < ShareNum; i++)
    {
        share->appendItem(ShareHistory[i]);
    }

    group = new FXGroupBox(contents, _("User Details"),
                                        GROUPBOX_TITLE_LEFT | FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    vframe = new FXVerticalFrame(group, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    matrix = new FXMatrix(vframe, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X);

    domainlabel = new FXLabel(matrix, _("Domain name:") + FXString("    "), NULL, JUSTIFY_LEFT);

    domain = new HistComboBox(matrix, 30, false, true, this, ID_DOMAIN_NAME,
                                COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X | LAYOUT_FILL_COLUMN |
                                LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    domain->setNumVisible(5);
    for (int i = 0; i < DomainNum; i++)
    {
        domain->appendItem(DomainHistory[i]);
    }

    new FXLabel(matrix, _("User name:") + FXString("    "), NULL, JUSTIFY_LEFT);

    user = new HistComboBox(matrix, 30, false, true, this, ID_USER_NAME,
                                COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X | LAYOUT_FILL_COLUMN |
                                LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    user->setNumVisible(5);
    for (int i = 0; i < UserNum; i++)
    {
        user->appendItem(UserHistory[i]);
    }

    new FXLabel(matrix, _("Password:") + FXString("    "), NULL, JUSTIFY_LEFT);

    passwd = new FXTextField(matrix, 30, this, ID_PASSWD,
                               TEXTFIELD_NORMAL | TEXTFIELD_PASSWD | LAYOUT_FILL_COLUMN | LAYOUT_FILL_X |
                               LAYOUT_CENTER_Y);

    new FXLabel(matrix, "", NULL, JUSTIFY_LEFT);
    savepasswd = new FXCheckButton(matrix, _("Save password"), this, ID_SAVE_PASSWD);

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(contents, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10,
                                                       5, 5);

    // Accept button
    acceptbtn = new FXButton(buttons, _("&Accept"), NULL, this, ConnectDialog::ID_ACCEPT,
                             FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    acceptbtn->addHotKey(KEY_Return);

    // Cancel button
    new FXButton(buttons, _("&Cancel"), NULL, this, ConnectDialog::ID_CANCEL,
                 FRAME_GROOVE | LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);

}


// Create window
void ConnectDialog::create()
{
    // Create dialog box
    DialogBox::create();

    // Set focus to the first empty field
    if (server->getText() == "")
    {
        server->CursorEnd();
    }
    else if (share->getText() == "")
    {
        share->CursorEnd();        
    }
    else if (domain->getText() == "")
    {
        domain->CursorEnd();
    }
    else if (user->getText() == "")
    {
        user->CursorEnd();
    }
    else
    {
        passwd->setFocus();
    }
}


// Delete objects
ConnectDialog::~ConnectDialog()
{
    delete acceptbtn;
    delete server;
    delete servertype;
    delete portlabel;
    delete port;
    delete sharelabel;
    delete domainlabel;
    delete domain;
    delete user;
    delete passwd;
    delete savepasswd;
}


// Changes are accepted
long ConnectDialog::onCmdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    DialogBox::onCmdAccept(sender, sel, ptr);

    // SMB server
    if (servertype->getText() == _("Windows share"))
    {
        return smbMount();
    }

    // SSH server
    if (servertype->getText() == _("SSH"))
    {
        return sftpMount();
    }
   
    return 0;
}


// Mount SMB share
long ConnectDialog::smbMount(void)
{
    // Password is empty, try to retrieve secret using secret-tool
    if (passwd->getText() == "")
    {
        FXString cmd = FXString("secret-tool search") +
                       " sharetype " + "smb" +
                       " server " + server->getText() +
                       " share " + xf_quote(share->getText()) +
                       " user " + xf_quote(user->getText());
        
        FXString ret = xf_getcommandoutput(cmd);

        if (ret != "")
        {   
            // secret-tool utility not found
            if (ret.find("secret-tool: not found") != -1)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Password cannot be retrieved because the secret-tool utility\
 was not found on your system.\n\nPlease install the package that provides secret-tool\
 (libsecret-tools on Debian based systems, libsecret on RedHat based systems)..."));
                execute(PLACEMENT_OWNER);
                return 1;                
            }

            // Secret found
            else
            {
                // Domain
                int pos1 = ret.find("attribute.domain = ") + 19;
                int pos2 = ret.find('\n', pos1);           
                domain->setText(ret.mid(pos1, pos2 - pos1));

                // User
                pos1 = ret.find("attribute.user = ") + 17;
                pos2 = ret.find('\n', pos1);
                user->setText(ret.mid(pos1, pos2 - pos1));

                // Password
                pos1 = ret.find("secret = ") + 9;
                pos2 = ret.find('\n', pos1);
                passwd->setText(ret.mid(pos1, pos2 - pos1));
            
                // Blank string
                ret = "";
            }
        }
        
        // Secret not found
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Connection failed, please check your parameters..."));
            execute(PLACEMENT_OWNER);
            return 1;
        }
    }

    // Connect to server using gio mount
    FXString cmd = FXString("echo '") +
                   passwd->getText() + "\\n' | " +
                   "gio mount smb://" +
                   domain->getText() + "\\;" +
                   xf_quote(user->getText()) + "@" +
                   server->getText() + "/" +
                   xf_quote(share->getText()) + "/";
   
    FXString ret = xf_getcommandoutput(cmd);

    // gio not found
    if (ret.contains("gio: not found"))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("gio command was not found on your system.\n\n\
Please install gio and gvfs to be able to connect to a server..."));
        return 0;                
    }

    // Connection failed
    else if (ret.contains("gio:"))
    {
        // Keep only significant part of the message
        FXString str = ret.rbefore('\n');
        str = str.rafter('\n');

        MessageBox::error(this, BOX_OK, _("Error"), "%s", str.text());
        return 0;
    }

    // Connection succeeded, enter mount point
    FXString gvfspath = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";
    FXString mntpath = gvfspath + PATHSEPSTRING + "smb-share:domain=" + domain->getText() +
           ",server=" + server->getText().lower() +
           ",share=" + share->getText().substitute(" ", "%20").lower() +
           ",user=" + user->getText().substitute(" ", "%20");

    // Set directory
    filepanel->getCurrent()->setDirectory(mntpath);
    dirpanel->setDirectory(mntpath, true);

    // If asked, save password using secret-tool
    if (savepasswd->getCheck())
    {
        FXString cmd = FXString("printf '") +
                       passwd->getText() +
                       "' | secret-tool store --label='SMB Share'" +
                       " sharetype " + "smb" +
                       " server " + server->getText() +
                       " share " + xf_quote(share->getText()) +
                       " domain " + domain->getText() +
                       " user " + xf_quote(user->getText());
                       
        FXString ret = xf_getcommandoutput(cmd);

        // secret-tool utility not found
        if (ret.contains("secret-tool: not found"))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Password cannot be saved because the secret-tool utility\
 was not found on your system.\n\nPlease install the package that provides secret-tool\
 (libsecret-tools on Debian based systems, libsecret on RedHat based systems)..."));
             return 0;                
        }
    }

    // Blank password
    passwd->setText("");

    return 1;
}


// Mount SSH share
long ConnectDialog::sftpMount(void)
{
    // Password is empty, try to retrieve secret using secret-tool
    if (passwd->getText() == "")
    {
        FXString cmd = FXString("secret-tool search") +
                       " sharetype " + "sftp" +
                       " server " + server->getText() +
                       " user " + user->getText() +
                       " port " + FXStringVal(port->getValue());
        
        FXString ret = xf_getcommandoutput(cmd);

        if (ret != "")
        {   
            // secret-tool utility not found
            if (ret.find("secret-tool: not found") != -1)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Password cannot be retrieved because the secret-tool utility\
 was not found on your system.\n\nPlease install the package that provides secret-tool\
 (libsecret-tools on Debian based systems, libsecret on RedHat based systems)..."));
                execute(PLACEMENT_OWNER);
                return 1;                
            }

            // Secret found
            else
            {
                // User
                int pos1 = ret.find("attribute.user = ") + 17;
                int pos2 = ret.find('\n', pos1);
                user->setText(ret.mid(pos1, pos2 - pos1));
            
                // Password
                pos1 = ret.find("secret = ") + 9;
                pos2 = ret.find('\n', pos1);
                passwd->setText(ret.mid(pos1, pos2 - pos1));
            
                // Blank string
                ret = "";
            }
        }
        
        // Secret not found
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Connection failed, please check your parameters..."));
            execute(PLACEMENT_OWNER);
            return 1;
        }
    }

    // Connect to server using gio mount
    FXString cmd;
    if (port->getValue() != 22)
    {
        cmd = FXString("echo '") +
                   passwd->getText() + "\\n' | " +
                   "gio mount sftp://" +
                   user->getText() + "@" +
                   server->getText() +
                   ":" + FXStringVal(port->getValue());
    }
    else
    {
        cmd = FXString("echo '") +
                   passwd->getText() + "\\n' | " +
                   "gio mount sftp://" +
                   user->getText() + "@" +
                   server->getText();
    }
    
    FXString ret = xf_getcommandoutput(cmd);

    // gio not found
    if (ret.contains("gio: not found"))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("gio command was not found on your system.\n\n\
Please install gio and gvfs to be able to connect to a server..."));
        return 0;                
    }

    // Connection failed
    else if (ret.contains("gio:"))
    {
        // Keep only significant part of the message
        FXString str = ret.rbefore('\n');
        str = str.rafter('\n');

        MessageBox::error(this, BOX_OK, _("Error"), "%s", str.text());
        return 0;
    }

    // Connection succeeded, enter mount point
    FXString gvfspath = "/run/user/" + FXStringVal(FXSystem::user()) + "/gvfs";
    
    FXString mntpath;
    if (port->getValue() != 22)
    {
        mntpath = gvfspath + PATHSEPSTRING + "sftp:host=" + server->getText() +
               ",port=" + FXStringVal(port->getValue()) +
               ",user=" + user->getText();
    }
    else
    {
        mntpath = gvfspath + PATHSEPSTRING + "sftp:host=" + server->getText() +
               ",user=" + user->getText();
    }

    // Set directory
    filepanel->getCurrent()->setDirectory(mntpath);
    dirpanel->setDirectory(mntpath, true);

    // If asked, save password using secret-tool
    if (savepasswd->getCheck())
    {
        FXString cmd = FXString("printf '") +
                       passwd->getText() +
                       "' | secret-tool store --label='SSH Share'" +
                       " sharetype " + "sftp" +
                       " server " + server->getText() +
                       " user " + xf_quote(user->getText()) +
                       " port " + FXStringVal(port->getValue());
                       
        FXString ret = xf_getcommandoutput(cmd);

        // secret-tool utility not found
        if (ret.contains("secret-tool: not found"))
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("Password cannot be saved because the secret-tool utility\
 was not found on your system.\n\nPlease install the package that provides secret-tool\
 (libsecret-tools on Debian based systems, libsecret on RedHat based systems)..."));
             return 0;                
        }
    }   

    // Blank password
    passwd->setText("");

    return 1;
}


// Changes are cancelled
long ConnectDialog::onCmdCancel(FXObject* sender, FXSelector sel, void* ptr)
{
    // Blank password
    passwd->setText("");
    
    DialogBox::onCmdCancel(sender, sel, ptr);
    return 0;
}


// Execute dialog box modally
FXuint ConnectDialog::execute(FXuint placement)
{
    // Blank password
    passwd->setText("");

    // Execute dialog
    FXuint ret = DialogBox::execute(placement);

    return ret;
}


// Update save password check button
long ConnectDialog::onUpdSavePasswd(FXObject* sender, FXSelector sel, void* ptr)
{
    // Disable button if password is empty
    if (passwd->getText() == "")
    {
        savepasswd->disable();
    }
    else
    {
        savepasswd->enable();
    }

    return 1;
}


// Update dialog items according to server type
long ConnectDialog::onCmdServerType(FXObject* sender, FXSelector sel, void* ptr)
{
    // SMB server
    if (servertype->getText() == _("Windows share"))
    {
        portlabel->disable();
        port->disable();
        port->setTextColor(getApp()->getBorderColor());
        port->setValue(0);
        sharelabel->show();
        share->show();
        domainlabel->show();
        domain->show();
    }

    // SSH server
    if (servertype->getText() == _("SSH"))
    {
        portlabel->enable();
        port->enable();
        port->setTextColor(getApp()->getForeColor());       
        if (port->getValue() == 0)
        {
            port->setValue(22);  // Default port is 22
        }
        sharelabel->hide();
        share->hide();
        domainlabel->hide();
        domain->hide();
    }

    // Force widget redrawing (don't know how to do it the right way)
    resize(getWidth() - 1, getHeight() - 1);

    return 1;
}


// Update accept button
long ConnectDialog::onUpdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    if (servertype->getText() == _("Windows share"))
    {
        // Disable button if server name or share name are empty
        if (server->getText() == "" || share->getText() == "")
        {
            acceptbtn->disable();
        }
        else
        {
            acceptbtn->enable();
        }
    }
    
    if (servertype->getText() == _("SSH"))
    {
        // Disable button if server name is empty
        if (server->getText() == "")
        {
            acceptbtn->disable();
        }
        else
        {
            acceptbtn->enable();
        }
    }
  
    // Sort server history
    server->setSortFunc(FXList::ascendingCase);
    server->sortItems();

    // Update server history
    ServerNum = 0;
    if (server->getNumItems() > 0)
    {
        for (int i = 0; i < server->getNumItems(); i++)
        {
            if (ServerNum > SERVER_HIST_SIZE - 1)
            {
                break;
            }
            FXString str = server->getItemText(i);
            xf_strlcpy(ServerHistory[i], str.text(), str.length() + 1);
            ServerNum++;
        }
    }

    // Sort share history
    share->setSortFunc(FXList::ascendingCase);
    share->sortItems();

    // Update share history
    ShareNum = 0;
    if (share->getNumItems() > 0)
    {
        for (int i = 0; i < share->getNumItems(); i++)
        {
            if (ShareNum > SHARE_HIST_SIZE - 1)
            {
                break;
            }
            FXString str = share->getItemText(i);
            xf_strlcpy(ShareHistory[i], str.text(), str.length() + 1);
            ShareNum++;
        }
    }

    // Sort domain history
    domain->setSortFunc(FXList::ascendingCase);
    domain->sortItems();

    // Update domain history
    DomainNum = 0;
    if (domain->getNumItems() > 0)
    {
        for (int i = 0; i < domain->getNumItems(); i++)
        {
            if (DomainNum > DOMAIN_HIST_SIZE - 1)
            {
                break;
            }
            FXString str = domain->getItemText(i);
            xf_strlcpy(DomainHistory[i], str.text(), str.length() + 1);
            DomainNum++;
        }
    }

    // Sort user history
    user->setSortFunc(FXList::ascendingCase);
    user->sortItems();

    // Update user history
    UserNum = 0;
    if (user->getNumItems() > 0)
    {
        for (int i = 0; i < user->getNumItems(); i++)
        {
            if (UserNum > USER_HIST_SIZE - 1)
            {
                break;
            }
            FXString str = user->getItemText(i);
            xf_strlcpy(UserHistory[i], str.text(), str.length() + 1);
            UserNum++;
        }
    }

    return 1;
}
