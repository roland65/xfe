#include "config.h"
#include "i18n.h"

#include "TabButtons.h"
#include "MessageBox.h"


extern FXString homedir;


FXDEFMAP(TabButtons) TabButtonsMap[] =
{
    FXMAPFUNC(SEL_COMMAND, TabButtons::ID_NEW_TAB, TabButtons::onCmdNewTab),
    FXMAPFUNC(SEL_COMMAND, TabButtons::ID_REMOVE_ALL_TABS, TabButtons::onCmdRemoveAllTabs),
    FXMAPFUNCS(SEL_COMMAND, TabButtons::ID_START_TAB, TabButtons::ID_END_TAB, TabButtons::onCmdPathButton),
    FXMAPFUNCS(SEL_COMMAND, TabButtons::ID_START_CLOSE, TabButtons::ID_END_CLOSE, TabButtons::onCmdCloseButton),
    FXMAPFUNCS(SEL_RIGHTBUTTONRELEASE, TabButtons::ID_START_TAB, TabButtons::ID_END_TAB, TabButtons::onCmdPopupMenu),
    FXMAPFUNCS(SEL_RIGHTBUTTONRELEASE, TabButtons::ID_START_CLOSE, TabButtons::ID_END_CLOSE, TabButtons::onCmdPopupMenu),
    FXMAPFUNCS(SEL_QUERY_TIP, TabButtons::ID_START_TAB, TabButtons::ID_END_TAB, TabButtons::onQueryTip),
    FXMAPFUNCS(SEL_COMMAND, TabButtons::ID_START_MOVE_LEFT, TabButtons::ID_END_MOVE_LEFT, TabButtons::onCmdMoveLeft),
    FXMAPFUNCS(SEL_COMMAND, TabButtons::ID_START_MOVE_RIGHT, TabButtons::ID_END_MOVE_RIGHT, TabButtons::onCmdMoveRight),
    FXMAPFUNCS(SEL_UPDATE, TabButtons::ID_START_MOVE_LEFT, TabButtons::ID_END_MOVE_LEFT, TabButtons::onUpdMoveLeft),
    FXMAPFUNCS(SEL_UPDATE, TabButtons::ID_START_MOVE_RIGHT, TabButtons::ID_END_MOVE_RIGHT, TabButtons::onUpdMoveRight),
    FXMAPFUNCS(SEL_UPDATE, TabButtons::ID_START_TAB, TabButtons::ID_END_TAB, TabButtons::onUpdCurrentTab),
};

FXIMPLEMENT(TabButtons, FXHorizontalFrame, TabButtonsMap, ARRAYNUMBER(TabButtonsMap))


// Construct
TabButtons::TabButtons(FXComposite* a, FilePanel* fpanel, DirPanel* dpanel, FXuint opts) :
    FXHorizontalFrame(a, opts, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2)
{
    // Frame background colors of selected and unselected tab
    basecolor = getApp()->getBaseColor();
    FXuint r = FXREDVAL(basecolor);
    FXuint g = FXGREENVAL(basecolor);
    FXuint b = FXBLUEVAL(basecolor);
    shadecolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);

    // File and dir panels
    filepanel = fpanel;
    dirpanel = dpanel;

    // Allocate vector of paths
    tabPaths = new FXString[NMAX_TABS];

    // Add tab and close buttons
    int idTab = ID_START_TAB;
    int idClose = ID_START_CLOSE;
    for (int i = 0; i < NMAX_TABS; i++)
    {
        FXHorizontalFrame* buttons = new FXHorizontalFrame(this, FRAME_GROOVE | LAYOUT_SIDE_TOP | LAYOUT_LEFT |
                                                           LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2);
        tabButtons.push_back(new FXButton(buttons, "", NULL, this, idTab, BUTTON_TOOLBAR, 0, 0, 0, 0, 4, 4, 2, 2));
        closeButtons.push_back(new FXButton(buttons, "", minicloseicon, this, idClose, BUTTON_TOOLBAR,
                               0, 0, 0, 0, 0, 0, 2, 2));

        idTab++;
        idClose++;
        
        tabButtons[i]->hide();
        tabButtons[i]->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
        tabButtons[i]->getParent()->hide();

        closeButtons[i]->hide();
    }

    // New tab button
    FXString key = getApp()->reg().readStringEntry("KEYBINDINGS", "new_tab", "Shift-F1");
    FXString str = FXString(_("\tNew Tab")) + " (" + key + ")";
    new FXButton(this, str, miniaddicon, this, ID_NEW_TAB, BUTTON_TOOLBAR | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 2, 2);

    // Initializations
    nbActiveTabs = 0;
    currentTab = 0;
    prevTab = 0;

    // Create highlight font (bold if normal font is normal, and normal if normal font is bold)
    FXFontDesc fontdesc;
    normalFont = getApp()->getNormalFont();
    normalFont->getFontDesc(fontdesc);
    if (fontdesc.weight == FXFont::Normal)
    {
        fontdesc.weight = FXFont::Bold;
    }
    else
    {
        fontdesc.weight = FXFont::Normal;
    }
    highlightFont = new FXFont(getApp(), fontdesc);

}


