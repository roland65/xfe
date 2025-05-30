man_MANS=xfe.1 xfa.1 xfi.1 xfp.1 xfw.1

if XFEAUTOMOUNTER
SUBDIRS = po m4 src automount icons
else
SUBDIRS = po m4 src icons
endif

DIST_SUBDIRS = po m4 src automount icons

rcdir = $(prefix)/share/xfe
rc_DATA = xferc
pkdir = $(prefix)/share/polkit-1/actions

EXTRA_DIST = autogen.sh buildrun installrun uninstall-xfe iconlinks.sh config.h i18n.h org.xfe.root.policy.in.in xfe.1 xfa.1 xfi.1 xfp.1 xfw.1 \
             ABOUT-NLS TODO BUGS xferc.in *.desktop.in.in \
	         $(top_srcdir)/icons/* $(top_srcdir)/debian

desktopdir = $(prefix)/share/applications
desktop_in_files = xfe.desktop.in xfa.desktop.in xfi.desktop.in xfp.desktop.in xfw.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

polkit_action_in_in_FILES = org.xfe.root.policy.in.in
polkit_action_in_FILES = org.xfe.root.policy.in
polkit_action_FILES = $(polkit_action_in_FILES:.policy.in=.policy)
polkit_actiondir = $(datadir)/polkit-1/actions
if INSTALL_POLKIT_ACTIONS
  polkit_action_DATA = $(polkit_action_FILES)
endif

do_subst = sed -e 's,[@]bindir[@],$(bindir),g'

xfe.desktop: xfe.desktop.in Makefile
		$(AM_V_GEN)$(MSGFMT) --desktop -d $(top_srcdir)/po --template $< -o $@
xfa.desktop: xfa.desktop.in Makefile
		$(AM_V_GEN)$(MSGFMT) --desktop -d $(top_srcdir)/po --template $< -o $@
xfi.desktop: xfi.desktop.in Makefile
		$(AM_V_GEN)$(MSGFMT) --desktop -d $(top_srcdir)/po --template $< -o $@
xfp.desktop: xfp.desktop.in Makefile
		$(AM_V_GEN)$(MSGFMT) --desktop -d $(top_srcdir)/po --template $< -o $@
xfw.desktop: xfw.desktop.in Makefile
		$(AM_V_GEN)$(MSGFMT) --desktop -d $(top_srcdir)/po --template $< -o $@

org.xfe.root.policy: org.xfe.root.policy.in Makefile
		$(AM_V_GEN)$(MSGFMT) --xml -d $(top_srcdir)/po --template $< -o $@

org.xfe.root.policy.in: org.xfe.root.policy.in.in Makefile
	$(do_subst) < org.xfe.root.policy.in.in > org.xfe.root.policy.in

dist-hook:
	cd po && $(MAKE) update-po
	cp po/*.po* $(distdir)/po
	rm -rf $(distdir)/po/*~
	rm -rf $(distdir)/debian/xfe

install-data-hook:
	sh iconlinks.sh $(top_srcdir) $(DESTDIR)$(rcdir)
if INSTALL_POLKIT_ACTIONS
	$(MKDIR_P) $(DESTDIR)$(pkdir)
	cp org.xfe.root.policy $(DESTDIR)$(pkdir)
endif
	
uninstall-hook:
	rm -rf $(DESTDIR)$(rcdir)
if INSTALL_POLKIT_ACTIONS
	rm -f $(DESTDIR)$(pkdir)/org.xfe.root.policy
endif

ACLOCAL_AMFLAGS = -I m4
