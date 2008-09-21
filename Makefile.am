SUBDIRS = include extensions data po help

ACLOCAL_AMFLAGS = -I m4

NULL =

EXTRA_DIST =			\
	autogen.sh		\
	COPYING.README		\
	ChangeLog		\
	ChangeLog-20070912	\
	po/ChangeLog.pre-2-24	\
	HACKING			\
	README 			\
	TODO 			\
	ephy-extension.symbols	\
	gnome-doc-utils.make	\
	tools/copy-template	\
	doc/writing-epiphany-extensions.xml	\
	$(NULL)

DISTCLEANFILES = 		\
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
	if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	elif test -f $(top_srcdir)/.svn/entries; then \
		svn log -v --xml -r HEAD:1572 $(SVN_ROOT)/$(SVN_MODULE)/$(SVN_BRANCH) | \
			xsltproc --stringparam strip-prefix "$(SVN_MODULE)/$(SVN_BRANCH)" \
				 --stringparam include-rev "yes" $(top_srcdir)/svn2cl.xsl - > $@; \
	fi

.PHONY: ChangeLog
