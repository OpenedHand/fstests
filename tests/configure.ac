AC_PREREQ(2.53)
AC_INIT([fstests], 0.1, [mallum@o-hand.com])
AC_CONFIG_SRCDIR([test-x.c])

AM_INIT_AUTOMAKE()

AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

AC_ARG_WITH(freetype-config, [  --with-freetype-config=PROG   Use FreeType configuration program PROG], freetype_config=$withval, freetype_config=yes)

if test "$freetype_config" = "yes"; then
        AC_PATH_PROG(ft_config,freetype-config,no)
        if test "$ft_config" = "no"; then
                AC_MSG_ERROR([You must have freetype installed; see http://www.freetype.org/])
        fi
else
        ft_config="$freetype_config"
fi

FREETYPE_CFLAGS="`$ft_config --cflags`"
FREETYPE_LIBS="`$ft_config --libs`"

AC_SUBST(FREETYPE_LIBS)
AC_SUBST(FREETYPE_CFLAGS)

#PKG_CHECK_MODULES(X11, x11, [have_libx11pc="yes"], [have_libx11pc="no"])

# Very lazy check, possibly do old way aswell, but damage will be needed 
# whatever so likely will need autoconfed ( fd.o ) xlibs.
PKG_CHECK_MODULES(XLIBS, x11 xext xft)

AC_SUBST(XLIBS_CFLAGS)
AC_SUBST(XLIBS_LIBS)

PKG_CHECK_MODULES(PANGO, pango pangoxft)

AC_SUBST(PANGO_CFLAGS)
AC_SUBST(PANGO_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-2.0)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)


if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

AC_SUBST(GCC_FLAGS)

# Checks for header files.
AC_OUTPUT([Makefile])
