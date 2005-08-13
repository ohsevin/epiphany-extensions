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

AC_INIT([Epiphany Extensions],[1.7.5],[http://bugzilla.gnome.org/enter_bug.cgi?product=epiphany-extensions],[epiphany-extensions])
GNOME_COMMON_INIT

EPIPHANY_API_VERSION=1.8

EPIPHANY_EXTENSIONS_MAJOR=1.7
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
GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS

dnl ********************************
dnl Internationalisation
dnl ********************************

dnl Translators: new languages must be added to the po/LINGUAS file
ALL_LINGUAS="`cat "$srcdir/po/LINGUAS" | grep -v '^#'`"

AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])

GETTEXT_PACKAGE=epiphany-extensions-$EPIPHANY_API_VERSION
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"], [Gettext package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([LOCALE_DIR])

dnl ********************************
dnl pkg config checks
dnl ********************************

EPIPHANY_REQUIRED=1.7.3
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
		  epiphany-$EPIPHANY_API_VERSION >= $EPIPHANY_REQUIRED])
AC_SUBST([EPIPHANY_DEPENDENCY_CFLAGS])
AC_SUBST([EPIPHANY_DEPENDENCY_LIBS])

EXTENSIONS_DIR="`$PKG_CONFIG --variable=extensionsdir epiphany-$EPIPHANY_API_VERSION`"
FEATURES="`$PKG_CONFIG --variable=features epiphany-$EPIPHANY_API_VERSION`"
AC_SUBST([EXTENSIONS_DIR])

dnl *******************************
dnl Check for OpenSP
dnl *******************************

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

dnl *******************************
dnl Extensions
dnl *******************************

AC_MSG_CHECKING([which extensions to build])

ALL_EXTENSIONS="actions adblock bookmarks-tray certificates dashboard error-viewer extensions-manager-ui gestures greasemonkey net-monitor page-info rss sample sample-mozilla select-stylesheet sidebar smart-bookmarks tab-groups tab-states tabsmenu"
USEFUL_EXTENSIONS="actions bookmarks-tray certificates dashboard error-viewer extensions-manager-ui gestures page-info select-stylesheet sidebar smart-bookmarks tab-groups tab-states"
DEFAULT_EXTENSIONS="actions bookmarks-tray certificates error-viewer extensions-manager-ui gestures page-info select-stylesheet sidebar smart-bookmarks tab-groups tab-states"

PYTHON_ALL_EXTENSIONS="python-console sample-python"
PYTHON_USEFUL_EXTENSIONS="python-console"
PYTHON_DEFAULT_EXTENSIONS="python-console"

DIST_EXTENSIONS="$ALL_EXTENSIONS $PYTHON_ALL_EXTENSIONS"

if echo "$FEATURES" | egrep '(^| )(python)($| )' > /dev/null; then
	ALL_EXTENSIONS="$ALL_EXTENSIONS $PYTHON_ALL_EXTENSIONS"
	USEFUL_EXTENSIONS="$USEFUL_EXTENSIONS $PYTHON_USEFUL_EXTENSIONS"
	DEFAULT_EXTENSIONS="$DEFAULT_EXTENSIONS $PYTHON_DEFAULT_EXTENSIONS"
