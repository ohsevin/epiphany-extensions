/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-tabs-menu-extension.h"
#include "ephy-tab-move-menu.h"

#include "ephy-debug.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

G_MODULE_EXPORT GType register_module (GTypeModule *module);

G_MODULE_EXPORT GType
register_module (GTypeModule *module)
{
	GType extension_type;

	ephy_tab_move_menu_register_type (module);

	extension_type = ephy_tabs_menu_extension_register_type (module);

#ifdef ENABLE_NLS
       /* Initialise the i18n stuff */
        bindtextdomain (GETTEXT_PACKAGE, EPHY_PLUGINS_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");	
#endif /* ENABLE_NLS */

	return extension_type;
}
