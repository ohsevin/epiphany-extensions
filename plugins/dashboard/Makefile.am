plugindir = $(libdir)/epiphany/plugins

INCLUDES = \
	-I$(top_srcdir)/include		\
	$(EPIPHANY_DEPENDENCY_CFLAGS)	\
	-DSHARE_DIR=\"$(pkgdatadir)\"

plugin_LTLIBRARIES = libdashboard.la

libdashboard_la_SOURCES = \
	dashboard.c
