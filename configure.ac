dnl Copyright (c) 2003  Christian Persch
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

AC_PREREQ(2.52)

AC_INIT(epiphany-extensions, 0.6, [http://lists.gnome.org/mailman/listinfo/epiphany-list])

AC_CONFIG_SRCDIR(extensions)
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AM_MAINTAINER_MODE

AM_DISABLE_STATIC
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)

AM_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS(error)

dnl ********************************
dnl Internationalisation
dnl ********************************

ALL_LINGUAS="cs de en_GB es fr ja pt sk sr sr@Latn sv"

AC_PROG_INTLTOOL([0.22])

GETTEXT_PACKAGE=epiphany-extensions-1.0
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT

dnl ********************************
dnl pkg config checks
dnl ********************************

EPIPHANY_REQUIRED=1.1.2
LIBXML_REQUIRED=2.6.0
LIBGLIB_REQUIRED=2.3.1
LIBGTK_REQUIRED=2.3.1

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
		  epiphany-1.0 >= $EPIPHANY_REQUIRED)
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
dnl Extensions
dnl *******************************

AC_MSG_CHECKING(extensions to build)

all_extensions="sample,gestures,popup-blocker,tabsmenu"
all_useful_extensions="gestures,popup-blocker,tabsmenu"
default_extensions="gestures,popup-blocker,tabsmenu"

dnl due to an autoconf bug, commas in the first arg to
dnl AC_HELP_STRING cause problems.
dnl
dnl AC_HELP_STRING([--with-extensions=ext1,ext2,...],[build the specified extensions])
AC_ARG_WITH(extensions,
[  --with-extensions=extension1,extension2,...
			  build the specified extensions. Available:
			  gestures, popup-blocker, sample, tabsmenu, all])

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

AM_CONDITIONAL(ENABLE_GESTURES_EXTENSION, echo "$extensions" | egrep '(^|,)gestures($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_SAMPLE_EXTENSION, echo "$extensions" | egrep '(^|,)sample($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_POPUP_BLOCKER_EXTENSION, echo "$extensions" | egrep '(^|,)popup-blocker($|,)' > /dev/null)
AM_CONDITIONAL(ENABLE_TABSMENU_EXTENSION, echo "$extensions" | egrep '(^|,)tabsmenu($|,)' > /dev/null)
dnl AM_CONDITIONAL(ENABLE_TOOLBAR_EXTRAS_EXTENSION, echo "$extensions" | egrep '(^|,)toolbar-extras($|,)' > /dev/null)

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
    AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Directory of data files])
  else
    AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${prefix}/share/${PACKAGE}", [Directory of data files])
  fi
else
  AC_DEFINE_UNQUOTED(EXTENSIONS_DATA_DIR, "${datadir}/${PACKAGE}", [Directory of data files])
fi

dnl *******************************
dnl *******************************

AC_OUTPUT([
Makefile
include/Makefile
extensions/Makefile
extensions/sample/Makefile
extensions/gestures/Makefile
extensions/tabsmenu/Makefile
extensions/popup-blocker/Makefile
po/Makefile.in
],
[])
