#ifndef PATHLINKER_H
#define PATHLINKER_H

#include <vector>

#include "xfedefs.h"
#include "xfeutils.h"
#include "TextLabel.h"
#include "DirList.h"
#include "FileList.h"

class FXAPI PathLinker : public FXHorizontalFrame
{
    FXDECLARE(PathLinker)

protected:
    typedef std::vector<FXButton*> vector_FXButton;
    vector_FXButton linkButtons;
    FXuint nbActiveButtons = 0;
    FXuint currentButton = 0;
    FXString visitedPath;
    TextLabel* focusButton = NULL;
    FXFont* normalFont = NULL;
    FXFont* highlightFont = NULL;
    FileList* filelist = NULL;
    DirList* dirlist = NULL;
    PathLinker()
    {
    }

private:
    void updatePath(FXString, FXuint);
    void setText(FXuint, FXString);

public:
    enum
    {
        ID_START_LINK = FXHorizontalFrame::ID_LAST,
        // Note: Place any additional id's AFTER ID_END_LINK
        ID_END_LINK = ID_START_LINK + NMAX_LINKS - 1,
        ID_FOCUS_BUTTON,
        ID_LAST
    };

    PathLinker(FXComposite*, FileList*, DirList* dlist = NULL, FXuint opts = 0);
    virtual void create();

    virtual ~PathLinker();
    long onCmdLinkButton(FXObject*, FXSelector, void*);
    long onCmdFocusButton(FXObject*, FXSelector, void*);
    long onUpdPath(FXObject*, FXSelector, void*);
    void setPath(FXString);
    void focus();
    void unfocus();
};

#endif
