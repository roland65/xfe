#ifndef XFEUTILS_H
#define XFEUTILS_H

// The functions xf_comparenat(), xf_comparewnat(), comparenat_left(), comparenat_right()
// comparewnat_left() and comparewnat_right() for natural sort order
// are adapted from the following software:

/*
 * strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
 * Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

// The convaccents() function and the accents table are adapted from code found here:
// http://rosettacode.org/wiki/Natural_sorting


#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <wctype.h>
#include <sys/statvfs.h>
#include <vector>
#include <sys/wait.h>
#include <time.h>

#include <fx.h>


// Global variables
#if defined(linux)
extern FXStringDict* mtdevices;
extern FXStringDict* updevices;
#endif


// Vector of strings
typedef std::vector<FXString>   vector_FXString;


// Single click types
enum
{
    SINGLE_CLICK_NONE,
    SINGLE_CLICK_DIR,
    SINGLE_CLICK_DIR_FILE,
};


// Wait cursor states
enum
{
    BEGIN_CURSOR,
    END_CURSOR,
    QUERY_CURSOR
};


// Indexes of default programs
enum
{
    NONE         = 0,
    TXTVIEWER    = 1,
    TXTEDITOR    = 2,
    IMGVIEWER    = 3,
    IMGEDITOR    = 4,
    PDFVIEWER    = 5,
    AUDIOPLAYER  = 6,
    VIDEOPLAYER  = 7,
    ARCHIVER     = 8,
};


// Start directory modes
enum
{
    START_HOMEDIR        = 0,
    START_CURRENTDIR     = 1,
    START_LASTDIR        = 2,
};


// New tab directory modes
enum
{
    NEWTAB_HOMEDIR        = 0,
    NEWTAB_CURRENTDIR     = 1,
    NEWTAB_ROOTDIR        = 2,
};


// Note : some inline functions must be declared in the header file or they won't compile!


// Convert a character to lower case
static inline int tolowercase(int c)
{
    return 'A' <= c && c <= 'Z' ? c + 32 : c;
}


static inline int comparenat_right(const char* a, const char* b)
{
    int bias = 0;

    /* The longest run of digits wins.  That aside, the greatest
     * value wins, but we can't know that it will until we've scanned
     * both numbers to know that they have the same magnitude, so we
     * remember it in BIAS. */
    for ( ; ; a++, b++)
    {
        if (!isdigit(*a) && !isdigit(*b))
        {
            return bias;
        }
        else if (!isdigit(*a))
        {
            return -1;
        }
        else if (!isdigit(*b))
        {
            return +1;
        }
        else if (*a < *b)
        {
            if (!bias)
            {
                bias = -1;
            }
        }
        else if (*a > *b)
        {
            if (!bias)
            {
                bias = +1;
            }
        }
        else if (!*a && !*b)
        {
            return bias;
        }
    }

    return 0;
}


static inline int comparenat_left(const char* a, const char* b)
{
    /* Compare two left-aligned numbers: the first to have a
     * different value wins. */
    for ( ; ; a++, b++)
    {
        if (!isdigit(*a) && !isdigit(*b))
        {
            return 0;
        }
        else if (!isdigit(*a))
        {
            return -1;
        }
        else if (!isdigit(*b))
        {
            return +1;
        }
        else if (*a < *b)
        {
            return -1;
        }
        else if (*a > *b)
        {
            return +1;
        }
    }

    return 0;
}


// Perform natural comparison on single byte strings (so foo10 comes after foo2, 0.2foo comes before 10.2foo, etc.)
static inline int xf_comparenat(const char* a, const char* b, FXbool igncase)
{
    int ai, bi;
    char ca, cb;
    int fractional, result;

    ai = bi = 0;
    while (1)
    {
        ca = a[ai];
        cb = b[bi];

        if ((ca == '\t') && (cb == '\t'))
        {
            return 0;
        }

        /* skip over leading spaces or zeros */
        while (isspace(ca))
        {
            ca = a[++ai];
        }

        while (isspace(cb))
        {
            cb = b[++bi];
        }

        /* process run of digits */
        if (isdigit(ca) && isdigit(cb))
        {
            fractional = (ca == '0' || cb == '0');

            if (fractional)
            {
                if ((result = comparenat_left(a + ai, b + bi)) != 0)
                {
                    return result;
                }
            }
            else
            {
                if ((result = comparenat_right(a + ai, b + bi)) != 0)
                {
                    return result;
                }
            }
        }

        if (!ca && !cb)
        {
            /* The strings compare the same.  Perhaps the caller
             *     will want to call strcmp to break the tie. */
            return 0;
        }

        if (igncase)
        {
            ca = tolowercase(ca);
            cb = tolowercase(cb);
        }

        if (ca < cb)
        {
            return -1;
        }
        else if (ca > cb)
        {
            return +1;
        }

        ++ai;
        ++bi;
    }
}


