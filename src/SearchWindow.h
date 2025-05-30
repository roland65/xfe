#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include "ComboBox.h"
#include "SearchPanel.h"

// Search window
class FXAPI SearchWindow : public FXTopWindow
{
    FXDECLARE(SearchWindow)
public:
    enum
    {
        ID_CANCEL = FXTopWindow::ID_LAST,
        ID_START,
        ID_STOP,
        ID_BROWSE_PATH,
        ID_READ_DATA,
        ID_CLOSE,
        ID_MORE_OPTIONS,
        ID_SIZE,
        ID_PERMS,
        ID_RESET_OPTIONS,
        ID_ROTATING_CURSOR,
        ID_LAST
    };
    SearchWindow(FXApp* app, const FXString& name, int x = 0, int y = 0, int w = 0, int h = 0,
                 int pl = 10, int pr = 10, int pt = 10, int pb = 10, int hs = 4, int vs = 4);
    virtual void show(FXuint placement = PLACEMENT_CURSOR);
    virtual void create();

    virtual ~SearchWindow();
protected:
    FXApp* application = NULL;
    FXLabel* searchresults = NULL;
    HistComboBox* findfile = NULL;
    FXTextField* wheredir = NULL;
    FXTextField* greptext = NULL;
    FXButton* dirbutton = NULL;
    FXButton* startbutton = NULL;
    FXButton* stopbutton = NULL;
    SearchPanel* searchpanel = NULL;
    TextWindow* warnwindow = NULL;
    int in[2] = { 0, 0 };                               // Input and output pipes
    int out[2] = { 0, 0 };
    int pid = -1;                                       // Proccess ID of child (valid if busy).
    FXuint count = 0;
    FXbool running = false;
    FXbool firstcall = true;
    FXString strprev;
    FXString searchcommand;
    FXString uid;
    FXString gid;
    FXGroupBox* moregroup = NULL;
    FXVerticalFrame* searchframe = NULL;
    FXCheckButton* grepigncase = NULL;
    FXCheckButton* findigncase = NULL;
    FXCheckButton* findhidden = NULL;
    FXCheckButton* moreoptions = NULL;
    FXSpinner* minsize = NULL;
    FXSpinner* maxsize = NULL;
    FXSpinner* mindays = NULL;
    FXSpinner* maxdays = NULL;
    ComboBox* user = NULL;
    ComboBox* grp = NULL;
    ComboBox* type = NULL;
    FXTextField* perms = NULL;
    FXCheckButton* userbtn = NULL;
    FXCheckButton* grpbtn = NULL;
    FXCheckButton* typebtn = NULL;
    FXCheckButton* permsbtn = NULL;
    FXCheckButton* emptybtn = NULL;
    FXCheckButton* linkbtn = NULL;
    FXCheckButton* norecbtn = NULL;
    FXCheckButton* nofsbtn = NULL;
    FXButton* resetoptions = NULL;
    FXString rotcur = "—"; // Em dash

    SearchWindow()
    {
    }

    SearchWindow(const SearchWindow&)
    {
    }
public:
    FXuint execute(FXuint placement = PLACEMENT_CURSOR);

    int execCmd(FXString);
    int readData();

    long onKeyPress(FXObject*, FXSelector, void*);
    long onCmdClose(FXObject*, FXSelector, void*);
    long onCmdStart(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    long onReadData(FXObject*, FXSelector, void*);
    long onCmdStop(FXObject*, FXSelector, void*);
    long onPermsVerify(FXObject*, FXSelector, void*);
    long onCmdMoreOptions(FXObject*, FXSelector, void*);
    long onCmdResetOptions(FXObject*, FXSelector, void*);
    long onUpdStart(FXObject*, FXSelector, void*);
    long onUpdStop(FXObject*, FXSelector, void*);
    long onUpdPerms(FXObject*, FXSelector, void*);
    long onUpdSize(FXObject*, FXSelector, void*);
    long onRotatingCursorRefresh(FXObject*, FXSelector, void*);
public:
    // Change sort function
    void setSortFunc(IconListSortFunc func)
    {
        searchpanel->setSortFunc(func);
    }

    // Return sort function
    IconListSortFunc getSortFunc() const
    {
        return searchpanel->getSortFunc();
    }

    // More option dialog shown ?
    FXbool shownMoreOptions(void) const
    {
        return moreoptions->getCheck();
    }

    // Get ignore case in find
    FXbool getFindIgnoreCase(void) const
    {
        return findigncase->getCheck();
    }

    // Set hidden files in find
    void setFindHidden(FXbool hidden)
    {
        findhidden->setCheck(hidden);
    }

    // Get hidden files in find
    FXbool getFindHidden(void) const
    {
        return findhidden->getCheck();
    }

    // Set ignore case in find
    void setFindIgnoreCase(FXbool ignorecase)
    {
        findigncase->setCheck(ignorecase);
    }

    // Get ignore case in grep
    FXbool getGrepIgnoreCase(void) const
    {
        return grepigncase->getCheck();
    }

    // Set ignore case in grep
    void setGrepIgnoreCase(FXbool ignorecase)
    {
        grepigncase->setCheck(ignorecase);
    }

    // Set ignore case
    void setIgnoreCase(FXbool ignorecase)
    {
        searchpanel->setIgnoreCase(ignorecase);
    }

    // Get ignore case
    FXbool getIgnoreCase(void)
    {
        return searchpanel->getIgnoreCase();
    }

    // Set directory first
    void setDirsFirst(FXbool dirsfirst)
    {
        searchpanel->setDirsFirst(dirsfirst);
    }

    // Set directory first
    FXbool getDirsFirst(void)
    {
        return searchpanel->getDirsFirst();
    }

    // Get the current icon list style
    FXuint getListStyle(void) const
    {
        return searchpanel->getListStyle();
    }

    // Get the current icon list style
    void setListStyle(FXuint style)
    {
        searchpanel->setListStyle(style);
    }

    // Thumbnails shown?
    FXbool shownThumbnails(void) const
    {
        return searchpanel->shownThumbnails();
    }

    // Show thumbnails
    void showThumbnails(FXbool shown)
    {
        searchpanel->showThumbnails(shown);
    }

    // Get header size given its index
    int getHeaderSize(FXuint index) const
    {
        return searchpanel->getHeaderSize(index);
    }

    // Get header index given its column id
    int getHeaderIndex(FXuint id) const
    {
        return searchpanel->getHeaderIndex(id);
    }

    // Set search directory
    void setSearchPath(const FXString dir)
    {
        wheredir->setText(dir);
    }

    // Deselect all items
    void deselectAll(void)
    {
        searchpanel->deselectAll();
    }
};

#endif
