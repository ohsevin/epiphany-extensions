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

AC_INIT([Epiphany Extensions],[1.6.1.90],[http://bugzilla.gnome.org/enter_bug.cgi?product=epiphany-extensions],[epiphany-extensions])
GNOME_COMMON_INIT

EPIPHANY_EXTENSIONS_MAJOR=1.6
AC_SUBST(EPIPHANY_EXTENSIONS_MAJOR)

AC_PREREQ([2.59])

AC_REVISION([$Revision$])

AC_CONFIG_SRCDIR([extensions])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip check-news])

dnl AC_SUBST([ACLOCAL_AMFLAGS], ["-I $ac_macro_dir \${ACLOCAL_FLAGS}"])
AC_SUBST([ACLOCAL_AMFLAGS], ["\${ACLOCAL_FLAGS}"])

AM_MAINTAINER_MODE
if test "x$enable_maintainer_mode" = "xyes"; then
	AC_DEFINE([MAINTAINER_MODE],[1],[Define to enable 'maintainer-only' behaviour])
	enable_debug=yes
	DEPRECATION_FLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGCONF_DISABLE_DEPRECATED -DGNOME_VFS_DISABLE_DEPRECATED -DBONOBO_UI_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED -DLIBGLADE_DISABLE_DEPRECATED -DPANGO_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED"
	MOZILLA_WARN_CXXFLAGS="-Wall -Wconversion -Wpointer-arith -Wcast-align -Woverloaded-virtual -Wsynth"
fi
MOZILLA_WARN_CXXFLAGS="$MOZILLA_WARN_CXXFLAGS -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor"

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
GNOME_COMPILE_WARNINGS([error])
GNOME_CXX_WARNINGS

dnl ********************************
dnl Internationalisation
dnl ********************************

dnl Translators: new languages must be added to the po/LINGUAS file
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"

AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])

GETTEXT_PACKAGE=epiphany-extensions-1.6
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"], [Gettext package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([LOCALE_DIR])

dnl ********************************
dnl pkg config checks
dnl ********************************

EPIPHANY_REQUIRED=1.6.0
LIBXML_REQUIRED=2.6.0
LIBGLIB_REQUIRED=2.6.0
LIBGTK_REQUIRED=2.6.0

AC_SUBST([EPIPHANY_REQUIRED])
AC_SUBST([LIBXML_REQUIRED])
AC_SUBST([LIBGLIB_REQUIRED])
AC_SUBST([LIBGTK_REQUIRED])

PKG_CHECK_MODULES([EPIPHANY_DEPENDENCY], [\
		  libxml-2.0 >= $LIBXML_REQUIRED \
		  libglade-2.0 \
		  glib-2.0 >= $LIBGLIB_REQUIRED \
		  gmodule-2.0 \
		  gtk+-2.0 >= $LIBGTK_REQUIRED \
		  epiphany-1.6 >= $EPIPHANY_REQUIRED])
AC_SUBST([EPIPHANY_DEPENDENCY_CFLAGS])
AC_SUBST([EPIPHANY_DEPENDENCY_LIBS])

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
			 [have_opensp="yes"],
			 [have_opensp="no"])

	AM_CONDITIONAL([HAVE_OPENSP], test "x$have_opensp" = "xyes")

	AC_LANG_POP([C++])
fi

dnl *******************************
dnl Extensions
dnl *******************************

AC_MSG_CHECKING([which extensions to build])

ALL_EXTENSIONS="actions adblock bookmarks-tray certificates dashboard error-viewer extensions-manager-ui find gestures net-monitor page-info sample sample-mozilla select-stylesheet sidebar smart-bookmarks tab-groups tab-states tabsmenu"
USEFUL_EXTENSIONS="actions bookmarks-tray certificates dashboard error-viewer extensions-manager-ui gestures page-info select-stylesheet sidebar smart-bookmarks tab-groups tab-states tabsmenu"
DEFAULT_EXTENSIONS="actions bookmarks-tray certificates error-viewer extensions-manager-ui gestures page-info select-stylesheet sidebar smart-bookmarks tabsmenu tab-groups tab-states"

