SUBDIRS = include extensions po help

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST =			\
	autogen.sh		\
	COPYING.README		\
	ChangeLog  		\
	HACKING			\
	README 			\
	TODO 			\
	ephy-extension.symbols	\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in	\
	gnome-doc-utils.make	\
	tools/copy-template	\
	doc/writing-epiphany-extensions.xml

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = \
	--with-extensions=really-all	\
	--with-gecko=$(MOZILLA)		\
	--disable-scrollkeeper		\
	--disable-schemas-install
