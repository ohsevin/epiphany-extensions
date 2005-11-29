SUBDIRS = include extensions po

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST =			\
	autogen.sh		\
	COPYING.README		\
	ChangeLog  		\
	HACKING			\
	README 			\
	TODO 			\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in	\
	tools/copy-template	\
	doc/writing-epiphany-extensions.xml

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --with-extensions=really-all --with-mozilla=$(MOZILLA)
