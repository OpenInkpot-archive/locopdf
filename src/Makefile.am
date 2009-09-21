bin_PROGRAMS = locopdf
locopdf_SOURCES = locopdf.c keyhandler.c choicebox.c entrybox.c dialogs.c database.c

INCLUDES= @EFL_CFLAGS@ @EPDF_CFLAGS@ @SQLITE_CFLAGS@ @LIBKEYS_CFLAGS@
locopdf_LDADD = @EFL_LIBS@ @EPDF_LIBS@ @SQLITE_LIBS@ @LIBKEYS_LIBS@
locopdf_LDFLAGS = -Wl,--as-needed
