dnl Copyright (c) 2003, 2004 Christian Persch
dnl
dnl This program is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by the
dnl Free Software Foundation; either version 2 of the License, or (at your
dnl option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
dnl
dnl $Id$

AC_INIT([Epiphany Extensions], [1.4.1], [http://bugzilla.gnome.org/enter_bug.cgi?product=epiphany-extensions], [epiphany-extensions])
GNOME_COMMON_INIT

EPIPHANY_EXTENSIONS_MAJOR=1.4
AC_SUBST(EPIPHANY_EXTENSIONS_MAJOR)

AC_PREREQ(2.57)

AC_REVISION($Revision$)

AC_CONFIG_SRCDIR(extensions)
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([dist-bzip2 check-news])

AM_MAINTAINER_MODE

AC_PROG_INTLTOOL([0.29])

AM_DISABLE_STATIC
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)

AM_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

AC_SUBST(AM_CXXFLAGS)

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS(error)
GNOME_CXX_WARNINGS

dnl ********************************
dnl Internationalisation
dnl ********************************

ALL_LINGUAS="bg ca cs da de el en_CA en_GB es fr hr hu it ja ko lt nl no pa pl pt pt_BR ru sk sq sr sr@Latn sv uk wa zh_CN"

GETTEXT_PACKAGE=epiphany-extensions-1.4
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT

dnl ********************************
dnl pkg config checks
dnl ********************************

EPIPHANY_REQUIRED=1.4.1
LIBXML_REQUIRED=2.6.0
LIBGLIB_REQUIRED=2.4.0
LIBGTK_REQUIRED=2.4.0

AC_SUBST(EPIPHANY_REQUIRED)
AC_SUBST(LIBXML_REQUIRED)
AC_SUBST(LIBGLIB_REQUIRED)
AC_SUBST(LIBGTK_REQUIRED)

dnl fixme -- combine the dependencies of only the enabled plugins
PKG_CHECK_MODULES(EPIPHANY_DEPENDENCY, \
		  libxml-2.0 >= $LIBXML_REQUIRED \
		  libglade-2.0 \
		  glib-2.0 >= $LIBGLIB_REQUIRED \
		  gmodule-2.0 \
		  gtk+-2.0 >= $LIBGTK_REQUIRED \
		  epiphany-1.4 >= $EPIPHANY_REQUIRED)
AC_SUBST(EPIPHANY_DEPENDENCY_CFLAGS)
AC_SUBST(EPIPHANY_DEPENDENCY_LIBS)

dnl *******************************
dnl GConf related settings
dnl *******************************

dnl currently no plugin has any gconf settings

dnl Specify the gconf configuration source, 
dnl default to xml::$(sysconfdir)/gconf/gconf.xml.defaults

dnl AC_PATH_PROG(GCONFTOOL, gconftool-2, no)

dnl if test x"$GCONFTOOL" = xno; then
dnl  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
dnl fi

dnl AM_GCONF_SOURCE_2

dnl *******************************
dnl Check for OpenSP
dnl *******************************
AC_ARG_ENABLE([opensp],
	      [ --enable-opensp=[yes,no]    Attempt to use OpenSP for SGML validation [default=yes]],
	      [enable_opensp="$enableval"],
	      [enable_opensp="yes"])

if test "x$enable_opensp" != "xno"; then
	AC_LANG_PUSH([C++])

	AC_CHECK_HEADERS([OpenSP/config.h OpenSP/ParserEventGeneratorKit.h],
			 [ have_opensp="yes" ],
			 [ have_opensp="no" ])

	AM_CONDITIONAL([HAVE_OPENSP], test "x$have_opensp" = "xyes")

	AC_LANG_POP([C++])
fi

dnl *******************************
dnl Extensions
dnl *******************************

AC_MSG_CHECKING([extensions to build])

all_extensions="certificates,dashboard,error-viewer,gestures,page-info,sample,sample-mozilla,select-stylesheet,sidebar,smart-bookmarks,tabsmenu,tab-groups"
all_useful_extensions="certificates,dashboard,error-viewer,gestures,page-info,select-stylesheet,sidebar,smart-bookmarks,tabsmenu,tab-groups"
default_extensions="certificates,error-viewer,gestures,page-info,select-stylesheet,smart-bookmarks,tabsmenu,tab-groups"
mozilla_dependent_extensions="certificates,error-viewer,page-info,sample-mozilla,select-stylesheet,sidebar,smart-bookmarks"

dnl due to an autoconf bug, commas in the first arg to
dnl AC_HELP_STRING cause problems.
dnl
dnl AC_HELP_STRING([--with-extensions=ext1,ext2,...],[build the specified extensions])
AC_ARG_WITH([extensions],
[  --with-extensions=extension1,extension2,...
			  build the specified extensions. Available:
			  certificates, dashboard, error-viewer, gestures,
			  page-info, select-stylesheet, smart-bookmarks,
			  tabs-groups, tabsmenu, all, really-all])

dnl If no extensions specified, build the default set
if test "x$with_extensions" = xyes; then
  extensions="$default_extensions"
else
  extensions="$with_extensions"
fi

if test "x$extensions" = x; then
  extensions="$default_extensions"
fi

if test "x$extensions" = xall; then
  extensions="$all_useful_extensions"
fi

if test "x$extensions" = "xreally-all"; then
  extensions="$all_extensions"
fi

AC_MSG_RESULT($extensions)

IFS="${IFS=	}"; ephyext_save_ifs="$IFS"; IFS=","
for ext in $extensions; do
 if echo "$all_extensions" | egrep "(^|,)$ext(\$|,)" > /dev/null; then
   :
 else
   AC_MSG_ERROR([the specified extension $ext does not exist])
 fi
done
IFS="$ephyext_save_ifs"

AM_CONDITIONAL(ENABLE_CERTIFICATES_EXTENSION, echo "$extensions" | egrep '(^|,)certificates($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_DASHBOARD_EXTENSION, echo "$extensions" | egrep '(^|,)dashboard($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_ERROR_VIEWER_EXTENSION, echo "$extensions" | egrep '(^|,)error-viewer($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_GESTURES_EXTENSION, echo "$extensions" | egrep '(^|,)gestures($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_PAGE_INFO_EXTENSION, echo "$extensions" | egrep '(^|,)page-info($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SAMPLE_EXTENSION, echo "$extensions" | egrep '(^|,)sample($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SAMPLE_MOZILLA_EXTENSION, echo "$extensions" | egrep '(^|,)sample-mozilla($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SELECT_STYLESHEET_EXTENSION, echo "$extensions" | egrep '(^|,)select-stylesheet($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SIDEBAR_EXTENSION, echo "$extensions" | egrep '(^|,)sidebar($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SMART_BOOKMARKS_EXTENSION, echo "$extensions" | egrep '(^|,)smart-bookmarks($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_TABSMENU_EXTENSION, echo "$extensions" | egrep '(^|,)tabsmenu($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_TAB_GROUPS_EXTENSION, echo "$extensions" | egrep '(^|,)tab-groups($|,)' > /dev/null)
dnl AM_CONDITIONAL(ENABLE_TOOLBAR_EXTRAS_EXTENSION, echo "$extensions" | egrep '(^|,)toolbar-extras($|,)' > /dev/null)

dnl ******************
dnl Mozilla name check
dnl ******************

AC_MSG_CHECKING([which mozilla to use])

MOZILLA=
AC_ARG_WITH([mozilla],
	AC_HELP_STRING([--with-mozilla@<:@=mozilla|firefox|thunderbird@:>@],
		       [Whether to use mozilla, firefox or thunderbird gtkmozembed (default: mozilla)]),
	[MOZILLA="$withval"])

dnl try to autodetect: if mozilla exists, use it, else try firefox, then thunderbird
if test "x$MOZILLA" = "x"; then
  if pkg-config --exists mozilla-gtkmozembed; then
    MOZILLA=mozilla
  elif pkg-config --exists firefox-gtkmozembed; then
    MOZILLA=firefox
  elif pkg-config --exists thunderbird-gtkmozembed; then
    MOZILLA=thunderbird
  else
    AC_MSG_ERROR([no mozilla installation found])
  fi
elif test "x$with_mozilla" != "xmozilla" -a "x$with_mozilla" != "xfirefox" -a "x$with_mozilla" != "xthunderbird"; then
  AC_MSG_ERROR([unknown mozilla name ($MOZILLA)])
fi

AC_SUBST(MOZILLA)

AC_MSG_RESULT([$MOZILLA])

case "$MOZILLA" in
mozilla) min_version=1.7 ;;
firefox) min_version=0.10 ;;
thunderbird) min_version=0.8 ;;
esac

PKG_CHECK_MODULES(MOZILLA_COMPONENT, $MOZILLA-gtkmozembed >= $min_version)
AC_SUBST(MOZILLA_COMPONENT_CFLAGS)
AC_SUBST(MOZILLA_COMPONENT_LIBS)

MOZILLA_INCLUDE_ROOT="`$PKG_CONFIG --variable=includedir $MOZILLA-gtkmozembed`"
AC_SUBST(MOZILLA_INCLUDE_ROOT)

MOZILLA_HOME="`$PKG_CONFIG --variable=libdir $MOZILLA-gtkmozembed`"
AC_SUBST(MOZILLA_HOME)

dnl whether to build with DEBUG defined
AC_ARG_WITH([mozilla-debug],
        [  --with-mozilla-debug        Use a debug mozilla build],
        AM_CXXFLAGS="-DDEBUG $AM_CXXFLAGS",)

AC_ARG_ENABLE([cpp-rtti],
        [  --enable-cpp-rtti       Enable C++ RTTI (for cvs gcc)],,
        enable_cpp_rtti=no)

if test "x$enable_cpp_rtti" = "xno"; then
  AM_CXXFLAGS="-fno-rtti $AM_CXXFLAGS"
fi

dnl This is from Mozilla's configure.in. They set almost all the config stuff
dnl they need in mozilla-config.h Except for this compiler flag, which can't
dnl go in mozilla-config.h So we check the flag too and now we can include
dnl mozilla-config.h without breaking epiphany.
dnl This is really gcc-only
dnl Do this test using CXX only since some versions of gcc
dnl 2.95-2.97 have a signed wchar_t in c++ only and some versions
dnl only have short-wchar support for c++.
AC_LANG_PUSH(C++)
_SAVE_CXXFLAGS=$CXXFLAGS
_SAVE_AM_CXXFLAGS=$AM_CXXFLAGS
AM_CXXFLAGS="$AM_CXXFLAGS -fshort-wchar"
CXXFLAGS="$CXXFLAGS $AM_CXXFLAGS"

AC_CACHE_CHECK([for compiler -fshort-wchar option],
    ac_cv_have_usable_wchar_option,
    [AC_TRY_RUN([#include <stddef.h>
                 int main () {
                   return (sizeof(wchar_t) != 2) ||
                          (wchar_t)-1 < (wchar_t) 0 ; } ],
                ac_cv_have_usable_wchar_option="yes",
                ac_cv_have_usable_wchar_option="no",
                ac_cv_have_usable_wchar_option="maybe")])

if test "$ac_cv_have_usable_wchar_option" != "yes"; then
    AM_CXXFLAGS=$_SAVE_AM_CXXFLAGS
fi

CXXFLAGS="$_SAVE_CXXFLAGS $AM_CXXFLAGS -I$MOZILLA_INCLUDE_ROOT `pkg-config --cflags $MOZILLA-xpcom`"

dnl insert mozilla API checks here

CXXFLAGS=$_SAVE_CXXFLAGS
AC_LANG_POP(C++)

dnl set warning flags
CFLAGS="$CFLAGS $WARN_CFLAGS"
CXXFLAGS="$CXXFLAGS $WARN_CXXFLAGS -Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

dnl *******************************
dnl Misc things
dnl *******************************

dnl uninstalled share dir to search data
dnl AC_DEFINE_UNQUOTED(SHARE_UNINSTALLED_DIR,"`pwd`/data",[path to source data dir])

dnl what about those? are they needed -- from gedit-plugins
dnl EXTENSION_LIBTOOL_FLAGS="-module -avoid-version"
dnl AC_SUBST(EXTENSION_LIBTOOL_FLAGS)

dnl *******************************
dnl share/data dir
dnl *******************************

if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}", [Directory of data files])
  else
    AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}", [Directory of data files])
  fi
else
  AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${datadir}/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}", [Directory of data files])
fi

EXTENSIONS_DIR="`$PKG_CONFIG --variable=extensionsdir epiphany-1.4`"
AC_SUBST(EXTENSIONS_DIR)

dnl *******************************
dnl *******************************

AC_CONFIG_FILES([
Makefile
include/Makefile
extensions/Makefile
extensions/certificates/Makefile
extensions/certificates/mozilla/Makefile
extensions/dashboard/Makefile
extensions/gestures/Makefile
extensions/error-viewer/Makefile
extensions/error-viewer/mozilla/Makefile
extensions/error-viewer/opensp/Makefile
extensions/page-info/Makefile
extensions/page-info/mozilla/Makefile
extensions/sample/Makefile
extensions/sample-mozilla/Makefile
extensions/sample-mozilla/mozilla/Makefile
extensions/select-stylesheet/Makefile
extensions/select-stylesheet/mozilla/Makefile
extensions/sidebar/Makefile
extensions/sidebar/mozilla/Makefile
extensions/smart-bookmarks/Makefile
extensions/smart-bookmarks/mozilla/Makefile
extensions/tabsmenu/Makefile
extensions/tab-groups/Makefile
po/Makefile.in
tools/Makefile
])

AC_OUTPUT
