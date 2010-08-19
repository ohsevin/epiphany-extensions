# Copyright (c) 2003, 2004, 2005, 2008 Christian Persch
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

m4_define([e_e_version_major],[2])
m4_define([e_e_version_minor],[32])
m4_define([e_e_version_micro],[1])
m4_define([e_e_version],[e_e_version_major.e_e_version_minor.e_e_version_micro])

# This can be either "trunk" or "branches/gnome-x-y"
m4_define([e_e_branch],["trunk"])

AC_INIT([GNOME Web Browser Extensions],[e_e_version],[http://bugzilla.gnome.org/enter_bug.cgi?product=epiphany-extensions],[epiphany-extensions])

EPIPHANY_API_VERSION=2.29

EPIPHANY_EXTENSIONS_MAJOR=e_e_version_major.e_e_version_minor
AC_SUBST(EPIPHANY_EXTENSIONS_MAJOR)

AC_PREREQ([2.59])

AC_REVISION([$Revision: 1.235 $])

AC_CONFIG_SRCDIR([extensions])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip tar-ustar])

# Use AM_SILENT_RULES if present
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

if test -z "$enable_maintainer_mode"; then
  enable_maintainer_mode=yes
fi
AM_MAINTAINER_MODE([enable])

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
AC_PATH_PROG(GCONFTOOL, [gconftool-2])
AM_GCONF_SOURCE_2

AC_SUBST([AM_CXXFLAGS])

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS
GNOME_MAINTAINER_MODE_DEFINES

MORE_WARN_FLAGS=
DEPRECATION_FLAGS=
MOZILLA_WARN_CXXFLAGS="-Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

if test "x$enable_maintainer_mode" = "xyes"; then
	AC_DEFINE([MAINTAINER_MODE],[1],[Define to enable 'maintainer-only' behaviour])
	enable_debug=yes
        DEPRECATION_FLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGCONF_DISABLE_DEPRECATED -DPANGO_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED -DG_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_SINGLE_INCLUDES"
	MOZILLA_WARN_CXXFLAGS="-Wall -Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth $MOZILLA_WARN_CXXFLAGS"
fi

# ********************************
# Internationalisation
# ********************************

IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=epiphany-extensions-$EPIPHANY_API_VERSION
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"], [Gettext package])
AM_GLIB_GNU_GETTEXT

# ********************************
# Documentation
# ********************************

GNOME_DOC_INIT

# ********************************
# pkg config checks
# ********************************

EPIPHANY_REQUIRED=2.27.4
LIBXML_REQUIRED=2.6.0
LIBGLIB_REQUIRED=2.15.5
LIBGTK_REQUIRED=2.21.6

AC_SUBST([EPIPHANY_REQUIRED])
AC_SUBST([LIBXML_REQUIRED])
AC_SUBST([LIBGLIB_REQUIRED])
AC_SUBST([LIBGTK_REQUIRED])


EPIPHANY_EXTENSION_INIT([$EPIPHANY_API_VERSION], [$EPIPHANY_REQUIRED])

PKG_CHECK_MODULES([EPIPHANY_DEPENDENCY], [\
		  libxml-2.0 >= $LIBXML_REQUIRED \
		  glib-2.0 >= $LIBGLIB_REQUIRED \
		  gio-2.0 >= $LIBGLIB_REQUIRED \
		  gmodule-2.0 \
		  gtk+-2.0 >= $LIBGTK_REQUIRED \
		  gconf-2.0 \
		  epiphany-$EPIPHANY_API_VERSION >= $EPIPHANY_REQUIRED])
AC_SUBST([EPIPHANY_DEPENDENCY_CFLAGS])
AC_SUBST([EPIPHANY_DEPENDENCY_LIBS])

PKG_CHECK_MODULES([WEBKIT], [webkit-1.0])
AC_SUBST([WEBKIT_CFLAGS])
AC_SUBST([WEBKIT_LIBS])

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

ALL_EXTENSIONS="actions adblock auto-reload auto-scroller certificates error-viewer extensions-manager-ui gestures greasemonkey html5tube java-console livehttpheaders page-info permissions push-scroller rss sample select-stylesheet smart-bookmarks soup-fly tab-groups tab-key-tab-navigate tab-states"
USEFUL_EXTENSIONS="actions adblock auto-reload auto-scroller certificates extensions-manager-ui html5tube java-console page-info push-scroller select-stylesheet smart-bookmarks soup-fly tab-groups tab-key-tab-navigate tab-states"
DEFAULT_EXTENSIONS="actions adblock auto-reload auto-scroller certificates extensions-manager-ui greasemonkey gestures html5tube push-scroller soup-fly tab-groups tab-key-tab-navigate tab-states rss"

