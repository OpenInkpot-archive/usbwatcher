AC_PREREQ(2.57)
AC_INIT(usbwatcher, 0.3, lunohod@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(EFL, [ecore ecore-con ecore-evas evas edje])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])
PKG_CHECK_MODULES(EOI, [libeoi])

AC_OUTPUT([Makefile src/Makefile scripts/Makefile themes/Makefile po/Makefile data/Makefile])