// Lookup table of accents and ligatures
// For comparisons, À is converted to A, é to e, etc.
static const wchar_t* const accents[] =    /* copied from Perl6 code */
{
    L"À", L"A", L"Á", L"A", L"Â", L"A", L"Ã", L"A", L"Ä", L"A", L"Å", L"A", L"à",
    L"a", L"á", L"a", L"â", L"a", L"ã", L"a", L"ä", L"a", L"å", L"a",
    L"Ç", L"C", L"ç", L"c", L"È", L"E", L"É", L"E", L"Ê", L"E", L"Ë",
    L"E", L"è", L"e", L"é", L"e", L"ê", L"e", L"ë", L"e", L"Ì",
    L"I", L"Í", L"I", L"Î", L"I", L"Ï", L"I", L"ì", L"i", L"í",
    L"i", L"î", L"i", L"ï", L"i", L"Ò", L"O", L"Ó", L"O", L"Ô",
    L"O", L"Õ", L"O", L"Ö", L"O", L"Ø", L"O", L"ò", L"o", L"ó", L"o",
    L"ô", L"o", L"õ", L"o", L"ö", L"o", L"ø", L"o", L"Ñ", L"N", L"ñ", L"n",
    L"Ù", L"U", L"Ú", L"U", L"Û", L"U", L"Ü", L"U", L"ù", L"u", L"ú", L"u",
    L"û", L"u", L"ü", L"u", L"Ý", L"Y", L"ÿ", L"y", L"ý", L"y",
    L"Þ", L"TH", L"þ", L"th", L"Ð", L"TH", L"ð", L"th",
    L"Æ", L"AE", L"æ", L"ae", L"ß", L"ss",
    L"ﬄ", L"ffl", L"ﬃ", L"ffi", L"ﬁ", L"fi", L"ﬀ", L"ff", L"ﬂ", L"fl",
    L"ſ", L"s", L"ʒ", L"z", L"ﬆ", L"st", L"œ", L"oe", /* ... come on ... */
};


// Convert accents and ligatures to Ascii for comparison purpose
// So when comparing wide chars, À is evaluated as A, é as e, etc.
static inline wchar_t convaccents(const wchar_t wc, const wchar_t* const* tbl, int len)
{
    // Don't convert an Ascii char
    if (wc < 127)
    {
        return wc;
    }

    wchar_t wr = wc;

    // Search the lookup table
    // and get the converted char if any
    for (int n = 0; n < len; n += 2)
    {
        if (wc != tbl[n][0])
        {
            continue;
        }
        else
        {
            wr = tbl[n + 1][0];
            break;
        }
    }

    return wr;
}


static inline int comparewnat_right(const wchar_t* wa, const wchar_t* wb)
{
    int bias = 0;

    /* The longest run of digits wins.  That aside, the greatest
     * value wins, but we can't know that it will until we've scanned
     * both numbers to know that they have the same magnitude, so we
     * remember it in BIAS. */
    for ( ; ; wa++, wb++)
    {
        if (!iswdigit(*wa) && !iswdigit(*wb))
        {
            return bias;
        }
        else if (!iswdigit(*wa))
        {
            return -1;
        }
        else if (!iswdigit(*wb))
        {
            return +1;
        }
        else if (*wa < *wb)
        {
            if (!bias)
            {
                bias = -1;
            }
        }
        else if (*wa > *wb)
        {
            if (!bias)
            {
                bias = +1;
            }
        }
        else if (!*wa && !*wb)
        {
            return bias;
        }
    }

    return 0;
}


