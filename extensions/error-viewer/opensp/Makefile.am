noinst_LTLIBRARIES = liberrorvieweropensp.la

liberrorvieweropensp_la_SOURCES = \
	validate.cpp	\
	validate.h

liberrorvieweropensp_la_CPPFLAGS = \
	-I..						\
	-I$(top_srcdir)/include				\
	-I$(top_srcdir)/extensions/error-viewer		\
	-DSHARE_DIR=\"$(pkgdatadir)\"   \
	$(AM_CPPFLAGS)

liberrorvieweropensp_la_CXXFLAGS = \
	$(EPIPHANY_DEPENDENCY_CFLAGS) 	\
	$(AM_CXXFLAGS)
