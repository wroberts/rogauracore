dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rogauracore], [1.4], [wildwilhelm@gmail.com])
AC_CONFIG_SRCDIR([src/rogauracore.c])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])
WITH_UDEV=yes

# Checks for programs.
AC_PROG_CC

# Set udevdir for udev rules
AC_ARG_WITH([udev],
	[AS_HELP_STRING([--with-udev],
		[Enable udev integration])],
	[WITH_UDEV=$withval])
AC_SUBST(WITH_UDEV)
# set default early
default_udevdir=/lib/udev
if test x"$with_udev" = x || \
   test x"$with_udev" = xyes ; then
	if test x"$PKG_CONFIG" != x; then
		udevdir=$($PKG_CONFIG --variable=udevdir udev)
	fi
	if test x"$udevdir" = x; then
		AC_MSG_NOTICE([Could not detect udev rules directory, using default])
		udevdir=$default_udevdir
	fi
	AC_MSG_RESULT([Using udev rules directory: $udevdir])
else
	udevdir=$default_udevdir
fi
dnl always replace, even if not used
AC_SUBST(udevdir)
udevrulesdir=$udevdir/rules.d
AC_SUBST(udevrulesdir)
dnl checks for programs
AC_PATH_PROG(UDEVADM, udevadm, [false], [/sbin$PATH_SEPARATOR$PATH])
AC_PATH_PROG(UDEVINFO, udevinfo, [false], [/sbin$PATH_SEPARATOR$PATH])
if test $UDEVADM = false && test $UDEVINFO = false; then
   if test "$WITH_UDEV" = "yes"; then
     AC_MSG_WARN([udev support enabled, but neither udevadm nor udevinfo found on this system.])
   fi
fi
dnl Checks for system services
UDEV_RULE_SUFFIX=""

# Checks for libraries.
AC_SEARCH_LIBS([libusb_init], [usb-1.0], [], [AC_MSG_ERROR([libusb library not found])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libusb-1.0/libusb.h])
AC_CHECK_HEADERS([errno.h stdint.h stdlib.h string.h strings.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strtol])

AM_CONDITIONAL(INSTALL_UDEV_RULES, test x$with_udevdir != xno)
AC_SUBST(UDEV_RULE_SUFFIX)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo \
        udev dir:                 ${with_udevdir}
