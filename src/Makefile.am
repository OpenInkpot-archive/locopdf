bin_PROGRAMS = locopdf
locopdf_SOURCES = locopdf.c keyhandler.c choicebox.c entrybox.c dialogs.c database.c info_export.c epdf_plugin.c edjvu_plugin.c

#AM_CFLAGS = -DNO_XCB

INCLUDES= @EFL_CFLAGS@ @EPDF_CFLAGS@ @EDJVU_CFLAGS@ @SQLITE_CFLAGS@ @LIBKEYS_CFLAGS@
locopdf_LDADD = @EFL_LIBS@ @EPDF_LIBS@ @EDJVU_LIBS@ @SQLITE_LIBS@ @LIBKEYS_LIBS@
locopdf_LDFLAGS = -Wl,--as-needed
