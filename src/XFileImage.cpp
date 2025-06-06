// This code is adapted from 'imageviewer', a demo image viewer found
// in the FOX library and written by Jeroen van der Zijp.

#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGImage.h>
#include <FXJPGImage.h>
#include <FXTIFImage.h>
#include <FXPNGIcon.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "startupnotification.h"
#include "FileDialog.h"
#include "InputDialog.h"
#include "DirHistBox.h"
#include "MessageBox.h"
#include "FileList.h"
#include "XFileImage.h"

// Add FOX hacks
#include "foxhacks.cpp"
#include "moderncontrols.cpp"


// Global variables
char** args;
FXString homedir;
FXString xdgconfighome;
FXString xdgdatahome;
FXString execpath;

// Filter history
char FilterHistory[FILTER_HIST_SIZE][MAX_PATTERN_SIZE];
int FilterNum = 0;

// Integer UI scaling factor
FXint scaleint = 1;


// Main window (not used but necessary for compilation)
FXMainWindow* mainWindow = NULL;


// Hand cursor replacement (integer scaling factor = 1)
#define hand1_width     32
#define hand1_height    32
#define hand1_x_hot     6
#define hand1_y_hot     1
static const FXuchar hand1_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
    0x90, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x90, 0x03, 0x00, 0x00,
    0x90, 0x1c, 0x00, 0x00, 0x10, 0xe4, 0x00, 0x00, 0x1c, 0x20, 0x01, 0x00,
    0x12, 0x00, 0x01, 0x00, 0x12, 0x00, 0x01, 0x00, 0x92, 0x24, 0x01, 0x00,
    0x82, 0x24, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x02, 0x00, 0x01, 0x00, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const FXuchar hand1_mask_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,
    0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00,
    0xf0, 0x1f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xfc, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00,
    0xfe, 0xff, 0x01, 0x00, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// Hand cursor replacement (integer scaling factor = 2)
#define hand2_width     32
#define hand2_height    32
#define hand2_x_hot     6
#define hand2_y_hot     1
static const FXuchar hand2_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00,
    0x60, 0x06, 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00,
    0x20, 0x1e, 0x00, 0x00, 0x60, 0x3e, 0x00, 0x00, 0x20, 0xe2, 0x03, 0x00,
    0x60, 0x62, 0x1e, 0x00, 0x38, 0x00, 0x74, 0x00, 0x7c, 0x00, 0x60, 0x00,
    0x24, 0x00, 0x40, 0x00, 0x64, 0x00, 0x60, 0x00, 0x26, 0x00, 0x40, 0x00,
    0x26, 0x22, 0x62, 0x00, 0x06, 0x22, 0x42, 0x00, 0x06, 0x00, 0x60, 0x00,
    0x06, 0x00, 0x40, 0x00, 0x06, 0x00, 0x60, 0x00, 0x04, 0x00, 0x60, 0x00,
    0xfc, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const FXuchar hand2_mask_bits[] =
{
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00,
    0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00,
    0xe0, 0x1f, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00,
    0xe0, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00,
    0xfc, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00,
    0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00,
    0xfe, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0xfc, 0xff, 0x7f, 0x00,
    0xfc, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// Hand cursor replacement (integer scaling factor = 3 or more)
#define hand3_width     32
#define hand3_height    32
#define hand3_x_hot     6
#define hand3_y_hot     1
static const FXuchar hand3_bits[] =
{
    0x80, 0x1f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00,
    0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00,
    0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0xf0, 0x03, 0x00,
    0xc0, 0xf0, 0x07, 0x00, 0xc0, 0x30, 0xfe, 0x00, 0xc0, 0x10, 0xfe, 0x01,
    0xc0, 0x10, 0x8c, 0x3f, 0xc0, 0x10, 0x04, 0x7f, 0xfc, 0x00, 0x04, 0xe1,
    0xfe, 0x00, 0x04, 0xc1, 0xc6, 0x00, 0x04, 0xc0, 0xc6, 0x00, 0x00, 0xc0,
    0xc6, 0x00, 0x00, 0xc0, 0xc3, 0x00, 0x00, 0xc0, 0xc3, 0x00, 0x00, 0xc0,
    0xc3, 0x10, 0x04, 0xc1, 0x03, 0x10, 0x04, 0xc1, 0x03, 0x10, 0x04, 0xc1,
    0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0,
    0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0,
    0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f
};

static const FXuchar hand3_mask_bits[] =
{
    0x80, 0x1f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00,
    0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00,
    0xc0, 0x3f, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0x03, 0x00,
    0xc0, 0xff, 0x07, 0x00, 0xc0, 0xff, 0xff, 0x00, 0xc0, 0xff, 0xff, 0x01,
    0xc0, 0xff, 0xff, 0x3f, 0xc0, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f
};


// Predefined zoom factors
#define NB_ZOOM    24
double zoomtab[NB_ZOOM] =
{
    0.01, 0.025, 0.05, 0.075, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1, \
    1.5, 2, 3, 4, 5, 7.5, 10, 15, 20, 30, 50, 75, 100
};
#define ZOOM_100       10


// Maximum image size (in pixels) for zooming in
#define MAX_IMGSIZE    5120

// Patterns for supported image formats
const char* patterns[] =
{
    _("All Files"), "*",
    _("GIF Image"), "*.gif",
    _("BMP Image"), "*.bmp",
    _("XPM Image"), "*.xpm",
    _("PCX Image"), "*.pcx",
    _("ICO Image"), "*.ico",
    _("RGB Image"), "*.rgb",
    _("XBM Image"), "*.xbm",
    _("TARGA Image"), "*.tga",
    _("PPM Image"), "*.ppm",
    _("PNG Image"), "*.png",
    _("JPEG Image"), "*.jpg",
    _("JPEG Image"), "*.jpeg",
    _("TIFF Image"), "*.tif",
    _("TIFF Image"), "*.tiff",
    NULL
};


const FXString imgpatterns = "*.gif,*.bmp,*.xpm,*.pcx,*.ico,*.rgb,*.xbm,*.tga,*.ppm,*.png,*.jpg,*.jpeg,*.tif,*.tiff";


// Helper function to draw a toolbar separator
void toolbarSeparator(FXToolBar* tb)
{
#define SEP_SPACE_TB    1
    new FXFrame(tb, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE_TB);
    new FXVerticalSeparator(tb, LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | SEPARATOR_GROOVE | LAYOUT_FILL_Y);
    new FXFrame(tb, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE_TB);
}


// Helper function to draw a separator in an horizontal frame
void hframeSeparator(FXHorizontalFrame* hframe)
{
#define SEP_SPACE_HF    5
    new FXFrame(hframe, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE_HF);
    new FXVerticalSeparator(hframe, LAYOUT_SIDE_TOP | LAYOUT_CENTER_Y | SEPARATOR_GROOVE | LAYOUT_FILL_Y);
    new FXFrame(hframe, LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE_HF);
}


// Map
FXDEFMAP(XFileImage) XFileImageMap[] =
{
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ABOUT, XFileImage::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_OPEN, XFileImage::onCmdOpen),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TITLE, XFileImage::onUpdTitle),
    FXMAPFUNC(SEL_SIGNAL, XFileImage::ID_HARVEST, XFileImage::onSigHarvest),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_QUIT, XFileImage::onCmdQuit),
    FXMAPFUNC(SEL_SIGNAL, XFileImage::ID_QUIT, XFileImage::onCmdQuit),
    FXMAPFUNC(SEL_CLOSE, XFileImage::ID_TITLE, XFileImage::onCmdQuit),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_RESTART, XFileImage::onCmdRestart),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_TOGGLEFILELIST_BEFORE, XFileImage::onCmdToggleFileListBefore),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_HORZ_PANELS, XFileImage::onCmdHorzVertPanels),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_VERT_PANELS, XFileImage::onCmdHorzVertPanels),
    FXMAPFUNC(SEL_DOUBLECLICKED, XFileImage::ID_FILELIST, XFileImage::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_CLICKED, XFileImage::ID_FILELIST, XFileImage::onCmdItemClicked),
    FXMAPFUNC(SEL_KEYPRESS, 0, XFileImage::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, XFileImage::onKeyRelease),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_RECENTFILE, XFileImage::onCmdRecentFile),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_TOGGLE_HIDDEN, XFileImage::onCmdToggleHidden),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_TOGGLE_THUMBNAILS, XFileImage::onCmdToggleThumbnails),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_SHOW_DETAILS, XFileImage::onCmdShowDetails),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_SHOW_MINI_ICONS, XFileImage::onCmdShowMini),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_SHOW_BIG_ICONS, XFileImage::onCmdShowBig),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ROTATE_90, XFileImage::onCmdRotate),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ROTATE_270, XFileImage::onCmdRotate),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_MIRROR_HOR, XFileImage::onCmdMirror),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_MIRROR_VER, XFileImage::onCmdMirror),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ZOOM_IN, XFileImage::onCmdZoomIn),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ZOOM_OUT, XFileImage::onCmdZoomOut),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ZOOM_100, XFileImage::onCmdZoom100),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_ZOOM_WIN, XFileImage::onCmdZoomWin),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_PRINT, XFileImage::onCmdPrint),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_DIR_UP, XFileImage::onCmdDirUp),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_DIR_UP, XFileImage::onUpdDirUp),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_DIR_BACK, XFileImage::onCmdDirBack),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_DIR_BACK, XFileImage::onUpdDirBack),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_DIR_FORWARD, XFileImage::onCmdDirForward),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_DIR_FORWARD, XFileImage::onUpdDirForward),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_DIR_BACK_HIST, XFileImage::onCmdDirBackHist),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_DIR_BACK_HIST, XFileImage::onUpdDirBackHist),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_DIR_FORWARD_HIST, XFileImage::onCmdDirForwardHist),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_DIR_FORWARD_HIST, XFileImage::onUpdDirForwardHist),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_GO_HOME, XFileImage::onCmdHome),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_PRINT, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ROTATE_90, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ROTATE_270, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_MIRROR_HOR, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_MIRROR_VER, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_VIEW_PREV, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_VIEW_NEXT, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ZOOM_IN, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ZOOM_OUT, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ZOOM_100, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_ZOOM_WIN, XFileImage::onUpdImage),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_SHOW_BIG_ICONS, XFileImage::onUpdFileView),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_SHOW_MINI_ICONS, XFileImage::onUpdFileView),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_SHOW_DETAILS, XFileImage::onUpdFileView),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TOGGLE_HIDDEN, XFileImage::onUpdToggleHidden),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TOGGLEFILELIST_BEFORE, XFileImage::onUpdToggleFileListBefore),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TOGGLE_THUMBNAILS, XFileImage::onUpdToggleThumbnails),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_TOGGLE_FIT_WIN, XFileImage::onCmdToggleFitWin),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_TOGGLE_FILTER_IMAGES, XFileImage::onCmdToggleFilterImages),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TOGGLE_FIT_WIN, XFileImage::onUpdToggleFitWin),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_TOGGLE_FILTER_IMAGES, XFileImage::onUpdToggleFilterImages),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_HORZ_PANELS, XFileImage::onUpdHorzVertPanels),
    FXMAPFUNC(SEL_UPDATE, XFileImage::ID_VERT_PANELS, XFileImage::onUpdHorzVertPanels),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_GO_HOME, XFileImage::onCmdHome),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_GO_WORK, XFileImage::onCmdWork),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, XFileImage::ID_FILELIST, XFileImage::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_POPUP_MENU, XFileImage::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_VIEW_PREV, XFileImage::onCmdViewPrev),
    FXMAPFUNC(SEL_COMMAND, XFileImage::ID_VIEW_NEXT, XFileImage::onCmdViewNext),
};


