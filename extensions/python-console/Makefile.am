consoledir = $(EXTENSIONS_DIR)
console_PYTHON = \
	console.py

extensionsinidir = $(EXTENSIONS_DIR)
extensionsini_in_files = python-console.ephy-extension.in
extensionsini_DATA = $(extensionsini_in_files:.ephy-extension.in=.ephy-extension)

@EPIPHANY_EXTENSION_RULE@

EXTRA_DIST = $(extensionsini_in_files)
CLEANFILES = $(extensionsini_DATA)
DISTCLEANFILES = $(extensionsini_DATA)