MOZILLA_ALL_EXTENSIONS="error-viewer java-console livehttpheaders page-info select-stylesheet smart-bookmarks"

DIST_EXTENSIONS="$ALL_EXTENSIONS"

# due to an autoconf bug, commas in the first arg to
# AS_HELP_STRING() cause problems.
#
# AS_HELP_STRING(--with-extensions=ext1,ext2,...,build the specified extensions)
AC_ARG_WITH([extensions],
[  --with-extensions=extension1,extension2,...
			  build the specified extensions. Available:
			  actions, adblock, auto-reload, auto-scroller,
			  certificates, error-viewer, extensions-manager-ui,
			  gestures, greasemonkey, java-console, livehttpheaders, page-info,
			  permissions, push-scroller, rss, sample,
			  select-stylesheet,
			  smart-bookmarks, soup-fly, tab-groups, tab-states,
			  as well as the aliases default, all, and really-all],
			  [extensions=$with_extensions],
			  [extensions="default"])

if test "x$with_extensions" = xyes; then
  extensions="default"
fi

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

done
IFS="$ephyext_save_ifs"

# Remove the non-ported mozilla extensions from our list
extensions=$EXTENSIONS; EXTENSIONS=
for ext in $extensions; do
  if echo "$MOZILLA_ALL_EXTENSIONS" | egrep -v "(^| )$ext(\$| )" > /dev/null; then
    EXTENSIONS="$EXTENSIONS $ext"
  fi
done

AC_SUBST([DIST_EXTENSIONS])
AC_SUBST([BUILD_EXTENSIONS],[$(echo $EXTENSIONS | tr ' ' '\n' | sort | uniq | tr '\n' ' ')])

AC_MSG_RESULT([$EXTENSIONS])

# ****
# DBUS
# ****

if echo "$EXTENSIONS" | egrep '(^| )(rss)($| )' > /dev/null; then
	DBUS_REQUIRED=0.34
	DBUS_GLIB_REQUIRED=0.34

	PKG_CHECK_MODULES([DBUS], [dbus-1 >= $DBUS_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED])
fi

# *****************
# Add warning flags
# *****************

AM_CPPFLAGS="$AM_CPPFLAGS $DEPRECATION_FLAGS"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS"
AM_CXXFLAGS="$AM_CXXFLAGS $WARN_CXXFLAGS"
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
data/Makefile
data/icons/Makefile
extensions/Makefile
extensions/actions/Makefile
extensions/adblock/Makefile
extensions/auto-reload/Makefile
extensions/auto-scroller/Makefile
extensions/certificates/Makefile
extensions/gestures/Makefile
extensions/error-viewer/Makefile
extensions/error-viewer/mozilla/Makefile
extensions/error-viewer/opensp/Makefile
extensions/extensions-manager-ui/Makefile
extensions/greasemonkey/Makefile
extensions/html5tube/Makefile
extensions/livehttpheaders/Makefile
extensions/livehttpheaders/mozilla/Makefile
extensions/page-info/Makefile
extensions/page-info/mozilla/Makefile
extensions/permissions/Makefile
extensions/push-scroller/Makefile
extensions/rss/Makefile
extensions/sample/Makefile
extensions/select-stylesheet/Makefile
extensions/select-stylesheet/mozilla/Makefile
extensions/smart-bookmarks/Makefile
extensions/smart-bookmarks/mozilla/Makefile
extensions/soup-fly/Makefile
extensions/tab-key-tab-navigate/Makefile
extensions/tab-states/Makefile
extensions/tab-groups/Makefile
extensions/java-console/Makefile
extensions/java-console/mozilla/Makefile
help/Makefile
po/Makefile.in
])

AC_OUTPUT

# *************************************
# *************************************

echo "
Epiphany Extensions was configured with the following options:

	Source code location       : $srcdir
	Compiler                   : $CC
	Prefix                     : $prefix
	Extra debugging support    : $enable_debug

	Epiphany API version	   : $EPIPHANY_API_VERSION

	OpenSP			   : $enable_opensp

	Extensions		   : $EXTENSIONS
"
