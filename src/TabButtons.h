#ifndef TABBUTTONS_H
#define TABBUTTONS_H

#include <vector>

#include "xfedefs.h"
#include "xfeutils.h"
#include "DirPanel.h"
#include "FilePanel.h"

class FXAPI TabButtons : public FXHorizontalFrame
{
    FXDECLARE(TabButtons)

protected:
    typedef std::vector<FXButton*> vector_FXButton;
    vector_FXButton tabButtons;                         // Vector of tab buttons
    vector_FXButton closeButtons;                       // Vector of close buttons
    
    FXString* tabPaths = NULL;                          // Vector of tab paths
    
    FXuint nbActiveTabs = 0;                            // Number of active tabs
    FXuint currentTab = 0;                              // Index of current tab
    FXuint prevTab = 0;                                 // Index of previous current tab
    
    FilePanel* filepanel = NULL;
    DirPanel* dirpanel = NULL;

    FXFont* normalFont = NULL;
    FXFont* highlightFont = NULL;
    
    FXColor shadecolor = FXRGB(0, 0, 0);                // Base color
    FXColor basecolor = FXRGB(0, 0, 0);                 // Shade color
    
    TabButtons()
    {
    }

private:


public:
    enum
    {
        ID_NEW_TAB = FXHorizontalFrame::ID_LAST,
        ID_REMOVE_ALL_TABS,
        
        ID_START_TAB,
        // Note: Place any additional id's AFTER ID_END_TAB
        ID_END_TAB = ID_START_TAB + NMAX_TABS - 1,

        ID_START_CLOSE,
        // Note: Place any additional id's AFTER ID_END_CLOSE
        ID_END_CLOSE = ID_START_CLOSE + NMAX_TABS - 1,

        ID_START_MOVE_LEFT,
        // Note: Place any additional id's AFTER ID_END_MOVE_LEFT
        ID_END_MOVE_LEFT = ID_START_MOVE_LEFT + NMAX_TABS - 1,

        ID_START_MOVE_RIGHT,
        // Note: Place any additional id's AFTER ID_END_MOVE_RIGHT
        ID_END_MOVE_RIGHT = ID_START_MOVE_RIGHT + NMAX_TABS - 1,

        ID_LAST
    };

    TabButtons(FXComposite*, FilePanel*, DirPanel*, FXuint opts = 0);
    virtual void create();

    virtual ~TabButtons();
    long onCmdPathButton(FXObject*, FXSelector, void*);
    long onCmdCloseButton(FXObject*, FXSelector, void*);
    long onCmdMoveLeft(FXObject*, FXSelector, void*);
    long onCmdMoveRight(FXObject*, FXSelector, void*);

    void addTab(FXString);
    long onQueryTip(FXObject*, FXSelector, void*);
    long onCmdNewTab(FXObject*, FXSelector, void*);
    long onCmdRemoveAllTabs(FXObject*, FXSelector, void*);
    long onUpdCurrentTab(FXObject*, FXSelector, void*);

    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onUpdMoveLeft(FXObject* sender, FXSelector sel, void* ptr);
    long onUpdMoveRight(FXObject* sender, FXSelector sel, void* ptr);
    
    // Number of active tabs
    FXuint getNumActiveTabs(void)
    {
        return nbActiveTabs;
    }
    
    // Tab label from index
    FXString getLabel(FXuint index)
    {
        FXString label;
        
        if (index >= 0 && index < nbActiveTabs)
        {
            label = tabButtons[index]->getText();
        }
        
        return label;
    }
    
    // Tab path from index
    FXString getPath(FXuint index)
    {
        FXString path;
        
        if (index < nbActiveTabs)
        {
            path = tabPaths[index];
        }
        
        return path;
    }
    
    // Current tab index
    FXuint getCurrentTab(void)
    {
        return currentTab;
    }
    
    // Set current tab from index
    void setCurrentTab(FXuint);

};

#endif
