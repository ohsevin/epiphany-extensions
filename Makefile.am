## Process this file with automake to produce Makefile.in

SUBDIRS = include plugins po

EXTRA_DIST =			\
	COPYING.README		\
	ChangeLog  		\
	README 			\
	TODO 			\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --with-extensions=all

