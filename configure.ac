#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([espeakup], [0.71], [w.d.hubbs@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([espeakup.c])
AC_CONFIG_HEADERS([config.h])

# process command line options
AC_ARG_WITH([alsa],
            [AS_HELP_STRING([--with-alsa],[build with ALSA support])],
            [],
            [with_alsa=no])
AM_CONDITIONAL([ALSA], [test x$with_alsa = xyes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
if test x$with_alsa = xyes; then
AC_SEARCH_LIBS([snd_pcm_open], [asound])
fi
AC_SEARCH_LIBS([espeak_Synth], [espeak])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h unistd.h espeak/speak_lib.h])
if test x$with_alsa = xyes; then
AC_CHECK_HEADERS([alsa/asoundlib.h])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove select strdup strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
