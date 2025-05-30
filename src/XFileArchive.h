#ifndef XFILEARCHIVE_H
#define XFILEARCHIVE_H

#include <map>

#include "FileDialog.h"


// Typedef for the map between program string identifiers and integer indexes
typedef std::map<FXString, int>   progsmap;


class XFileArchive : public FXMainWindow
{
    FXDECLARE(XFileArchive)
protected:
    int pid = 0;                                        // Proccess ID of child (valid if busy).
    int pipes[2] = { 0, 0 };                            // Pipes to communicate with child process.
    FXMenuBar* menubar = NULL;                          // Menu bar
    FXMenuPane* filemenu = NULL;                        // File menu
    FXMenuPane* helpmenu = NULL;                        // Help menu
    FXMenuPane* prefsmenu = NULL;                       // Preferences menu
    FXToolBar* toolbar = NULL;                          // Toolbar
    FXButton* extractbtn = NULL;                        // Extract button
    FXButton* extracttobtn = NULL;                      // Extract to button
    FXRecentFiles mrufiles;                             // Recent files
    HistInputDialog* opendialog = NULL;                 // Open dialog
    FXString archpathname;                              // Current archive path name
    FXString tmpdir;                                    // tmp directory
    FXString tmppathname;                               // Path name of the tmp file to open
    FXTreeList* list = NULL;                            // Tree list
    FXTreeItem* topmost = NULL;                         // Top tree item
    FXLabel* status = NULL;                             // Status text
    FXString ext1;                                      // Archive double extension like tar.gz
    FXString ext2;                                      // Archive simple extension like tgz or zip
    FXString linefrag;                                  // Line fragment for pipe reading
    FXString rotcur;                                    // Status rotating cursor
    FXbool opening = false;                             // Flag for opening
    CommandWindow* cmdwin = NULL;                       // Command window for extracting
    progsmap progs;                                     // Map between program string identifiers and integer indexes
    FXbool smoothscroll = false;
    FXbool errorflag = false;

    FXuint single_click = SINGLE_CLICK_NONE;            // Single click navigation
    FXbool save_win_pos = false;                        // Save window position

protected:
    XFileArchive()
    {
    }
public:
    enum
    {
        ID_LIST=FXMainWindow::ID_LAST,
        ID_RECENTFILE,
        ID_EXTRACT_TO,
        ID_EXTRACT,
        ID_ABOUT,
        ID_OPEN,
        ID_HARVEST,
        ID_WATCHPROCESS,
        ID_FILE_OPEN,
        ID_FILE_OPEN_WITH,
        ID_FILE_EXTRACT,
        ID_FILE_EXTRACT_TO,
        ID_QUIT,
        ID_LAST
    };
    void start(FXString);
    void create();

    XFileArchive(FXApp*);
    ~XFileArchive();
    void setSmoothScroll(FXbool smooth)
    {
        smoothscroll = smooth;
    }

    long onCmdRecentFile(FXObject*, FXSelector, void*);
    long onCmdExtractTo(FXObject*, FXSelector, void*);
    long onCmdExtract(FXObject*, FXSelector, void*);
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdOpen(FXObject*, FXSelector, void*);
    long onWatchProcess(FXObject*, FXSelector, void*);
    long onQueryTip(FXObject*, FXSelector, void*);
    long onSigHarvest(FXObject*, FXSelector, void*);
    long onCmdQuit(FXObject*, FXSelector, void*);
    long onUpdOpen(FXObject*, FXSelector, void*);
    long onUpdExtractTo(FXObject*, FXSelector, void*);
    long onUpdExtract(FXObject*, FXSelector, void*);
    long onUpdQuit(FXObject*, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdClicked(FXObject*, FXSelector, void*);
    long onCmdDoubleClicked(FXObject*, FXSelector, void*);
    long onCmdFileOpen(FXObject*, FXSelector, void*);
    long onCmdFileOpenWith(FXObject*, FXSelector, void*);
    long onCmdFileExtract(FXObject*, FXSelector, void*);
    long onCmdFileExtractTo(FXObject*, FXSelector, void*);

    int execCmd(FXString);
    int readArchive(void);
    void saveConfig(void);
};

#endif
