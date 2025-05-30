AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = xfe xfa xfi xfp xfw

xfe_SOURCES = ../st/x.c \
              ../st/st.c \
              icons.cpp \
			  startupnotification.cpp \
			  xfeutils.cpp \
			  ArchInputDialog.cpp \
			  BookmarkDialog.cpp \
			  BrowseInputDialog.cpp \
			  ColorSelector.cpp \
			  ComboBox.cpp \
			  CommandWindow.cpp \
			  ConnectDialog.cpp \
			  DialogBox.cpp \
			  DirHistBox.cpp \
			  DirList.cpp \
			  DirPanel.cpp \
			  ExecuteBox.cpp \
              File.cpp \
			  FileDialog.cpp \
			  FileDict.cpp \
			  FileList.cpp \
			  FilePanel.cpp \
			  FontDialog.cpp \
			  HistInputDialog.cpp \
			  IconList.cpp \
			  InputDialog.cpp \
			  KeyBindings.cpp \
			  KeyBindingsDialog.cpp \
			  MessageBox.cpp \
			  OverwriteBox.cpp \
			  PathLinker.cpp \
			  Places.cpp \
			  Preferences.cpp \
              Properties.cpp \
			  RenameDialog.cpp \
			  SearchPanel.cpp \
			  SearchWindow.cpp \
			  StringList.cpp \
			  TabButtons.cpp \
			  TextLabel.cpp \
			  TextWindow.cpp \
			  XFileExplorer.cpp \
              main.cpp

if STARTUPNOTIFY
xfe_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfe_LDADD = @LIBINTL@ -lutil


xfa_SOURCES = ../st/x.c \
              ../st/st.c \
              icons.cpp \
			  startupnotification.cpp \
			  xfeutils.cpp \
			  ComboBox.cpp \
              CommandWindow.cpp \
			  DialogBox.cpp \
 			  DirHistBox.cpp \
			  DirList.cpp \
			  File.cpp \
              FileDialog.cpp \
              FileDict.cpp \
              FileList.cpp \
			  HistInputDialog.cpp \
			  IconList.cpp \
              InputDialog.cpp \
              MessageBox.cpp \
              OverwriteBox.cpp \
			  PathLinker.cpp \
			  StringList.cpp \
			  TextLabel.cpp \
              XFileArchive.cpp

if STARTUPNOTIFY
xfa_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfa_LDADD = @LIBINTL@ -lutil


xfi_SOURCES = ../st/x.c \
              ../st/st.c \
              icons.cpp \
			  startupnotification.cpp \
			  xfeutils.cpp \
			  ComboBox.cpp \
              CommandWindow.cpp \
			  DialogBox.cpp \
			  DirList.cpp \
 			  DirHistBox.cpp \
			  File.cpp \
              FileDialog.cpp \
              FileDict.cpp \
              FileList.cpp \
			  HistInputDialog.cpp \
			  IconList.cpp \
              InputDialog.cpp \
              MessageBox.cpp \
              OverwriteBox.cpp \
 			  PathLinker.cpp \
 			  StringList.cpp \
 			  TextLabel.cpp \
              XFileImage.cpp

if STARTUPNOTIFY
xfi_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfi_LDADD = @LIBINTL@ -lutil


xfp_SOURCES = ../st/x.c \
              ../st/st.c \
              icons.cpp \
			  startupnotification.cpp \
			  xfeutils.cpp \
			  ComboBox.cpp \
              CommandWindow.cpp \
			  DialogBox.cpp \
 			  DirHistBox.cpp \
			  DirList.cpp \
			  File.cpp \
              FileDialog.cpp \
              FileDict.cpp \
              FileList.cpp \
			  HistInputDialog.cpp \
			  IconList.cpp \
              InputDialog.cpp \
              MessageBox.cpp \
              OverwriteBox.cpp \
			  PathLinker.cpp \
			  StringList.cpp \
			  TextLabel.cpp \
              XFilePackage.cpp

if STARTUPNOTIFY
xfp_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfp_LDADD = @LIBINTL@ -lutil
	  

xfw_SOURCES = ../st/x.c \
              ../st/st.c \
              icons.cpp \
			  startupnotification.cpp \
			  xfeutils.cpp \
			  ComboBox.cpp \
			  ColorSelector.cpp \
			  CommandWindow.cpp \
			  DialogBox.cpp \
 			  DirHistBox.cpp \
			  DirList.cpp \
			  File.cpp \
              FileDialog.cpp \
			  FileDict.cpp \
              FileList.cpp \
			  FontDialog.cpp \
			  HistInputDialog.cpp \
			  IconList.cpp \
              InputDialog.cpp \
			  MessageBox.cpp \
              OverwriteBox.cpp \
			  PathLinker.cpp \
			  StringList.cpp \
			  TextLabel.cpp \
              WriteWindow.cpp \
              XFileWrite.cpp

if STARTUPNOTIFY
xfw_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfw_LDADD = @LIBINTL@ -lutil

				


localedir = $(datadir)/locale

AM_CPPFLAGS = -I. -I$(top_srcdir) -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST =  ../libsn/sn-common.h	\
	          ../libsn/sn-internals.h \
	          ../libsn/sn-launchee.h \
			  ../libsn/sn-launcher.h \
	          ../libsn/sn-list.h \
	          ../libsn/sn-monitor.h \
	          ../libsn/sn-util.h \
	          ../libsn/sn-xmessages.h \
			  ../libsn/sn-xutils.h \
			  ../libsn/sn.h \
			  ../st/arg.h \
			  ../st/config.h \
              ../st/st.c \
              ../st/st.h \
			  ../st/win.h \
              ../st/x.c \
			  ../st/x.h \
			  xfedefs.h \
              help.h \
              icons.h \
			  startupnotification.h \
			  xfeutils.h \
			  foxhacks.cpp \
			  moderncontrols.cpp \
			  ArchInputDialog.h \
			  BookmarkDialog.h \
			  BrowseInputDialog.h \
			  ColorSelector.h \
			  ComboBox.h \
			  CommandWindow.h \
			  ConnectDialog.h \
			  DialogBox.h \
			  DirHistBox.h \
			  DirList.h \
			  DirPanel.h \
			  ExecuteBox.h \
              File.h \
			  FileDialog.h \
			  FileDict.h \
			  FileList.h \
			  FilePanel.h \
			  FontDialog.h \
			  HistInputDialog.h \
			  IconList.h \
			  InputDialog.h \
			  KeyBindings.h \
			  KeyBindingsDialog.h \
			  MessageBox.h \
			  OverwriteBox.h \
			  PathLinker.h \
			  Places.h \
			  Properties.h \
			  Preferences.h \
			  RenameDialog.h \
			  SearchPanel.h \
			  SearchWindow.h \
			  StringList.h \
			  TabButtons.h \
			  TextLabel.h \
			  TextWindow.h \
			  WriteWindow.h \
			  XFileExplorer.h \
			  XFileArchive.h \
			  XFileImage.h \
			  XFilePackage.h \
			  XFileWrite.h