// Create tab buttons
void TabButtons::create()
{
    highlightFont->create();
    FXHorizontalFrame::create();   
}


// Destruct object
TabButtons::~TabButtons()
{
    delete highlightFont;
    delete tabPaths;    
}


// Add new tab
void TabButtons::addTab(FXString pathname)
{
    if (nbActiveTabs < NMAX_TABS)
    {
        // Update previous and current tab indices
        FXuint index = nbActiveTabs++;
        prevTab = currentTab;
        currentTab = index;

        // Show buttons
        tabButtons[index]->show();
        tabButtons[index]->getParent()->show();
        closeButtons[index]->show();

        // Tab path
        tabPaths[index] = pathname;

        // Tab label
        if (pathname == PATHSEPSTRING)
        {
            tabButtons[index]->setText(PATHSEPSTRING);
        }
        else
        {
            FXString name = FXPath::name(pathname);

            // SMB mount
            if (name.left(10) == "smb-share:")
            {
                FXString tmp;
                int pos = 0;

                // Server name
                pos = name.find("server=") + 7;
                tmp = name.right(name.length() - pos);
                FXString server = tmp.before(',');

                // Share name
                pos = name.find("share=") + 6;
                tmp = name.right(name.length() - pos);
                FXString share = tmp.before(',');
                
                // Tab label
                FXString label = share.substitute("%20", " ") + "@" + server;
                tabButtons[index]->setText(label);
            }
            
            // MTP mount
            else if (name.left(4) == "mtp:")
            {
                // Tab label
                FXString label = name.after('_');
                label = label.before('_', 2);
                tabButtons[index]->setText(label);
            }
            
            // Other
            else
            {
                tabButtons[index]->setText(name);
            }
        }

        // Set directory
        filepanel->getCurrent()->setDirectory(pathname);
        dirpanel->setDirectory(pathname, true);

        // Change font and state
        tabButtons[prevTab]->setFont(normalFont);
        tabButtons[prevTab]->setState(STATE_UP);
        closeButtons[prevTab]->setState(STATE_UP);
        tabButtons[currentTab]->setFont(highlightFont);
        tabButtons[currentTab]->setState(STATE_DOWN);
        closeButtons[currentTab]->setState(STATE_DOWN);

        // Change frame background color
        closeButtons[prevTab]->getParent()->setBackColor(basecolor);
        closeButtons[currentTab]->getParent()->setBackColor(shadecolor);
    } 
}


// Path button was pressed
long TabButtons::onCmdPathButton(FXObject* sender, FXSelector sel, void* ptr)
{
    // Set the focus on the file list
    filepanel->getCurrent()->getList()->setFocus();

    // Index of pressed button
    FXuint index = FXSELID(sel) - ID_START_TAB;
    prevTab = currentTab;
    currentTab = index;

    // Update the FileList and DirList directory
    FXString filePath = tabPaths[index];

    filepanel->getCurrent()->setDirectory(filePath);
    dirpanel->setDirectory(filePath, true);

    // Change fonts and states
    tabButtons[prevTab]->setFont(normalFont);
    tabButtons[prevTab]->setState(STATE_UP);
    closeButtons[prevTab]->setState(STATE_UP);
    tabButtons[currentTab]->setFont(highlightFont);
    tabButtons[currentTab]->setState(STATE_DOWN);
    closeButtons[currentTab]->setState(STATE_DOWN);

    // Change frame background color
    closeButtons[prevTab]->getParent()->setBackColor(basecolor);
    closeButtons[currentTab]->getParent()->setBackColor(shadecolor);

    return 1;
}


