/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
 *  Copyright © 2004 Adam Hooper
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "config.h"

#include "ephy-error-viewer-extension.h"
#include "error-viewer.h"
#include "link-checker.h"
#include "sgml-validator.h"
#include "ephy-debug.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

G_MODULE_EXPORT GType register_module (GTypeModule *module);

G_MODULE_EXPORT GType
register_module (GTypeModule *module)
{
	LOG ("Registering EphyErrorViewerExtension");

#ifdef ENABLE_NLS
	/* Initialize the i18n stuff */
	bindtextdomain (GETTEXT_PACKAGE, EPHY_EXTENSIONS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */

	link_checker_register_type (module);

#ifdef HAVE_OPENSP
	sgml_validator_register_type (module);
#endif /* HAVE_OPENSP */

	error_viewer_register_type (module);

	return ephy_error_viewer_extension_register_type (module);
}
