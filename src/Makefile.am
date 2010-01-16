bin_PROGRAMS = locopdf
locopdf_SOURCES = locopdf.c choices.c entry.c dialogs.c help.c database.c info_export.c epdf_plugin.c edjvu_plugin.c

AM_CFLAGS = -DTHEME_DIR='"$(pkgdatadir)/themes/"'

#AM_CFLAGS += -DNO_XCB

INCLUDES= @EFL_CFLAGS@ @EPDF_CFLAGS@ @EDJVU_CFLAGS@ @SQLITE_CFLAGS@ @LIBKEYS_CFLAGS@ @LIBEOI_CFLAGS@ @LIBCHOICEBOX_CFLAGS@
locopdf_LDADD = @EFL_LIBS@ @EPDF_LIBS@ @EDJVU_LIBS@ @SQLITE_LIBS@ @LIBKEYS_LIBS@ @LIBEOI_LIBS@ @LIBCHOICEBOX_LIBS@

locopdf_LDFLAGS = -Wl,--as-needed
