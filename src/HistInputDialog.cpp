// Input dialog with history list and an optional check box

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>
#include <gio/gdesktopappinfo.h>

#include "xfedefs.h"
#include "icons.h"
#include "xfeutils.h"
#include "FileDialog.h"
#include "HistInputDialog.h"


// Global variables
extern FXString homedir;


// Object implementation
FXIMPLEMENT(HistComboBox, ComboBox, NULL, 0)

HistComboBox::HistComboBox(FXComposite* p, int cols, FXbool addr, FXbool clrbtn, FXObject* tgt, FXSelector sel, FXuint opts) :
    ComboBox(p, cols, addr, clrbtn, tgt, sel, opts)
{
}


void HistComboBox::create()
{
    ComboBox::create();
    setFocus();
}


FXDEFMAP(HistInputDialog) HistInputDialogMap[] =
{
    FXMAPFUNC(SEL_COMMAND, HistInputDialog::ID_APPLICATION, HistInputDialog::onCmdApplication),
    FXMAPFUNC(SEL_KEYPRESS, 0, HistInputDialog::onCmdKeyPress),
    FXMAPFUNC(SEL_COMMAND, HistInputDialog::ID_BROWSE_PATH, HistInputDialog::onCmdBrowsePath),
    FXMAPFUNC(SEL_UPDATE, HistInputDialog::ID_ACCEPT, HistInputDialog::onUpdAccept),
};


// Object implementation
FXIMPLEMENT(HistInputDialog, DialogBox, HistInputDialogMap, ARRAYNUMBER(HistInputDialogMap))