static inline int comparewnat_left(const wchar_t* wa, const wchar_t* wb)
{
    /* Compare two left-aligned numbers: the first to have a
     * different value wins. */
    for ( ; ; wa++, wb++)
    {
        if (!iswdigit(*wa) && !iswdigit(*wb))
        {
            return 0;
        }
        else if (!iswdigit(*wa))
        {
            return -1;
        }
        else if (!iswdigit(*wb))
        {
            return +1;
        }
        else if (*wa < *wb)
        {
            return -1;
        }
        else if (*wa > *wb)
        {
            return +1;
        }
    }

    return 0;
}


// Perform natural comparison on wide strings (so foo10 comes after foo2, 0.2foo comes before 10.2foo, etc.)
static inline int xf_comparewnat(const wchar_t* wa, const wchar_t* wb, int igncase)
{
    wint_t ai, bi;
    wchar_t wca, wcb;
    int fractional, result;

    ai = bi = 0;
    while (1)
    {
        wca = wa[ai];
        wcb = wb[bi];

        /* skip over leading spaces or zeros */
        while (iswspace(wca))
        {
            wca = wa[++ai];
        }

        while (iswspace(wcb))
        {
            wcb = wb[++bi];
        }

        /* convert accents  */
        wca = convaccents(wca, accents, sizeof(accents) / sizeof(wchar_t*));
        wcb = convaccents(wcb, accents, sizeof(accents) / sizeof(wchar_t*));

        /* process run of digits */
        if (iswdigit(wca) && iswdigit(wcb))
        {
            fractional = (wca == L'0' || wcb == L'0');

            if (fractional)
            {
                if ((result = comparewnat_left(wa + ai, wb + bi)) != 0)
                {
                    return result;
                }
            }
            else
            {
                if ((result = comparewnat_right(wa + ai, wb + bi)) != 0)
                {
                    return result;
                }
            }
        }

        if (!wca && !wcb)
        {
            /* The strings compare the same.  Perhaps the caller
             * will want to call strcmp to break the tie. */
            return 0;
        }

        if (igncase)
        {
            wca = towlower(wca);
            wcb = towlower(wcb);
        }

        if (wca < wcb)
        {
            return -1;
        }
        else if (wca > wcb)
        {
            return +1;
        }

        ++ai;
        ++bi;
    }
}


// To test if two strings are equal (strcmp replacement, thanks to Francesco Abbate)
static inline int xf_strequal(const char* a, const char* b)
{
    if ((a == NULL) || (b == NULL))
    {
        return 0;
    }
    return strcmp(a, b) == 0;
}


// Convert a string to lower cases and returns the string size
static inline void xf_tolowercase(char* str)
{
    while (*str)
    {
        *str = tolowercase(*str);
        ++str;
    }
}


// Stat function with a timeout, used to test if a mount point is up or down
// Return 0 on success, -1 on error with errno set and 1 on timeout
static inline int xf_statvfs_timeout(const char* filename, FXuint timeout)
{
    // Create child process
    int pid = fork();

    if (pid == -1)
    {
        return -1;
    }

    if (pid == 0)
    {
        // Here, we are running as the child process!

        struct statvfs buf;
        int ret = statvfs(filename, &buf);
        exit(ret);
    }
    else
    {
        // Here, we are running as the parent process!

        int ret = -1;
        time_t t0;
        time(&t0);

        while (1)
        {
            if ((waitpid(pid, &ret, WNOHANG) == 0))
            {
                // Child is still running, wait
                time_t t;
                time(&t);

                // Timeout exceeded, abort
                if (difftime(t, t0) > timeout)
                {
                    kill(pid, SIGKILL);
                    int ret = waitpid(pid, NULL, WNOHANG);      // Avoids zombies
                    if (ret < 0)                                // Should not happen
                    {
                        fprintf(stderr, "%s", strerror(errno));
                    }
                    return 1;                                   // Return 1 on timeout
                }
            }
            else
            {
                // Child has finished
                if (ret == 0)
                {
                    struct statvfs buf;
                    int ret = statvfs(filename, &buf);
                    return ret;
                }
                else
                {
                    return -1;
                }
            }
        }
    }

    return -1;
}


