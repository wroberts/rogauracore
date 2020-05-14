dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rogauracore], [1.3], [wildwilhelm@gmail.com])
AC_CONFIG_SRCDIR([src/rogauracore.c])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([libusb_init], [usb-1.0])

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