// Object implementation
FXIMPLEMENT(XFileImage, FXMainWindow, XFileImageMap, ARRAYNUMBER(XFileImageMap))


// Construct
XFileImage::XFileImage(FXApp* a, FXbool smoothscroll, FXColor listbackcolor,
                       FXColor listforecolor, FXColor highlightcolor) :
                       FXMainWindow(a, "Xfi ", NULL, NULL, DECOR_TITLE | DECOR_MINIMIZE | DECOR_MAXIMIZE |
                                    DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE)
{
    setIcon(xfiicon);

    FXButton* btn = NULL;
    FXHotKey hotkey;
    FXString key;

    setTarget(this);
    setSelector(ID_TITLE);

    // Make menu bar
    menubar = new FXMenuBar(this, LAYOUT_DOCK_NEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE);

    // Sites where to dock
    FXDockSite* topdock = new FXDockSite(this, LAYOUT_SIDE_TOP | LAYOUT_FILL_X);
    new FXDockSite(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    new FXDockSite(this, LAYOUT_SIDE_LEFT | LAYOUT_FILL_Y);
    new FXDockSite(this, LAYOUT_SIDE_RIGHT | LAYOUT_FILL_Y);

    // Tool bar
    FXToolBarShell* dragshell1 = new FXToolBarShell(this, FRAME_NONE);
    toolbar = new FXToolBar(topdock, dragshell1,
                            LAYOUT_DOCK_NEXT | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE);
    new FXToolBarGrip(toolbar, toolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_DOUBLE);

    // File menu
    filemenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&File"), NULL, filemenu);

    // Image Menu
    imagemenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&Image"), NULL, imagemenu);

    // View menu
    viewmenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&View"), NULL, viewmenu);

    // Preferences Menu
    prefsmenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&Preferences"), NULL, prefsmenu);

    // Help menu
    helpmenu = new FXMenuPane(this);
    new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    // Splitter
    FXVerticalFrame* vframe = new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // Set order of the file list and image widgets
    filelistbefore = getApp()->reg().readUnsignedEntry("OPTIONS", "filelist_before", false);
    if (filelistbefore)
    {
        splitter = new FXSplitter(vframe,
                                  LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_TRACKING |
                                  SPLITTER_VERTICAL);
        filebox = new FXVerticalFrame(splitter, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0);
        imageview = new FXImageView(splitter, NULL, NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE);
    }
    else
    {
        splitter = new FXSplitter(vframe,
                                  LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_TRACKING |
                                  SPLITTER_VERTICAL | SPLITTER_REVERSED);
        imageview = new FXImageView(splitter, NULL, NULL, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE);
        filebox = new FXVerticalFrame(splitter, LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0);
    }

    // Stack panels horizontally or vertically
    vertpanels = getApp()->reg().readUnsignedEntry("OPTIONS", "vertical_panels", false);
    if (vertpanels)
    {
        splitter->setSplitterStyle(splitter->getSplitterStyle() & ~SPLITTER_VERTICAL);
    }
    else
    {
        splitter->setSplitterStyle(splitter->getSplitterStyle() | SPLITTER_VERTICAL);
    }

    // Container for the action buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(filebox, LAYOUT_SIDE_TOP | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0,
                                                       0, 5, 5, 5, 5, 0, 0);

    // Container for the path linker
    FXHorizontalFrame* pathframe = new FXHorizontalFrame(filebox, LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                         0);

    // File list
    FXuint options;
    if (smoothscroll)
    {
        options = LAYOUT_FILL_X | LAYOUT_FILL_Y | ICONLIST_MINI_ICONS | ICONLIST_BROWSESELECT;
    }
    else
    {
        options = LAYOUT_FILL_X | LAYOUT_FILL_Y | ICONLIST_MINI_ICONS | ICONLIST_BROWSESELECT | SCROLLERS_DONT_TRACK;
    }

    thumbnails = getApp()->reg().readUnsignedEntry("OPTIONS", "thumbnails", 0);

    // Read file list columns order and shown status
    FXuint idCol[NMAX_COLS] = { 0 };
    FXbool colShown[FileList::ID_COL_NAME + NMAX_COLS] = { 0 };
    FXuint nbCols = 0;

    FXuint i = FileList::ID_COL_NAME;
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_name", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);
    colShown[i++] = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);

    FXuint id = 0;
    i = 0;

    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_0", FileList::ID_COL_NAME);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_1", FileList::ID_COL_SIZE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_2", FileList::ID_COL_TYPE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_3", FileList::ID_COL_EXT);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_4", FileList::ID_COL_DATE);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_5", FileList::ID_COL_USER);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_6", FileList::ID_COL_GROUP);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_7", FileList::ID_COL_PERMS);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    id = getApp()->reg().readUnsignedEntry("SETTINGS", "id_col_8", FileList::ID_COL_LINK);
    if (id > 0 && colShown[id])
    {
        idCol[i++] = id;
    }
    nbCols = i;

    // File list
    filelist = new FileList(this, filebox, idCol, nbCols, this, ID_FILELIST, thumbnails, 0, 0, options);
    filelist->setTextColor(listforecolor);
    filelist->setBackColor(listbackcolor);

    // Set list headers size
    for (FXuint i = 0; i < nbCols; i++)
    {
        FXuint size = 0;
        FXuint id = filelist->getHeaderId(i);

        switch (id)
        {
        case FileList::ID_COL_NAME:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "name_size", MIN_NAME_SIZE);
            break;

        case FileList::ID_COL_SIZE:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "size_size", 60);
            break;

        case FileList::ID_COL_TYPE:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "type_size", 100);
            break;

        case FileList::ID_COL_EXT:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "ext_size", 100);
            break;

        case FileList::ID_COL_DATE:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "date_size", 150);
            break;

        case FileList::ID_COL_USER:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "user_size", 50);
            break;

        case FileList::ID_COL_GROUP:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "group_size", 50);
            break;

        case FileList::ID_COL_PERMS:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "perms_size", 100);
            break;

        case FileList::ID_COL_LINK:
            size = getApp()->reg().readUnsignedEntry("OPTIONS", "link_size", 100);
            break;
        }

        filelist->setHeaderSize(i, size);
    }

    // Action buttons
    new FXFrame(buttons, LAYOUT_FIX_WIDTH, 0, 0, 4, 1);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_back", "Ctrl-Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Previous Folder") + PARS(key), minidirbackicon, this, ID_DIR_BACK,
                       BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);
    btnbackhist = new FXArrowButton(buttons, this, ID_DIR_BACK_HIST,
                                    LAYOUT_FILL_Y | FRAME_GROOVE | ARROW_DOWN | ARROW_TOOLBAR);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_forward", "Shift-Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Next Folder") + PARS(key), minidirforwardicon, this, ID_DIR_FORWARD,
                       BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);
    btnforwardhist = new FXArrowButton(buttons, this, ID_DIR_FORWARD_HIST,
                                       LAYOUT_FILL_Y | FRAME_GROOVE | ARROW_DOWN | ARROW_TOOLBAR);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    btn = new FXButton(buttons, TAB + _("Go to Parent Folder") + PARS(key), minidirupicon, this, ID_DIR_UP,
                       BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Separator
    hframeSeparator(buttons);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_home", "Ctrl-H");
    new FXButton(buttons, TAB + _("Go to Home Folder") + PARS(key), minihomeicon, this, ID_GO_HOME,
                 BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_work", "Shift-F2");
    new FXButton(buttons, TAB + _("Go to Working Folder") + PARS(key), miniworkicon, this, ID_GO_WORK,
                 BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Separator
    hframeSeparator(buttons);

    // Switch display modes
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    btn = new FXButton(buttons, TAB + _("Big Icon List") + PARS(key), minibigiconsicon, this, ID_SHOW_BIG_ICONS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    btn = new FXButton(buttons, TAB + _("Small Icon List") + PARS(key), minismalliconsicon, this, ID_SHOW_MINI_ICONS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    btn = new FXButton(buttons, TAB + _("Detailed File List") + PARS(key), minidetailsicon, this, ID_SHOW_DETAILS,
                       BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Separator
    hframeSeparator(buttons);

    // Vertical panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    btn = new FXButton(buttons, TAB + _("Vertical Panels") + PARS(key), minivertpanelsicon, this,
                       XFileImage::ID_VERT_PANELS,
                       BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Horizontal panels
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");
    btn = new FXButton(buttons, TAB + _("Horizontal Panels") + PARS(key), minihorzpanelsicon, this,
                       XFileImage::ID_HORZ_PANELS,
                       BUTTON_TOOLBAR | FRAME_GROOVE | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT);
    hotkey = xf_parseaccel(key);
    btn->addHotKey(hotkey);

    // Panel title
    pathtext = new TextLabel(pathframe, 0, this, 0, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    pathtext->setBackColor(getApp()->getBaseColor());

    // Path linker
    pathlink = new PathLinker(pathframe, filelist, NULL, LAYOUT_FILL_X);

    // Status bar
    statusbar = new FXHorizontalFrame(vframe, JUSTIFY_LEFT | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 3, 3, 0, 0);

    // Read and set sort function for file list
    FXString sort_func = getApp()->reg().readStringEntry("OPTIONS", "sort_func", "ascendingCase");
    if (sort_func == "ascendingCase")
    {
        filelist->setSortFunc(filelist->ascendingCase);
    }
    if (sort_func == "ascendingCaseMix")
    {
        filelist->setSortFunc(filelist->ascendingCaseMix);
    }
    else if (sort_func == "descendingCase")
    {
        filelist->setSortFunc(filelist->descendingCase);
    }
    else if (sort_func == "descendingCaseMix")
    {
        filelist->setSortFunc(filelist->descendingCaseMix);
    }
    else if (sort_func == "ascending")
    {
        filelist->setSortFunc(filelist->ascending);
    }
    else if (sort_func == "ascendingMix")
    {
        filelist->setSortFunc(filelist->ascendingMix);
    }
    else if (sort_func == "descending")
    {
        filelist->setSortFunc(filelist->descending);
    }
    else if (sort_func == "descendingMix")
    {
        filelist->setSortFunc(filelist->descendingMix);
    }
    else if (sort_func == "ascendingSize")
    {
        filelist->setSortFunc(filelist->ascendingSize);
    }
    else if (sort_func == "ascendingSizeMix")
    {
        filelist->setSortFunc(filelist->ascendingSizeMix);
    }
    else if (sort_func == "descendingSize")
    {
        filelist->setSortFunc(filelist->descendingSize);
    }
    else if (sort_func == "descendingSizeMix")
    {
        filelist->setSortFunc(filelist->descendingSizeMix);
    }
    else if (sort_func == "ascendingType")
    {
        filelist->setSortFunc(filelist->ascendingType);
    }
    else if (sort_func == "ascendingTypeMix")
    {
        filelist->setSortFunc(filelist->ascendingTypeMix);
    }
    else if (sort_func == "descendingType")
    {
        filelist->setSortFunc(filelist->descendingType);
    }
    else if (sort_func == "descendingTypeMix")
    {
        filelist->setSortFunc(filelist->descendingTypeMix);
    }
    else if (sort_func == "ascendingExt")
    {
        filelist->setSortFunc(filelist->ascendingExt);
    }
    else if (sort_func == "ascendingExtMix")
    {
        filelist->setSortFunc(filelist->ascendingExtMix);
    }
    else if (sort_func == "descendingExt")
    {
        filelist->setSortFunc(filelist->descendingExt);
    }
    else if (sort_func == "descendingExtMix")
    {
        filelist->setSortFunc(filelist->descendingExtMix);
    }
    else if (sort_func == "ascendingDate")
    {
        filelist->setSortFunc(filelist->ascendingDate);
    }
    else if (sort_func == "ascendingDateMix")
    {
        filelist->setSortFunc(filelist->ascendingDateMix);
    }
    else if (sort_func == "descendingDate")
    {
        filelist->setSortFunc(filelist->descendingDate);
    }
    else if (sort_func == "descendingDateMix")
    {
        filelist->setSortFunc(filelist->descendingDateMix);
    }
    else if (sort_func == "ascendingUser")
    {
        filelist->setSortFunc(filelist->ascendingUser);
    }
    else if (sort_func == "ascendingUserMix")
    {
        filelist->setSortFunc(filelist->ascendingUserMix);
    }
    else if (sort_func == "descendingUser")
    {
        filelist->setSortFunc(filelist->descendingUser);
    }
    else if (sort_func == "descendingUserMix")
    {
        filelist->setSortFunc(filelist->descendingUserMix);
    }
    else if (sort_func == "ascendingGroup")
    {
        filelist->setSortFunc(filelist->ascendingGroup);
    }
    else if (sort_func == "ascendingGroupMix")
    {
        filelist->setSortFunc(filelist->ascendingGroupMix);
    }
    else if (sort_func == "descendingGroup")
    {
        filelist->setSortFunc(filelist->descendingGroup);
    }
    else if (sort_func == "descendingGroupMix")
    {
        filelist->setSortFunc(filelist->descendingGroupMix);
    }
    else if (sort_func == "ascendingPerms")
    {
        filelist->setSortFunc(filelist->ascendingPerms);
    }
    else if (sort_func == "ascendingPermsMix")
    {
        filelist->setSortFunc(filelist->ascendingPermsMix);
    }
    else if (sort_func == "descendingPerms")
    {
        filelist->setSortFunc(filelist->descendingPerms);
    }
    else if (sort_func == "descendingPermsMix")
    {
        filelist->setSortFunc(filelist->descendingPermsMix);
    }
    else if (sort_func == "ascendingLink")
    {
        filelist->setSortFunc(filelist->ascendingLink);
    }
    else if (sort_func == "ascendingLinkMix")
    {
        filelist->setSortFunc(filelist->ascendingLinkMix);
    }
    else if (sort_func == "descendingLink")
    {
        filelist->setSortFunc(filelist->descendingLink);
    }
    else if (sort_func == "descendingLinkMix")
    {
        filelist->setSortFunc(filelist->descendingLinkMix);
    }

    // Single click navigation
    single_click = getApp()->reg().readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        filelist->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }

    // Status bar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    new FXToggleButton(statusbar, TAB + _("Show Hidden Files") + PARS(key), TAB + _("Hide Hidden Files") + PARS(key),
                       minishowhiddenicon, minihidehiddenicon, this->filelist,
                       FileList::ID_TOGGLE_HIDDEN, BUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    new FXToggleButton(statusbar, TAB + _("Show Thumbnails") + PARS(key), TAB + _("Hide Thumbnails") + PARS(key),
                       minishowthumbicon, minihidethumbicon, this->filelist,
                       FileList::ID_TOGGLE_THUMBNAILS, BUTTON_TOOLBAR | LAYOUT_LEFT | ICON_BEFORE_TEXT);

    new FXStatusBar(statusbar, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);
    new FXDragCorner(statusbar);

    // Toolbar button: Open file
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    new FXButton(toolbar, TAB + _("Open") + PARS(key) + TAB + _("Open image file.") + PARS(key), minifileopenicon, this,
                 ID_OPEN, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Toolbar button: Print
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "print", "Ctrl-P");
    new FXButton(toolbar, TAB + _("Print") + PARS(key) + TAB + _("Print image file.") + PARS(key), miniprinticon, this,
                 ID_PRINT, BUTTON_TOOLBAR | FRAME_GROOVE);

    // Separator
    toolbarSeparator(toolbar);

    // Previous and next
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_prev", "Ctrl-J");
    new FXButton(toolbar, TAB + _("Previous") + PARS(key) + TAB + _("View previous image.") + PARS(key),
                 minidirbackicon,
                 this, ID_VIEW_PREV, BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_next", "Ctrl-K");
    new FXButton(toolbar, TAB + _("Next") + PARS(key) + TAB + _("View next image.") + PARS(key), minidirforwardicon,
                 this, ID_VIEW_NEXT, BUTTON_TOOLBAR | LAYOUT_CENTER_Y | LAYOUT_LEFT | ICON_BEFORE_TEXT | FRAME_GROOVE);

    // Separator
    toolbarSeparator(toolbar);

    // Note : Ctrl+ and Ctrl- cannot be changed from the registry!

    // Toolbar button: Zoom in
    btn = new FXButton(toolbar, TAB + _("Zoom in") + PARS("Ctrl+") + TAB + _("Zoom in image.") + PARS("Ctrl+"),
                       minizoominicon, this, ID_ZOOM_IN, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);
    hotkey = (CONTROLMASK << 16) | KEY_KP_Add;
    btn->addHotKey(hotkey);

    // Toolbar button: Zoom out
    btn = new FXButton(toolbar, TAB + _("Zoom out") + PARS("Ctrl-") + TAB + _("Zoom out image.") + PARS("Ctrl-"),
                       minizoomouticon, this, ID_ZOOM_OUT, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);
    hotkey = (CONTROLMASK << 16) | KEY_KP_Subtract;
    btn->addHotKey(hotkey);

    // Toolbar button: Zoom 100%
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_100", "Ctrl-I");
    new FXButton(toolbar, TAB + _("Zoom 100%") + PARS(key) + TAB + _("Zoom image to 100%.") + PARS(key),
                 minizoom100icon, this, ID_ZOOM_100, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Toolbar button: Zoom to fit window
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_win", "Ctrl-F");
    new FXButton(toolbar, TAB + _("Zoom to Fit") + PARS(key) + TAB + _("Zoom to fit window.") + PARS(key),
                 minizoomwinicon, this, ID_ZOOM_WIN, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Separator
    toolbarSeparator(toolbar);

    // Toolbar button: Rotate left
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_left", "Ctrl-L");
    new FXButton(toolbar, TAB + _("Rotate Left") + PARS(key) + TAB + _("Rotate left image.") + PARS(key),
                 minirotatelefticon, this, ID_ROTATE_90, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Toolbar button: Rotate right
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_right", "Ctrl-R");
    new FXButton(toolbar, TAB + _("Rotate Right") + PARS(key) + TAB + _("Rotate right image.") + PARS(key),
                 minirotaterighticon, this, ID_ROTATE_270, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Toolbar button: mirror horizontally
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_horizontally", "Ctrl-Shift-H");
    new FXButton(toolbar,
                 TAB + _("Mirror Horizontally") + PARS(key) + TAB + _("Mirror image horizontally.") + PARS(key),
                 minifliplricon, this, ID_MIRROR_HOR, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // Toolbar button: mirror vertically
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_vertically", "Ctrl-Shift-V");
    new FXButton(toolbar, TAB + _("Mirror Vertically") + PARS(key) + TAB + _("Mirror image vertically.") + PARS(key),
                 miniflipudicon, this, ID_MIRROR_VER, ICON_ABOVE_TEXT | BUTTON_TOOLBAR | FRAME_GROOVE);

    // File Menu entries
    FXMenuCommand* mc = NULL;
    FXString text;

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "open", "Ctrl-O");
    text = _("&Open...") + TABS(key) + _("Open image file.") + PARS(key);
    mc = new FXMenuCommand(filemenu, text, minifileopenicon, this, ID_OPEN);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "print", "Ctrl-P");
    text = _("&Print...") + TABS(key) + _("Print image file.") + PARS(key);
    mc = new FXMenuCommand(filemenu, text, miniprinticon, this, ID_PRINT);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    // Recent file menu; this automatically hides if there are no files
    FXMenuSeparator* sep1 = new FXMenuSeparator(filemenu);
    sep1->setTarget(&mrufiles);
    sep1->setSelector(FXRecentFiles::ID_ANYFILES);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_1);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_2);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_3);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_4);
    new FXMenuCommand(filemenu, FXString::null, NULL, &mrufiles, FXRecentFiles::ID_FILE_5);
    new FXMenuCommand(filemenu, _("&Clear Recent Files") + TAB2 + _("Clear recent file menu."), NULL, &mrufiles,
                      FXRecentFiles::ID_CLEAR);
    FXMenuSeparator* sep2 = new FXMenuSeparator(filemenu);
    sep2->setTarget(&mrufiles);
    sep2->setSelector(FXRecentFiles::ID_ANYFILES);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "quit", "Ctrl-Q");
    text = _("&Quit") + TABS(key) + _("Quit Xfi.") + PARS(key);
    mc = new FXMenuCommand(filemenu, text, miniquiticon, this, ID_QUIT);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));
    getAccelTable()->addAccel(KEY_Escape, this, FXSEL(SEL_COMMAND, ID_QUIT));

    // Image Menu entries
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_prev", "Ctrl-J");
    text = _("&Previous") + TABS(key) + _("View previous image.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minidirbackicon, this, ID_VIEW_PREV);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "view_next", "Ctrl-K");
    text = _("&Next") + TABS(key) + _("View next image.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minidirforwardicon, this, ID_VIEW_NEXT);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(viewmenu);

    new FXMenuCommand(imagemenu, _("Zoom &in") + TAB + (FXString)"Ctrl+" + TAB + _("Zoom in image.") + PARS("Ctrl+"),
                      minizoominicon, this, ID_ZOOM_IN);
    new FXMenuCommand(imagemenu, _("Zoom &out") + TAB + (FXString)"Ctrl-" + TAB + _("Zoom out image.") + PARS("Ctrl-"),
                      minizoomouticon, this, ID_ZOOM_OUT);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_100", "Ctrl-I");
    text = _("Zoo&m 100%") + TABS(key) + _("Zoom image to 100%.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minizoom100icon, this, ID_ZOOM_100);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "zoom_win", "Ctrl-F");
    text = _("Zoom to Fit &Window") + TABS(key) + _("Zoom to fit window.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minizoomwinicon, this, ID_ZOOM_WIN);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_right", "Ctrl-R");
    text = _("Rotate &Right") + TABS(key) + _("Rotate right.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minirotaterighticon, this, ID_ROTATE_270);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "rotate_left", "Ctrl-L");
    text = _("Rotate &Left") + TABS(key) + _("Rotate left.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minirotatelefticon, this, ID_ROTATE_90);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_horizontally", "Ctrl-Shift-H");
    text = _("Mirror &Horizontally") + TABS(key) + _("Mirror horizontally.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, minifliplricon, this, ID_MIRROR_HOR);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "mirror_vertically", "Ctrl-Shift-V");
    text = _("Mirror &Vertically") + TABS(key) + _("Mirror vertically.") + PARS(key);
    mc = new FXMenuCommand(imagemenu, text, miniflipudicon, this, ID_MIRROR_VER);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    // View Menu entries
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "hidden_files", "Ctrl-F6");
    text = _("&Hidden Files") + TABS(key) + _("Show hidden files and folders.") + PARS(key);
    mc = new FXMenuCheck(viewmenu, text, this, ID_TOGGLE_HIDDEN);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    text = _("&Thumbnails") + TABS(key) + _("Show image thumbnails.") + PARS(key);
    mc = new FXMenuCheck(viewmenu, text, this, ID_TOGGLE_THUMBNAILS);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(viewmenu);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    text = _("&Big Icons") + TABS(key) + _("Display folders with big icons.") + PARS(key);
    mc = new FXMenuRadio(viewmenu, text, this, ID_SHOW_BIG_ICONS);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    text = _("&Small Icons") + TABS(key) + _("Display folders with small icons.") + PARS(key);
    mc = new FXMenuRadio(viewmenu, text, this, ID_SHOW_MINI_ICONS);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    text = _("&Detailed File List") + TABS(key) + _("Display detailed folder listing.") + PARS(key);
    mc = new FXMenuRadio(viewmenu, text, this, ID_SHOW_DETAILS);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    new FXMenuSeparator(viewmenu);

    mc = new FXMenuRadio(viewmenu, _("&Vertical Panels"), this, XFileImage::ID_VERT_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "vert_panels", "Ctrl-Shift-F1");
    mc->setAccelText(key);

    mc = new FXMenuRadio(viewmenu, _("&Horizontal Panels"), this, XFileImage::ID_HORZ_PANELS);
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "horz_panels", "Ctrl-Shift-F2");
    mc->setAccelText(key);

    new FXMenuSeparator(viewmenu);
    new FXMenuCheck(viewmenu, _("Autos&ize") + TAB2 + _("Autosize icon names."), filelist, FileList::ID_AUTOSIZE);
    new FXMenuRadio(viewmenu, _("&Rows") + TAB2 + _("View icons row-wise."), filelist, FileList::ID_ARRANGE_BY_ROWS);
    new FXMenuRadio(viewmenu, _("&Columns") + TAB2 + _("View icons column-wise."), filelist,
                    FileList::ID_ARRANGE_BY_COLUMNS);

    // Preferences menu
    new FXMenuCheck(prefsmenu, _("&Toolbar") + TAB2 + _("Display toolbar."), toolbar, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(prefsmenu, _("&File List") + TAB2 + _("Display file list."), filebox, FXWindow::ID_TOGGLESHOWN);
    new FXMenuCheck(prefsmenu, _("File List &Before") + TAB2 + _("Display file list before image window."), this,
                    ID_TOGGLEFILELIST_BEFORE);
    new FXMenuCheck(prefsmenu, _("&Filter Images") + TAB2 + _("List only image files."), this, ID_TOGGLE_FILTER_IMAGES);
    new FXMenuCheck(prefsmenu, _("Fit &Window When Opening") + TAB2 + _("Zoom to fit window when opening an image."),
                    this, ID_TOGGLE_FIT_WIN);

    // Help Menu entries
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "help", "F1");
    text = _("&About X File Image") + TABS(key) + _("About X File Image.") + PARS(key);
    mc = new FXMenuCommand(helpmenu, text, NULL, this, ID_ABOUT, 0);
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, mc, FXSEL(SEL_COMMAND, FXMenuCommand::ID_ACCEL));

    // Close accelerator
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, XFileImage::ID_QUIT));

    // Make a tool tip
    new FXToolTip(getApp(), TOOLTIP_NORMAL);

    // Images
    img = NULL;
    tmpimg = NULL;

    // Dialogs
    printdialog = NULL;

    // Recent files
    mrufiles.setTarget(this);
    mrufiles.setSelector(ID_RECENTFILE);

    // Initialize file name
    filename = "";

    // Initialize some flags
    fileview = ID_SHOW_MINI_ICONS;
    hiddenfiles = false;

    // Read settings
    relative_resize = getApp()->reg().readUnsignedEntry("SETTINGS", "relative_resize", false);
    save_win_pos = getApp()->reg().readUnsignedEntry("SETTINGS", "save_win_pos", false);

    // Initialize zoom to 100%
    indZoom = ZOOM_100;
    zoomval = zoomtab[indZoom];
    fitwin = false;
    filterimgs = false;

    // Initialize previous window width / height
    prev_width = getWidth();
    prev_height = getHeight();
}


// Clean up
XFileImage::~XFileImage()
{
    delete toolbar;
    delete menubar;
    delete statusbar;
    delete filemenu;
    delete imagemenu;
    delete helpmenu;
    delete prefsmenu;
    delete viewmenu;
    delete dragshell1;
    delete pathlink;
    delete pathtext;
    delete filelist;
    delete img;
    delete tmpimg;
    delete printdialog;
    delete btnbackhist;
    delete btnforwardhist;
}


long XFileImage::onCmdPopupMenu(FXObject* sender, FXSelector sel, void* ptr)
{
    // Popup menu pane
    FXMenuPane menu(this);
    int x, y;
    FXuint state;

    getRoot()->getCursorPosition(x, y, state);

    new FXMenuCommand(&menu, _("Go Ho&me"), minihomeicon, this, ID_GO_HOME);
    new FXMenuCommand(&menu, _("Go &Work"), miniworkicon, this, ID_GO_WORK);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("&Hidden Files"), this, ID_TOGGLE_HIDDEN);
    new FXMenuCheck(&menu, _("Thum&bnails"), this, ID_TOGGLE_THUMBNAILS);
    new FXMenuSeparator(&menu);
    new FXMenuRadio(&menu, _("B&ig Icons"), this, ID_SHOW_BIG_ICONS);
    new FXMenuRadio(&menu, _("&Small Icons"), this, ID_SHOW_MINI_ICONS);
    new FXMenuRadio(&menu, _("&Detailed File List"), this, ID_SHOW_DETAILS);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("Autos&ize"), filelist, FileList::ID_AUTOSIZE);
    new FXMenuRadio(&menu, _("&Rows"), filelist, FileList::ID_ARRANGE_BY_ROWS);
    new FXMenuRadio(&menu, _("&Columns"), filelist, FileList::ID_ARRANGE_BY_COLUMNS);
    new FXMenuSeparator(&menu);
    new FXMenuRadio(&menu, _("&Name"), filelist, FileList::ID_COL_NAME);
    new FXMenuRadio(&menu, _("Si&ze"), filelist, FileList::ID_COL_SIZE);
    new FXMenuRadio(&menu, _("&Type"), filelist, FileList::ID_COL_TYPE);
    new FXMenuRadio(&menu, _("E&xtension"), filelist, FileList::ID_COL_EXT);
    new FXMenuRadio(&menu, _("&Date"), filelist, FileList::ID_COL_DATE);
    new FXMenuRadio(&menu, _("&User"), filelist, FileList::ID_COL_USER);
    new FXMenuRadio(&menu, _("&Group"), filelist, FileList::ID_COL_GROUP);
    new FXMenuRadio(&menu, _("&Permissions"), filelist, FileList::ID_COL_PERMS);
    new FXMenuRadio(&menu, _("&Link"), filelist, FileList::ID_COL_LINK);
    new FXMenuSeparator(&menu);
    new FXMenuCheck(&menu, _("Ignore C&ase"), filelist, FileList::ID_SORT_CASE);
    new FXMenuCheck(&menu, _("Fold&ers First"), filelist, FileList::ID_DIRS_FIRST);
    new FXMenuCheck(&menu, _("Re&verse Order"), filelist, FileList::ID_SORT_REVERSE);

    menu.create();
    menu.popup(NULL, x, y);
    getApp()->runModalWhileShown(&menu);
    return 1;
}