dnl due to an autoconf bug, commas in the first arg to
dnl AS_HELP_STRING() cause problems.
dnl
dnl AS_HELP_STRING(--with-extensions=ext1,ext2,...,build the specified extensions)
AC_ARG_WITH([extensions],
[  --with-extensions=extension1,extension2,...
			  build the specified extensions. Available:
			  actions, adblock, bookmarks-tray, certificates, dashboard,
			  error-viewer, extensions-manager-ui, gestures, net-monitor,
			  page-info, sample, sampls-mozilla, select-stylesheet,
			  sidebar, smart-bookmarks, tab-groups, and tabsmenu,
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

AC_SUBST([ALL_EXTENSIONS])
AC_SUBST([BUILD_EXTENSIONS],[`echo $EXTENSIONS | tr ' ' '\n' | sort | uniq | tr '\n' ' '`])

AC_MSG_RESULT([$EXTENSIONS])

dnl ****
dnl DBUS
dnl ****

if echo "$EXTENSIONS" | egrep '(^| )net-monitor($| )' > /dev/null; then

	DBUS_REQUIRED=0.22
	DBUS_GLIB_REQUIRED=0.22

	PKG_CHECK_MODULES([DBUS], [dbus-1 >= $DBUS_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED])
	INCLUDE_DIR="`$PKG_CONFIG --variable=includedir epiphany-1.6`"
	AC_CHECK_FILE([$INCLUDE_DIR/epiphany/ephy-dbus.h],,[AC_MSG_ERROR([Epiphany was not compiled with dbus support])])
fi

dnl ****
dnl PCRE
dnl ****

if echo "$EXTENSIONS" | egrep '(^| )adblock($| )' > /dev/null; then
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

dnl ******************
dnl Mozilla name check
dnl ******************

AC_MSG_CHECKING([which mozilla to use])

MOZILLA=
AC_ARG_WITH([mozilla],
	AS_HELP_STRING(--with-mozilla@<:@=mozilla|firefox|thunderbird@:>@,Whether to use mozilla, firefox or thunderbird gtkmozembed (default: mozilla)),
	[MOZILLA="$withval"])

dnl try to autodetect: if mozilla exists, use it, else try firefox, then thunderbird
if test "x$MOZILLA" = "x"; then
  if $PKG_CONFIG --exists mozilla-gtkmozembed; then
    MOZILLA=mozilla
  elif $PKG_CONFIG --exists firefox-gtkmozembed; then
    MOZILLA=firefox
  elif $PKG_CONFIG --exists thunderbird-gtkmozembed; then
    MOZILLA=thunderbird
  else
    AC_MSG_ERROR([no mozilla installation found])
  fi
elif test "x$with_mozilla" != "xmozilla" -a "x$with_mozilla" != "xfirefox" -a "x$with_mozilla" != "xthunderbird"; then
  AC_MSG_ERROR([unknown mozilla name ($MOZILLA)])
fi

AC_SUBST([MOZILLA])

AC_MSG_RESULT([$MOZILLA])

case "$MOZILLA" in
mozilla) min_version=1.7 ;;
firefox) min_version=0.10 ;;
thunderbird) min_version=0.8 ;;
esac

PKG_CHECK_MODULES([MOZILLA_COMPONENT], [$MOZILLA-gtkmozembed >= $min_version])
AC_SUBST([MOZILLA_COMPONENT_CFLAGS])
AC_SUBST([MOZILLA_COMPONENT_LIBS])

MOZILLA_INCLUDE_ROOT="`$PKG_CONFIG --variable=includedir $MOZILLA-gtkmozembed`"
AC_SUBST([MOZILLA_INCLUDE_ROOT])

MOZILLA_HOME="`$PKG_CONFIG --variable=libdir $MOZILLA-gtkmozembed`"
AC_SUBST([MOZILLA_HOME])

dnl whether to build with DEBUG defined
AC_ARG_WITH([mozilla-debug],
        [  --with-mozilla-debug        Use a debug mozilla build],
        [AM_CXXFLAGS="-DDEBUG $AM_CXXFLAGS"])

AC_ARG_ENABLE([cpp-rtti],
        [  --enable-cpp-rtti       Enable C++ RTTI (for cvs gcc)],,
        [enable_cpp_rtti=no])

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
AC_LANG_PUSH([C++])
_SAVE_CXXFLAGS=$CXXFLAGS
_SAVE_AM_CXXFLAGS=$AM_CXXFLAGS
AM_CXXFLAGS="$AM_CXXFLAGS -fshort-wchar"
CXXFLAGS="$CXXFLAGS $AM_CXXFLAGS"

AC_CACHE_CHECK([for compiler -fshort-wchar option],
	ac_cv_have_usable_wchar_option,
	[AC_RUN_IFELSE(
		[AC_LANG_SOURCE(
			[[#include <stddef.h>
			  int main () {
			    return (sizeof(wchar_t) != 2) || (wchar_t)-1 < (wchar_t) 0 ;
			   }]])],
		[ac_cv_have_usable_wchar_option="yes"],
		[ac_cv_have_usable_wchar_option="no"],
		[ac_cv_have_usable_wchar_option="maybe"])])

if test "$ac_cv_have_usable_wchar_option" != "yes"; then
    AM_CXXFLAGS=$_SAVE_AM_CXXFLAGS
fi

if test "$ac_cv_have_usable_wchar_option" != "yes"; then
    AM_CXXFLAGS=$_SAVE_AM_CXXFLAGS
fi

dnl Changed in 1.8a1 and again in 1.8a3 to its present form (variant 2);
dnl variant 1 is on 1.7 branch. Aviary branch also has variant 2.

