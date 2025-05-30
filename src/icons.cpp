// Global icons for all applications

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "icons.h"
#include "MessageBox.h"

// Application exec path
extern FXString execpath;

// Icons (global variables)

FXIcon* bigattribicon;
FXIcon* bigblockdevicon;
FXIcon* bigbrokenlinkicon;
FXIcon* bigcdromicon;
FXIcon* bigchardevicon;
FXIcon* bigcompareicon;
FXIcon* bigdocicon;
FXIcon* bigexecicon;
FXIcon* bigfileopenicon;
FXIcon* bigfiltericon;
FXIcon* bigfloppyicon;
FXIcon* bigfoldericon;
FXIcon* bigfolderopenicon;
FXIcon* bigfolderlinkicon;
FXIcon* bigfolderupicon;
FXIcon* bigfoldernewicon;
FXIcon* bigfolderlockedicon;
FXIcon* bigfolderremoteicon;
FXIcon* bigharddriveicon;
FXIcon* bignewfileicon;
FXIcon* bignewlinkicon;
FXIcon* bignetdriveicon;
FXIcon* bignetdriveumticon;
FXIcon* bigpipeicon;
FXIcon* bigsocketicon;
FXIcon* bigzipdiskicon;
FXIcon* bigrenameicon;
FXIcon* bigbookmarkicon;
FXIcon* bigaddbookicon;
FXIcon* bigeditbookicon;
FXIcon* bigremovebookicon;
FXIcon* bigcopyicon;
FXIcon* bigdeletepermicon;
FXIcon* bigdeleteicon;
FXIcon* bigerroricon;
FXIcon* biggotoicon;
FXIcon* biglinktoicon;
FXIcon* biglinkicon;
FXIcon* biginfoicon;
FXIcon* bigmoveicon;
FXIcon* bigprinticon;
FXIcon* bigquestionicon;
FXIcon* bigtrashfullicon;
FXIcon* bigwarningicon;
FXIcon* bigrestoreicon;
FXIcon* bigarchaddicon;
FXIcon* bigarchexticon;
FXIcon* bigkeybindingsicon;
FXIcon* bigsearchicon;
FXIcon* bigreplaceicon;
FXIcon* bigrunicon;
FXIcon* biglinkbadgeicon;
FXIcon* bigsavefileicon;
FXIcon* bigsaveasicon;
FXIcon* bigservericon;