// Replacement of the stat function
static inline int xf_stat(const char* path, struct stat* buf)
{
#if defined(linux)
    static int ret;

    // It's a mount point
    if (mtdevices != NULL && mtdevices->find(path))
    {
        // Mount point is down
        if (xf_strequal(updevices->find(path), "down"))
        {
            return -1;
        }
        // Mount point is up
        else
        {
            ret = stat(path, buf);
            if ((ret == -1) && (errno != EACCES))
            {
                updevices->remove(path);
                updevices->insert(path, "down");
            }
            return ret;
        }
    }
    // It's not a mount point
    else
#endif
    return stat(path, buf);
}


// Replacement of the lstat function
static inline int xf_lstat(const char* path, struct stat* buf)
{
#if defined(linux)
    static int ret;

    // It's a mount point
    if (mtdevices != NULL && mtdevices->find(path))
    {
        // Mount point is down
        if (xf_strequal(updevices->find(path), "down"))
        {
            return -1;
        }
        // Mount point is up
        else
        {
            ret = lstat(path, buf);
            if ((ret == -1) && (errno != EACCES))
            {
                updevices->remove(path);
                updevices->insert(path, "down");
            }
            return ret;
        }
    }
    // It's not a mount point
    else
#endif
    return lstat(path, buf);
}


FXString xf_mimetype(FXString);
FXlong xf_getavailablespace(const FXString&);
FXHotKey xf_parseaccel(const FXString&);
FXbool xf_existcommand(const FXString);
FXString xf_getkeybinding(FXEvent*);

int xf_mkpath(const char*, mode_t);
FXString xf_create_trashpathname(FXString, FXString);
int xf_create_trashinfo(FXString, FXString, FXString, FXString);
int xf_istextfile(FXString);
FXString xf_quote(FXString);
FXbool xf_isutf8(const char*, FXuint);

size_t xf_strlcpy(char*, const char*, size_t);
size_t xf_strlcat(char*, const char*, size_t);

FXulong xf_dirsize(const char*);
FXulong xf_pathsize(char*, FXuint*, FXuint*, FXulong*, int* = NULL);

FXString xf_humansize(char*);
FXString xf_cleanpath(const FXString);
FXString xf_filepath(const FXString);
FXString xf_filepath(const FXString, const FXString);
FXString xf_realpath(const FXString);
FXString xf_execpath(char*);

FXString xf_filefromuri(FXString);
FXString xf_filetouri(const FXString&);
FXString xf_buildcopyname(const FXString&, const FXbool, FXString&, const FXuint);

FXlong xf_deltime(FXString);
int xf_isemptydir(const FXString);
int xf_hassubdirs(const FXString);
FXbool xf_existfile(const FXString&);
FXbool xf_isdirectory(const FXString&);
FXbool xf_isfile(const FXString&);

FXbool xf_isgroupmember(gid_t);
FXbool xf_iswritable(const FXString&);
FXbool xf_isreadable(const FXString&);
FXbool xf_isreadexecutable(const FXString&);
FXbool xf_islink(const FXString&);
FXbool xf_info(const FXString&, struct stat&);
FXString xf_permissions(FXuint);
FXString xf_readlink(const FXString&);
FXbool xf_isidentical(const FXString&, const FXString&);

int xf_setwaitcursor(FXApp*, FXuint);
int xf_runst(FXString);
FXString xf_getcommandoutput(FXString);
FXIcon* xf_loadiconfile(FXApp*, const FXString, const FXString, const double, const FXColor);

FXString xf_truncline(FXString, FXuint);
FXString xf_multilines(FXString, FXuint);

FXbool xf_keepmount(FXString, FXString);

FXulong xf_getcurrenttime(void);
FXString xf_secondstotimestring(FXuint);

void xf_replacestring(FXchar*, FXint*, FXint, FXint, const FXchar*, FXint);
FXString xf_substitutecase(const FXString&, const FXString&, const FXString&, bool all = true);

FXulong xf_hashfile(const FXchar*);

#endif
