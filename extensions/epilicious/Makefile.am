epiliciousdir = $(EXTENSIONS_DIR)
epiliciouslibdir = $(EXTENSIONS_DIR)/libepilicious
nodist_epilicious_PYTHON = \
	epilicious.py

epiliciouslib_PYTHON = \
	BaseStore.py \
	DeliciousStore.py \
	EpiphanyStore.py \
	__init__.py \
	backend.py \
	config.py \
	magnolia.py \
	progress.py

gladedir = $(pkgdatadir)/glade
glade_DATA = \
	epilicious-config.glade \
	epilicious-progress.glade

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = epilicious.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

extensioninidir = $(EXTENSIONS_DIR)
extensionini_in_files = epilicious.ephy-extension.in
extensionini_DATA = $(extensionini_in_files:.ephy-extension.in=.ephy-extension)

@EPIPHANY_EXTENSION_RULE@

epilicious.py : epilicious.py.in Makefile
	sed -e "s|%EXTENSION_DIR%|$(EXTENSIONS_DIR)|" \
		-e "s|%GLADE_DIR%|$(gladedir)|" \
		-e "s|%LOCALE_DIR%|$(localedir)|" \
		-e "s|%GETTEXT_PACKAGE%|$(GETTEXT_PACKAGE)|" \
		$< > $@

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $^ ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		$(GCONFTOOL) --makefile-install-rule $$p >&1 > /dev/null; \
	done \
	fi
endif

CLEANFILES = \
	$(schema_DATA)		\
	$(extensionini_DATA)	\
	epilicious.py

DISTCLEANFILES = $(CLEANFILES)

EXTRA_DIST = \
	$(glade_DATA)			\
	$(schema_in_files)		\
	$(extensionini_in_files) 	\
	epilicious.py.in