// Construct a dialog box with an optional check box
HistInputDialog::HistInputDialog(FXWindow* w, FXString inp, FXString message, FXString title, FXString label,
                                 FXIcon* ic, FXbool addr, FXbool clrbtn, FXuint browse, FXbool option, FXString optiontext) :
    DialogBox(w, title, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MAXIMIZE | DECOR_CLOSE)
{
    // Browse type flag
    browsetype = browse;

    // Buttons
    buttons = new FXHorizontalFrame(this, PACK_UNIFORM_WIDTH | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10, 5, 5);

    // Accept
    accept = new FXButton(buttons, _("&Accept"), NULL, this, ID_ACCEPT, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Cancel
    cancel = new FXButton(buttons, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_GROOVE | LAYOUT_RIGHT, 0, 0, 0, 0, 20, 20);

    // Optional check box
    checkbutton = new FXHorizontalFrame(this, JUSTIFY_RIGHT | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 10, 10, 0, 0);

    if (option)
    {
        new FXCheckButton(checkbutton, optiontext + FXString(" "), this, ID_TOGGLE_OPTION);
    }

    // Vertical frame
    FXVerticalFrame* contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    // Icon and first line
    FXMatrix* matrix1 = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix1, "", ic, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    new FXLabel(matrix1, message, NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW | FRAME_NONE);

    if (browse == HIST_INPUT_EXECUTABLE_FILE)
    {
        applications = new FXList(contents, this, ID_APPLICATION,
                                  LIST_SINGLESELECT | FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y);
        applications->setNumVisible(8);
        applications->hide();
    }

    // Label and input field (combo box)
    FXMatrix* matrix2 = new FXMatrix(contents, 3, MATRIX_BY_COLUMNS | LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(matrix2, applications ? _("Command:") : label, NULL, LAYOUT_LEFT | LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    input = new HistComboBox(matrix2, 40, addr, clrbtn, NULL, 0, COMBOBOX_INSERT_LAST | LAYOUT_CENTER_Y | LAYOUT_CENTER_X |
                             LAYOUT_FILL_COLUMN | LAYOUT_FILL_ROW | LAYOUT_FILL_X);
    input->setNumVisible(8);
    input->setText(inp);
    new FXButton(matrix2, _("\tSelect Destination..."), minifiledialogicon, this, ID_BROWSE_PATH, FRAME_GROOVE |
                 LAYOUT_RIGHT | LAYOUT_CENTER_Y, 0, 0, 0, 0, 20, 20);
    if (!xf_isutf8(message.text(), message.length()))
    {
        new FXLabel(contents, _("=> Warning: file name is not UTF-8 encoded!"), NULL, LAYOUT_LEFT |
                    LAYOUT_CENTER_Y | LAYOUT_FILL_ROW);
    }

    // Default initial directory for browsing
    initialdir = homedir;
}


void HistInputDialog::create()
{
    DialogBox::create();
    input->setFocus();
}


void HistInputDialog::CursorEnd()
{
    input->CursorEnd();
    input->setFocus();
}


void HistInputDialog::selectAll()
{
    input->onFwdToText(this, FXSEL(SEL_FOCUSIN, 0), NULL);
    input->onFwdToText(this, FXSEL(SEL_COMMAND, FXTextField::ID_SELECT_ALL), NULL);
}


long HistInputDialog::onCmdKeyPress(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    switch (event->code)
    {
    case KEY_Escape:
        handle(this, FXSEL(SEL_COMMAND, ID_CANCEL), NULL);
        return 1;

    case KEY_KP_Enter:
    case KEY_Return:
        handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);
        return 1;

    default:
        FXTopWindow::onKeyPress(sender, sel, ptr);
        return 1;
    }
    return 0;
}


long HistInputDialog::onCmdBrowsePath(FXObject* sender, FXSelector sel, void* ptr)
{
    FXString title;

    if (browsetype == HIST_INPUT_FOLDER)
    {
        title = _("Select a Destination Folder");
    }
    else if (browsetype == HIST_INPUT_FILE)
    {
        title = _("Select a File");
    }
    else if (browsetype == HIST_INPUT_EXECUTABLE_FILE)
    {
        title = _("Select an Executable File");
    }
    else
    {
        title = _("Select a File or a Destination Folder");
    }

    // File dialog
    FileDialog browse(this, title);

    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };
    browse.setDirectory(initialdir);
    browse.setPatternList(patterns);

    // Browse files in directory, or existing, or mixed mode depending on the flag
    if (browsetype == HIST_INPUT_FOLDER)
    {
        browse.setSelectMode(SELECT_FILE_DIRECTORY);
    }
    else if (browsetype == HIST_INPUT_FILE)
    {
        browse.setSelectMode(SELECT_FILE_EXISTING);
    }
    else if (browsetype == HIST_INPUT_EXECUTABLE_FILE)
    {
        browse.setSelectMode(SELECT_FILE_EXISTING);
    }
    else
    {
        browse.setSelectMode(SELECT_FILE_MIXED);
    }
    if (browse.execute())
    {
        FXString path = browse.getFilename();
        input->setText(path);
    }

    return 1;
}


// Set initial directory
void HistInputDialog::setDirectory(const FXString& path)
{
    initialdir = path;
}


// Update accept button
long HistInputDialog::onUpdAccept(FXObject* sender, FXSelector sel, void* ptr)
{
    if (input->getText() == "")
    {
        accept->disable();
    }
    else
    {
        accept->enable();
    }
    
    return 1;    
}


// GIO applies XDG desktop-file precedence, MIME inheritance and mimeapps.list.
// Keep the existing command execution and association paths: file arguments are
// appended there, so desktop Exec file/URI placeholders must be removed here.
void HistInputDialog::setApplications(const FXString& pathname, const FXString& defaultCommand, FXIcon* defaultIcon)
{
    if (!applications)
    {
        return;
    }

    for (int i = 0; i < applications->getNumItems(); ++i)
    {
        delete static_cast<FXString*>(applications->getItemData(i));
    }

    // Fractional scaling factor
    FXint res = getApp()->reg().readUnsignedEntry("SETTINGS", "screenres", 100);
    double scalefrac = FXMAX(1.0, res / 100.0);

    applications->clearItems();
    input->setText(FXString::null);
    applications->show();

    GFile* file = g_file_new_for_path(pathname.text());
    GFileInfo* info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                        G_FILE_QUERY_INFO_NONE, NULL, NULL);
    const char* type = info ? g_file_info_get_content_type(info) : NULL;

    FXIconSource source(getApp());
    GList* apps = type ? g_app_info_get_all_for_type(type) : NULL;

    for (GList* node = apps; node; node = node->next)
    {
        GAppInfo* app = G_APP_INFO(node->data);
        if (!g_app_info_should_show(app))
        {
            continue;
        }
        const char* exec = g_app_info_get_commandline(app);
        int argc = 0;
        char** argv = NULL;
        if (!exec || !g_shell_parse_argv(exec, &argc, &argv, NULL))
        {
            continue;
        }

        FXString command;
        bool valid = true;

        // Respect the desktop entry terminal requirement.
        bool terminal = G_IS_DESKTOP_APP_INFO(app) &&
                        g_desktop_app_info_get_boolean(G_DESKTOP_APP_INFO(app), "Terminal");
        for (int i = 0; i < argc; ++i)
        {
            FXString arg;
            bool placeholder = false;
            for (const char* c = argv[i]; *c; ++c)
            {
                if (*c != '%')
                {
                    arg += *c; continue;
                }
                ++c;
                if (!*c)
                {
                    valid = false; break;
                }
                if (*c == '%')
                {
                    arg += '%';
                }
                else if (*c == 'c')
                {
                    arg += g_app_info_get_display_name(app);
                }
                else if (*c == 'k' && G_IS_DESKTOP_APP_INFO(app))
                {
                    arg += g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(app));
                }
                else if (*c == 'f' || *c == 'F' || *c == 'u' || *c == 'U' || *c == 'i')
                {
                    placeholder = true;
                }
                else
                {
                    valid = false;
                }
            }
            if (placeholder)
            {
                if (!arg.empty())
                {
                    valid = false;
                }
                continue;
            }
            if (!command.empty())
            {
                command += " ";
            }
            // Keep ordinary executable names readable and compatible with
            // startup notification's executable-name handling.
            if (i == 0 && strspn(arg.text(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_./+-") == (size_t)arg.length())
            {
                command += arg;
            }
            else
            {
                command += xf_quote(arg);
            }
        }
        g_strfreev(argv);
        if (!valid || command.empty())
        {
            continue;
        }
        if (terminal)
        {
            command = FXString(getApp()->reg().readStringEntry("PROGS", "xterm", "xterm -sb")) + " -e " + command;
        }

        FXIcon* icon = NULL;
        GIcon* gicon = g_app_info_get_icon(app);
        if (gicon && G_IS_FILE_ICON(gicon))
        {
            char* path = g_file_get_path(g_file_icon_get_file(G_FILE_ICON(gicon)));
            if (path)
            {
                icon = source.loadIconFile(path); g_free(path);
            }
        }
        else if (gicon && G_IS_THEMED_ICON(gicon))
        {
            const char* const* names = g_themed_icon_get_names(G_THEMED_ICON(gicon));
            const char* roots[] = { "/usr/share/icons/hicolor/32x32/apps", "/usr/share/icons/hicolor/48x48/apps",
                                    "/usr/share/pixmaps", "/usr/share/icons/Yaru/32x32/apps" };
            for (int n = 0; names[n] && !icon; ++n)
            {
                for (unsigned r = 0; r < ARRAYNUMBER(roots) && !icon; ++r)
                {
                    const char* extensions[] = { ".png", ".xpm" };
                    for (unsigned e = 0; e < ARRAYNUMBER(extensions) && !icon; ++e)
                    {
                        FXString name = FXString(names[n]) + extensions[e];
                        if (FXStat::exists(FXString(roots[r]) + "/" + name))
                        {
                            icon = source.loadIconFile(FXString(roots[r]) + "/" + name);
                        }
                    }
                }
            }
        }
        if (icon)
        {
            // Scale to big icon size
            icon->scale(scalefrac * 32, scalefrac * 32);
            icon->create();
        }
        FXint index = applications->appendItem(g_app_info_get_display_name(app), icon ? icon : bigexecicon);
        if (icon)
        {
            applications->setItemIcon(index, icon, true);
        }
        // Store the normalized command separately from the translated label.
        applications->getItem(index)->setData(new FXString(command));
    }
    g_list_free_full(apps, g_object_unref);
    if (info)
    {
        g_object_unref(info);
    }
    g_object_unref(file);
    if (applications->getNumItems() == 0 && defaultCommand.empty())
    {
        int index = applications->appendItem(_("No applications found. Enter a custom command."));
        applications->disableItem(index);
    }
    applications->setSortFunc(FXList::ascendingCase);
    applications->sortItems();
    if (!defaultCommand.empty())
    {
        // Default programs
        FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
        FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
        FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
        FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
        FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
        FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
        FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
        FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

        FXString command;
        if (defaultCommand == "<txtviewer>")
        {
            command = txtviewer;
        }
        else if (defaultCommand == "<txteditor>")
        {
            command = txteditor;
        }
        else if (defaultCommand == "<imgviewer>")
        {
            command = imgviewer;
        }
        else if (defaultCommand == "<imgeditor>")
        {
            command = imgeditor;
        }
        else if (defaultCommand == "<pdfviewer>")
        {
            command = pdfviewer;
        }
        else if (defaultCommand == "<audioplayer>")
        {
            command = audioplayer;
        }
        else if (defaultCommand == "<videoplayer>")
        {
            command = videoplayer;
        }
        else if (defaultCommand == "<archiver>")
        {
            command = archiver;
        }
        else
        {
            command = defaultCommand;
        }

        // Keep Xfe's association visible even when it has no desktop entry or
        // differs from the desktop MIME default. Preserve its command exactly.
        int index = applications->prependItem(command + " (" + _("Default") + ")",
                                              defaultIcon ? defaultIcon : bigexecicon);

        applications->setItemData(index, new FXString(command));
        applications->setCurrentItem(index);
        applications->selectItem(index);
        applications->makeItemVisible(index);
        input->setText(command);
    }
    recalc();
}


long HistInputDialog::onCmdApplication(FXObject*, FXSelector, void*)
{
    FXint index = applications->getCurrentItem();

    if (index >= 0 && applications->getItemData(index))
    {
        input->setText(*static_cast<FXString*>(applications->getItemData(index)));
    }
    return 1;
}


HistInputDialog::~HistInputDialog()
{
    if (applications)
    {
        for (int i = 0; i < applications->getNumItems(); ++i)
        {
            delete static_cast<FXString*>(applications->getItemData(i));
        }
    }
}
