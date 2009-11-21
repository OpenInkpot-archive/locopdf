AC_PREREQ(2.57)
AC_INIT(locopdf, 0.1, quickhand@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC_C99
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(EFL,[ecore ecore-file ecore-evas evas edje efreet efreet-mime])
PKG_CHECK_MODULES(EPDF, [epdf])
PKG_CHECK_MODULES(EDJVU, [edjvu])
PKG_CHECK_MODULES(SQLITE, [sqlite3])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])

AC_OUTPUT(Makefile src/Makefile themes/Makefile data/Makefile)
