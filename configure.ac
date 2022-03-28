# Autoconf requirements
# Force autoconf to be at least this version number:
AC_PREREQ([2.69])
# Initialize, but use more options.  Note parameters brackets and whitespace.
AC_INIT([maxmindtest], [1.2], [w7og@yahoo.com])
# Safety check - list a source file that wouldn't be in other directories:
AC_CONFIG_SRCDIR([maxmindtest.c])

# Put autotools auxiliary files in a subdir, so they don't clutter top dir:
AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_TARGET
# see GNU autoconf archive = perdefined macros
AX_GENERATE_CHANGELOG

# Enable "automake" to simplify creating makefiles:
AM_INIT_AUTOMAKE([1.11 subdir-objects -Wall -Werror])

# Look for a C compiler:
AC_PROG_CC([gcc cl cc])
AC_PROG_CXX
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_MKDIR_P
AM_PROG_CC_C_O
AC_C_BIGENDIAN

passwin_version_1=`echo $PACKAGE_VERSION | cut -d . -f 1`
passwin_version_2=`echo $PACKAGE_VERSION | cut -d . -f 2`
passwin_version_3=`echo $PACKAGE_VERSION | cut -d . -f 3`
passwin_version=$passwin_version_1.$passwin_version_2.$passwin_version_3
AC_DEFINE_UNQUOTED(PASSWIN_VERSION,  "$passwin_version",  [ PASSWIN version string.  ])
AC_DEFINE_UNQUOTED(PASSWIN_VERSION_1, $passwin_version_1, [ PASSWIN version level 1. ])
AC_DEFINE_UNQUOTED(PASSWIN_VERSION_2, $passwin_version_2, [ PASSWIN version level 2. ])
AC_DEFINE_UNQUOTED(PASSWIN_VERSION_3, $passwin_version_3, [ PASSWIN version level 3. ])
AC_SUBST(PASSWIN_VERSION, $passwin_version)
# Perhaps check for: (more) programs, libraries, header files, types,
# structures, compiler characteristics, library functions, system services.

# Checks for libraries.
AC_CHECK_LIB([csv], [csv_init])
AC_CHECK_LIB([maxminddb], [MMDB_open])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h limits.h malloc.h stddef.h stdint.h time.h ctype.h stdlib.h stdio.h intrin.h string.h sys/ioctl.h sys/time.h unistd.h termios.h getopt.h gmp.h sys/select.h sys/socket.h arpa/inet.h netinet/in.h libssh2.h curl/curl.h csv.h maxminddb.h])
# Checks for typedefs, structures, macros, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for compiler characteristics.
AC_C_CONST
AC_C_INLINE

AC_DEFINE(_GNU_SOURCE, 1, [ Define to enable GNU extensions in glibc ])
AC_HEADER_STDC

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_ALLOCA

# Options
AC_ARG_ENABLE(debug,
  AC_HELP_STRING(
    [--enable-debug],
    [enable development-time debugging @<:@default=disabled@:>@]),
  [ passwin_debug="$enableval" ],
  [ passwin_debug="no" ])
if test "$passwin_debug" = "yes"; then
  AM_CONDITIONAL(DEBUG, true)
  AC_DEFINE(DEBUG, 1,
    [ Define if you want passwin to print debug messages to stdout. ])
  CFLAGS="$CFLAGS -g"
else
  AM_CONDITIONAL(DEBUG, false)
  AC_DEFINE(NDEBUG, 1, [ Define if you want to disable debug assertions. ])
  CFLAGS="-O2"
  CXXFLAGS="-O2"
fi


AM_PROG_AR
LT_INIT([disable-shared])
AC_SUBST([LIBTOOL_DEPS])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memchr memset strchr strerror strtol memcpy])

# Output files
# Put configuration results here, so we can easily #include them:
AC_CONFIG_HEADERS([config.h])
# other files to creat
AC_CONFIG_FILES([
Makefile
m4/Makefile
build-aux/Makefile
])

MY_CPPFLAGS="-I$TUTLIBLOC -I$TRELIBLOC -I$RESLIBLOC"
AC_SUBST(MY_CPPFLAGS)

# Do final output.
AC_OUTPUT


# Print configuration summary

cat <<EOF


Configuration summary
=====================

passwin is now configured as follows:

* Compilation environment

  CC       = $CC
  CFLAGS   = $CFLAGS
  CPP      = $CPP
  CPPFLAGS = $CPPFLAGS
  LD       = $LD
  LDFLAGS  = $LDFLAGS
  LIBS     = $LIBS

* maxmintest options
  Development-time debugging:		$passwin_debug


EOF

