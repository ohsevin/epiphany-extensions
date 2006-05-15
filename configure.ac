# Copyright (c) 2003, 2004, 2005 Christian Persch
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#
# $Id$

AC_INIT([GNOME Web Browser Extensions],[2.14.1.1],[http://bugzilla.gnome.org/enter_bug.cgi?product=epiphany-extensions],[epiphany-extensions])

GNOME_COMMON_INIT

EPIPHANY_API_VERSION=2.14

EPIPHANY_EXTENSIONS_MAJOR=2.14
AC_SUBST(EPIPHANY_EXTENSIONS_MAJOR)

AC_PREREQ([2.59])

AC_REVISION([$Revision$])

AC_CONFIG_SRCDIR([extensions])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip tar-ustar])

AM_MAINTAINER_MODE

AC_PROG_INTLTOOL([0.29])

AM_DISABLE_STATIC
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)

AM_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC

AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

AC_SUBST([AM_CXXFLAGS])

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS

MORE_WARN_FLAGS=
DEPRECATION_FLAGS=
MOZILLA_WARN_CXXFLAGS="-Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

if test "x$enable_maintainer_mode" = "xyes"; then
	AC_DEFINE([MAINTAINER_MODE],[1],[Define to enable 'maintainer-only' behaviour])
	enable_debug=yes
        DEPRECATION_FLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGCONF_DISABLE_DEPRECATED -DGNOME_VFS_DISABLE_DEPRECATED -DLIBGLADE_DISABLE_DEPRECATED -DPANGO_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED"
	MOZILLA_WARN_CXXFLAGS="-Wall -Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth $MOZILLA_WARN_CXXFLAGS"
fi

# ********************************
# Internationalisation
# ********************************

# Translators: new languages must be added to the po/LINGUAS file
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"

AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])

GETTEXT_PACKAGE=epiphany-extensions-$EPIPHANY_API_VERSION
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"], [Gettext package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([LOCALE_DIR])

# ********************************
# Documentation
# ********************************

GNOME_DOC_INIT

# ********************************
# pkg config checks
# ********************************

EPIPHANY_REQUIRED=2.14.1
LIBXML_REQUIRED=2.6.0
LIBGLIB_REQUIRED=2.8.0
LIBGTK_REQUIRED=2.8.0

AC_SUBST([EPIPHANY_REQUIRED])
AC_SUBST([LIBXML_REQUIRED])
AC_SUBST([LIBGLIB_REQUIRED])
AC_SUBST([LIBGTK_REQUIRED])


EPIPHANY_EXTENSION_INIT([$EPIPHANY_API_VERSION], [$EPIPHANY_REQUIRED])

PKG_CHECK_MODULES([EPIPHANY_DEPENDENCY], [\
		  libxml-2.0 >= $LIBXML_REQUIRED \
		  libglade-2.0 \
		  glib-2.0 >= $LIBGLIB_REQUIRED \
		  gmodule-2.0 \
		  gtk+-2.0 >= $LIBGTK_REQUIRED \
		  epiphany-$EPIPHANY_API_VERSION >= $EPIPHANY_REQUIRED])
AC_SUBST([EPIPHANY_DEPENDENCY_CFLAGS])
AC_SUBST([EPIPHANY_DEPENDENCY_LIBS])

# TODO remove the EXTENSIONS_DIR var
EXTENSIONS_DIR=$EPIPHANY_EXTENSIONS_DIR
AC_SUBST([EXTENSIONS_DIR])

# *******************************
# Check for OpenSP
# *******************************

AC_MSG_CHECKING([whether OpenSP support is requested])
AC_ARG_ENABLE([opensp],
	      [ --enable-opensp=[yes,no]    Attempt to use OpenSP for SGML validation [default=yes]],
	      [enable_opensp="$enableval"],
	      [enable_opensp="yes"])
AC_MSG_RESULT([$enable_opensp])

if test "x$enable_opensp" != "xno"; then
	AC_LANG_PUSH([C++])

	AC_CHECK_HEADERS([OpenSP/config.h OpenSP/ParserEventGeneratorKit.h],
			 [have_opensp=yes],
			 [enable_opensp=no have_opensp=no])

	AC_LANG_POP([C++])

	if test "x$have_opensp" = "xno"; then
		AC_MSG_WARN([OpenSP is not installed; OpenSP support disabled])
	fi
fi

AM_CONDITIONAL([HAVE_OPENSP],[test "x$enable_opensp" = "xyes"])

# *******************************
# Extensions
# *******************************

AC_MSG_CHECKING([which extensions to build])

