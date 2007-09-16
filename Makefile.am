SUBDIRS = include extensions data po help

ACLOCAL_AMFLAGS = -I m4

NULL =

EXTRA_DIST =			\
	autogen.sh		\
	COPYING.README		\
	ChangeLog		\
	ChangeLog-20070912	\
	HACKING			\
	README 			\
	TODO 			\
	ephy-extension.symbols	\
	intltool-extract.in 	\
	intltool-merge.in 	\
	intltool-update.in	\
	gnome-doc-utils.make	\
	tools/copy-template	\
	doc/writing-epiphany-extensions.xml	\
	$(NULL)

DISTCLEANFILES = 		\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update		\
	gnome-doc-utils.make	\
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--with-extensions=really-all	\
	--with-gecko=$(MOZILLA)		\
	--disable-scrollkeeper		\
	--disable-schemas-install

# Build ChangeLog from SVN history (r1572 was the first ChangeLog-less checkin).
# Only build this when in an svn checkout.

ChangeLog:
	@if test -f $(top_srcdir)/.svn/entries; then \
		svn log -v --xml -r HEAD:1572 http://svn.gnome.org/svn/epiphany | \
			xsltproc --stringparam strip-prefix "epiphany/trunk" \
				 --stringparam include-rev "yes" $(top_srcdir)/svn2cl.xsl - > $@; \
	fi

.PHONY: ChangeLog