else
	AC_MSG_WARN([Epiphany compiled without python support, extensions \'$PYTHON_ALL_EXTENSIONS\' will not be available])
fi

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

dnl ****
dnl DBUS
dnl ****

if echo "$EXTENSIONS" | egrep '(^| )(net-monitor)|(rss)($| )' > /dev/null; then
	if ! (echo "$FEATURES" | egrep '(^| )(dbus)($| )' > /dev/null); then
		AC_MSG_ERROR([Epiphany was not compiled with DBUS support])
	fi

	AC_MSG_CHECKING([for new DBUS API])

	_SAVE_LDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs dbus-1`"
	AC_TRY_LINK_FUNC([dbus_message_iter_get_boolean],[have_new_dbus=no],[have_new_dbus=yes])
	LDFLAGS="$_SAVE_LDFLAGS"

	AC_MSG_RESULT([$have_new_dbus])

	if test "x$have_new_dbus" = "xyes"; then
		AC_DEFINE([HAVE_NEW_DBUS],[1],[Define if compiling with DBUS 0.3])

		DBUS_REQUIRED=0.34
		DBUS_GLIB_REQUIRED=0.34
	else
		DBUS_REQUIRED=0.22
		DBUS_GLIB_REQUIRED=0.22
	fi

	PKG_CHECK_MODULES([DBUS], [dbus-1 >= $DBUS_REQUIRED dbus-glib-1 >= $DBUS_GLIB_REQUIRED])
fi

dnl ****
dnl PCRE
dnl ****

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

dnl ******
dnl Python
dnl ******

if test "x$need_python" = "xyes"; then
	AM_PATH_PYTHON([2.3])

	if test "x$PYTHON" = "x:"; then
		AC_MSG_ERROR([Python not found])
	fi
fi

dnl *******
dnl Mozilla
dnl *******

AC_MSG_CHECKING([which gecko to use])

AC_ARG_WITH([mozilla],
	AS_HELP_STRING([--with-mozilla@<:@=mozilla|firefox|thunderbird@:>@],
		       [Which gecko engine to use (default: autodetect)]))

GECKOS="firefox mozilla-firefox seamonkey mozilla thunderbird mozilla-thunderbird"
gecko=$with_mozilla

if test "x$gecko" = "x"; then
	dnl Autodetect gecko
	for g in $GECKOS; do
		if $PKG_CONFIG --exists $g-xpcom; then
			gecko=$g
			break;
		fi
	done
fi

if test "x$gecko" = "x"; then
	AC_MSG_ERROR([No gecko found])
elif ! ( echo "$GECKOS" | egrep "(^| )$gecko(\$| )" > /dev/null); then
	AC_MSG_ERROR([Unknown gecko "$gecko" specified])
fi

AC_MSG_RESULT([$gecko])

case "$gecko" in
mozilla) min_version=1.7.5 flavour=mozilla ;;
seamonkey) min_version=1.0 flavour=mozilla ;;
*firefox) min_version=1.0 flavour=toolkit ;;
*thunderbird) min_version=1.0 flavour=toolkit ;;
esac

MOZILLA=$gecko
AC_SUBST([MOZILLA])

MOZILLA_FLAVOUR=$flavour
AC_SUBST([MOZILLA_FLAVOUR])

PKG_CHECK_MODULES([MOZILLA_COMPONENT], [$MOZILLA-gtkmozembed >= $min_version])
AC_SUBST([MOZILLA_COMPONENT_CFLAGS])
AC_SUBST([MOZILLA_COMPONENT_LIBS])

MOZILLA_INCLUDE_ROOT="`$PKG_CONFIG --variable=includedir $MOZILLA-gtkmozembed`"
AC_SUBST([MOZILLA_INCLUDE_ROOT])

MOZILLA_HOME="`$PKG_CONFIG --variable=libdir $MOZILLA-gtkmozembed`"
AC_SUBST([MOZILLA_HOME])

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
_SAVE_CPPFLAGS=$CPPFLAGS
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

CPPFLAGS="$_SAVE_CPPFLAGS $AM_CPPFLAGS -I$MOZILLA_INCLUDE_ROOT `$PKG_CONFIG --cflags-only-I $MOZILLA-xpcom` -I$MOZILLA_INCLUDE_ROOT/necko"
CXXFLAGS="$_SAVE_CXXFLAGS $AM_CXXFLAGS `$PKG_CONFIG --cflags-only-other $MOZILLA-xpcom`"

dnl Check whether we have a mozilla debug build

AC_MSG_CHECKING([[whether we have a mozilla debug build]])

AC_PREPROC_IFELSE(
	[AC_LANG_SOURCE(
		[[#include <mozilla-config.h>
		  #if !defined(MOZ_REFLOW_PERF) || !defined(MOZ_REFLOW_PERF_DSP)
		  #error No
		  #endif]]
	)],
	[AM_CXXFLAGS="-DDEBUG -D_DEBUG $AM_CXXFLAGS" have_mozilla_debug=yes],
	[have_mozilla_debug=no])

AC_MSG_RESULT([$have_mozilla_debug])

dnl Determine gecko version
dnl We don't use the version from the .pc file, since that's the app version
dnl and not the Gecko version

AC_MSG_CHECKING([[for gecko version]])

gecko_version_major=`cat $MOZILLA_INCLUDE_ROOT/mozilla-config.h | grep MOZILLA_VERSION_MAJOR | awk '{ print $[3]; }'`
gecko_version_minor=`cat $MOZILLA_INCLUDE_ROOT/mozilla-config.h | grep MOZILLA_VERSION_MINOR | awk '{ print $[3]; }'`

dnl These variables are only available since gecko 1.8
if test -z "$gecko_version_major" -o -z "$gecko_version_minor"; then
	gecko_version_major=1
	gecko_version_minor=7
fi

AC_MSG_RESULT([$gecko_version_major.$gecko_version_minor])

if test "$gecko_version_major" = "1" -a "$gecko_version_minor" = "9"; then
	AC_MSG_ERROR([Gecko 1.9 is not supported yet])
	have_gecko_1_8=yes
elif test "$gecko_version_major" = "1" -a "$gecko_version_minor" = "8"; then
	have_gecko_1_8=yes
elif test "$gecko_version_major" = "1" -a "$gecko_version_minor" = "7"; then
	have_gecko_1_8=no
else
	AC_MSG_ERROR([Unsupported Gecko version $gecko_version_major.$gecko_version_minor])
fi

if test "x$have_gecko_1_8" = "xyes"; then
	AC_DEFINE([HAVE_GECKO_1_8],[1],[Define if we have gecko 1.8])
fi

AM_CONDITIONAL([HAVE_GECKO_1_8],[test "x$have_gecko_1_8" = "xyes"])

dnl Changed in 1.8a1 and again in 1.8a3 to its present form (variant 2);
dnl variant 1 is on 1.7 branch. Aviary branch also has variant 2.

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

dnl restore flags
CPPFLAGS=$_SAVE_CPPFLAGS
CXXFLAGS=$_SAVE_CXXFLAGS
AC_LANG_POP([C++])

dnl Needed since $EPIPHANY_API_VERSIONb2
dnl Define this down here so it doesn't affect the API checks above

AC_DEFINE([MOZILLA_INTERNAL_API],[1],[Define for access to internal mozilla API])

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
extensions/gestures/Makefile
extensions/error-viewer/Makefile
extensions/error-viewer/mozilla/Makefile
extensions/error-viewer/opensp/Makefile
extensions/extensions-manager-ui/Makefile
extensions/greasemonkey/Makefile
extensions/greasemonkey/mozilla/Makefile
extensions/net-monitor/Makefile
extensions/page-info/Makefile
extensions/page-info/mozilla/Makefile
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
extensions/tabsmenu/Makefile
extensions/tab-groups/Makefile
po/Makefile.in
])

AC_OUTPUT
