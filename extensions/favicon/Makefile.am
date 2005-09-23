favicondir = $(EXTENSIONS_DIR)
favicon_PYTHON = \
	favicon.py

extensionsinidir = $(EXTENSIONS_DIR)
extensionsini_in_files = favicon.ephy-extension.in
extensionsini_DATA = $(extensionsini_in_files:.ephy-extension.in=.ephy-extension)

@EPIPHANY_EXTENSION_RULE@

EXTRA_DIST = $(extensionsini_in_files)
CLEANFILES = $(extensionsini_DATA)
DISTCLEANFILES = $(extensionsini_DATA)