ALL_EXTENSIONS="actions adblock auto-reload auto-scroller certificates dashboard error-viewer extensions-manager-ui gestures greasemonkey imagebar java-console page-info permissions push-scroller rss sample sample-mozilla select-stylesheet sidebar smart-bookmarks tab-groups tab-states"
USEFUL_EXTENSIONS="actions auto-reload auto-scroller certificates error-viewer extensions-manager-ui gestures java-console page-info push-scroller select-stylesheet sidebar smart-bookmarks tab-groups tab-states"
DEFAULT_EXTENSIONS="actions auto-scroller certificates error-viewer extensions-manager-ui gestures java-console page-info push-scroller select-stylesheet sidebar smart-bookmarks tab-groups tab-states"

PYTHON_ALL_EXTENSIONS="python-console sample-python favicon"
PYTHON_USEFUL_EXTENSIONS="python-console favicon"
PYTHON_DEFAULT_EXTENSIONS="python-console favicon"

DIST_EXTENSIONS="$ALL_EXTENSIONS $PYTHON_ALL_EXTENSIONS"

if test $EPIPHANY_HAS_PYTHON; then
	ALL_EXTENSIONS="$ALL_EXTENSIONS $PYTHON_ALL_EXTENSIONS"
	USEFUL_EXTENSIONS="$USEFUL_EXTENSIONS $PYTHON_USEFUL_EXTENSIONS"
	DEFAULT_EXTENSIONS="$DEFAULT_EXTENSIONS $PYTHON_DEFAULT_EXTENSIONS"
else
	AC_MSG_WARN([Epiphany compiled without python support, extensions $PYTHON_ALL_EXTENSIONS will not be available])
fi

# due to an autoconf bug, commas in the first arg to
# AS_HELP_STRING() cause problems.
#
# AS_HELP_STRING(--with-extensions=ext1,ext2,...,build the specified extensions)
AC_ARG_WITH([extensions],
[  --with-extensions=extension1,extension2,...
			  build the specified extensions. Available:
			  actions, adblock, certificates, dashboard,
			  error-viewer, extensions-manager-ui, gestures,
			  page-info, rss, sample, sampls-mozilla, select-stylesheet,
			  sidebar, smart-bookmarks, tab-groups,
			  as well as the aliases default, all, and really-all],
			  [extensions=$with_extensions],
			  [extensions="default"])

if test "x$with_extensions" = xyes; then
  extensions="default"
fi

need_python=no
EXTENSIONS=
IFS="${IFS=	}"; ephyext_save_ifs="$IFS"; IFS=","
for ext in $extensions; do
 if echo "$ALL_EXTENSIONS" | egrep "(^| )$ext(\$| )" > /dev/null; then
   EXTENSIONS="$EXTENSIONS $ext"
 elif test "x$ext" = "xdefault"; then
   EXTENSIONS="$EXTENSIONS $DEFAULT_EXTENSIONS"
 elif test "x$ext" = "xall"; then
   EXTENSIONS="$EXTENSIONS $USEFUL_EXTENSIONS"
 elif test "x$ext" = "xreally-all"; then
   EXTENSIONS="$EXTENSIONS $ALL_EXTENSIONS"
 else
   AC_MSG_ERROR([the specified extension $ext does not exist])
 fi
 
 if echo "$PYTHON_ALL_EXTENSIONS" | egrep "(^| )$ext(\$| )" > /dev/null; then
   need_python=yes
 fi
done
IFS="$ephyext_save_ifs"

AC_SUBST([DIST_EXTENSIONS])
AC_SUBST([BUILD_EXTENSIONS],[`echo $EXTENSIONS | tr ' ' '\n' | sort | uniq | tr '\n' ' '`])

AC_MSG_RESULT([$EXTENSIONS])

# ****
# DBUS
# ****

if echo "$EXTENSIONS" | egrep '(^| )(rss)($| )' > /dev/null; then
	DBUS_REQUIRED=0.34
	DBUS_GLIB_REQUIRED=0.34

	PKG_CHECK_MODULES([DBUS], [dbus-1 >= $DBUS_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED])
fi

# ****
# PCRE
# ****

