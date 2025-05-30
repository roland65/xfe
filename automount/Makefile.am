AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = xfe-automount

xfe_automount_SOURCES = xfe-automount.cpp

xfe_automount_LDADD = $(UDISKS2_LIBS) $(LIBNOTIFY_LIBS)

xfe_automount_CXXFLAGS = -std=c++17 $(UDISKS2_CFLAGS) $(LIBNOTIFY_CFLAGS)
