#ifndef HELP_H
#define HELP_H

#include "config.h"
#include "i18n.h"

#define HELP_TEXT    _( \
"\n \
\n \
\n \
                                  XFE, X File Explorer File Manager\n \
\n \
                                    <http://roland65.free.fr/xfe>\n \
                                <http://sourceforge.net/projects/xfe>\n \
\n \
\n \
\n \
 This program is free software; you can redistribute it and/or modify it under the terms of the GNU\n \
 General Public License as published by the Free Software Foundation; either version 2, or (at your option)\n \
 any later version.\n \
\n \
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; \n \
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. \n \
 See the GNU General Public License for more details.\n \
\n \
\n \
\n \
  Description\n \
  =-=-=-=-=-=\n \
\n \
 X File Explorer (Xfe) is a lightweight file manager for X11, written using the FOX toolkit.\n \
 It is desktop independent and can easily be customized.\n \
 It has Commander or Explorer styles and is very fast and small.\n \
 Xfe is based on X Win Commander, written by Maxim Baranov.\n \
\n \
\n \
\n \
  Features\n \
  =-=-=-=-=\n \
\n \
    - Very fast graphic user interface\n \
    - Small memory footprint\n \
    - UTF-8 support\n \
    - HiDPI monitor support\n \
    - Commander/Explorer interface with four file manager modes : a) one panel, b) a folder tree\n \
      and one panel, c) two panels and d) a folder tree and two panels\n \
    - Horizontal or vertical file panels stacking\n \
    - Dockable toolbars\n \
    - Panels synchronization and switching\n \
    - Tabs\n \
    - Places panel with customizable bookmarks\n \
    - Customizable columns in detailed file list mode\n \
    - Integrated text editor and viewer (X File Write, Xfw)\n \
    - Integrated image viewer (X File Image, Xfi)\n \
    - Integrated archive viewer and extractor (X File Archive, Xfa)\n \
    - Integrated package (rpm or deb) viewer / installer / uninstaller (X File Package, Xfp)\n \
    - Custom shell scripts (like Nautilus scripts)\n \
    - Mount/Unmount devices with optional automounter (Linux only, supports MTP devices)\n \
    - Managing network shares (Windows shares or connecting to SSH server)\n \
    - Search files and directories\n \
    - Bulk rename files\n \
    - Natural sort order (foo10.txt comes after foo2.txt...)\n \
    - Copy/Cut/Paste files from and to your favorite desktop (GNOME/KDE/XFCE)\n \
    - Drag and Drop files from and to your favorite desktop (GNOME/KDE/XFCE)\n \
    - Disk usage command \n \
    - Root mode with authentication by pkexec, sudo or su\n \
    - File associations\n \
    - Optional trash can for file delete operations (compliant with freedesktop.org standards)\n \
    - Double click or single click file and folder navigation\n \
    - Mouse right click pop-up menu in tree list and file list\n \
    - Change file attributes\n \
    - Customizable color themes (GNOME, KDE, XFCE, Dark, ...)\n \
    - Warn when mount point are not responding (Linux only)\n \
    - Back and forward history lists for folder navigation\n \
    - Customizable color themes (GNOME, KDE, XFCE, Dark, ...)\n \
    - Icon themes (Default, GNOME, KDE, XFCE, ...)\n \
    - Create archives (tar, zip, gzip, bzip2, xz, zst, 7z, ...)\n \
    - Extract archives (many formats are supported)\n \
    - File / folder comparison (through external tool, ex: meld)\n \
    - Tooltips with file properties\n \
    - Progress bars or dialogs for lengthy file operations\n \
    - Thumbnails image previews\n \
    - Configurable key bindings\n \
    - Startup notification (optional)\n \
    - and much more...\n \
\n \
\n \
\n \
  Default Key bindings\n \
  =-=-=-=-=-=-=-=-=-=-=\n \
\n \
 Below are the global default key bindings. These key bindings are common to all X File applications.\n \
\n \
    * Select all                         - Ctrl-A\n \
    * Copy to clipboard                  - Ctrl-C\n \
    * Search                             - Ctrl-F\n \
    * Search previous                    - Ctrl-Shift-G\n \
    * Search next                        - Ctrl-G\n \
    * Go to home folder                  - Ctrl-H\n \
    * Invert selection                   - Ctrl-I\n \
    * Open file                          - Ctrl-O\n \
    * Print file                         - Ctrl-P\n \
    * Quit application                   - Ctrl-Q\n \
    * Paste from clipboard               - Ctrl-V\n \
    * Close window                       - Ctrl-W\n \
    * Cut to clipboard                   - Ctrl-X\n \
    * Deselect all                       - Ctrl-Z\n \
    * Display help                       - F1\n \
    * Create new file                    - Ctrl-N\n \
    * Create new folder                  - F7\n \
    * Big icon list                      - F10\n \
    * Small icon list                    - F11\n \
    * Detailed file list                 - F12\n \
    * Toggle display hidden files        - Ctrl-F6\n \
    * Toggle display thumbnails          - Ctrl-F7\n \
    * Vertical panels                    - Ctrl-Shift-F1\n \
    * Horizontal panels                  - Ctrl-Shift-F2\n \
    * Go to working folder               - Shift-F2\n \
    * Go to parent folder                - Backspace\n \
    * Go to previous folder              - Ctrl-Backspace\n \
    * Go to next folder                  - Shift-Backspace\n \
\n \
\n \
 Below are the default X File Explorer key bindings. These key bindings are specific to the Xfe application.\n \
\n \
    * Add bookmark                      - Ctrl-B\n \
    * Filter files                      - Ctrl-D\n \
    * Execute command                   - Ctrl-E\n \
    * Create new symbolic link          - Ctrl-J\n \
    * Switch panels                     - Ctrl-K\n \
    * Clear address bar                 - Ctrl-L\n \
    * Mount file system (Linux only)    - Ctrl-M\n \
    * Rename files                      - F2\n \
    * Refresh panels                    - Ctrl-R\n \
    * Symlink files to location         - Ctrl-S\n \
    * Launch terminal                   - Ctrl-T\n \
    * Unmount file system (Linux only)  - Ctrl-U\n \
    * Synchronize panels                - Ctrl-Y\n \
    * Create new window                 - F3\n \
    * Edit                              - F4\n \
    * Copy files to location            - F5\n \
    * Move files to location            - F6\n \
    * File properties                   - F9\n \
    * One panel mode                    - Ctrl-F1\n \
    * Tree and panel mode               - Ctrl-F2\n \
    * Two panels mode                   - Ctrl-F3\n \
    * Tree and two panels mode          - Ctrl-F4\n \
    * Toggle display hidden folders     - Ctrl-F5\n \
    * Go to trash can                   - Ctrl-F8\n \
    * Create new root window            - Shift-F3\n \
    * View                              - Shift-F4\n \
    * Move files to trash can           - Del\n \
    * Restore files from trash can      - Alt-Del\n \
    * Delete files                      - Shift-Del\n \
    * Copy file names to clipboard      - Ctrl-Shift-N\n \
    * Empty trash can                   - Ctrl-Del\n \
    * New Tab                           - Shift-F1\n \
    * Connect to Server                 - Shift-F5\n \
\n \
\n \
 Below are the default X File Image key bindings. These key bindings are specific to the Xfi application.\n \
\n \
    * View previous image               - Ctrl-J\n \
    * View next image                   - Ctrl-K\n \
    * Zoom to fit window                - Ctrl-F\n \
    * Zoom to fit window                - Ctrl-F\n \
    * Mirror image horizontally         - Ctrl-H\n \
    * Zoom image to 100%                - Ctrl-I\n \
    * Rotate image to left              - Ctrl-L\n \
    * Rotate image to right             - Ctrl-R\n \
    * Mirror image vertically           - Ctrl-V\n \
\n \
\n \
 Below are the default X File Write key bindings. These key bindings are specific to the Xfw application.\n \
\n \
    * Toggle word wrap mode             - Ctrl-K\n \
    * Goto line                         - Ctrl-L\n \
    * Create new document               - Ctrl-N\n \
    * Replace string                    - Ctrl-R\n \
    * Save changes to file              - Ctrl-S\n \
    * Toggle line numbers mode          - Ctrl-T\n \
    * Toggle upper case mode            - Ctrl-Shift-U\n \
    * Toggle lower case mode            - Ctrl-U\n \
    * Redo last change                  - Ctrl-Y\n \
    * Undo last change                  - Ctrl-Z\n \
\n \
\n \
 X File Archive (Xfa) and X File Package (Xfp) only use some of the global key bindings.\n \
\n \
 Note that all the default key bindings listed above can be customized in the Xfe Preferences dialog. However,\n \
 some key actions are hardcoded an cannot be changed. These include:\n \
\n \
    * Ctrl-+ and Ctrl--                 - zoom in and zoom out image in Xfi\n \
    * Ctrl-Shift-F10                    - display context menus in Xfe\n \
    * Space                             - select an item in file list\n \
    * Return                            - enter folders in file lists, open files, select button actions, etc.\n \
    * Esc                               - close current dialog, unselect files, etc.\n \
\n \
\n \
\n \
  Drag and Drop operations\n \
  =-=-=-=-=-=-=-=-=-=-=-=-=\n \
\n \
 Dragging a file or group or files (by moving the mouse while maintaining the left button pressed)\n \
 to a folder or a file panel optionally opens a dialog that allows one to select the file operation: copy,\n \
 move, link or cancel.\n \
\n \
\n \
\n \
  Trash system\n \
  =-=-=-=-=-=-=\n \
\n \
 Xfe implements a trash system that is fully compliant with the freedesktop.org standards.\n \
 This allows the user to move files to the trash can and to restore files from within Xfe or your favorite desktop.\n \
 Note that the trash files location is: ~/.local/share/Trash/files\n \
\n \
\n \
\n \
  Configuration\n \
  =-=-=-=-=-=-=\n \
\n \
 You can perform any Xfe customization (layout, file associations, key bindings, etc.) without editing any file\n \
 by hand. However, you may want to understand the configuration principles, because some customizations can also\n \
 easily be done by manually editing the configurations files.\n \
 Be careful to quit Xfe before manually editing any configuration file, otherwise changes could not be taken\n \
 into account.\n \
\n \
 The system-wide configuration file xferc is located in /usr/share/xfe, /usr/local/share/xfe\n \
 or /opt/local/share/xfe, in the given order of precedence.\n \
\n \
 The local configuration files for Xfe, Xfw, Xfi, Xfa and Xfp are now located in the ~/.config/xfe folder.\n \
 They are named xferc, xfwrc, xfirc, xfarc and xfprc.\n \
 \n \
 At the very first Xfe run, the system-wide configuration file is copied into the local configuration file\n \
 ~/.config/xfe/xferc which does not exists yet. If the system-wide configuration file is not found\n \
 (in case of an unusal install place), a dialog asks the user to select the right place. It is thus easier to\n \
 customize Xfe (this is particularly true for the file associations) by hand editing because all the local options\n \
 are located in the same file.\n \
\n \
 Default PNG icons are located in /usr/share/xfe/icons/default-theme or /usr/local/share/xfe/icons/default-theme,\n \
 depending on your installation. You can easily change the icon theme path in the Preferences dialog.\n \
\n \
\n \
\n \
  HiDPI support\n \
  =-=-=-=-=-=-=\n \
\n \
 Starting with version 1.44, Xfe supports HiDPI monitors. All users have to do is to manually adjust the screen\n \
 resolution using the Edit / Preferences / Appearance / DPI option. A value of 200 - 240 dpi should be fine for Ultra\n \
 HD (4K) monitors.\n \
\n \
\n \
\n \
  Dockable Toolbars\n \
  =-=-=-=-=-=-=-=-=\n \
\n \
 Starting with version 2.1, toolbars can be docked to the four corners of the window and their positions are saved\n \
 (Xfe only) if the Auto save layout option is enabled in Edit Preferences / General dialog. To dock a toolbar, \n \
 left click on its handle and drag it to the destination corner, or right click on it and use the popup menu.\n \
\n \
\n \
\n \
  Places\n \
  =-=-=-=\n \
\n \
 Starting with version 2.0, the folder panel features two tabs: one is the classic Tree view and the other\n \
 is a Places view. This Places view allows to view desktop folders (home, downloads, documents, music, ...), of\n \
 drive mounts, network shares and user's bookmarks.\n \
\n \
 Network shares mounted from the desktop using gvfs (GNOME or XFCE) appear in the Places view and can be unmounted\n \
 (KDE network shares are not supported). Mount points for removable devices also appear in the Places view.\n \
\n \
 Bookmarks can be added, removed and customized (order, name and icon) using a context menu.\n \
\n \
 The Places view is optional as is also the Tree view (related settings are located in the\n \
 Preferences / General dialog).\n \
\n \
\n \
\n \
  Tabs\n \
  =-=-=\n \
\n \
 Starting with version 2.1, it is possible to add tabs by pressing Shift-F1 or by clicking on the + icon of the tab\n \
 toolbar. A right click on a tab displays a popup menu that offers to move a tab or close it.\n \
\n \
 The tab toolbar can be always shown or hidden if not used. Options related to tabs usage can be found in the\n \
 Edit / Preferences / Settings dialog.\n \
\n \
\n \
\n \
  Scripts\n \
  =-=-=-=\n \
\n \
 Custom shell scripts can be executed from within Xfe on the files that are selected in a panel. You have to first\n \
 select the files you want to proceed, then right click on the file list and go to the Scripts sub menu. Last, choose\n \
 the script you want to apply on the selected files.\n \
\n \
 The script files must be located in the ~/.config/xfe/scripts folder and have to be executable. You can organize\n \
 this folder as you like by using sub-folders. You can use the Tools / Go to script folder menu item to directly go\n \
 to the script folder and manage it.\n \
\n \
 Here is an example of a simple shell script that list each selected file on the terminal from where Xfe was\n \
 launched:\n \
\n \
 #!/bin/sh\n \
 for arg\n \
 do\n \
 /bin/ls -la \"$arg\"\n \
 done\n \
\n \
 You can of course use programs like xmessage, zenity or kdialog to display a window with buttons that allows you to\n \
 interact with the script. Here is a modification of the above example that uses xmessage:\n \
\n \
 #!/bin/sh\n \
 (\n \
 echo  \"ls -la\"\n \
 for arg\n \
 do\n \
 /bin/ls -la \"$arg\"\n \
 done\n \
 ) | xmessage -file -\n \
\n \
 Most often, it is possible to directly use Nautilus scripts found on the Internet without modifications.\n \
\n \
\n \
\n \
  Search files and directories\n \
  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n \
\n \
\n \
 Xfe can quickly search files and directories by using find and grep command backends. This is done through the\n \
 Tools / Search files menu item (or by using the Ctrl-F shortcut).\n \
\n \
 In the search window, users can then specify usual search patterns like name and text, but more sophisticated search\n \
 options are also available (size, date, permissions, users, groups, follow symlinks and empty files). Results appear\n \
 in a file list and users can use the right click menu to manage their files, the same way as they do in the file\n \
 panels.\n \
\n \
 The search can be interrupted by clicking on the Stop button or pressing the Escape key.\n \
\n \
\n \
\n \
  Bulk rename files\n \
  =-=-=-=-=-=-=-=-=\n \
\n \
 Xfe can perform a bulk rename operation on selected files and folders from the file panels. The rename dialog allows\n \
 to replace a text pattern or insert / overwrite / remove text at a given position. It is also possible to number\n \
 files.\n \
\n \
 Bulk rename can also be used from the search panel results. However, in that case it is not possible to rename\n \
 folders to avoid moving file locations.\n \
\n \
\n \
\n \
  Non Latin based languages\n \
  =-=-=-=-=-=-=-=-=-=-=-=-=\n \
\n \
 Xfe can display its user interface and also the file names in non latin character based languages, provided that you\n \
 have selected a Unicode font that supports your character set. To select a suitable font, use the\n \
 Edit / Preferences / Font menu item.\n \
\n \
 Multilingual Unicode TrueType fonts can be found at this address: http:    //www.slovo.info/unifonts.htm\n \
\n \
\n \
\n \
  Automounting\n \
  =-=-=-=-=-=-=\n \
\n \
 Starting with version 2.1, Xfe has an integrated automounter program that allows to mount removable disks or media\n \
 when they are inserted. The automounter also supports MTP devices (ex: Android phones or tablets).\n \
\n \
 The automounter is optional and should only be used if your system does not already provides that feature. By\n \
 default, it is disabled and can be enabled in the Edit / Preferences / Settings dialog.\n \
\n \
 The automounter relies on udisks2 and gvfs / gio, so these components must be installed on your system. The process\n \
 is called xfe-automount and is launched at first startup of Xfe. It can possibly be added to your session startup\n \
 programs.\n \
\n \
\n \
\n \
  Network Shares\n \
  =-=-=-=-=-=-=-=\n \
\n \
 Starting with version 2.1, it is possible to mount a Windows network share or to connect to an SSH server\n \
 (Linux only, gvfs / gio must be installed). For that, use the Tools / Connect to Server menu item (Shift-F5\n \
 shortcut) or use the Connect to Server toolbar icon. This will display a dialog asking for your connection\n \
 parameters.\n \
\n \
 It is possible to safely retain connection passwords provided that the secret-tool utility is installed on your\n \
 system. Note that in Debian based distributions this utility is provided by the libsecret-tools package. In other\n \
 distributions, it is usually included in the libsecret library.\n \
\n \
\n \
\n \
  Custom Color Themes\n \
  =-=-=-=-=-=-=-=-=-=\n \
 Starting with version 2.1, it is possible to save the current theme to a custom theme, that can be renamed\n \
 or removed. You can thus have different custom color themes and easily switch between them.\n \
\n \
\n \
\n \
  Tips\n \
  =-=-=\n \
\n \
 File list\n \
    - Select files and right click to open a context menu on the selected files\n \
    - Press Ctrl + right click to open a context menu on the file panel\n \
    - When dragging a file/folder to a folder, hold on the mouse on the folder to open it\n \
    - Displayed colums and column order can be selected in the Preferences / File List dialog\n \
\n \
 Tree list\n \
    - Select a folder and right click to open a context menu on the selected folder\n \
    - Press Ctrl + right click to open a context menu on the tree panel\n \
    - When dragging a file/folder to a folder, hold on the mouse on the folder to expand it\n \
\n \
 Copy/paste file path or name\n \
    - Select a file and press Ctrl-C to copy the file path to the clipboard. Then in a dialog or an application,\n \
      press Ctrl-V to paste the file path. Use the Copy name menu item (or press Ctrl-Shift-N) to copy the file\n \
      name (without path) to the clipboard. \n \
    - In a file operation dialog, select a file name in the line containing the source name and paste it directly\n \
      to the destination using the middle button of your mouse. Then modify it to suit your needs.\n \
\n \
 Add files to the clipboard\n \
    - You can select files from a folder, copy them to the clipboard by pressing Ctrl-C. This erases the previous\n \
      clipboard content. Then, you can move to another folder, select other files and add them to the clipboard\n \
      content by pressing Ctrl-Shift-C. This does not erase the previous clipboard content. At last, you can move\n \
      to the destination and press Ctrl-V to copy all the files you have in the clipboard. Of course, this also works\n \
      with Ctrl-X and Ctrl-Shift-X to cut and paste the files.\n \
\n \
 Startup notification\n \
    - Startup notification is the process that displays a feedback (a sandbox cursor or whatever) to the user when\n \
      he has started an action (file copying, application launching, etc.). Depending on the system, there can be\n \
      some issues with startup notification. If Xfe was compiled with startup notification support, the user can\n \
      disable it for all applications at the global Preferences level. He can also disable it for individual\n \
      applications, by using the dedicated option in the first tab of the Properties dialog. This latter way is\n \
      only available when the file is an executable. Disabling startup notification can be useful when starting\n \
      an old application that doesn't support the startup notification protocol (e.g. Xterm).\n \
\n \
  Root mode\n \
    - If pkexec is installed and configured on your system, then this is the best way to switch to root mode\n \
    - If you use the sudo root mode, it can be useful to add password feedback to the sudo command. For this purpose,\n \
      edit your sudoers file like this:\n \
          sudo visudo -f /etc/suoders\n \
      and then add 'pwfeedback' to the default options, as shown below:\n \
          Defaults         env_reset,pwfeedback\n \
      After that, you should see stars (like *****) when you type your password in the small authentication window.\n \
\n \
\n \
\n \
  Bugs\n \
  =-=-=\n \
\n \
 Please report any found bug to Roland Baudin <roland65@free.fr>. Don't forget to mention the Xfe version you use,\n \
 the FOX library version and your system name and version.\n \
\n \
\n \
\n \
  Translations\n \
  =-=-=-=-=-=-=\n \
 \n \
 Xfe is now available in 24 languages but some translations are only partial. To translate Xfe to your language,\n \
 open the Xfe.pot file located in the po folder of the source tree with a software like poedit, kbabel\n \
 or gtranslator and fill it with your translated strings (be careful to the hotkeys and c-format characters),\n \
 and then send it back to me. I'll be pleased to integrate your work in the next Xfe release.\n \
\n \
\n \
\n \
  Patches\n \
  =-=-=-=\n \
\n \
 If you have coded some interesting patch, please send it to me, I will try to include it in the next release...\n \
\n \
 Many thanks to Maxim Baranov for his excellent X Win Commander, to Jeroen van der Zijp for the FOX toolkit\n \
 and  to all people that have provided useful patches, translations, tests and advices.\n \
\n \
 [Last revision: 11/05/2025]\n \
\n \
");

#endif