// If Shift-F10 or Menu is pressed, opens the popup menu
long XFileImage::onKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    // Shift-F10 or Menu was pressed : open popup menu
    if ((event->state & SHIFTMASK && event->code == KEY_F10) || event->code == KEY_Menu)
    {
        this->handle(sender, FXSEL(SEL_COMMAND, XFileImage::ID_POPUP_MENU), ptr);
        return 1;
    }
    // Any other key was pressed : handle the pressed key in the usual way
    else
    {
        if (FXTopWindow::onKeyPress(sender, sel, ptr))
        {
            return 1;
        }
    }

    return 0;
}


long XFileImage::onKeyRelease(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXTopWindow::onKeyRelease(sender, sel, ptr))
    {
        return 1;
    }

    return 0;
}


// User clicked up directory button
long XFileImage::onCmdDirUp(FXObject*, FXSelector, void*)
{
    filelist->setDirectory(FXPath::upLevel(filelist->getDirectory()));
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());

    filelist->setFocus();

    return 1;
}


// Can we still go up
long XFileImage::onUpdDirUp(FXObject* sender, FXSelector, void*)
{
    if (FXPath::isTopDirectory(filelist->getDirectory()))
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL);
    }
    return 1;
}


// Directory back
long XFileImage::onCmdDirBack(FXObject*, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;
    forwardhist = filelist->forwardhist;

    // Get the previous directory
    item = backhist->getFirst();
    if (item)
    {
        pathname = backhist->getString(item);
    }

    // Update the history
    backhist->removeFirstItem();
    forwardhist->insertFirstItem(filelist->getDirectory());

    // Go to to the previous directory
    filelist->setDirectory(pathname, false);
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());

    filelist->setFocus();

    return 1;
}


