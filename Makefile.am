SUBDIRS = include extensions po

EXTRA_DIST =			\
	COPYING.README		\
	ChangeLog  		\
	HACKING			\
	README 			\
	TODO 			\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in	\
	tools/copy-template	\
	po/LINGUAS		\
	doc/writing-epiphany-extensions.xml

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --with-extensions=really-all
