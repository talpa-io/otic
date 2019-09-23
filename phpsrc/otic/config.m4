dnl config.m4 for extension otic

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 5; then
    AC_MSG_ERROR([need at least PHP 5 or newer])
fi

PHP_ARG_ENABLE(otic, whether to enable otic support,
[  --enable-otic           Enable otic support])

if test "$PHP_OTIC" != "no"; then

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

  AC_MSG_CHECKING(for libzstd)
  if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists libzstd; then
    if $PKG_CONFIG libzstd --atleast-version 1; then
      LIBZSTD_CFLAGS=`$PKG_CONFIG libzstd --cflags`
      LIBZSTD_LIBDIR=`$PKG_CONFIG libzstd --libs`
      LIBZSTD_VERSON=`$PKG_CONFIG libzstd --modversion`
      AC_MSG_RESULT(from pkgconfig: version $LIBZSTD_VERSON)
    else
      AC_MSG_ERROR(system libzstd is too old)
    fi
  else
    AC_MSG_ERROR(pkg-config not found)
  fi
  PHP_EVAL_LIBLINE($LIBZSTD_LIBDIR, OTIC_SHARED_LIBADD)
  PHP_EVAL_INCLINE($LIBZSTD_CFLAGS)
  PHP_ADD_INCLUDE(../../src)
  PHP_NEW_EXTENSION(otic, otic.c, $ext_shared)
  PHP_SUBST(OTIC_SHARED_LIBADD)

  PHP_ADD_MAKEFILE_FRAGMENT
fi