// Update directory back
long XFileImage::onUpdDirBack(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;

    // Gray out the button if no item in history
    if (backhist->getNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory forward
long XFileImage::onCmdDirForward(FXObject*, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;
    forwardhist = filelist->forwardhist;

    // Get the next directory
    item = forwardhist->getFirst();
    if (item)
    {
        pathname = forwardhist->getString(item);
    }

    // Update the history
    forwardhist->removeFirstItem();
    backhist->insertFirstItem(filelist->getDirectory());

    // Go to to the previous directory
    filelist->setDirectory(pathname, false);
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());

    filelist->setFocus();

    return 1;
}


// Update directory forward
long XFileImage::onUpdDirForward(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* forwardhist;
    FXString pathname;

    // Get the filelist history
    forwardhist = filelist->forwardhist;

    // Gray out the button if no item in history
    if (forwardhist->getNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory back history
long XFileImage::onCmdDirBackHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;
    forwardhist = filelist->forwardhist;

    // Get all string items and display them in a list box
    int num = backhist->getNumItems();
    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = backhist->getFirst();
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = backhist->getString(item);
                dirs[i] = str;
                strlist = strlist + str + "\n";
                item = backhist->getNext(item);
            }
        }

        // Display list box
        int pos = DirHistBox::box(btnbackhist, DECOR_NONE, strlist, this->getX() + 245, this->getY() + 37);

        // If an item was selected
        if (pos != -1)
        {
            // Update back history
            if (pos == num - 1)
            {
                backhist->removeAllItems();
            }
            else
            {
                item = backhist->getItemAtPos(pos + 1);
                backhist->removeAllItemsBefore(item);
            }

            // Update forward history
            forwardhist->insertFirstItem(filelist->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    forwardhist->insertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            filelist->setDirectory(pathname, false);
            pathlink->setPath(filelist->getDirectory());
            pathtext->setText(filelist->getDirectory());
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory back
long XFileImage::onUpdDirBackHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;

    // Gray out the button if no item in history
    if (backhist->getNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Directory forward history
long XFileImage::onCmdDirForwardHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* backhist, * forwardhist;
    StringItem* item;
    FXString pathname;

    // Get the filelist history
    backhist = filelist->backhist;
    forwardhist = filelist->forwardhist;

    // Get all string items and display them in a list box
    int num = forwardhist->getNumItems();
    if (num > 0)
    {
        FXString* dirs = new FXString[num];
        FXString strlist = "";

        // Get string items
        item = forwardhist->getFirst();
        for (int i = 0; i <= num - 1; i++)
        {
            if (item)
            {
                FXString str = forwardhist->getString(item);
                dirs[i] = str;
                strlist = strlist + str + "\n";
                item = forwardhist->getNext(item);
            }
        }

        // Display list box
        int pos = DirHistBox::box(btnforwardhist, DECOR_NONE, strlist, this->getX() + 285, this->getY() + 37);

        // If an item was selected
        if (pos != -1)
        {
            // Update forward history
            if (pos == num - 1)
            {
                forwardhist->removeAllItems();
            }
            else
            {
                item = forwardhist->getItemAtPos(pos + 1);
                forwardhist->removeAllItemsBefore(item);
            }

            // Update back history
            backhist->insertFirstItem(filelist->getDirectory());
            if (pos > 0)
            {
                for (int i = 0; i <= pos - 1; i++)
                {
                    backhist->insertFirstItem(dirs[i]);
                }
            }

            // Go to to the selected directory
            pathname = dirs[pos];
            filelist->setDirectory(pathname, false);
            pathlink->setPath(filelist->getDirectory());
            pathtext->setText(filelist->getDirectory());
        }
        delete[]dirs;
    }

    return 1;
}


// Update directory forward
long XFileImage::onUpdDirForwardHist(FXObject* sender, FXSelector sel, void* ptr)
{
    StringList* forwardhist;
    FXString pathname;

    // Get the filelist history
    forwardhist = filelist->forwardhist;

    // Gray out the button if no item in history
    if (forwardhist->getNumItems() == 0)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), ptr);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), ptr);
    }

    return 1;
}


