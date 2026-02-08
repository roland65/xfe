// Common Xfe constants


#ifndef CLIPBOARD
#define CLIPBOARD

// Clipboard operations
enum
{
    COPY_CLIPBOARD,
    COPYNAME_CLIPBOARD,
    CUT_CLIPBOARD,
};

#endif


#ifndef COPYRIGHT
#define COPYRIGHT    "Copyright (C) 2002-2026 Roland Baudin (roland65@free.fr)"
#endif

// Default normal font
#ifndef DEFAULT_NORMAL_FONT
#define DEFAULT_NORMAL_FONT    "Sans,100,normal,regular"
#endif

// Default text font
#ifndef DEFAULT_TEXT_FONT
#define DEFAULT_TEXT_FONT    "Mono,100,normal,regular"
#endif

// Default file and directory list time format
#ifndef DEFAULT_TIME_FORMAT
#define DEFAULT_TIME_FORMAT    "%x %X"
#endif

// Default copy suffix
#ifndef DEFAULT_COPY_SUFFIX
#define DEFAULT_COPY_SUFFIX    "copy"
#endif

// Default initial main window width
#ifndef DEFAULT_WINDOW_WIDTH
#define DEFAULT_WINDOW_WIDTH    1024
#endif

// Default initial main window heigth
#ifndef DEFAULT_WINDOW_HEIGHT
#define DEFAULT_WINDOW_HEIGHT    768
#endif

// Default initial main window X position
#ifndef DEFAULT_WINDOW_XPOS
#define DEFAULT_WINDOW_XPOS    50
#endif

// Default initial main window Y position
#ifndef DEFAULT_WINDOW_YPOS
#define DEFAULT_WINDOW_YPOS    50
#endif

// Minimum width of an horizontal splitter
#ifndef MIN_SPLITTER_WIDTH
#define MIN_SPLITTER_WIDTH    100
#endif

// Minimum height of a vertical splitter
#ifndef MIN_SPLITTER_HEIGHT
#define MIN_SPLITTER_HEIGHT    150
#endif

// Maximum number of files that can be opened in a file dialog
#ifndef MAX_OPEN_FILES
#define MAX_OPEN_FILES   1000
#endif

// Maximum length of a file path
#ifndef MAXPATHLEN
#define MAXPATHLEN    4096
#endif

// Maximum length of a command line
#ifndef MAX_COMMAND_SIZE
#define MAX_COMMAND_SIZE    256
#endif

// Maximum length of a filter pattern
#ifndef MAX_PATTERN_SIZE
#define MAX_PATTERN_SIZE    128
#endif

// Maximum length of a find string
#ifndef MAX_FIND_SIZE
#define MAX_FIND_SIZE    256
#endif

// Maximum length of a server name
#ifndef MAX_SERVER_SIZE
#define MAX_SERVER_SIZE    256
#endif

// Maximum length of a share name
#ifndef MAX_SHARE_SIZE
#define MAX_SHARE_SIZE    256
#endif

// Maximum length of a domain name
#ifndef MAX_DOMAIN_SIZE
#define MAX_DOMAIN_SIZE    256
#endif

// Maximum length of a user name
#ifndef MAX_USER_SIZE
#define MAX_USER_SIZE    256
#endif

// Maximum number of characters per line for one line messages
#ifndef MAX_MESSAGE_LENGTH
#define MAX_MESSAGE_LENGTH    96
#endif

// Minimum file name size (in pixels) in detailed view
#ifndef MIN_NAME_SIZE
#define MIN_NAME_SIZE    250
#endif

// Maximum file name size (in pixels) in detailed view
#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE    1000
#endif

// Root directory string
#ifndef ROOTDIR
#define ROOTDIR    "/"
#endif

// Path separator
#ifndef PATHSEPSTRING
#define PATHSEPSTRING    "/"
#endif

// Path separator
#ifndef PATHSEPCHAR
#define PATHSEPCHAR    '/'
#endif

#ifndef NMAX_COLS
// Maximum number of columns in file lists
// In search list => one column more
// In trash => two columns more
#define NMAX_COLS  9
#endif

// Maximum number of tabs
#ifndef NMAX_TABS
#define NMAX_TABS    100
#endif

// Maximum number of path links
#ifndef NMAX_LINKS
#define NMAX_LINKS    100
#endif

// Run history size
#ifndef RUN_HIST_SIZE
#define RUN_HIST_SIZE    100
#endif

// Open with history size
#ifndef OPEN_HIST_SIZE
#define OPEN_HIST_SIZE    100
#endif

// Filter history size
#ifndef FILTER_HIST_SIZE
#define FILTER_HIST_SIZE    100
#endif

// Find history size
#ifndef FIND_HIST_SIZE
#define FIND_HIST_SIZE    100
#endif

// Server history size
#ifndef SERVER_HIST_SIZE
#define SERVER_HIST_SIZE    100
#endif

// Share history size
#ifndef SHARE_HIST_SIZE
#define SHARE_HIST_SIZE    100
#endif

// Domain history size
#ifndef DOMAIN_HIST_SIZE
#define DOMAIN_HIST_SIZE    100
#endif

// User history size
#ifndef USER_HIST_SIZE
#define USER_HIST_SIZE    100
#endif

// Buffer length for file copy
#ifndef BUFFER_COPY_SIZE
#define BUFFER_COPY_SIZE    1048576
#endif

// Maximum number of copy speeds to average
#ifndef NMAX_COPY_SPEED
#define NMAX_COPY_SPEED   20
#endif