FXIcon* miniarchaddicon;
FXIcon* miniarchexticon;
FXIcon* miniattribicon;
FXIcon* minibigiconsicon;
FXIcon* minicdromicon;
FXIcon* minicharticon;
FXIcon* miniclosefileicon;
FXIcon* minicolltreeicon;
FXIcon* minicopyicon;
FXIcon* minicuticon;
FXIcon* miniunselicon;
FXIcon* minidetailsicon;
FXIcon* minidirupicon;
FXIcon* miniediticon;
FXIcon* miniexptreeicon;
FXIcon* minideletepermicon;
FXIcon* minideleteicon;
FXIcon* minifileopenicon;
FXIcon* miniviewicon;
FXIcon* minifiltericon;
FXIcon* minicompareicon;
FXIcon* minifindagainicon;
FXIcon* minifliplricon;
FXIcon* miniflipudicon;
FXIcon* minifloppyicon;
FXIcon* minifontsicon;
FXIcon* minigotodiricon;
FXIcon* minigotoicon;
FXIcon* miniharddriveicon;
FXIcon* minihelpicon;
FXIcon* minihidehiddenicon;
FXIcon* minihidenumbersicon;
FXIcon* minihidethumbicon;
FXIcon* minihomeicon;
FXIcon* minidesktopicon;
FXIcon* miniinvselicon;
FXIcon* minilowercaseicon;
FXIcon* minimounticon;
FXIcon* miniappicon;
FXIcon* miniblockdevicon;
FXIcon* minibrokenlinkicon;
FXIcon* minichardevicon;
FXIcon* minidocicon;
FXIcon* miniexecicon;
FXIcon* minifoldericon;
FXIcon* minifolderopenicon;
FXIcon* minifolderlockedicon;
FXIcon* minifolderlinkicon;
FXIcon* minifolderupicon;
FXIcon* minifolderremoteicon;
FXIcon* minifolderdownloadicon;
FXIcon* minifoldernewicon;
FXIcon* minifolderhomeicon;
FXIcon* minifolderdocumentsicon;
FXIcon* minifolderimagesicon;
FXIcon* minifoldermusicicon;
FXIcon* minifoldervideosicon;
FXIcon* minilinktoicon;
FXIcon* minilinkicon;
FXIcon* minipipeicon;
FXIcon* minishellicon;
FXIcon* minisocketicon;
FXIcon* minimoveicon;
FXIcon* mininewfileicon;
FXIcon* mininetdriveicon;
FXIcon* mininetdriveumticon;
FXIcon* minionepanelicon;
FXIcon* minipackageicon;
FXIcon* minipasteicon;
FXIcon* miniprefsicon;
FXIcon* miniprinticon;
FXIcon* miniquiticon;
FXIcon* miniredoicon;
FXIcon* minireloadicon;
FXIcon* minirenameicon;
FXIcon* minireplaceicon;
FXIcon* minireverticon;
FXIcon* minirotatelefticon;
FXIcon* minirotaterighticon;
FXIcon* minirunicon;
FXIcon* minisaveasicon;
FXIcon* minisavefileicon;
FXIcon* minisearchnexticon;
FXIcon* minisearchicon;
FXIcon* minisearchprevicon;
FXIcon* miniselallicon;
FXIcon* minibookmarkicon;
FXIcon* miniaddbookicon;
FXIcon* minieditbookicon;
FXIcon* miniremovebookicon;
FXIcon* minishowhiddenicon;
FXIcon* minishownumbersicon;
FXIcon* minishowthumbicon;
FXIcon* minismalliconsicon;
FXIcon* minitrashfullicon;
FXIcon* minitreeonepanelicon;
FXIcon* minitreetwopanelsicon;
FXIcon* minitwopanelsicon;
FXIcon* miniundoicon;
FXIcon* miniunmounticon;
FXIcon* miniuppercaseicon;
FXIcon* miniworkicon;
FXIcon* miniwrapofficon;
FXIcon* miniwraponicon;
FXIcon* minizipdiskicon;
FXIcon* minizoom100icon;
FXIcon* minizoominicon;
FXIcon* minizoomouticon;
FXIcon* minizoomwinicon;
FXIcon* minitotrashicon;
FXIcon* minidirbackicon;
FXIcon* minidirforwardicon;
FXIcon* minixferooticon;
FXIcon* minixfeicon;
FXIcon* minifiledialogicon;
FXIcon* miniswitchpanelsicon;
FXIcon* minisyncpanelsicon;
FXIcon* mininewlinkicon;
FXIcon* minigreenbuttonicon;
FXIcon* minigraybuttonicon;
FXIcon* minikeybindingsicon;
FXIcon* minirestoreicon;
FXIcon* minihorzpanelsicon;
FXIcon* minivertpanelsicon;
FXIcon* minitreeicon;
FXIcon* miniplacesicon;
FXIcon* miniupicon;
FXIcon* minidownicon;
FXIcon* miniremovabledeviceicon;
FXIcon* miniclosefiltericon;
FXIcon* minilinkbadgeicon;
FXIcon* minicloseicon;
FXIcon* miniaddicon;
FXIcon* mininewtabicon;
FXIcon* miniremovetabicon;
FXIcon* miniservericon;

FXIcon* xfeicon;
FXIcon* xfaicon;
FXIcon* xfiicon;
FXIcon* xfpicon;
FXIcon* xfwicon;

FXIcon* cmymodeicon;
FXIcon* dialmodeicon;
FXIcon* eyedropicon;
FXIcon* hsvmodeicon;
FXIcon* listmodeicon;
FXIcon* rgbmodeicon;

FXIcon* docktopicon;
FXIcon* docklefticon;
FXIcon* dockrighticon;
FXIcon* dockbottomicon;
FXIcon* dockflipicon;
FXIcon* dockfreeicon;