// Back to home directory
long XFileImage::onCmdHome(FXObject*, FXSelector, void*)
{
    filelist->setDirectory(FXSystem::getHomeDirectory());
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());
    filelist->setFocus();
    return 1;
}


// Back to current working directory
long XFileImage::onCmdWork(FXObject*, FXSelector, void*)
{
    filelist->setDirectory(FXSystem::getCurrentDirectory());
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());
    filelist->setFocus();
    return 1;
}


// View previous image
long XFileImage::onCmdViewPrev(FXObject*, FXSelector, void*)
{
    FXEvent event;

    if (!(filelist->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)))
    {
        event.code = KEY_Up;
    }
    else
    {
        if (filelist->getListStyle() & ICONLIST_COLUMNS)
        {
            event.code = KEY_Left;
        }
        else
        {
            event.code = KEY_Up;
        }
    }

    filelist->handle(this, FXSEL(SEL_KEYPRESS, 0), &event);

    if (!xf_isdirectory(filelist->getCurrentFile()))
    {
        event.code = KEY_Return;
        filelist->handle(this, FXSEL(SEL_KEYPRESS, 0), &event);
    }

    return 1;
}


// View next image
long XFileImage::onCmdViewNext(FXObject*, FXSelector, void*)
{
    FXEvent event;

    if (!(filelist->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)))
    {
        event.code = KEY_Down;
    }
    else
    {
        if (filelist->getListStyle() & ICONLIST_COLUMNS)
        {
            event.code = KEY_Right;
        }
        else
        {
            event.code = KEY_Down;
        }
    }
    filelist->handle(this, FXSEL(SEL_KEYPRESS, 0), &event);

    if (!xf_isdirectory(filelist->getCurrentFile()))
    {
        event.code = KEY_Return;
        filelist->handle(this, FXSEL(SEL_KEYPRESS, 0), &event);
    }


    return 1;
}


// About box
long XFileImage::onCmdAbout(FXObject*, FXSelector, void*)
{
    FXString msg;

    msg.format(_("X File Image Version %s is a simple image viewer.\n\n"), VERSION);
    msg += COPYRIGHT;
    MessageBox about(this, _("About X File Image"), msg.text(), xfiicon, BOX_OK | DECOR_TITLE | DECOR_BORDER,
                     JUSTIFY_CENTER_X | ICON_BEFORE_TEXT | LAYOUT_CENTER_Y | LAYOUT_LEFT | LAYOUT_FILL_X |
                     LAYOUT_FILL_Y);
    about.execute(PLACEMENT_OWNER);

    filelist->setFocus();

    return 1;
}


// Load file
FXbool XFileImage::loadimage(const FXString& file)
{
    FXString ext = FXPath::extension(file);

    FILE* fp = fopen(file.text(), "r");

    if (!fp)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Unable to open file: %s"), file.text());
        return false;
    }
    else
    {
        fclose(fp);
    }

    // Free old image if any, before loading a new one
    if (img)
    {
        delete img;
        img = NULL;
    }
    if (tmpimg)
    {
        delete tmpimg;
        tmpimg = NULL;
    }

    if (comparecase(ext, "gif") == 0)
    {
        img = new FXGIFImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXGIFImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "bmp") == 0)
    {
        img = new FXBMPImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXBMPImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "xpm") == 0)
    {
        img = new FXXPMImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXXPMImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "pcx") == 0)
    {
        img = new FXPCXImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXPCXImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if ((comparecase(ext, "ico") == 0) || (comparecase(ext, "cur") == 0))
    {
        img = new FXICOImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXICOImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "tga") == 0)
    {
        img = new FXTGAImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXTGAImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "rgb") == 0)
    {
        img = new FXRGBImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "xbm") == 0)
    {
        img = new FXXBMImage(getApp(), NULL, NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXXBMImage(getApp(), NULL, NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "ppm") == 0)
    {
        img = new FXPPMImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXPPMImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if (comparecase(ext, "png") == 0)
    {
        img = new FXPNGImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXPNGImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if ((comparecase(ext, "jpg") == 0) || (comparecase(ext, "jpeg") == 0))
    {
        img = new FXJPGImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXJPGImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else if ((comparecase(ext, "tif") == 0) || (comparecase(ext, "tiff") == 0))
    {
        img = new FXTIFImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
        tmpimg = new FXTIFImage(getApp(), NULL, IMAGE_KEEP | IMAGE_SHMI | IMAGE_SHMP);
    }
    else
    {
        img = NULL;
        tmpimg = NULL;
    }

    // Perhaps failed
    if (img == NULL)
    {
        MessageBox::error(this, BOX_OK, _("Error Loading Image"), _("Unsupported type: %s"), ext.text());
        return false;
    }

    if (tmpimg == NULL)
    {
        MessageBox::error(this, BOX_OK, _("Error Loading Image"), _("Unsupported type: %s"), ext.text());
        return false;
    }

    // Load it
    FXFileStream stream;
    if (stream.open(file, FXStreamLoad))
    {
        getApp()->beginWaitCursor();
        FXbool res = img->loadPixels(stream);

        stream.close();

        // If failed
        if (!res)
        {
            MessageBox::error(this, BOX_OK, _("Error Loading Image"),
                              _("Unable to load image, the file may be corrupted"));
            getApp()->endWaitCursor();
            return false;
        }

        if (!FXMEMDUP(&tmpdata, img->getData(), FXColor, img->getWidth() * img->getHeight()))
        {
            throw FXMemoryException(_("Unable to load image"));
        }
        tmpimg->setData(tmpdata, IMAGE_OWNED, img->getWidth(), img->getHeight());

        img->create();
        tmpimg->create();

        imageview->setImage(tmpimg);

        // Initial zoom and image format
        indZoom = ZOOM_100;
        zoomval = zoomtab[indZoom];
        getApp()->endWaitCursor();

        // Zoom to fit window if asked
        if (fitwin)
        {
            handle(this, FXSEL(SEL_COMMAND, ID_ZOOM_WIN), NULL);
        }
    }

    filelist->setDirectory(FXPath::directory(file));
    pathlink->setPath(filelist->getDirectory());
    pathtext->setText(filelist->getDirectory());

    // Select image in file list
    for (int u = 0; u < filelist->getNumItems(); u++)
    {
        FXString name = filelist->getItemFilename(u);

        if (name == FXPath::name(file))
        {
            filelist->selectItem(u);
            break;
        }
    }

    return true;
}


// Toggle file list before image
long XFileImage::onCmdToggleFileListBefore(FXObject* sender, FXSelector, void*)
{
    filelistbefore = !filelistbefore;

    if (BOX_CLICKED_CANCEL != MessageBox::question(this, BOX_OK_CANCEL, _("Restart"),
                              _("Change will be taken into account after restart.\nRestart X File Image now?")))
    {
        this->handle(this, FXSEL(SEL_COMMAND, XFileImage::ID_RESTART), NULL);
    }

    return 1;
}


// Update file list before image
long XFileImage::onUpdToggleFileListBefore(FXObject* sender, FXSelector, void*)
{
    if (filebox->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_ENABLE), NULL);
        if (filelistbefore)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_DISABLE), NULL);
    }

    return 1;
}


// Toggle zoom to fit window on startup
long XFileImage::onCmdToggleFitWin(FXObject*, FXSelector, void*)
{
    fitwin = !fitwin;
    filelist->setFocus();
    return 1;
}


// Update toggle wrap mode
long XFileImage::onUpdToggleFitWin(FXObject* sender, FXSelector, void*)
{
    if (fitwin)
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
    }
    return 1;
}


// Toggle filter image files
long XFileImage::onCmdToggleFilterImages(FXObject*, FXSelector, void*)
{
    filterimgs = !filterimgs;
    if (filterimgs)
    {
        filelist->setPattern(imgpatterns);
    }
    else
    {
        filelist->setPattern("*");
    }

    filelist->setFocus();

    return 1;
}


// Update filter image files
long XFileImage::onUpdToggleFilterImages(FXObject* sender, FXSelector, void*)
{
    // Disable menu item if the file list is not shown
    if (filebox->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);

        // Update menu item
        if (filterimgs)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
        }
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);

        // Update menu item
        if (filterimgs)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
        }
    }
    return 1;
}


// Open
long XFileImage::onCmdOpen(FXObject*, FXSelector, void*)
{
    FileDialog opendialog(this, _("Open Image"));

    opendialog.setSelectMode(SELECTFILE_EXISTING);
    opendialog.setPatternList(patterns);
    opendialog.setDirectory(filelist->getDirectory());
    if (opendialog.execute(PLACEMENT_CURSOR))
    {
        filename = opendialog.getFilename();
        filelist->setCurrentFile(filename);
        mrufiles.appendFile(filename);
        loadimage(filename);
    }

    return 1;
}


// Print the text
long XFileImage::onCmdPrint(FXObject*, FXSelector, void*)
{
    // Read the current print command from the registry
    FXString printcommand, command;

    printcommand = getApp()->reg().readStringEntry("OPTIONS", "print_command", "lpr -P printer");

    // Open print dialog filled with the current print command
    int rc = 1;
    if (printdialog == NULL)
    {
        printdialog = new InputDialog(this, printcommand, _("Print command: \n(ex: lpr -P <printer>)"), _("Print"), "",
                                      bigprinticon);
    }
    printdialog->setText(printcommand);
    printdialog->CursorEnd();
    rc = printdialog->execute(PLACEMENT_CURSOR);
    printcommand = printdialog->getText();

    // If cancel was pressed, exit
    if (!rc)
    {
        return 0;
    }

    // Write the new print command to the registry
    getApp()->reg().writeStringEntry("OPTIONS", "print_command", printcommand.text());

    // Perform the print command
    command = "cat " + filename + " |" + printcommand + " &";
    int ret = system(command.text());
    if (ret < 0)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Can't execute command %s"), command.text());
        return 0;
    }

    return 1;
}


// Handle toggle hidden command
long XFileImage::onCmdToggleHidden(FXObject* sender, FXSelector sel, void* ptr)
{
    filelist->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_TOGGLE_HIDDEN), ptr);
    filelist->setFocus();

    return 1;
}


