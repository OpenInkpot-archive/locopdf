AC_PREREQ(2.57)
AC_INIT(locopdf, 0.1, quickhand@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_CPLUSPLUS
AC_PROG_CXX
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(EFL,[ecore ecore-file ecore-evas evas edje])
PKG_CHECK_MODULES(EPDF, [epdf])
PKG_CHECK_MODULES(SQLITE, [sqlite3])

AC_OUTPUT(Makefile src/Makefile themes/Makefile)