if echo "$EXTENSIONS" | egrep '(^| )(adblock|greasemonkey)($| )' > /dev/null; then
	AC_PATH_PROG([PCRE_CONFIG], [pcre-config], [no])

	if test "x$PCRE_CONFIG" = "xno"; then
		AC_ERROR([pcre development files not found])
	fi

	PCRE_CFLAGS="`$PCRE_CONFIG --cflags`"
	AC_SUBST([PCRE_CFLAGS])
	PCRE_LIBS="`$PCRE_CONFIG --libs`"
	AC_SUBST([PCRE_LIBS])

	pcre_saved_CFLAGS=$CFLAGS
	pcre_saved_LIBS=$LIBS
	CFLAGS="$CFLAGS $PCRE_CFLAGS"
	LIBS="$LIBS $PCRE_LIBS"

	AC_CACHE_CHECK([whether libpcre was compiled with UTF-8 support],
		[ac_cv_have_preg_utf8_support],
		[AC_RUN_IFELSE(
		 	[AC_LANG_SOURCE(
				 [[#include <pcre.h>

				   int main() {
				     int t;
				     return (pcre_config (PCRE_CONFIG_UTF8, &t) || !t);
				   }]])],
			[ac_cv_have_preg_utf8_support="yes"],
			[ac_cv_have_preg_utf8_support="no"],
			[ac_cv_have_preg_utf8_support="maybe"])])

	CFLAGS=$pcre_saved_CFLAGS
	LIBS=$pcre_saved_LIBS

	if test "$ac_cv_have_preg_utf8_support" != "yes"; then
		AC_ERROR([pcre does not support utf-8])
	fi
fi

# ******
# Python
# ******

if test "x$need_python" = "xyes"; then
	AM_PATH_PYTHON([2.3])

	if test "x$PYTHON" = "x:"; then
		AC_MSG_ERROR([Python not found])
	fi
fi

# *******
# Mozilla
# *******

GECKO_INIT([MOZILLA])

AC_SUBST([MOZILLA])
AC_SUBST([MOZILLA_FLAVOUR])
AC_SUBST([MOZILLA_HOME])
AC_SUBST([MOZILLA_PREFIX])
AC_SUBST([MOZILLA_INCLUDE_ROOT])

case "$MOZILLA" in
mozilla) min_version=1.7.5 ;;
seamonkey) min_version=1.0 ;;
*firefox) min_version=1.0 ;;
xulrunner) min_version=1.8 ;;
esac

PKG_CHECK_MODULES([MOZILLA_COMPONENT],[${gecko_cv_gecko}-xpcom >= $min_version $gecko_cv_extra_pkg_dependencies])
AC_SUBST([MOZILLA_COMPONENT_CFLAGS])
AC_SUBST([MOZILLA_COMPONENT_LIBS])

# Needed since 1.8b2
# Define this down here so it doesn't affect the API checks above

AC_DEFINE([MOZILLA_INTERNAL_API],[1],[Define for access to internal mozilla API])

# *****************
# Add warning flags
# *****************

AM_CPPFLAGS="$AM_CPPFLAGS $DEPRECATION_FLAGS"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS"
AM_CXXFLAGS="$AM_CXXFLAGS $WARN_CXXFLAGS $MOZILLA_WARN_CXXFLAGS"
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])

# *******************************
# share/data dir
# *******************************

if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${ac_default_prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
  else
    AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
  fi
else
  AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${datadir}/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
fi

# *******************************
# *******************************

AC_CONFIG_FILES([
Makefile
include/Makefile
extensions/Makefile
extensions/actions/Makefile
extensions/adblock/Makefile
extensions/auto-reload/Makefile
extensions/auto-scroller/Makefile
extensions/certificates/Makefile
extensions/dashboard/Makefile
extensions/gestures/Makefile
extensions/error-viewer/Makefile
extensions/error-viewer/mozilla/Makefile
extensions/error-viewer/opensp/Makefile
extensions/extensions-manager-ui/Makefile
extensions/favicon/Makefile
extensions/greasemonkey/Makefile
extensions/greasemonkey/mozilla/Makefile
extensions/imagebar/Makefile
extensions/page-info/Makefile
extensions/page-info/mozilla/Makefile
extensions/permissions/Makefile
extensions/push-scroller/Makefile
extensions/python-console/Makefile
extensions/rss/Makefile
extensions/sample/Makefile
extensions/sample-mozilla/Makefile
extensions/sample-mozilla/mozilla/Makefile
extensions/sample-python/Makefile
extensions/select-stylesheet/Makefile
extensions/select-stylesheet/mozilla/Makefile
extensions/sidebar/Makefile
extensions/smart-bookmarks/Makefile
extensions/smart-bookmarks/mozilla/Makefile
extensions/tab-states/Makefile
extensions/tab-groups/Makefile
extensions/java-console/Makefile
extensions/java-console/mozilla/Makefile
help/Makefile
po/Makefile.in
])

AC_OUTPUT