// Update toggle hidden command
long XFileImage::onUpdToggleHidden(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuint msg = FXWindow::ID_UNCHECK;

    hiddenfiles = filelist->shownHiddenFiles();

    if (hiddenfiles == true)
    {
        msg = FXWindow::ID_CHECK;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);

    // Disable menu item if the file list is not shown
    if (filebox->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Handle toggle hidden command
long XFileImage::onCmdToggleThumbnails(FXObject* sender, FXSelector sel, void* ptr)
{
    filelist->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_TOGGLE_THUMBNAILS), ptr);
    filelist->setFocus();

    return 1;
}


// Update toggle hidden command
long XFileImage::onUpdToggleThumbnails(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuint msg = FXWindow::ID_UNCHECK;

    thumbnails = filelist->shownThumbnails();

    if (thumbnails == true)
    {
        msg = FXWindow::ID_CHECK;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), ptr);

    // Disable menu item if the file list is not shown
    if (filebox->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Show mini icons in file list
long XFileImage::onCmdShowMini(FXObject*, FXSelector, void*)
{
    fileview = ID_SHOW_MINI_ICONS;
    filelist->handle(this, FXSEL(SEL_COMMAND, FileList::ID_SHOW_MINI_ICONS), NULL);
    filelist->setFocus();

    return 1;
}


// Show big icons in file list
long XFileImage::onCmdShowBig(FXObject*, FXSelector, void*)
{
    fileview = ID_SHOW_BIG_ICONS;
    filelist->handle(this, FXSEL(SEL_COMMAND, FileList::ID_SHOW_BIG_ICONS), NULL);
    filelist->setFocus();

    return 1;
}


// Show details in file list
long XFileImage::onCmdShowDetails(FXObject*, FXSelector, void*)
{
    fileview = ID_SHOW_DETAILS;
    filelist->handle(this, FXSEL(SEL_COMMAND, FileList::ID_SHOW_DETAILS), NULL);
    filelist->setFocus();

    return 1;
}


// Update filelist
long XFileImage::onUpdFileView(FXObject* sender, FXSelector sel, void* ptr)
{
    // Keep the filebox width / height relative to the window width / height

    // Get the current width and height
    int width = getWidth();
    int height = getHeight();

    // Panel stacked horizontally
    if (vertpanels)
    {
        if (relative_resize && (prev_width != width))
        {
            // File box shown
            if (filebox->shown())
            {
                filebox->setWidth((int)round(filewidth_pct * width));
            }
        }

        // Update the relative width (only if window width is sufficient)
        prev_width = width;
        if (getWidth() > 10)
        {
            filewidth_pct = (double)(filebox->getWidth()) / (double)(getWidth());
        }
    }
    // Panel stacked vertically
    else
    {
        if (relative_resize && (prev_height != height))
        {
            // File box shown
            if (filebox->shown())
            {
                filebox->setHeight((int)round(fileheight_pct * height));
            }
        }

        // Update the relative height (only if window height is sufficient)
        prev_height = height;
        if (getHeight() > 10)
        {
            fileheight_pct = (double)(filebox->getHeight()) / (double)(getHeight());
        }
    }

    // Update radio buttons
    FXuint msg = FXWindow::ID_UNCHECK;

    switch (FXSELID(sel))
    {
    case ID_SHOW_MINI_ICONS:
        if (fileview == ID_SHOW_MINI_ICONS)
        {
            msg = FXWindow::ID_CHECK;
        }
        break;

    case ID_SHOW_BIG_ICONS:
        if (fileview == ID_SHOW_BIG_ICONS)
        {
            msg = FXWindow::ID_CHECK;
        }
        break;

    case ID_SHOW_DETAILS:
        if (fileview == ID_SHOW_DETAILS)
        {
            msg = FXWindow::ID_CHECK;
        }
        break;
    }
    sender->handle(this, FXSEL(SEL_COMMAND, msg), NULL);

    // Disable menus items if the file list is not shown
    if (filebox->shown())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return 1;
}


// Harvest the zombies
long XFileImage::onSigHarvest(FXObject*, FXSelector, void*)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
    return 1;
}


// Quit
long XFileImage::onCmdQuit(FXObject*, FXSelector, void*)
{
    // Save settings
    saveConfig();

    // Quit
    getApp()->exit(EXIT_SUCCESS);
    return 1;
}


// Update title (display image size and actual zoom)
long XFileImage::onUpdTitle(FXObject* sender, FXSelector, void*)
{
    FXString title = "Xfi " + filename;
    FXImage* image = imageview->getImage();

    if (image && (img != NULL))
    {
        title += " (" + FXStringVal(img->getWidth()) + "x" + FXStringVal(img->getHeight()) + " - " +
                 FXStringVal(zoomval * 100) + "%" ")";
    }
    sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SETSTRINGVALUE), (void*)&title);
    return 1;
}


// Open recent file
long XFileImage::onCmdRecentFile(FXObject*, FXSelector, void* ptr)
{
    filename = (char*)ptr;
    filelist->setCurrentFile(filename);
    loadimage(filename);
    return 1;
}


// Double clicked in the file list
long XFileImage::onCmdItemDoubleClicked(FXObject*, FXSelector, void* ptr)
{
    int index = (int)(FXival)ptr;

    if (0 <= index)
    {
        if (filelist->isItemDirectory(index))
        {
            FXString pathname = filelist->getItemPathname(index);

            // Does not have access
            if (!xf_isreadexecutable(pathname))
            {
                MessageBox::error(this, BOX_OK, _("Error"), _(" Permission to: %s denied."), pathname.text());
                return 0;
            }
            filelist->setDirectory(pathname);
            pathlink->setPath(pathname);
            pathtext->setText(pathname);
        }
        else if (filelist->isItemFile(index))
        {
            filename = filelist->getItemPathname(index);
            mrufiles.appendFile(filename);
            loadimage(filename);
            filelist->setCurrentItem(index);
        }
    }
    return 1;
}


// Single clicked in the file list
long XFileImage::onCmdItemClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    if (single_click != SINGLE_CLICK_NONE)
    {
        // In detailed mode, avoid single click when cursor is not over the first column
        int x, y;
        FXuint state;
        filelist->getCursorPosition(x, y, state);
        FXbool allow = true;
        if (!(filelist->getListStyle() & (ICONLIST_BIG_ICONS | ICONLIST_MINI_ICONS)) &&
            ((x - filelist->getXPosition()) > filelist->getHeaderSize(0)))
        {
            allow = false;
        }

        int index = (int)(FXival)ptr;
        if (0 <= index)
        {
            if (filelist->isItemDirectory(index) && allow)
            {
                FXString pathname = filelist->getItemPathname(index);

                // Does not have access
                if (!xf_isreadexecutable(pathname))
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _(" Permission to: %s denied."), pathname.text());
                    return 0;
                }
                filelist->setDirectory(pathname);
                pathlink->setPath(pathname);
                pathtext->setText(pathname);
            }
            else if ((single_click == SINGLE_CLICK_DIR_FILE) && filelist->isItemFile(index) && allow)
            {
                filename = filelist->getItemPathname(index);
                mrufiles.appendFile(filename);
                loadimage(filename);
                filelist->setCurrentItem(index);
            }
        }
    }
    return 1;
}