// Load all application icons as global variables
FXbool loadAppIcons(FXApp* app, FXuint* iconpathstatus)
{
    *iconpathstatus = ICONPATH_FOUND;
    FXbool success = true;

    // Fractional scaling factor
    FXint res = app->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    double scalefrac = FXMAX(1.0, res / 100.0);

    // Default icon path
    FXString defaulticonpath = xf_realpath(FXPath::directory(execpath) + "/../share/xfe/icons/default-theme");

    // Select icon path
    FXString iconpath = xf_realpath(app->reg().readStringEntry("SETTINGS", "iconpath", defaulticonpath.text()));
    if (iconpath[0] == '~')
    {
        iconpath = FXSystem::getHomeDirectory() + iconpath.after('~');
    }

    // Icon path does not exist
    if (!xf_existfile(iconpath))
    {
        *iconpathstatus = ICONPATH_NOT_FOUND;
    
        // Default icon path does not exist
        if (!xf_existfile(defaulticonpath))
        {
            *iconpathstatus = DEFAULTICONPATH_NOT_FOUND;
        }

        // Set icon path to default
        else
        {
            iconpath = defaulticonpath;
            app->reg().writeStringEntry("SETTINGS", "iconpath", iconpath.text());
        }
    }

    // Colors for blending
    FXColor baseColor = app->getBaseColor();
    FXColor backColor = app->getBackColor();

    // Load icons and set the success flag
    FXbool firstpass = true;

load:

    // Big icons with base color background
    success = ((bigattribicon = xf_loadiconfile(app, iconpath, "bigattrib.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigcompareicon = xf_loadiconfile(app, iconpath, "bigcompare.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigfiltericon = xf_loadiconfile(app, iconpath, "bigfilter.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigfoldernewicon = xf_loadiconfile(app, iconpath, "bigfoldernew.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigfolderlinkicon = xf_loadiconfile(app, iconpath, "bigfolderlink.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bignewfileicon = xf_loadiconfile(app, iconpath, "bignewfile.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bignewlinkicon = xf_loadiconfile(app, iconpath, "bignewlink.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigcopyicon = xf_loadiconfile(app, iconpath, "bigcopy.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigdeletepermicon = xf_loadiconfile(app, iconpath, "bigdeleteperm.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigdeleteicon = xf_loadiconfile(app, iconpath, "bigdelete.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigerroricon = xf_loadiconfile(app, iconpath, "bigerror.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigfileopenicon = xf_loadiconfile(app, iconpath, "bigfileopen.png", scalefrac, baseColor)) != NULL) & success;
    success = ((biggotoicon = xf_loadiconfile(app, iconpath, "biggoto.png", scalefrac, baseColor)) != NULL) & success;
    success = ((biglinktoicon = xf_loadiconfile(app, iconpath, "biglinkto.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigmoveicon = xf_loadiconfile(app, iconpath, "bigmove.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigquestionicon = xf_loadiconfile(app, iconpath, "bigquestion.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigprinticon = xf_loadiconfile(app, iconpath, "bigprint.png", scalefrac, baseColor)) != NULL) & success;
    success = ((biginfoicon = xf_loadiconfile(app, iconpath, "biginfo.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigrenameicon = xf_loadiconfile(app, iconpath, "bigrename.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigbookmarkicon = xf_loadiconfile(app, iconpath, "bigbookmark.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigaddbookicon = xf_loadiconfile(app, iconpath, "bigaddbook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigeditbookicon = xf_loadiconfile(app, iconpath, "bigeditbook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigremovebookicon = xf_loadiconfile(app, iconpath, "bigremovebook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigwarningicon = xf_loadiconfile(app, iconpath, "bigwarning.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigtrashfullicon = xf_loadiconfile(app, iconpath, "bigtrashfull.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigrestoreicon = xf_loadiconfile(app, iconpath, "bigrestore.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigarchaddicon = xf_loadiconfile(app, iconpath, "bigarchadd.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigarchexticon = xf_loadiconfile(app, iconpath, "bigarchext.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigkeybindingsicon = xf_loadiconfile(app, iconpath, "bigkeybindings.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigsearchicon = xf_loadiconfile(app, iconpath, "bigsearch.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigreplaceicon = xf_loadiconfile(app, iconpath, "bigreplace.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigrunicon = xf_loadiconfile(app, iconpath, "bigrun.png", scalefrac, baseColor)) != NULL) & success;
    success = ((biglinkbadgeicon = xf_loadiconfile(app, iconpath, "biglinkbadge.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigsavefileicon = xf_loadiconfile(app, iconpath, "bigsavefile.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigsaveasicon = xf_loadiconfile(app, iconpath, "bigsaveas.png", scalefrac, baseColor)) != NULL) & success;
    success = ((bigservericon = xf_loadiconfile(app, iconpath, "bigserver.png", scalefrac, baseColor)) != NULL) & success;

    // Big icons with back color background
    success = ((bigblockdevicon = xf_loadiconfile(app, iconpath, "bigblockdev.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigbrokenlinkicon = xf_loadiconfile(app, iconpath, "bigbrokenlink.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigcdromicon = xf_loadiconfile(app, iconpath, "bigcdrom.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigchardevicon = xf_loadiconfile(app, iconpath, "bigchardev.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigdocicon = xf_loadiconfile(app, iconpath, "bigdoc.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigexecicon = xf_loadiconfile(app, iconpath, "bigexec.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfloppyicon = xf_loadiconfile(app, iconpath, "bigfloppy.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfolderlockedicon = xf_loadiconfile(app, iconpath, "bigfolderlocked.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfoldericon = xf_loadiconfile(app, iconpath, "bigfolder.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfolderopenicon = xf_loadiconfile(app, iconpath, "bigfolderopen.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfolderupicon = xf_loadiconfile(app, iconpath, "bigfolderup.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigfolderremoteicon = xf_loadiconfile(app, iconpath, "bigfolderremote.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigharddriveicon = xf_loadiconfile(app, iconpath, "bigharddrive.png", scalefrac, backColor)) != NULL) & success;
    success = ((bignetdriveicon = xf_loadiconfile(app, iconpath, "bignetdrive.png", scalefrac, backColor)) != NULL) & success;
    success = ((bignetdriveumticon = xf_loadiconfile(app, iconpath, "bignetdriveumt.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigpipeicon = xf_loadiconfile(app, iconpath, "bigpipe.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigsocketicon = xf_loadiconfile(app, iconpath, "bigsocket.png", scalefrac, backColor)) != NULL) & success;
    success = ((bigzipdiskicon = xf_loadiconfile(app, iconpath, "bigzipdisk.png", scalefrac, backColor)) != NULL) & success;
    success = ((biglinkicon = xf_loadiconfile(app, iconpath, "biglink.png", scalefrac, backColor)) != NULL) & success;

    // Mini icons with base color background
    success = ((miniarchaddicon = xf_loadiconfile(app, iconpath, "miniarchadd.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniarchexticon = xf_loadiconfile(app, iconpath, "miniarchext.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniattribicon = xf_loadiconfile(app, iconpath, "miniattrib.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minibigiconsicon = xf_loadiconfile(app, iconpath, "minibigicons.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicharticon = xf_loadiconfile(app, iconpath, "minichart.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniclosefileicon = xf_loadiconfile(app, iconpath, "miniclosefile.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniaddbookicon = xf_loadiconfile(app, iconpath, "miniaddbook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minieditbookicon = xf_loadiconfile(app, iconpath, "minieditbook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniremovebookicon = xf_loadiconfile(app, iconpath, "miniremovebook.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicolltreeicon = xf_loadiconfile(app, iconpath, "minicolltree.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicopyicon = xf_loadiconfile(app, iconpath, "minicopy.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicuticon = xf_loadiconfile(app, iconpath, "minicut.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniunselicon = xf_loadiconfile(app, iconpath, "miniunsel.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minidetailsicon = xf_loadiconfile(app, iconpath, "minidetails.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minidirupicon = xf_loadiconfile(app, iconpath, "minidirup.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniediticon = xf_loadiconfile(app, iconpath, "miniedit.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniexptreeicon = xf_loadiconfile(app, iconpath, "miniexptree.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicompareicon = xf_loadiconfile(app, iconpath, "minicompare.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minideletepermicon = xf_loadiconfile(app, iconpath, "minideleteperm.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minideleteicon = xf_loadiconfile(app, iconpath, "minidelete.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifileopenicon = xf_loadiconfile(app, iconpath, "minifileopen.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniviewicon = xf_loadiconfile(app, iconpath, "miniview.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifiltericon = xf_loadiconfile(app, iconpath, "minifilter.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifindagainicon = xf_loadiconfile(app, iconpath, "minifindagain.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifliplricon = xf_loadiconfile(app, iconpath, "minifliplr.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniflipudicon = xf_loadiconfile(app, iconpath, "miniflipud.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifontsicon = xf_loadiconfile(app, iconpath, "minifonts.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minigotodiricon = xf_loadiconfile(app, iconpath, "minigotodir.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minigotoicon = xf_loadiconfile(app, iconpath, "minigoto.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihelpicon = xf_loadiconfile(app, iconpath, "minihelp.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihidehiddenicon = xf_loadiconfile(app, iconpath, "minihidehidden.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihidenumbersicon = xf_loadiconfile(app, iconpath, "minihidenumbers.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihidethumbicon = xf_loadiconfile(app, iconpath, "minihidethumb.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihomeicon = xf_loadiconfile(app, iconpath, "minihome.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniinvselicon = xf_loadiconfile(app, iconpath, "miniinvsel.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minilowercaseicon = xf_loadiconfile(app, iconpath, "minilowercase.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minimounticon = xf_loadiconfile(app, iconpath, "minimount.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifoldernewicon = xf_loadiconfile(app, iconpath, "minifoldernew.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minilinktoicon = xf_loadiconfile(app, iconpath, "minilinkto.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minishellicon = xf_loadiconfile(app, iconpath, "minishell.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minimoveicon = xf_loadiconfile(app, iconpath, "minimove.png", scalefrac, baseColor)) != NULL) & success;
    success = ((mininewfileicon = xf_loadiconfile(app, iconpath, "mininewfile.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minionepanelicon = xf_loadiconfile(app, iconpath, "minionepanel.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minipackageicon = xf_loadiconfile(app, iconpath, "minipackage.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minipasteicon = xf_loadiconfile(app, iconpath, "minipaste.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniprefsicon = xf_loadiconfile(app, iconpath, "miniprefs.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniprinticon = xf_loadiconfile(app, iconpath, "miniprint.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniquiticon = xf_loadiconfile(app, iconpath, "miniquit.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniredoicon = xf_loadiconfile(app, iconpath, "miniredo.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minireloadicon = xf_loadiconfile(app, iconpath, "minireload.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minirenameicon = xf_loadiconfile(app, iconpath, "minirename.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minireplaceicon = xf_loadiconfile(app, iconpath, "minireplace.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minireverticon = xf_loadiconfile(app, iconpath, "minirevert.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minirotatelefticon = xf_loadiconfile(app, iconpath, "minirotateleft.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minirotaterighticon = xf_loadiconfile(app, iconpath, "minirotateright.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minirunicon = xf_loadiconfile(app, iconpath, "minirun.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisaveasicon = xf_loadiconfile(app, iconpath, "minisaveas.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisavefileicon = xf_loadiconfile(app, iconpath, "minisavefile.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisearchnexticon = xf_loadiconfile(app, iconpath, "minisearchnext.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisearchicon = xf_loadiconfile(app, iconpath, "minisearch.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisearchprevicon = xf_loadiconfile(app, iconpath, "minisearchprev.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniselallicon = xf_loadiconfile(app, iconpath, "miniselall.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minishowhiddenicon = xf_loadiconfile(app, iconpath, "minishowhidden.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minishownumbersicon = xf_loadiconfile(app, iconpath, "minishownumbers.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minishowthumbicon = xf_loadiconfile(app, iconpath, "minishowthumb.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minismalliconsicon = xf_loadiconfile(app, iconpath, "minismallicons.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitrashfullicon = xf_loadiconfile(app, iconpath, "minitrashfull.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitreeonepanelicon = xf_loadiconfile(app, iconpath, "minitreeonepanel.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitreetwopanelsicon = xf_loadiconfile(app, iconpath, "minitreetwopanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitwopanelsicon = xf_loadiconfile(app, iconpath, "minitwopanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniundoicon = xf_loadiconfile(app, iconpath, "miniundo.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniunmounticon = xf_loadiconfile(app, iconpath, "miniunmount.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniuppercaseicon = xf_loadiconfile(app, iconpath, "miniuppercase.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniworkicon = xf_loadiconfile(app, iconpath, "miniwork.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniwrapofficon = xf_loadiconfile(app, iconpath, "miniwrapoff.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniwraponicon = xf_loadiconfile(app, iconpath, "miniwrapon.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minizoom100icon = xf_loadiconfile(app, iconpath, "minizoom100.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minizoominicon = xf_loadiconfile(app, iconpath, "minizoomin.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minizoomouticon = xf_loadiconfile(app, iconpath, "minizoomout.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minizoomwinicon = xf_loadiconfile(app, iconpath, "minizoomwin.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitotrashicon = xf_loadiconfile(app, iconpath, "minitotrash.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minidirbackicon = xf_loadiconfile(app, iconpath, "minidirback.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minidirforwardicon = xf_loadiconfile(app, iconpath, "minidirforward.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minixferooticon = xf_loadiconfile(app, iconpath, "minixferoot.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minixfeicon = xf_loadiconfile(app, iconpath, "minixfe.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minifiledialogicon = xf_loadiconfile(app, iconpath, "minifiledialog.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniswitchpanelsicon = xf_loadiconfile(app, iconpath, "miniswitchpanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minisyncpanelsicon = xf_loadiconfile(app, iconpath, "minisyncpanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((mininewlinkicon = xf_loadiconfile(app, iconpath, "mininewlink.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minigreenbuttonicon = xf_loadiconfile(app, iconpath, "minigreenbutton.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minigraybuttonicon = xf_loadiconfile(app, iconpath, "minigraybutton.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minikeybindingsicon = xf_loadiconfile(app, iconpath, "minikeybindings.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minirestoreicon = xf_loadiconfile(app, iconpath, "minirestore.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minihorzpanelsicon = xf_loadiconfile(app, iconpath, "minihorzpanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minivertpanelsicon = xf_loadiconfile(app, iconpath, "minivertpanels.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minitreeicon = xf_loadiconfile(app, iconpath, "minitree.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniplacesicon = xf_loadiconfile(app, iconpath, "miniplaces.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniupicon = xf_loadiconfile(app, iconpath, "miniup.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minidownicon = xf_loadiconfile(app, iconpath, "minidown.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniremovabledeviceicon = xf_loadiconfile(app, iconpath, "miniremovabledevice.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniclosefiltericon = xf_loadiconfile(app, iconpath, "miniclosefilter.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minilinkbadgeicon = xf_loadiconfile(app, iconpath, "minilinkbadge.png", scalefrac, baseColor)) != NULL) & success;
    success = ((minicloseicon = xf_loadiconfile(app, iconpath, "miniclose.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniaddicon = xf_loadiconfile(app, iconpath, "miniadd.png", scalefrac, baseColor)) != NULL) & success;
    success = ((mininewtabicon = xf_loadiconfile(app, iconpath, "mininewtab.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniremovetabicon = xf_loadiconfile(app, iconpath, "miniremovetab.png", scalefrac, baseColor)) != NULL) & success;
    success = ((miniservericon = xf_loadiconfile(app, iconpath, "miniserver.png", scalefrac, baseColor)) != NULL) & success;

    // Mini icons with back color background
    success = ((minicdromicon = xf_loadiconfile(app, iconpath, "minicdrom.png", scalefrac, backColor)) != NULL) & success;
    success = ((minibookmarkicon = xf_loadiconfile(app, iconpath, "minibookmark.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifloppyicon = xf_loadiconfile(app, iconpath, "minifloppy.png", scalefrac, backColor)) != NULL) & success;
    success = ((miniharddriveicon = xf_loadiconfile(app, iconpath, "miniharddrive.png", scalefrac, backColor)) != NULL) & success;
    success = ((minidesktopicon = xf_loadiconfile(app, iconpath, "minidesktop.png", scalefrac, backColor)) != NULL) & success;
    success = ((miniblockdevicon = xf_loadiconfile(app, iconpath, "miniblockdev.png", scalefrac, backColor)) != NULL) & success;
    success = ((minibrokenlinkicon = xf_loadiconfile(app, iconpath, "minibrokenlink.png", scalefrac, backColor)) != NULL) & success;
    success = ((minichardevicon = xf_loadiconfile(app, iconpath, "minichardev.png", scalefrac, backColor)) != NULL) & success;
    success = ((minidocicon = xf_loadiconfile(app, iconpath, "minidoc.png", scalefrac, backColor)) != NULL) & success;
    success = ((miniexecicon = xf_loadiconfile(app, iconpath, "miniexec.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderlockedicon = xf_loadiconfile(app, iconpath, "minifolderlocked.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderopenicon = xf_loadiconfile(app, iconpath, "minifolderopen.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifoldericon = xf_loadiconfile(app, iconpath, "minifolder.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderlinkicon = xf_loadiconfile(app, iconpath, "minifolderlink.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderupicon = xf_loadiconfile(app, iconpath, "minifolderup.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderremoteicon = xf_loadiconfile(app, iconpath, "minifolderremote.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderhomeicon = xf_loadiconfile(app, iconpath, "minifolderhome.png", scalefrac, backColor)) != NULL) & success;
    success = ((miniappicon = xf_loadiconfile(app, iconpath, "miniapp.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderdownloadicon = xf_loadiconfile(app, iconpath, "minifolderdownload.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderdocumentsicon = xf_loadiconfile(app, iconpath, "minifolderdocuments.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifoldermusicicon = xf_loadiconfile(app, iconpath, "minifoldermusic.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifolderimagesicon = xf_loadiconfile(app, iconpath, "minifolderimages.png", scalefrac, backColor)) != NULL) & success;
    success = ((minifoldervideosicon = xf_loadiconfile(app, iconpath, "minifoldervideos.png", scalefrac, backColor)) != NULL) & success;
    success = ((minilinkicon = xf_loadiconfile(app, iconpath, "minilink.png", scalefrac, backColor)) != NULL) & success;
    success = ((minipipeicon = xf_loadiconfile(app, iconpath, "minipipe.png", scalefrac, backColor)) != NULL) & success;
    success = ((minisocketicon = xf_loadiconfile(app, iconpath, "minisocket.png", scalefrac, backColor)) != NULL) & success;
    success = ((mininetdriveicon = xf_loadiconfile(app, iconpath, "mininetdrive.png", scalefrac, backColor)) != NULL) & success;
    success = ((mininetdriveumticon = xf_loadiconfile(app, iconpath, "mininetdriveumt.png", scalefrac, backColor)) != NULL) & success;
    success = ((minizipdiskicon = xf_loadiconfile(app, iconpath, "minizipdisk.png", scalefrac, backColor)) != NULL) & success;

    // Mini icons with base color background
    success = ((xfeicon = xf_loadiconfile(app, iconpath, "xfe.png", scalefrac, baseColor)) != NULL) & success;
    success = ((xfaicon = xf_loadiconfile(app, iconpath, "xfa.png", scalefrac, baseColor)) != NULL) & success;
    success = ((xfiicon = xf_loadiconfile(app, iconpath, "xfi.png", scalefrac, baseColor)) != NULL) & success;
    success = ((xfpicon = xf_loadiconfile(app, iconpath, "xfp.png", scalefrac, baseColor)) != NULL) & success;
    success = ((xfwicon = xf_loadiconfile(app, iconpath, "xfw.png", scalefrac, baseColor)) != NULL) & success;

    // Mini icons with base color background
    success = ((cmymodeicon = xf_loadiconfile(app, iconpath, "cmymode.png", scalefrac, baseColor)) != NULL) & success;
    success = ((dialmodeicon = xf_loadiconfile(app, iconpath, "dialmode.png", scalefrac, baseColor)) != NULL) & success;
    success = ((eyedropicon = xf_loadiconfile(app, iconpath, "eyedrop.png", scalefrac, baseColor)) != NULL) & success;
    success = ((hsvmodeicon = xf_loadiconfile(app, iconpath, "hsvmode.png", scalefrac, baseColor)) != NULL) & success;
    success = ((listmodeicon = xf_loadiconfile(app, iconpath, "listmode.png", scalefrac, baseColor)) != NULL) & success;
    success = ((rgbmodeicon = xf_loadiconfile(app, iconpath, "rgbmode.png", scalefrac, baseColor)) != NULL) & success;

    success = ((docktopicon = xf_loadiconfile(app, iconpath, "docktop.png", scalefrac, baseColor)) != NULL) & success;
    success = ((dockbottomicon = xf_loadiconfile(app, iconpath, "dockbottom.png", scalefrac, baseColor)) != NULL) & success;
    success = ((docklefticon = xf_loadiconfile(app, iconpath, "dockleft.png", scalefrac, baseColor)) != NULL) & success;
    success = ((dockrighticon = xf_loadiconfile(app, iconpath, "dockright.png", scalefrac, baseColor)) != NULL) & success;
    success = ((dockflipicon = xf_loadiconfile(app, iconpath, "dockflip.png", scalefrac, baseColor)) != NULL) & success;
    success = ((dockfreeicon = xf_loadiconfile(app, iconpath, "dockfree.png", scalefrac, baseColor)) != NULL) & success;


    // If some icons from icon path were missing, reload using default icon path 
    if (!success && firstpass)
    {
        *iconpathstatus = ICONPATH_MISSING_ICONS;
        
        // Default icon path does not exist
        if (!xf_existfile(defaulticonpath))
        {
            *iconpathstatus = DEFAULTICONPATH_NOT_FOUND;
        }

        // Set icon path to default
        else
        {
            iconpath = defaulticonpath;
            app->reg().writeStringEntry("SETTINGS", "iconpath", iconpath.text());
        }

        firstpass = false;
        goto load;
    }
    
    return success;
}
