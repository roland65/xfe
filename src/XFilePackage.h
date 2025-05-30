#ifndef XFILEPACKAGE_H
#define XFILEPACKAGE_H

#include "FileDialog.h"

class XFilePackage : public FXMainWindow
{
    FXDECLARE(XFilePackage)
protected:
    FXMenuBar* menubar = NULL;                      // Menu bar
    FXMenuPane* filemenu = NULL;                    // File menu
    FXMenuPane* helpmenu = NULL;                    // Help menu
    FXMenuPane* prefsmenu = NULL;                   // Preferences menu
    FXRecentFiles mrufiles;                         // Recent files
    FXToolBar* toolbar = NULL;                      // Toolbar
    FXString filename;                              // Current package name
    FXTreeList* list = NULL;                        // File list
    FXText* description = NULL;                     // Package description
    FXbool smoothscroll = false;
    FXbool errorflag = false;
    FXTabBook* tabbook = NULL;

    FXbool save_win_pos = false;                    // Save window position

protected:
    XFilePackage()
    {
    }
public:
    enum
    {
        ID_DESCRIPTION=FXMainWindow::ID_LAST,
        ID_RECENTFILE,
        ID_FILELIST,
        ID_UNINSTALL,
        ID_INSTALL,
        ID_ABOUT,
        ID_OPEN,
        ID_HARVEST,
        ID_QUIT,
        ID_TAB,
        ID_LAST
    };
    void start(FXString);
    void create();

    XFilePackage(FXApp*);
    ~XFilePackage();
    void setSmoothScroll(FXbool smooth)
    {
        smoothscroll = smooth;
    }

    long onCmdRecentFile(FXObject*, FXSelector, void*);
    long onCmdUninstall(FXObject*, FXSelector, void*);
    long onCmdInstall(FXObject*, FXSelector, void*);
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdOpen(FXObject*, FXSelector, void*);
    int readDescription();
    int readFileList();
    void saveConfig();

    long onSigHarvest(FXObject*, FXSelector, void*);
    long onCmdQuit(FXObject*, FXSelector, void*);
    long onCmdTabClicked(FXObject*, FXSelector, void*);
};

#endif