// Rotate image
long XFileImage::onCmdRotate(FXObject*, FXSelector sel, void*)
{
    getApp()->beginWaitCursor();
    FXImage* image = imageview->getImage();
    switch (FXSELID(sel))
    {
    case ID_ROTATE_90:

        // Rotate the actual image
        image->rotate(90);

        // Need to also rotate the original image only if the actual size is different
        if ((image->getWidth() != img->getWidth()) || (image->getHeight() != img->getHeight()))
        {
            img->rotate(90);
        }
        break;

    case ID_ROTATE_270:

        // Rotate the actual image
        image->rotate(270);

        // Need to also rotate the original image only if the actual size is different
        if ((image->getWidth() != img->getWidth()) || (image->getHeight() != img->getHeight()))
        {
            img->rotate(270);
        }
        break;
    }
    imageview->setImage(image);
    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Update image
long XFileImage::onUpdImage(FXObject* sender, FXSelector, void*)
{
    if (imageview->getImage())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return 1;
}


// Mirror image
long XFileImage::onCmdMirror(FXObject*, FXSelector sel, void*)
{
    getApp()->beginWaitCursor();
    FXImage* image = imageview->getImage();
    switch (FXSELID(sel))
    {
    case ID_MIRROR_HOR:

        // Mirror the actual image
        image->mirror(true, false);

        // Need to also mirror the original image only if the actual size is different
        if ((image->getWidth() != img->getWidth()) || (image->getHeight() != img->getHeight()))
        {
            img->mirror(true, false);
        }
        break;

    case ID_MIRROR_VER:

        // Mirror the actual image
        image->mirror(false, true);

        // Need to also mirror the original image only if the actual size is different
        if ((image->getWidth() != img->getWidth()) || (image->getHeight() != img->getHeight()))
        {
            img->mirror(false, true);
        }
        break;
    }
    imageview->setImage(image);
    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Zoom in image
long XFileImage::onCmdZoomIn(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();

    // Copy the original image into the actual one
    if (!FXMEMDUP(&tmpdata, img->getData(), FXColor, img->getWidth() * img->getHeight()))
    {
        throw FXMemoryException(_("Unable to load image"));
    }
    tmpimg->setData(tmpdata, IMAGE_OWNED, img->getWidth(), img->getHeight());

    // Resize the actual image according to the new zoom factor
    indZoom += 1;
    if (indZoom > NB_ZOOM - 1)
    {
        indZoom = NB_ZOOM - 1;
    }
    int sx = (int)(tmpimg->getWidth() * zoomtab[indZoom]);
    int sy = (int)(tmpimg->getHeight() * zoomtab[indZoom]);

    // Scale only if the actual image size is different
    if (indZoom == ZOOM_100)
    {
        imageview->setImage(img);
    }
    else
    {
        // Maximum zoom allowed
        if ((sx > MAX_IMGSIZE) || (sy > MAX_IMGSIZE))
        {
            indZoom -= 1;
            if (indZoom < 0)
            {
                indZoom = 0;
            }
            sx = (int)(tmpimg->getWidth() * zoomtab[indZoom]);
            sy = (int)(tmpimg->getHeight() * zoomtab[indZoom]);
        }

        // Scale image according to the new zoom factor
        tmpimg->scale(sx, sy, 1);
        imageview->setImage(tmpimg);
    }

    // Set zoom value for window title
    zoomval = zoomtab[indZoom];

    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Zoom out image
long XFileImage::onCmdZoomOut(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();

    // Copy the original image into the actual one
    if (!FXMEMDUP(&tmpdata, img->getData(), FXColor, img->getWidth() * img->getHeight()))
    {
        throw FXMemoryException(_("Unable to load image"));
    }
    tmpimg->setData(tmpdata, IMAGE_OWNED, img->getWidth(), img->getHeight());

    // Resize the image according to the new zoom factor
    indZoom -= 1;
    if (indZoom < 0)
    {
        indZoom = 0;
    }
    int sx = (int)(tmpimg->getWidth() * zoomtab[indZoom]);
    int sy = (int)(tmpimg->getHeight() * zoomtab[indZoom]);

    // Scale only if the actual image size is different
    if (indZoom == ZOOM_100)
    {
        imageview->setImage(img);
    }
    else
    {
        // Scale image according to the new zoom factor
        tmpimg->scale(sx, sy, 1);
        imageview->setImage(tmpimg);
    }

    // Set zoom value for window title
    zoomval = zoomtab[indZoom];

    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Zoom to 100%
long XFileImage::onCmdZoom100(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();
    indZoom = ZOOM_100;
    zoomval = zoomtab[indZoom];
    imageview->setImage(img);
    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Zoom to fit window
long XFileImage::onCmdZoomWin(FXObject*, FXSelector, void*)
{
    getApp()->beginWaitCursor();

    // Window and image sizes
    int winw = imageview->getWidth();
    int winh = imageview->getHeight();
    int w = img->getWidth();
    int h = img->getHeight();

    // Compute zoom factor
    double fitwin;
    if (double(w) / double(h) > double(winw) / double(winh))
    {
        fitwin = 0.98 * (double)winw / (double)w;
    }
    else
    {
        fitwin = 0.98 * (double)winh / (double)h;
    }

    // Find the most approaching predefined zoom
    // This is used in other zoom functions
    for (int k = 0; k < NB_ZOOM; k++)
    {
        if (zoomtab[k] > fitwin)
        {
            indZoom = k - 1;
            break;
        }
    }
    if (indZoom < 0)
    {
        indZoom = 0;
    }
    if (indZoom >= NB_ZOOM)
    {
        indZoom = NB_ZOOM - 1;
    }

    // Copy the original image into the actual one
    if (!FXMEMDUP(&tmpdata, img->getData(), FXColor, img->getWidth() * img->getHeight()))
    {
        throw FXMemoryException(_("Unable to load image"));
    }
    tmpimg->setData(tmpdata, IMAGE_OWNED, img->getWidth(), img->getHeight());

    // Resize the image according to the new zoom factor
    int sx = (int)(w * fitwin);
    int sy = (int)(h * fitwin);

    // Scale image according to the new zoom factor
    tmpimg->scale(sx, sy, 1);
    imageview->setImage(tmpimg);

    // Set zoom value for window title
    zoomval = fitwin;

    filelist->setFocus();
    getApp()->endWaitCursor();
    return 1;
}


// Restart the application when required
long XFileImage::onCmdRestart(FXObject*, FXSelector, void*)
{
    saveConfig();

    if (fork() == 0) // Child
    {
        execvp("xfi", args);
    }
    else // Parent
    {
        exit(EXIT_SUCCESS);
    }
    return 1;
}


// Start the ball rolling
void XFileImage::start(FXString startimage)
{
    filename = startimage;
    if (filename != "")
    {
        loadimage(filename);
        filelist->setCurrentFile(filename);
    }
}


// Create and show window
void XFileImage::create()
{
    // Get size and position
    FXuint ww = getApp()->reg().readUnsignedEntry("OPTIONS", "width", DEFAULT_WINDOW_WIDTH);            // Workaround for a possible bug in some WMs
    FXuint hh = getApp()->reg().readUnsignedEntry("OPTIONS", "height", DEFAULT_WINDOW_HEIGHT);          // Workaround for a possible bug in some WMs

    filewidth_pct = getApp()->reg().readRealEntry("OPTIONS", "filewidth_pct", 0.25);
    fileheight_pct = getApp()->reg().readRealEntry("OPTIONS", "fileheight_pct", 0.25);
    FXuint fs = getApp()->reg().readIntEntry("OPTIONS", "filesshown", true);

    filelist->setDirectory(FXSystem::getCurrentDirectory());
    pathlink->setPath(FXSystem::getCurrentDirectory());
    pathtext->setText(FXSystem::getCurrentDirectory());

    // Display or hide path linker and path text
    FXbool show_pathlink = getApp()->reg().readUnsignedEntry("SETTINGS", "show_pathlinker", true);
    if (show_pathlink)
    {
        pathtext->hide();
        pathlink->show();
    }
    else
    {
        pathlink->hide();
        pathtext->show();
    }

    // Hide tree if asked for
    if (!fs)
    {
        filebox->hide();
    }

    // Set toolbar status
    if (getApp()->reg().readUnsignedEntry("OPTIONS", "showtoolbar", true) == false)
    {
        toolbar->hide();
    }

    // Set status bar status
    if (getApp()->reg().readUnsignedEntry("OPTIONS", "showstatusbar", true) == false)
    {
        statusbar->hide();
    }

    // Set hidden file status
    hiddenfiles = getApp()->reg().readUnsignedEntry("OPTIONS", "hiddenfiles", 0);
    filelist->showHiddenFiles(hiddenfiles);

    // Set thumbnails status
    thumbnails = getApp()->reg().readUnsignedEntry("OPTIONS", "thumbnails", 0);
    filelist->showThumbnails(thumbnails);

    // Set list style
    liststyle = getApp()->reg().readUnsignedEntry("OPTIONS", "liststyle", ICONLIST_MINI_ICONS | ICONLIST_AUTOSIZE);
    filelist->setListStyle(liststyle | ICONLIST_BROWSESELECT);

    // Set file view
    fileview = getApp()->reg().readUnsignedEntry("OPTIONS", "fileview", ID_SHOW_MINI_ICONS);
    this->handle(this, FXSEL(SEL_COMMAND, fileview), NULL);

    // Set startup zoom
    fitwin = getApp()->reg().readUnsignedEntry("OPTIONS", "fitwin", 0);

    // Set filter images flag
    filterimgs = getApp()->reg().readUnsignedEntry("OPTIONS", "filterimgs", false);

    // Set position and position window
    if (save_win_pos)
    {
        int xpos = getApp()->reg().readIntEntry("OPTIONS", "xpos", DEFAULT_WINDOW_XPOS);
        int ypos = getApp()->reg().readIntEntry("OPTIONS", "ypos", DEFAULT_WINDOW_YPOS);
        position(xpos, ypos, ww, hh);
    }
    else
    {
        position(getX(), getY(), ww, hh);
    }

    // Read filter history
    FXString history = getApp()->reg().readStringEntry("HISTORY", "filter", "");
    FXString histent = "";
    FilterNum = 0;
    if (history != "")
    {
        int i;
        for (i = 0; ; i++)
        {
            if (i < FILTER_HIST_SIZE)
            {
                histent = history.section(':', i);
                if (xf_strequal(histent.text(), ""))
                {
                    break;
                }
                xf_strlcpy(FilterHistory[i], histent.text(), histent.length() + 1);
            }
            else
            {
                break;
            }
        }
        FilterNum = i;
    }

    FXMainWindow::create();

    if (filterimgs)
    {
        filelist->setPattern(imgpatterns);
    }

    // Set filebox width or height
    if (vertpanels)
    {
        filebox->setWidth((int)round(filewidth_pct * getWidth()));
    }
    else
    {
        filebox->setHeight((int)round(fileheight_pct * getHeight()));
    }

    // Set focus on file list
    filelist->setFocus();

    show();

#ifdef STARTUP_NOTIFICATION
    startup_completed();
#endif
}


// Switch between vertical and horizontal panels
long XFileImage::onCmdHorzVertPanels(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_VERT_PANELS:
        splitter->setSplitterStyle(splitter->getSplitterStyle() & ~SPLITTER_VERTICAL);
        vertpanels = true;
        filebox->setWidth((int)round(filewidth_pct * getWidth()));

        break;

    case ID_HORZ_PANELS:
        splitter->setSplitterStyle(splitter->getSplitterStyle() | SPLITTER_VERTICAL);
        vertpanels = false;
        filebox->setHeight((int)round(fileheight_pct * getHeight()));

        break;
    }

    filelist->setFocus();

    return 1;
}


// Update the horizontal / vertical panel radio menus
long XFileImage::onUpdHorzVertPanels(FXObject* sender, FXSelector sel, void* ptr)
{
    sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), ptr);

    if (vertpanels)
    {
        if (FXSELID(sel) == ID_HORZ_PANELS)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_UNCHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_CHECK), ptr);
        }
    }
    else
    {
        if (FXSELID(sel) == ID_VERT_PANELS)
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_UNCHECK), ptr);
        }
        else
        {
            sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_CHECK), ptr);
        }
    }

    return 1;
}