// Time interval to check for MTP devices (ms)
#ifndef MTP_CHECK_INTERVAL
#define MTP_CHECK_INTERVAL    1000
#endif

// Maximum length of an MTP name string
#ifndef MAX_MTP_NAME_SIZE
#define MAX_MTP_NAME_SIZE     1024
#endif

// If startup notification is used, this is the timeout value (seconds)
#ifdef STARTUP_NOTIFICATION

#ifndef STARTUP_TIMEOUT
#define STARTUP_TIMEOUT    15
#endif

#endif

// If startup notification is not used, we use a simulation of a startup time (seconds)
#define SIMULATED_STARTUP_TIME    1

// Default timeout for mount points that are not responding in Linux
#if defined(linux)

#ifndef MOUNT_TIMEOUT
#define MOUNT_TIMEOUT    30
#endif

#endif

// Local data path
#ifndef DATAPATH
#define DATAPATH    ".local/share"
#endif

// Local config path
#ifndef CONFIGPATH
#define CONFIGPATH    ".config"
#endif

// Xfe config path
#ifndef XFECONFIGPATH
#define XFECONFIGPATH    "xfe"
#endif

// Scripts path
#ifndef SCRIPTPATH
#define SCRIPTPATH    "scripts"
#endif

// Local trashcan directory path
#ifndef TRASHPATH
#define TRASHPATH    "Trash"
#endif

// Local trashcan directory path for files
#ifndef TRASHFILESPATH
#define TRASHFILESPATH    "Trash/files"
#endif

// Local trashcan directory path for infos
#ifndef TRASHINFOPATH
#define TRASHINFOPATH    "Trash/info"
#endif

// Xfe application name
#ifndef XFEAPPNAME
#define XFEAPPNAME    "xfe"
#endif

// Xfe vendor name
#ifndef XFEVDRNAME
#define XFEVDRNAME    "Xfe"
#endif

// Xfe config file name
#ifndef XFECONFIGNAME
#define XFECONFIGNAME    "xferc"
#endif

// Command to launch Xfe as root with pkexec
#ifndef DEFAULT_PKEXEC_CMD
#define DEFAULT_PKEXEC_CMD    "pkexec xfe"
#endif

// Command to launch Xfe as root with sudo or su, using st as a terminal
#ifndef DEFAULT_SUDO_CMD
#define DEFAULT_SUDO_CMD    "sudo -b xfe"
#endif

#ifndef DEFAULT_SU_CMD
#define DEFAULT_SU_CMD    "su -c 'nohup xfe >& /dev/null &'"
#endif

// Tooltips setup time and duration
#ifndef TOOLTIP_PAUSE
#define TOOLTIP_PAUSE    500
#endif

#ifndef TOOLTIP_TIME
#define TOOLTIP_TIME    10000
#endif

// Coefficient used to darken the sorted column in detailed mode
#ifndef DARKEN_SORT
#define DARKEN_SORT    0.96
#endif

// Default terminal program
#ifndef DEFAULT_TERMINAL
#define DEFAULT_TERMINAL    "xterm -sb"
#endif


// These have to be the same as in xferc.in

// Default text viewer program
#ifndef DEFAULT_TXTVIEWER
#define DEFAULT_TXTVIEWER    "xfw -r"
#endif

// Default text editor program
#ifndef DEFAULT_TXTEDITOR
#define DEFAULT_TXTEDITOR    "xfw"
#endif

// Default file comparator program
#ifndef DEFAULT_FILECOMPARATOR
#define DEFAULT_FILECOMPARATOR    "meld"
#endif

// Default image editor program
#ifndef DEFAULT_IMGEDITOR
#define DEFAULT_IMGEDITOR    "gimp"
#endif

// Default image viewer program
#ifndef DEFAULT_IMGVIEWER
#define DEFAULT_IMGVIEWER    "xfi"
#endif

// Default archiver program
#ifndef DEFAULT_ARCHIVER
#define DEFAULT_ARCHIVER    "xfa"
#endif

// Default PDF viewer program
#ifndef DEFAULT_PDFVIEWER
#define DEFAULT_PDFVIEWER    "atril"
#endif

// Default audio player program
#ifndef DEFAULT_AUDIOPLAYER
#define DEFAULT_AUDIOPLAYER    "audacious"
#endif

// Default video player program
#ifndef DEFAULT_VIDEOPLAYER
#define DEFAULT_VIDEOPLAYER    "vlc"
#endif

// Default mount command
#ifndef DEFAULT_MOUNTCMD
#define DEFAULT_MOUNTCMD    "mount"
#endif

// Default unmount command
#ifndef DEFAULT_UMOUNTCMD
#define DEFAULT_UMOUNTCMD   "umount"
#endif


// FOX hacks

// FXTextField without frame, for clearlooks controls
#define _TEXTFIELD_NOFRAME    0x10000000


// Common macros

// Tab character
#define TAB     (FXString)"\t"
#define TAB2    (FXString)"\t\t"

// Macro to add tab characters before and after a given string
#define TABS(s)    ((FXString)"\t" + (s) + (FXString)"\t")

// Macro to add parentheses before and after a given string
#define PARS(s)    ((FXString)" (" + (s) + (FXString)")")

#if defined(__FreeBSD__)
#define OTHER_PKG    2
#endif

// Linux specials

#if defined(linux)

// fstab path
#ifndef FSTAB_PATH
#define FSTAB_PATH    "/etc/fstab"
#endif

// mtab path
#ifndef MTAB_PATH
#define MTAB_PATH    "/proc/mounts"
#endif

// Package format
#define DEB_PKG      0
#define RPM_PKG      1
#define OTHER_PKG    2

#endif
