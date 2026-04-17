dnl $Header: /home/cvsuser/VERITAS/software/common/VBF/configure.ac,v 1.11 2015/03/23 15:41:41 sembrosk Exp $
dnl configure.ac for the VBF distribution
dnl by Filip Pizlo, 2004, 2005
dnl updated Juniper Foote, 2022-2024

AC_INIT(VBF, 0.3.4)
AC_CONFIG_SRCDIR([VBF/Adler32.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_PREREQ([2.71])

AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE

summary_GZIP=yes
summary_BZIP2=yes

AC_PROG_CC
AC_PROG_CXX
LT_INIT

AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])

XTRA_LIBS=
XTRA_LDFLAGS=
XTRA_CPPFLAGS=

AC_CHECK_HEADERS([stdio.h stdlib.h string.h inttypes.h unistd.h],
                 [],
                 [echo "Cannot find required headers.  Sorry."; exit 1])

AC_CHECK_HEADERS([strings.h sys/uio.h arpa/inet.h])

case $GCC in
    yes)
        case $CFLAGS in
            *-O2*)
                CFLAGS="$CFLAGS -fno-strict-aliasing"
                ;;
            *)
                ;;
        esac
        AM_CFLAGS="-Wall $AM_CFLAGS"
        case $CXXFLAGS in
            *-O2*)
                CXXFLAGS="$CXXFLAGS -fno-strict-aliasing"
                ;;
            *)
                ;;
        esac
        AM_CXXFLAGS="-Wall $AXXM_CFLAGS"
        ;;
    *)
        ;;
esac

AC_SYS_LARGEFILE

AC_MSG_CHECKING(for zlib);
my_save_LIBS="$LIBS"
LIBS="-lz $LIBS"
AC_LINK_IFELSE([
  AC_LANG_SOURCE([[
    #include <zlib.h>
    int main() {
        deflate(0, 0);
        return 0;
    }
  ]])
  ], [
    AC_MSG_RESULT([yes])
    XTRA_LIBS="-lz $XTRA_LIBS"
    AC_DEFINE(HAVE_GZIP,1, [Defined if we have zlib])
  ], [
    AC_MSG_RESULT([no])
    summary_GZIP="no (zlib not found)"
  ])

LIBS="$my_save_LIBS"

AC_MSG_CHECKING(for libbzip2);
my_save_LIBS="$LIBS"
LIBS="-lbz2 $LIBS"
AC_LINK_IFELSE([
  AC_LANG_SOURCE([[
    #include <bzlib.h>
    int main() {
        BZ2_bzCompress(NULL, 0);
        return 0;
    }
  ]])
  ], [
    AC_MSG_RESULT([yes])
    XTRA_LIBS="-lbz2 $XTRA_LIBS"
    AC_DEFINE(HAVE_BZIP2,1,
              [Defined if we have libbzip2])
  ], [
    AC_MSG_RESULT([no])
    summary_BZIP2="no (libbzip2 not found)"
])

LIBS="$my_save_LIBS"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(XTRA_LIBS)
AC_SUBST(XTRA_LDFLAGS)
AC_SUBST(XTRA_CPPFLAGS)

AM_SANITY_CHECK

AC_CONFIG_FILES([Makefile \
          VBF.pc \
          VBF/Makefile \
          vbfTools/Makefile \
          scripts/Makefile])
AC_OUTPUT

echo
echo "VBF Configuration Summary:"
echo
echo "                Binary directory: `sh -c \"echo \`sh -c \\\"echo ${prefix}${bindir}\\\"\`\"`"
echo "                Library directory: `sh -c \"echo \`sh -c \\\"echo ${prefix}${libdir}\\\"\`\"`"
echo "                Include directory: `sh -c \"echo \`sh -c \\\"echo ${prefix}${includedir}\\\"\`\"`"
echo "                zlib support: $summary_GZIP"
echo "                libbzip2 support: $summary_BZIP2"
echo
echo "Now you can do 'make' followed by 'make install'!"
echo