// Close button was pressed
long TabButtons::onCmdCloseButton(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of pressed button
    FXuint index = FXSELID(sel) - ID_START_CLOSE;
    
    // Change fonts and states
    tabButtons[index]->setFont(normalFont);
    tabButtons[index]->setState(STATE_UP);
    closeButtons[index]->setState(STATE_UP);
   
    // Shift labels and paths
    for (FXuint i = index; i < nbActiveTabs - 1; i++)
    {
        tabButtons[i]->setText(tabButtons[i + 1]->getText());
        tabPaths[i] = tabPaths[i + 1];
    }

    // Hide last button and reset its label and path
    tabButtons[nbActiveTabs - 1]->hide();
    tabButtons[nbActiveTabs - 1]->getParent()->hide();
    closeButtons[nbActiveTabs - 1]->hide();
    tabButtons[nbActiveTabs - 1]->setText("");
    tabPaths[nbActiveTabs - 1] = "";
    nbActiveTabs--;

    // Update previous and current tab
    if (index < currentTab)
    {
        prevTab = currentTab;
        currentTab = (currentTab <= 1 ? 0 : currentTab - 1);
    }
    
    if (index == currentTab)
    {
        currentTab = (currentTab <= 1 ? 0 : currentTab - 1);
        filepanel->getCurrent()->setDirectory(tabPaths[currentTab]);
    }
    
    // Change fonts and states
    tabButtons[prevTab]->setFont(normalFont);
    tabButtons[prevTab]->setState(STATE_UP);
    closeButtons[prevTab]->setState(STATE_UP);
    tabButtons[currentTab]->setFont(highlightFont);
    tabButtons[currentTab]->setState(STATE_DOWN);
    closeButtons[currentTab]->setState(STATE_DOWN);

    // Change frame background color
    closeButtons[prevTab]->getParent()->setBackColor(basecolor);
    closeButtons[currentTab]->getParent()->setBackColor(shadecolor);

    return 1;
}


// Add a new tab
long TabButtons::onCmdNewTab(FXObject*, FXSelector, void*)
{
    // Read new tab directory mode
    FXuint newtabdirmode = getApp()->reg().readUnsignedEntry("OPTIONS", "newtabdir_mode", NEWTAB_HOMEDIR);

    FXString pathname;
    if (newtabdirmode == NEWTAB_HOMEDIR)
    {
        // Home directory
        pathname = homedir;
    }
    else if (newtabdirmode == NEWTAB_CURRENTDIR)
    {
        // Current path
        pathname = filepanel->getCurrent()->getDirectory();
    }
    else // Root directory
    {
        pathname = PATHSEPSTRING;
    }

    // Add new tab
    addTab(pathname);

    return 1;
}


// Remove all tabs
long TabButtons::onCmdRemoveAllTabs(FXObject*, FXSelector, void*)
{
    if (BOX_CLICKED_CANCEL == MessageBox::question(FXApp::instance()->getActiveWindow(), BOX_OK_CANCEL,
                                                   _("Confirm Remove Tabs"),
                                                   _("Do you really want to remove all tabs?")))
    {
        return 0;
    }
    else
    {
        for (FXuint i = 0; i < nbActiveTabs; i++)
        {
            tabButtons[i]->setText("");
            tabPaths[i] = "";
            tabButtons[i]->hide();
            tabButtons[i]->getParent()->hide();
            closeButtons[i]->hide();        
        }
        
        nbActiveTabs = 0;
        prevTab = 0;
        currentTab = 0;
    }

    return 1;
}


// Update current tab path
long TabButtons::onUpdCurrentTab(FXObject* senderbj, FXSelector sel, void* ptr)
{
    // Current path
    FXString pathname = filepanel->getCurrent()->getDirectory();
    
    // Set current name and path
    tabPaths[currentTab] = pathname;
    if (pathname == PATHSEPSTRING)
    {
        tabButtons[currentTab]->setText(PATHSEPSTRING);
    }
    else
    {
        FXString name = FXPath::name(pathname);

        // SMB mount
        if (name.left(10) == "smb-share:")
        {
            FXString tmp;
            int pos = 0;

            // Server name
            pos = name.find("server=") + 7;
            tmp = name.right(name.length() - pos);
            FXString server = tmp.before(',');

            // Share name
            pos = name.find("share=") + 6;
            tmp = name.right(name.length() - pos);
            FXString share = tmp.before(',');
            
            // Tab label
            FXString label = share.substitute("%20", " ") + "@" + server;
            tabButtons[currentTab]->setText(label);
        }
        
        // MTP mount
        else if (name.left(4) == "mtp:")
        {
            // Tab label
            FXString label = name.after('_');
            label = label.before('_', 2);
            tabButtons[currentTab]->setText(label);
        }
        
        // Other
        else
        {
            tabButtons[currentTab]->setText(name);
        }
    }

    return 0;
}


// Display path tooltip
long TabButtons::onQueryTip(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of hovered button
    FXuint index = FXSELID(sel) - ID_START_TAB;

    FXString tip = tabPaths[index];

    sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);

    return 1;
}


