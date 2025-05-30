#ifndef XFILEIMAGE_H
#define XFILEIMAGE_H

#include "InputDialog.h"
#include "PathLinker.h"

class XFileImage : public FXMainWindow
{
    FXDECLARE(XFileImage)
protected:
    FXbool hiddenfiles = false;                       // Show or hide hidden files
    FXbool thumbnails = false;                        // Show or hide image thumbnails
    FXuint fileview = 0;                              // File list view
    FXuint liststyle = 0;                             // Icon list style
    FXImageView* imageview = NULL;                    // Image viewer
    FXRecentFiles mrufiles;                           // Recent files
    FXString filename;                                // File being viewed
    FXMenuBar* menubar = NULL;                        // Menu bar
    FXToolBar* toolbar = NULL;                        // Tool bar
    FXToolBarShell* dragshell1 = NULL;                // Shell for floating menubar
    FXHorizontalFrame* statusbar = NULL;              // Status bar
    FXbool filelistbefore = false;
    FXbool vertpanels = false;
    FXSplitter* splitter = NULL;                      // Splitter
    FXVerticalFrame* filebox = NULL;                  // Box containing directories/files
    FileList* filelist = NULL;                        // File List
    FXLabel* label = NULL;                            // Directory path
    FXMenuPane* filemenu = NULL;                      // File menu
    FXMenuPane* viewmenu = NULL;                      // View menu
    FXMenuPane* helpmenu = NULL;                      // Help menu
    FXMenuPane* imagemenu = NULL;                     // Image menu
    FXMenuPane* prefsmenu = NULL;                     // Preferences menu
    FXTextField* filter = NULL;                       // Filter for tree list
    FXImage* img = NULL;                              // Image loaded
    FXImage* tmpimg = NULL;                           // Temporary image
    FXColor* tmpdata = NULL;                          // Temporary image data
    int indZoom = 0;                                  // Zoom index
    double zoomval = 0;                               // Actual zoom factor
    FXbool fitwin = false;                            // Fit window when opening an image
    FXbool filterimgs = false;                        // List only image files in file list
    InputDialog* printdialog = NULL;
    FXbool smoothscroll = false;
    double filewidth_pct = 0;
    double fileheight_pct = 0;
    FXArrowButton* btnbackhist = NULL;                // Back history
    FXArrowButton* btnforwardhist = NULL;             // Forward history
    PathLinker* pathlink = NULL;
    TextLabel* pathtext = NULL;
    int prev_width = 0;
    int prev_height = 0;

