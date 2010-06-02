#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([espeakup], [0.71], [w.d.hubbs@gmail.com])
AM_INIT_AUTOMAKE([foreign])
LT_INIT()
AC_CONFIG_SRCDIR([espeakup.c])
AC_CONFIG_HEADERS([config.h])

# process command line options
AC_ARG_ENABLE([standalone],
	[AS_HELP_STRING([--enable-standalone],[build a standalone executable])],
	[],
	[enable_standalone=no])
AM_CONDITIONAL([STANDALONE], [test x$enable_standalone = xyes])

AC_ARG_WITH([alsa],
	[AS_HELP_STRING([--with-alsa],[build with ALSA support])],
	[],
	[with_alsa=no])
AM_CONDITIONAL([ALSA], [test x$with_alsa = xyes])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_SEARCH_LIBS([pow], [m], [],
	[AC_MSG_FAILURE([math library missing -- unable to continue.])])
AC_SEARCH_LIBS([pthread_create], [pthread], [],
	[AC_MSG_FAILURE([threads library missing -- unable to continue.])])

AS_IF([test x$with_alsa = xyes],
	[AC_SEARCH_LIBS([snd_pcm_open], [asound], [], 
		[AC_MSG_FAILURE([ALSA library missing -- unable to continue.])])])

AC_SEARCH_LIBS([Pa_Initialize], [portaudio])
AC_SEARCH_LIBS([espeak_Synth], [espeak], [],
	[AC_MSG_FAILURE([espeak library missing -- unable to continue.])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h unistd.h])
AS_IF([test x$with_alsa = xyes],
	[AC_CHECK_HEADERS([alsa/asoundlib.h])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove select strdup strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
