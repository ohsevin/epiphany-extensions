INCLUDES = \
	-I..					\
	-I$(top_srcdir)/include			\
	-I$(top_srcdir)/extensions/error-viewer/mozilla	\
	-I$(top_srcdir)/extensions/error-viewer	\
	$(MOZILLA_COMPONENT_CFLAGS)		\
	-I$(MOZILLA_INCLUDE_ROOT)		\
	-I$(MOZILLA_INCLUDE_ROOT)/webbrwsr	\
	-I$(MOZILLA_INCLUDE_ROOT)/widget	\
	-I$(MOZILLA_INCLUDE_ROOT)/js		\
	-I$(MOZILLA_INCLUDE_ROOT)/jsconsole	\
	-I$(MOZILLA_INCLUDE_ROOT)/content	\
	-I$(MOZILLA_INCLUDE_ROOT)/docshell	\
	-I$(MOZILLA_INCLUDE_ROOT)/dom		\
	-I$(MOZILLA_INCLUDE_ROOT)/xpcom		\
	-I$(MOZILLA_INCLUDE_ROOT)/necko		\
	-I$(MOZILLA_INCLUDE_ROOT)/string	\
	$(EPIPHANY_DEPENDENCY_CFLAGS)		\
	-DSHARE_DIR=\"$(pkgdatadir)\"		\
	-include $(MOZILLA_INCLUDE_ROOT)/mozilla-config.h

if MAINTAINER_MODE
INCLUDES += -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DBONOBO_DISABLE_DEPRECATED $(WARN_CXXFLAGS)
endif

noinst_LTLIBRARIES = liberrorviewermozilla.la

liberrorviewermozilla_la_SOURCES = \
	ErrorViewerConsoleListener.cpp		\
	ErrorViewerConsoleListener.h		\
	ErrorViewerURICheckerObserver.cpp	\
	ErrorViewerURICheckerObserver.h		\
	register-listener.cpp			\
	register-listener.h			\
	mozilla-helpers.cpp			\
	mozilla-helpers.h			\
	mozilla-link-checker.cpp		\
	mozilla-link-checker.h
