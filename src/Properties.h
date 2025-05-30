#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "ComboBox.h"
#include "TextLabel.h"
#include "DialogBox.h"


class PropertiesBox;

class PermsFrame : public FXVerticalFrame
{
    FXDECLARE(PermsFrame)
    friend class PropertiesBox;
private:
    FXCheckButton* ur = NULL;
    FXCheckButton* uw = NULL;
    FXCheckButton* ux = NULL;
    FXCheckButton* gr = NULL;
    FXCheckButton* gw = NULL;
    FXCheckButton* gx = NULL;
    FXCheckButton* or_ = NULL;
    FXCheckButton* ow = NULL;
    FXCheckButton* ox = NULL;
    FXCheckButton* suid = NULL;
    FXCheckButton* sgid = NULL;
    FXCheckButton* svtx = NULL;
    FXDataTarget cmd_radiotarget;
    FXDataTarget flt_radiotarget;
    FXRadioButton* set = NULL;
    FXRadioButton* clear = NULL;
    FXRadioButton* dironly = NULL;
    FXRadioButton* fileonly = NULL;
    FXRadioButton* all = NULL;
    FXRadioButton* add = NULL;
    int cmd = 0;
    int flt = 0;
    FXCheckButton* rec = NULL;
    FXCheckButton* own = NULL;
    ComboBox* user = NULL;
    ComboBox* grp = NULL;

    PermsFrame()
    {
    }

public:
    PermsFrame(FXComposite* parent, FXObject* target);
};

class PropertiesBox : public DialogBox
{
    FXDECLARE(PropertiesBox)

private:
    int pid = -1;                            // Proccess ID of child (valid if busy).
    int pipes[2] = { 0, 0 };                 // Pipes to communicate with child process.

    double scalefrac = 1.0;                  // Icon scaling factor

    FXuint totalnbfiles = 0;
    FXuint totalnbsubdirs = 0;
    FXulong totaldirsize = 0;
    int nbseldirs = 0;
    FXLabel* fileSize = NULL;
    FXLabel* fileSizeDetails = NULL;
    TextLabel* location = NULL;
    FXLabel* origlocation = NULL;
    FXLabel* linkto = NULL;
    FXLabel* deletiondate = NULL;
    FXTextField* ext = NULL;
    FXString* files = NULL;
    FXString* paths = NULL;
    FXLabel* name_encoding = NULL;
    FXString source;
    FXString parentdir;
    FXString filename;
    FXString oldusr;
    FXString oldgrp;
    FXString descr_prev;
    FXString open_prev;
    FXString view_prev;
    FXString edit_prev;
    FXString bigic_prev;
    FXString miniic_prev;
    int num = 0;
    FXString trashfileslocation;
    FXString trashinfolocation;
    FXbool executable = false;
#ifdef STARTUP_NOTIFICATION
    FXCheckButton* snbutton = NULL;
    FXGroupBox* sngroup = NULL;
    FXbool sndisable_prev = false;
#endif
    FXTabItem* general = NULL;
    FXTextField* input = NULL;
    FXTextField* username = NULL;
    FXTextField* grpname = NULL;
    FXTextField* open = NULL;
    FXTextField* view = NULL;
    FXTextField* edit = NULL;
    FXTextField* descr = NULL;
    FXTextField* bigic = NULL;
    FXButton* bigicbtn = NULL;
    FXTextField* miniic = NULL;
    FXButton* miniicbtn = NULL;
    FXbool isDirectory = false;
    FXbool isMountpoint = false;
    FXbool recsize = false;
    mode_t mode = 0;
    mode_t orig_mode = 0;
    PermsFrame* perms = NULL;

    PropertiesBox()
    {
    }

public:
    enum
    {
        ID_ACCEPT_SINGLE=DialogBox::ID_LAST,
        ID_ACCEPT_MULT,
        ID_CANCEL,
        ID_SET,
        ID_CLEAR,
        ID_ADD,
        ID_DIRONLY,
        ID_FILEONLY,
        ID_SNDISABLE,
        ID_WATCHPROCESS,
        ID_ALL,
        ID_BIG_ICON,
        ID_MINI_ICON,
        ID_BROWSE_OPEN,
        ID_BROWSE_VIEW,
        ID_BROWSE_EDIT,
        ID_RUSR,
        ID_WUSR,
        ID_XUSR,
        ID_RGRP,
        ID_WGRP,
        ID_XGRP,
        ID_ROTH,
        ID_WOTH,
        ID_XOTH,
        ID_SUID,
        ID_SGID,
        ID_SVTX,
        ID_LAST
    };

public:
    virtual void create();

    PropertiesBox(FXWindow* win, FXString file, FXString path, FXbool placeitem = false);
    PropertiesBox(FXWindow* win, FXString* file, int num, FXString* path);
    long onCmdAcceptSingle(FXObject*, FXSelector, void*);
    long onCmdAcceptMult(FXObject*, FXSelector, void*);
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onCmdCheck(FXObject*, FXSelector, void*);
    long onCmdCommand(FXObject*, FXSelector, void*);
    long onCmdFilter(FXObject*, FXSelector, void*);
    long onCmdBrowseIcon(FXObject*, FXSelector, void*);
    long onCmdBrowse(FXObject*, FXSelector, void*);
    long onUpdSizeAndPerms(FXObject*, FXSelector, void*);
    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onWatchProcess(FXObject*, FXSelector, void*);
#ifdef STARTUP_NOTIFICATION
    long onUpdSnDisable(FXObject*, FXSelector, void*);
#endif
};

#endif
