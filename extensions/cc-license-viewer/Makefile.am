extensionsdir = $(EXTENSIONS_DIR)
extensions_PYTHON = \
	cc-license-viewer.py

extensionsinidir = $(EXTENSIONS_DIR)
extensionsini_in_files = \
	cc-license-viewer.ephy-extension.in	
extensionsini_DATA = $(extensionsini_in_files:.ephy-extension.in=.ephy-extension)

@EPIPHANY_EXTENSION_RULE@

EXTRA_DIST = \
	$(extensionsini_in_files)

CLEANFILES = $(extensionsini_DATA)
DISTCLEANFILES = $(extensionsini_DATA)