    FXuint single_click = SINGLE_CLICK_NONE;          // Single click navigation
    FXbool relative_resize = true;                    // Relative resizing of the panels and columns in detailed mode
    FXbool save_win_pos = false;                      // Save window position

protected:
    XFileImage()
    {
    }

public:
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdOpen(FXObject*, FXSelector, void*);
    long onCmdPrint(FXObject*, FXSelector, void*);
    long onCmdShowMini(FXObject*, FXSelector, void*);
    long onCmdShowBig(FXObject*, FXSelector, void*);
    long onCmdShowDetails(FXObject*, FXSelector, void*);
    long onCmdShowRows(FXObject*, FXSelector, void*);
    long onCmdShowCols(FXObject*, FXSelector, void*);
    long onCmdAutosize(FXObject*, FXSelector, void*);
    long onCmdSave(FXObject*, FXSelector, void*);
    long onSigHarvest(FXObject*, FXSelector, void*);
    long onCmdQuit(FXObject*, FXSelector, void*);
    long onCmdRestart(FXObject*, FXSelector, void*);
    long onUpdTitle(FXObject*, FXSelector, void*);
    long onCmdRecentFile(FXObject*, FXSelector, void*);
    long onCmdRotate(FXObject*, FXSelector, void*);
    long onCmdMirror(FXObject*, FXSelector, void*);
    long onCmdZoomIn(FXObject*, FXSelector, void*);
    long onCmdZoomOut(FXObject*, FXSelector, void*);
    long onCmdZoom100(FXObject*, FXSelector, void*);
    long onCmdZoomWin(FXObject*, FXSelector, void*);
    long onUpdImage(FXObject*, FXSelector, void*);
    long onUpdFileView(FXObject*, FXSelector, void*);
    long onUpdIconView(FXObject*, FXSelector, void*);
    long onCmdToggleHidden(FXObject*, FXSelector, void*);
    long onUpdToggleHidden(FXObject*, FXSelector, void*);
    long onCmdToggleThumbnails(FXObject*, FXSelector, void*);
    long onUpdToggleThumbnails(FXObject*, FXSelector, void*);
    long onCmdItemDoubleClicked(FXObject*, FXSelector, void*);
    long onCmdItemClicked(FXObject*, FXSelector, void*);
    long onCmdToggleFitWin(FXObject*, FXSelector, void*);
    long onUpdToggleFitWin(FXObject*, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdHome(FXObject*, FXSelector, void*);
    long onCmdWork(FXObject*, FXSelector, void*);
    long onCmdDirUp(FXObject*, FXSelector, void*);
    long onUpdDirUp(FXObject*, FXSelector, void*);
    long onCmdDirBack(FXObject*, FXSelector, void*);
    long onUpdDirBack(FXObject*, FXSelector, void*);
    long onCmdDirForward(FXObject*, FXSelector, void*);
    long onUpdDirForward(FXObject*, FXSelector, void*);
    long onCmdDirBackHist(FXObject*, FXSelector, void*);
    long onUpdDirBackHist(FXObject*, FXSelector, void*);
    long onCmdDirForwardHist(FXObject*, FXSelector, void*);
    long onUpdDirForwardHist(FXObject*, FXSelector, void*);
    long onCmdToggleFilterImages(FXObject*, FXSelector, void*);
    long onUpdToggleFilterImages(FXObject*, FXSelector, void*);
    long onCmdHorzVertPanels(FXObject*, FXSelector, void*);
    long onUpdHorzVertPanels(FXObject*, FXSelector, void*);
    long onCmdToggleFileListBefore(FXObject*, FXSelector, void*);
    long onUpdToggleFileListBefore(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onCmdViewPrev(FXObject*, FXSelector, void*);
    long onCmdViewNext(FXObject*, FXSelector, void*);


public:
    enum
    {
        ID_ABOUT=FXMainWindow::ID_LAST,
        ID_OPEN,
        ID_POPUP_MENU,
        ID_TOGGLE_HIDDEN,
        ID_TOGGLE_THUMBNAILS,
        ID_SHOW_MINI_ICONS,
        ID_SHOW_BIG_ICONS,
        ID_SHOW_DETAILS,
        ID_COLS,
        ID_ROWS,
        ID_AUTO,
        ID_TITLE,
        ID_PRINT,
        ID_HARVEST,
        ID_QUIT,
        ID_RESTART,
        ID_FILELIST,
        ID_RECENTFILE,
        ID_ROTATE_90,
        ID_ROTATE_270,
        ID_MIRROR_HOR,
        ID_MIRROR_VER,
        ID_SCALE,
        ID_ZOOM_IN,
        ID_ZOOM_OUT,
        ID_ZOOM_100,
        ID_ZOOM_WIN,
        ID_TOGGLE_FIT_WIN,
        ID_TOGGLE_FILTER_IMAGES,
        ID_GO_HOME,
        ID_GO_WORK,
        ID_DIR_UP,
        ID_DIR_BACK,
        ID_DIR_FORWARD,
        ID_DIR_BACK_HIST,
        ID_DIR_FORWARD_HIST,
        ID_HORZ_PANELS,
        ID_VERT_PANELS,
        ID_TOGGLEFILELIST_BEFORE,
        ID_VIEW_PREV,
        ID_VIEW_NEXT,
        ID_LAST
    };
public:
    XFileImage(FXApp*, FXbool, FXColor, FXColor, FXColor);
    virtual void create();
    FXbool loadimage(const FXString&);
    void saveConfig();

    void start(FXString);
    virtual ~XFileImage();
    void setSmoothScroll(FXbool smooth)
    {
        smoothscroll = smooth;
    }
};
#endif
