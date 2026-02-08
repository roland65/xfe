#ifndef FILE_H
#define FILE_H


#include <fx.h>
#include "DialogBox.h"
#include "OverwriteBox.h"
#include "MessageBox.h"


// File operations
enum
{
    COPY,
    RENAME,
    MOVE,
    SYMLINK,
    DELETE,
    CHMOD,
    CHOWN,
    EXTRACT,
    ARCHIVE,
#if defined(linux)
    PKG_INSTALL,
    PKG_UNINSTALL,
    MOUNT,
    UNMOUNT
#endif
};

// To search visited inodes
struct inodelist
{
    ino_t st_ino = 0;
    inodelist* next = NULL;
};


class File : public DialogBox
{
    FXDECLARE(File)

private:
    FXuint op = 0;
    FXWindow* ownerwin = NULL;
    FXulong totalsourcesize = 0;
    FXString copysuffix;
    FXuint copysuffix_pos = 0;
    FXlong totaldataread = 0;
    FXlong prevTotaldataread = 0;
    FXulong copyspeedSourcesize = 0;
    FXString copyspeedHsourcesize;
    FXuint numsel = 0;
    FXdouble vecCopyspeed[NMAX_COPY_SPEED];
    FXuint numcopyspeed = 0;

protected:

    // Inline function
    // Force check of timeout for progress dialog (to avoid latency problems)
    int checkProgressBarTimeout(void)
    {
        if (getApp()->hasTimeout(this, File::ID_PROGRESSBAR))
        {
            if (getApp()->remainingTimeout(this, File::ID_PROGRESSBAR) == 0)
            {
                getApp()->removeTimeout(this, File::ID_PROGRESSBAR);
                show(PLACEMENT_OWNER);

                getApp()->forceRefresh();
                getApp()->flush();
                return 1;
            }
        }
        return 0;
    }

    void forceTimeout(void);
    void restartTimeout(void);
    FXlong fullread(int, FXuchar*, FXlong);
    FXlong fullwrite(int, const FXuchar*, FXlong);

    FXuint getOverwriteAnswer(FXString, FXString, FXbool restart_timeout = true);
    int copyfile(const FXString&, const FXString&, const FXString&, const FXulong, const FXulong, const FXbool);
    int copyrec(const FXString&, const FXString&, const FXString&, const FXulong, const FXulong, inodelist*,
                const FXbool);
    int copydir(const FXString&, const FXString&, const FXString&, const FXulong, const FXulong, struct stat&,
                inodelist*, const FXbool);
    int rchmod(char*, char*, mode_t, const FXbool, const FXbool);
    int rchown(char*, char*, uid_t, gid_t, const FXbool, const FXbool);

    FXLabel* uplabel = NULL;
    FXLabel* downlabel = NULL;
    FXLabel* datalabel = NULL;
    FXLabel* timelabel = NULL;
    FXString datatext;
    FXString timetext;
    FXProgressBar* progressbar = NULL;
    FXButton* cancelButton = NULL;
    FXbool overwrite = false;
    FXbool overwrite_all = false;
    FXbool skip_all = false;
    FXbool cancelled = false;
    MessageBox* mbox = NULL;

public:
    File()
    {
    }
    ~File();
    void create();

    File(FXWindow* owner, FXString title, const FXuint operation, const FXuint num = 1);

    enum
    {
        ID_CANCEL_BUTTON=DialogBox::ID_LAST,
        ID_PROGRESSBAR,
        ID_SOURCESIZE,
        ID_COPYSPEED,
        ID_LAST
    };


    FXbool isCancelled()
    {
        return cancelled;
    }

    void hideProgressDialog()
    {
        forceTimeout();
    }

    void showProgressDialog()
    {
        restartTimeout();
    }

    FXulong getTotalSourceSize() const
    {
        return totalsourcesize;
    }

    void setTotalSourceSize(FXulong size)
    {
        totalsourcesize = size;
    }

    int copy(const FXString&, const FXString&, const FXString&, const FXulong, const FXulong,
             const FXbool confirm_dialog = true, const FXbool preserve_date = true);
    int fmove(const FXString&, const FXString&, const FXString& hsourcesize = "",
              const FXulong sourcesize = 0, const FXulong tstart = 0, const FXbool restore = false);
    int rename(const FXString&, const FXString&);
    int symlink(const FXString&, const FXString&);
    int remove(const FXString&);

    FXString sourcesize(const FXString, FXulong*, const FXbool url = false);

    int chmod(char*, char*, mode_t, const FXbool, const FXbool dironly = false, const FXbool fileonly = false);
    int chown(char*, char*, uid_t, gid_t, const FXbool, const FXbool dironly = false, const FXbool fileonly = false);
    int extract(const FXString, const FXString, const FXString);
    int archive(const FXString, const FXString);

#if defined(linux)
    int mount(const FXString, const FXString, const FXString);
    int pkgInstall(const FXString, const FXString);
    int pkgUninstall(const FXString, const FXString);

#endif
    long onCmdCancel(FXObject*, FXSelector, void*);
    long onTimeout(FXObject*, FXSelector, void*);
    long onSourceSizeRefresh(FXObject*, FXSelector, void*);
    long onCopySpeed(FXObject*, FXSelector, void*);
};
#endif