// Save configuration when quitting
void XFileImage::saveConfig()
{
    // Write new window size and position back to registry
    getApp()->reg().writeUnsignedEntry("OPTIONS", "width", (FXuint)getWidth());
    getApp()->reg().writeUnsignedEntry("OPTIONS", "height", (FXuint)getHeight());

    if (save_win_pos)
    {
        // Account for the Window Manager border size
        XWindowAttributes xwattr;
        if (XGetWindowAttributes((Display*)getApp()->getDisplay(), this->id(), &xwattr))
        {
            getApp()->reg().writeIntEntry("OPTIONS", "xpos", getX() - xwattr.x);
            getApp()->reg().writeIntEntry("OPTIONS", "ypos", getY() - xwattr.y);
        }
        else
        {
            getApp()->reg().writeIntEntry("OPTIONS", "xpos", getX());
            getApp()->reg().writeIntEntry("OPTIONS", "ypos", getY());
        }
    }

    // Width and height of filebox
    getApp()->reg().writeRealEntry("OPTIONS", "filewidth_pct", (int)(filewidth_pct * 100) / 100.0);
    getApp()->reg().writeRealEntry("OPTIONS", "fileheight_pct", (int)(fileheight_pct * 100) / 100.0);

    // Was filebox shown
    getApp()->reg().writeIntEntry("OPTIONS", "filesshown", filebox->shown());

    // Toolbar status
    if (toolbar->shown())
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "showtoolbar", true);
    }
    else
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "showtoolbar", false);
    }

    // Hidden files status
    getApp()->reg().writeUnsignedEntry("OPTIONS", "hiddenfiles", hiddenfiles);

    // Thumbnails status
    getApp()->reg().writeUnsignedEntry("OPTIONS", "thumbnails", thumbnails);

    // File view
    getApp()->reg().writeUnsignedEntry("OPTIONS", "fileview", fileview);

    // List style
    getApp()->reg().writeUnsignedEntry("OPTIONS", "liststyle", filelist->getListStyle());

    // Startup zoom
    getApp()->reg().writeUnsignedEntry("OPTIONS", "fitwin", fitwin);

    // Filter images in file list
    getApp()->reg().writeUnsignedEntry("OPTIONS", "filterimgs", filterimgs);

    // Filelist columns sizes
    getApp()->reg().writeUnsignedEntry("OPTIONS", "name_size", filelist->getHeaderSize(0)); // Name is at position 0

    FXbool isShown;
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_size", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "size_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_SIZE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_type", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "type_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_TYPE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_ext", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "ext_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_EXT)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_date", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "date_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_DATE)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_user", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "user_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_USER)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_group", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "group_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_GROUP)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_perms", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "perms_size",
                                           (FXuint)(FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_PERMS)));
    }
    isShown = getApp()->reg().readUnsignedEntry("SETTINGS", "view_col_link", 1);
    if (isShown)
    {
        getApp()->reg().writeUnsignedEntry("OPTIONS", "link_size",
                                           (FXuint)filelist->getHeaderSize((FXuint)filelist->getHeaderIndex(FileList::ID_COL_LINK)));
    }

    // Panel stacking
    getApp()->reg().writeUnsignedEntry("OPTIONS", "filelist_before", filelistbefore);
    getApp()->reg().writeUnsignedEntry("OPTIONS", "vertical_panels", vertpanels);

    // Get and write sort function for search window
    FXString sort_func;
    if (filelist->getSortFunc() == filelist->ascendingCase)
    {
        sort_func = "ascendingCase";
    }
    if (filelist->getSortFunc() == filelist->ascendingCaseMix)
    {
        sort_func = "ascendingCaseMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingCase)
    {
        sort_func = "descendingCase";
    }
    else if (filelist->getSortFunc() == filelist->descendingCaseMix)
    {
        sort_func = "descendingCaseMix";
    }
    else if (filelist->getSortFunc() == filelist->ascending)
    {
        sort_func = "ascending";
    }
    else if (filelist->getSortFunc() == filelist->ascendingMix)
    {
        sort_func = "ascendingMix";
    }
    else if (filelist->getSortFunc() == filelist->descending)
    {
        sort_func = "descending";
    }
    else if (filelist->getSortFunc() == filelist->descendingMix)
    {
        sort_func = "descendingMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingSize)
    {
        sort_func = "ascendingSize";
    }
    else if (filelist->getSortFunc() == filelist->ascendingSizeMix)
    {
        sort_func = "ascendingSizeMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingSize)
    {
        sort_func = "descendingSize";
    }
    else if (filelist->getSortFunc() == filelist->descendingSizeMix)
    {
        sort_func = "descendingSizeMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingType)
    {
        sort_func = "ascendingType";
    }
    else if (filelist->getSortFunc() == filelist->ascendingTypeMix)
    {
        sort_func = "ascendingTypeMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingType)
    {
        sort_func = "descendingType";
    }
    else if (filelist->getSortFunc() == filelist->descendingTypeMix)
    {
        sort_func = "descendingTypeMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingExt)
    {
        sort_func = "ascendingExt";
    }
    else if (filelist->getSortFunc() == filelist->ascendingExtMix)
    {
        sort_func = "ascendingExtMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingExt)
    {
        sort_func = "descendingExt";
    }
    else if (filelist->getSortFunc() == filelist->descendingExtMix)
    {
        sort_func = "descendingExtMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingDate)
    {
        sort_func = "ascendingDate";
    }
    else if (filelist->getSortFunc() == filelist->ascendingDateMix)
    {
        sort_func = "ascendingDateMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingDate)
    {
        sort_func = "descendingDate";
    }
    else if (filelist->getSortFunc() == filelist->descendingDateMix)
    {
        sort_func = "descendingDateMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingUser)
    {
        sort_func = "ascendingUser";
    }
    else if (filelist->getSortFunc() == filelist->ascendingUserMix)
    {
        sort_func = "ascendingUserMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingUser)
    {
        sort_func = "descendingUser";
    }
    else if (filelist->getSortFunc() == filelist->descendingUserMix)
    {
        sort_func = "descendingUserMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingGroup)
    {
        sort_func = "ascendingGroup";
    }
    else if (filelist->getSortFunc() == filelist->ascendingGroupMix)
    {
        sort_func = "ascendingGroupMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingGroup)
    {
        sort_func = "descendingGroup";
    }
    else if (filelist->getSortFunc() == filelist->descendingGroupMix)
    {
        sort_func = "descendingGroupMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingPerms)
    {
        sort_func = "ascendingPerms";
    }
    else if (filelist->getSortFunc() == filelist->ascendingPermsMix)
    {
        sort_func = "ascendingPermsMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingPerms)
    {
        sort_func = "descendingPerms";
    }
    else if (filelist->getSortFunc() == filelist->descendingPermsMix)
    {
        sort_func = "descendingPermsMix";
    }
    else if (filelist->getSortFunc() == filelist->ascendingLink)
    {
        sort_func = "ascendingLink";
    }
    else if (filelist->getSortFunc() == filelist->ascendingLinkMix)
    {
        sort_func = "ascendingLinkMix";
    }
    else if (filelist->getSortFunc() == filelist->descendingLink)
    {
        sort_func = "descendingLink";
    }
    else if (filelist->getSortFunc() == filelist->descendingLinkMix)
    {
        sort_func = "descendingLinkMix";
    }
    else
    {
        sort_func = "ascendingCase";
    }
    getApp()->reg().writeStringEntry("OPTIONS", "sort_func", sort_func.text());

    // Write filter history
    FXString history = "";
    for (int i = 0; i < FilterNum; i++)
    {
        history += FilterHistory[i];
        history += ":";
    }
    if (FilterNum)
    {
        getApp()->reg().writeStringEntry("HISTORY", "filter", history.text());
    }

    // Write registry settings
    getApp()->reg().write();
}


// Usage message
#define USAGE_MSG    _( \
            "\
\nUsage: xfi [options] [image] \n\
\n\
    [options] can be any of the following:\n\
\n\
        -h, --help         Print (this) help screen and exit.\n\
        -v, --version      Print version information and exit.\n\
\n\
    [image] is the path to the image file you want to open on start up.\n\
\n")



// Start the whole thing
int main(int argc, char* argv[])
{
    int i;
    FXString startimage = "";
    const char* appname = "xfi";
    const char* xfename = XFEAPPNAME;
    const char* vdrname = XFEVDRNAME;
    FXbool loadicons;
    FXString xmodifiers;

    // Get environment variables $HOME, $XDG_DATA_HOME and $XDG_CONFIG_HOME
    homedir = FXSystem::getHomeDirectory();
    if (homedir == "")
    {
        homedir = ROOTDIR;
    }
    xdgdatahome = getenv("XDG_DATA_HOME");
    if (xdgdatahome == "")
    {
        xdgdatahome = homedir + PATHSEPSTRING DATAPATH;
    }
    xdgconfighome = getenv("XDG_CONFIG_HOME");
    if (xdgconfighome == "")
    {
        xdgconfighome = homedir + PATHSEPSTRING CONFIGPATH;
    }

#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL.
    setlocale(LC_ALL, "");
#endif

#ifdef ENABLE_NLS
    // Set the text message domain.
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
#endif

    // Parse basic arguments
    for (i = 1; i < argc; ++i)
    {
        if ((compare(argv[i], "-v") == 0) || (compare(argv[i], "--version") == 0))
        {
            fprintf(stdout, "%s version %s\n", PACKAGE, VERSION);
            exit(EXIT_SUCCESS);
        }
        else if ((compare(argv[i], "-h") == 0) || (compare(argv[i], "--help") == 0))
        {
            fprintf(stdout, USAGE_MSG);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Start image, if any
            startimage = argv[i];
        }
    }

    args = argv;

    // Make application
    FXApp* application = new FXApp(appname, vdrname);

    // Open display
    application->init(argc, argv);

    // Read the Xfe registry
    FXRegistry* reg_xfe = new FXRegistry(xfename, vdrname);
    reg_xfe->read();

    // Compute integer and fractional scaling factors depending on the monitor resolution
    FXint res = reg_xfe->readUnsignedEntry("SETTINGS", "screenres", 100);
    scaleint = round(res / 100.0);

    // Redefine the default hand cursor depending on the integer scaling factor
    FXCursor* hand;
    if (scaleint == 1)
    {
        hand = new FXCursor(application, hand1_bits, hand1_mask_bits, hand1_width, hand1_height, hand1_x_hot,
                            hand1_y_hot);
    }
    else if (scaleint == 2)
    {
        hand = new FXCursor(application, hand2_bits, hand2_mask_bits, hand2_width, hand2_height, hand2_x_hot,
                            hand2_y_hot);
    }
    else
    {
        hand = new FXCursor(application, hand3_bits, hand3_mask_bits, hand3_width, hand3_height, hand3_x_hot,
                            hand3_y_hot);
    }
    application->setDefaultCursor(DEF_HAND_CURSOR, hand);

    // Load all application icons
    FXuint iconpathstatus;
    execpath = xf_execpath(argv[0]);
    loadicons = loadAppIcons(application, &iconpathstatus);

    // Set base and border colors (to change the default colors at first run)
    FXColor basecolor = reg_xfe->readColorEntry("SETTINGS", "basecolor", FXRGB(237, 236, 235));
    FXColor bordercolor = reg_xfe->readColorEntry("SETTINGS", "bordercolor", FXRGB(125, 125, 125));
    application->setBaseColor(basecolor);
    application->setBorderColor(bordercolor);

    // Set Xfi normal font according to the Xfe registry
    FXString fontspec;
    fontspec = reg_xfe->readStringEntry("SETTINGS", "font", DEFAULT_NORMAL_FONT);
    if (!fontspec.empty())
    {
        FXFont* normalFont = new FXFont(application, fontspec);
        normalFont->create();
        application->setNormalFont(normalFont);
    }

    // Set Xfi file list colors according to the Xfe registry
    FXColor listbackcolor = reg_xfe->readColorEntry("SETTINGS", "listbackcolor", FXRGB(255, 255, 255));
    FXColor listforecolor = reg_xfe->readColorEntry("SETTINGS", "listforecolor", FXRGB(0, 0, 0));
    FXColor highlightcolor = reg_xfe->readColorEntry("SETTINGS", "highlightcolor", FXRGB(238, 238, 238));

    // Set single click navigation according to the Xfe registry
    FXuint single_click = reg_xfe->readUnsignedEntry("SETTINGS", "single_click", SINGLE_CLICK_NONE);
    application->reg().writeUnsignedEntry("SETTINGS", "single_click", single_click);

    // Set smooth scrolling according to the Xfe registry
    FXbool smoothscroll = reg_xfe->readUnsignedEntry("SETTINGS", "smooth_scroll", true);

    // Set file list tooltip flag according to the Xfe registry
    FXbool file_tooltips = reg_xfe->readUnsignedEntry("SETTINGS", "file_tooltips", true);
    application->reg().writeUnsignedEntry("SETTINGS", "file_tooltips", file_tooltips);

    // Set relative resizing flag according to the Xfe registry
    FXbool relative_resize = reg_xfe->readUnsignedEntry("SETTINGS", "relative_resize", true);
    application->reg().writeUnsignedEntry("SETTINGS", "relative_resize", relative_resize);

    // Set display pathlinker flag according to the Xfe registry
    FXbool show_pathlink = reg_xfe->readUnsignedEntry("SETTINGS", "show_pathlinker", true);
    application->reg().writeUnsignedEntry("SETTINGS", "show_pathlinker", show_pathlink);

    // Get value of the window position flag
    FXbool save_win_pos = reg_xfe->readUnsignedEntry("SETTINGS", "save_win_pos", false);
    application->reg().writeUnsignedEntry("SETTINGS", "save_win_pos", save_win_pos);

    // Delete the Xfe registry
    delete reg_xfe;

    // Make window
    XFileImage* window = new XFileImage(application, smoothscroll, listbackcolor, listforecolor, highlightcolor);

    // Catch SIGCHLD to harvest zombie child processes
    application->addSignal(SIGCHLD, window, XFileImage::ID_HARVEST, true);

    // Handle interrupt to save stuff nicely
    application->addSignal(SIGINT, window, XFileImage::ID_QUIT);

    // Create it
    application->create();

    // Smooth scrolling
    window->setSmoothScroll(smoothscroll);

    // Icon path doesn't exist
    if (iconpathstatus == ICONPATH_NOT_FOUND)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Icon path doesn't exist, default icon path was selected.\
\n\nFrom Xfe, please check your icon path in Edit / Preferences / Appearance..."));
    }

    // Some icons not found
    if (!loadicons && iconpathstatus == ICONPATH_MISSING_ICONS)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Unable to load some icons, default icon theme was selected.\
\n\nFrom Xfe, please check your icon theme in Edit / Preferences / Appearance..."));       
    }

    // Default icon path doesn't exist
    if (iconpathstatus == DEFAULTICONPATH_NOT_FOUND)
    {
        MessageBox::error(application->getRootWindow(), BOX_OK, _("Error loading icons"),
                          _("Unable to load default icons, no icons can be shown.\
\n\nPlease check your Xfe installation..."));       
    }

    // Tooltips setup time and duration
    application->setTooltipPause(TOOLTIP_PAUSE);
    application->setTooltipTime(TOOLTIP_TIME);

    // Start
    window->start(startimage);

    // Run
    return application->run();
}