CXXFLAGS="$_SAVE_CXXFLAGS $AM_CXXFLAGS -I$MOZILLA_INCLUDE_ROOT `$PKG_CONFIG --cflags $MOZILLA-xpcom` -I$MOZILLA_INCLUDE_ROOT/necko -DMOZILLA_STRICT_API"

AC_MSG_CHECKING([for nsIContentPolicy API])

AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <nsEmbedString.h>
		  #include <nsIURI.h>
		  #include <content/nsIContentPolicy.h>]],
		[[nsIContentPolicy *p;
		  nsISupports *arg4;
		  p->ShouldLoad(0,nsnull,nsnull,arg4,nsEmbedCString(),nsnull,nsnull);]]
	)],
	[variant=2],
	[variant=1])

AC_DEFINE_UNQUOTED([MOZ_NSICONTENTPOLICY_VARIANT], [$variant], [Define the nsIContentPolicy API variant])

AC_MSG_RESULT([variant $variant])

AC_MSG_CHECKING([whether nsICacheSession::OpenCacheEntry takes nsACString&])

AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <nsEmbedString.h>
		  #include <nkcache/nsICacheSession.h>]],
		[[nsICacheSession *p;
		  nsEmbedCString arg1;
		  p->OpenCacheEntry(arg1, 0, 0, nsnull);]]
	)],
	[AC_DEFINE([MOZ_NSICACHESESSION_OPENCACHEENTRY_NSACSTRING_],[1],[Define if nsICacheSession::OpenCacheEntry takes nsACString&]) result=yes],
	[result=no])

AC_MSG_RESULT([$result])

dnl Since 1.8b2

AC_MSG_CHECKING([whether nsIScriptError has nsAString& properties])

AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <nsEmbedString.h>
		  #include <xpconnect/nsIScriptError.h>]],
		[[nsIScriptError *p;
		  nsEmbedString arg1;
		  p->GetErrorMessage(arg1);]]
	)],
	[AC_DEFINE([MOZ_NSISCRIPTERROR_NSASTRING_],[1],[Define if nsIScriptError has nsAString& properties]) result=yes],
	[result=no])

AC_MSG_RESULT([$result])

dnl Find extension needs toolkit nsITypeAheadFind

if echo "$EXTENSIONS" | egrep '(^| )find($| )' > /dev/null; then
	AC_CHECK_FILE([$MOZILLA_INCLUDE_ROOT/fastfind/nsITypeAheadFind.h],
	        [AC_DEFINE([HAVE_FASTFIND_NSITYPEAHEADFIND_H],[1],[Define if we have toolkit's nsITypeAheadFind])],
		[AC_MSG_ERROR([Find extension needs toolkit's nsITypeAheadFind])])
fi

dnl restore flags
CXXFLAGS=$_SAVE_CXXFLAGS
AC_LANG_POP([C++])

dnl *****************
dnl Add warning flags
dnl *****************

AM_CPPFLAGS="$AM_CPPFLAGS $DEPRECATION_FLAGS"
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS"
AM_CXXFLAGS="$AM_CXXFLAGS $WARN_CXXFLAGS $MOZILLA_WARN_CXXFLAGS"
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])

dnl *******************************
dnl share/data dir
dnl *******************************

if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${ac_default_prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
  else
    AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${prefix}/share/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
  fi
else
  AC_DEFINE_UNQUOTED([EXTENSIONS_DATA_DIR], ["${datadir}/${PACKAGE}-${EPIPHANY_EXTENSIONS_MAJOR}"], [Directory of data files])
fi

EXTENSIONS_DIR="`$PKG_CONFIG --variable=extensionsdir epiphany-1.6`"
AC_SUBST([EXTENSIONS_DIR])

dnl *******************************
dnl *******************************

AC_CONFIG_FILES([
Makefile
include/Makefile
extensions/Makefile
extensions/actions/Makefile
extensions/adblock/Makefile
extensions/adblock/mozilla/Makefile
extensions/bookmarks-tray/Makefile
extensions/certificates/Makefile
extensions/dashboard/Makefile
extensions/find/Makefile
extensions/gestures/Makefile
extensions/error-viewer/Makefile
extensions/error-viewer/mozilla/Makefile
extensions/error-viewer/opensp/Makefile
extensions/extensions-manager-ui/Makefile
extensions/net-monitor/Makefile
extensions/page-info/Makefile
extensions/page-info/mozilla/Makefile
extensions/sample/Makefile
extensions/sample-mozilla/Makefile
extensions/sample-mozilla/mozilla/Makefile
extensions/select-stylesheet/Makefile
extensions/select-stylesheet/mozilla/Makefile
extensions/sidebar/Makefile
extensions/smart-bookmarks/Makefile
extensions/smart-bookmarks/mozilla/Makefile
extensions/tab-states/Makefile
extensions/tabsmenu/Makefile
extensions/tab-groups/Makefile
po/Makefile.in
])

AC_OUTPUT
