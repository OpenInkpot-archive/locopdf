SUBDIRS = src themes data po help

desktopdir = $(datarootdir)/applications
desktop_DATA = po/locopdf.desktop

install-exec-hook:
	mkdir -p ${DESTDIR}/usr/share/help/locopdf
	oi-help-compiler -u $(srcdir)/help ${DESTDIR}/usr/share/help/locopdf