// Context menu
long TabButtons::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of button
    FXuint id = FXSELID(sel);
    FXuint index;
    if (id >= ID_START_TAB && id <= ID_END_TAB)
    {
        index = id - ID_START_TAB;    // Tab button
    }
    else
    {
        index = id - ID_START_CLOSE;  // Close button
    }

    int x, y;
    FXuint state;

    getCursorPosition(x, y, state);

    // Menu items
    FXMenuPane* menu = new FXMenuPane(this);

    getRoot()->getCursorPosition(x, y, state);

    new FXMenuCommand(menu, _("New &Tab"), mininewtabicon, this, TabButtons::ID_NEW_TAB);
    
    new FXMenuSeparator(menu);

    new FXMenuCommand(menu, _("Move &Left"), minidirbackicon, this, index + ID_START_MOVE_LEFT);
    new FXMenuCommand(menu, _("Move &Right"), minidirforwardicon, this, index + ID_START_MOVE_RIGHT);

    new FXMenuSeparator(menu);
    
    new FXMenuCommand(menu, _("&Close Tab"), minicloseicon, this, index + ID_START_CLOSE);

    menu->create();
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);

    delete(menu);

    return 1;
}


// Move tab left
long TabButtons::onCmdMoveLeft(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of button
    FXuint index = FXSELID(sel) - ID_START_MOVE_LEFT;

    // Shift left
    if (index > 0)
    {
        FXString left_label = tabButtons[index - 1]->getText();
        FXString left_path = tabPaths[index - 1];
        
        tabButtons[index - 1]->setText(tabButtons[index]->getText());
        tabPaths[index - 1] = tabPaths[index];

        tabButtons[index]->setText(left_label);
        tabPaths[index] = left_path;        

        // Update previous and current tab
        if (index == currentTab)
        {
            prevTab = index;
            currentTab = index - 1;
            filepanel->getCurrent()->setDirectory(tabPaths[currentTab]);

        }
        else if (index == currentTab + 1)
        {
            prevTab = currentTab;
            currentTab = index;
            filepanel->getCurrent()->setDirectory(tabPaths[currentTab]);
        }        

        // Change fonts and states
        tabButtons[prevTab]->setFont(normalFont);
        tabButtons[prevTab]->setState(STATE_UP);
        closeButtons[prevTab]->setState(STATE_UP);
        tabButtons[currentTab]->setFont(highlightFont);
        tabButtons[currentTab]->setState(STATE_DOWN);
        closeButtons[currentTab]->setState(STATE_DOWN);
    }

    return 1;
}


// Move tab right
long TabButtons::onCmdMoveRight(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of button
    FXuint index = FXSELID(sel) - ID_START_MOVE_RIGHT;

    // Shift right
    if (index < nbActiveTabs - 1)
    {
        FXString right_label = tabButtons[index + 1]->getText();
        FXString right_path = tabPaths[index + 1];
        
        tabButtons[index + 1]->setText(tabButtons[index]->getText());
        tabPaths[index + 1] = tabPaths[index];

        tabButtons[index]->setText(right_label);
        tabPaths[index] = right_path;        

        // Update previous and current tab
        if (index == currentTab)
        {
            prevTab = index;
            currentTab = index + 1;
            filepanel->getCurrent()->setDirectory(tabPaths[currentTab]);
        }
        else if (index == currentTab - 1)
        {
            prevTab = currentTab;
            currentTab = index;
            filepanel->getCurrent()->setDirectory(tabPaths[currentTab]);
        }        

        // Change fonts and states
        tabButtons[prevTab]->setFont(normalFont);
        tabButtons[prevTab]->setState(STATE_UP);
        closeButtons[prevTab]->setState(STATE_UP);
        tabButtons[currentTab]->setFont(highlightFont);
        tabButtons[currentTab]->setState(STATE_DOWN);
        closeButtons[currentTab]->setState(STATE_DOWN);
    }

    return 1;
}


// Update move left menu item
long TabButtons::onUpdMoveLeft(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of button
    FXuint index = FXSELID(sel) - ID_START_MOVE_LEFT;

    if (index < 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);        
    }

    return 0;
}


// Update move right menu item
long TabButtons::onUpdMoveRight(FXObject* sender, FXSelector sel, void* ptr)
{
    // Index of button
    FXuint index = FXSELID(sel) - ID_START_MOVE_RIGHT;

    if (index >= nbActiveTabs - 1)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);        
    }
    
    return 0;
}


// Set current tab from index
void TabButtons::setCurrentTab(FXuint index)
{
    // Update previous and current tab indices
    prevTab = currentTab;
    currentTab = index;

    // Show buttons
    tabButtons[index]->show();
    tabButtons[index]->getParent()->show();
    closeButtons[index]->show();

    // Set directory
    FXString pathname = tabPaths[index];
    filepanel->getCurrent()->setDirectory(pathname);
    dirpanel->setDirectory(pathname, true);

    // Change font and state
    tabButtons[prevTab]->setFont(normalFont);
    tabButtons[prevTab]->setState(STATE_UP);
    closeButtons[prevTab]->setState(STATE_UP);
    tabButtons[currentTab]->setFont(highlightFont);
    tabButtons[currentTab]->setState(STATE_DOWN);
    closeButtons[currentTab]->setState(STATE_DOWN);
}
