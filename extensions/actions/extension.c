/* 
 * Copyright (C) 2005 Jean-Yves Lefort <jylefort@brutele.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include "ephy-actions-extension.h"
#include "ephy-actions-extension-editor-dialog.h"
#include "ephy-actions-extension-properties-dialog.h"

GType register_module (GTypeModule *module);

G_MODULE_EXPORT GType
register_module (GTypeModule *module)
{
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, EPHY_EXTENSIONS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */

	ephy_actions_extension_editor_dialog_register_type (module);
	ephy_actions_extension_properties_dialog_register_type (module);

	return ephy_actions_extension_register_type (module);
}
