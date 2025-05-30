#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include "IconList.h"
#include "DialogBox.h"


class RenameDialog : public DialogBox
{
    FXDECLARE(RenameDialog)

protected:

    FileDict* associations = NULL;
    FXDragType urilistType = 0;                 // Standard uri-list type
    FXDragType xfelistType = 0;                 // Xfe, Gnome and XFCE list type
    FXDragType kdelistType = 0;                 // KDE list type
    FXDragType utf8Type = 0;                    // UTF-8 text type

    IconList* list = NULL;

    FXLabel* itemslbl = NULL;

    FXuint numfiles = 0;
    FXbool showfolder = false;
    FXString source;
    FXString target;

    FXString prev_searchtxt;
    FXString prev_replacetxt;
    FXString prev_insertstr;

    FXuint mode = 0;
    FXuint nameoption = 0;
    FXbool ignorecase = false;
    FXuint caseoption = 0;
    FXuint removechars = 0;
    FXuint removefrompos = 0;
    FXbool removefromend = 0;
    FXuint insertfrompos = 0;
    FXuint insertfromend = 0;
    FXuint insertoption = 0;

    FXButton* acceptbtn = NULL;

    FXGroupBox* replacegroup = NULL;
    FXGroupBox* casegroup = NULL;
    FXGroupBox* removegroup = NULL;
    FXGroupBox* insertgroup = NULL;

    FXRadioButton* replaceradio = NULL;
    FXRadioButton* datetimeradio = NULL;
    FXRadioButton* caseradio = NULL;
    FXRadioButton* insertradio = NULL;
    FXRadioButton* removeradio = NULL;
    FXRadioButton* nameonlyradio = NULL;
    FXRadioButton* extonlyradio = NULL;
    FXRadioButton* wholenameradio = NULL;
    FXRadioButton* lowercaseradio = NULL;
    FXRadioButton* uppercaseradio = NULL;
    FXRadioButton* titlecaseradio = NULL;
    FXRadioButton* firstuppercaseradio = NULL;
    FXCheckButton* ignorecasechk = NULL;
    FXRadioButton* insradio = NULL;
    FXRadioButton* ovwradio = NULL;

    FXTextField* searchfld = NULL;
    FXTextField* replacefld = NULL;
    FXTextField* insertfld = NULL;

    FXSpinner* removecharsspin = NULL;
    FXSpinner* removefromposspin = NULL;
    FXSpinner* insertfromposspin = NULL;

    FXCheckButton* removefromendchk = NULL;
    FXCheckButton* insertfromendchk = NULL;

private:
    RenameDialog()
    {
    }

public:
    enum
    {
        ID_ACCEPT=DialogBox::ID_LAST,
        ID_CANCEL,
        ID_COPYNAME_CLIPBOARD,
        ID_LIST,
        ID_SORT_BY_NAME,
        ID_SORT_BY_FOLDERNAME,
        ID_SORT_BY_NEWNAME,
        ID_MODE_REPLACE,
        ID_MODE_CASE,
        ID_MODE_INSERT,
        ID_MODE_REMOVE,
        ID_SEARCH_TEXT,
        ID_REPLACE_TEXT,
        ID_NAME_ONLY,
        ID_EXT_ONLY,
        ID_WHOLE_NAME,
        ID_IGNORE_CASE,
        ID_LOWER_CASE,
        ID_UPPER_CASE,
        ID_TITLE_CASE,
        ID_FIRST_UPPER_CASE,
        ID_REMOVE_CHARS,
        ID_REMOVE_FROM_POS,
        ID_REMOVE_FROM_END,
        ID_INSERT_INS,
        ID_INSERT_OVW,
        ID_INSERT_TEXT,
        ID_INSERT_FROM_POS,
        ID_INSERT_FROM_END,
        ID_LAST
    };
    RenameDialog(FXWindow*, FXString, FXString, FXuint, FXbool showfld = false, int w = 800, int h = 600);
    virtual void create();

    virtual ~RenameDialog();
    FXuint execute(FXuint);
    long onCmdAccept(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdSortByName(FXObject*, FXSelector, void*);
    long onCmdSortByFolderName(FXObject*, FXSelector, void*);
    long onCmdSortByNewName(FXObject*, FXSelector, void*);
    long onCmdHeaderClicked(FXObject*, FXSelector, void*);
    long onUpdHeader(FXObject*, FXSelector, void*);
    long onCmdMode(FXObject*, FXSelector, void*);
    long onUpdMode(FXObject*, FXSelector, void*);
    long onCmdNameOptions(FXObject*, FXSelector, void*);
    long onCmdCaseOptions(FXObject*, FXSelector, void*);
    long onCmdRemoveOptions(FXObject*, FXSelector, void*);
    long onCmdInsertOptions(FXObject*, FXSelector, void*);
    long onCmdIgnoreCase(FXObject*, FXSelector, void*);
    long onUpdNameOptions(FXObject*, FXSelector, void*);
    long onUpdCaseOptions(FXObject*, FXSelector, void*);
    long onUpdInsertOptions(FXObject*, FXSelector, void*);
    long onTimeoutListReplace(FXObject*, FXSelector, void*);
    long onTimeoutListInsert(FXObject*, FXSelector, void*);

    void appendItem(FXString, FXString);
    FXString doReplace(FXbool, FXuint, FXString, FXuint, FXString, FXString);
    FXString doChangeCase(FXuint, FXuint, FXString, FXuint);
    FXString doRemove(FXuint, FXuint, FXuint, FXbool, FXString, FXuint);
    FXString doInsert(FXuint, FXString, FXuint, FXuint, FXbool, FXString, FXuint);

    void updateListReplace(void);
    long onUpdListReplace(FXObject*, FXSelector, void*);
    long onUpdListInsert(FXObject*, FXSelector, void*);
    void updateListCase(void);
    void updateListRemove(void);
    void updateListInsert(void);

    long onUpdAccept(FXObject*, FXSelector, void*);

    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdCopyName(FXObject*, FXSelector, void*);

    long onClipboardGained(FXObject*, FXSelector, void*);
    long onClipboardLost(FXObject*, FXSelector, void*);
    long onClipboardRequest(FXObject*, FXSelector, void*);

    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);

public:

    // Return destination file names
    FXString getTarget() const
    {
        return target;
    }

    // Get number of selected items
    int getNumSelectedItems(void) const
    {
        int num = 0;

        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                num++;
            }
        }
        return num;
    }

public:
    static int compareSection(const char*, const char*, int);
    static int ascendingName(const IconItem*, const IconItem*);
    static int descendingName(const IconItem*, const IconItem*);
    static int ascendingFolderName(const IconItem*, const IconItem*);
    static int descendingFolderName(const IconItem*, const IconItem*);
    static int ascendingNewName(const IconItem*, const IconItem*);
    static int descendingNewName(const IconItem*, const IconItem*);
};
#endif
