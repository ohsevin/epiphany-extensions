SUBDIRS = icons

gsettings_files = org.gnome.epiphanyextensions.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_files:.xml.in=.xml)

@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

EXTRA_DIST = \
	     $(gsettings_files)

DISTCLEANFILES = \
		 $(gsettings_SCHEMAS)
