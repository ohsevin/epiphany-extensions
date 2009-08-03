/*
 *  Copyright © 2009 Benjamin Otte
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
 */

#include "config.h"

#include "ephy-tabs-reloaded-extension.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

static void ephy_tabs_reloaded_extension_class_init (EphyTabsReloadedExtensionClass *class);
static void ephy_tabs_reloaded_extension_iface_init (EphyExtensionIface *iface);
static void ephy_tabs_reloaded_extension_init	 (EphyTabsReloadedExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_tabs_reloaded_extension_get_type (void)
{
	return type;
}

GType
ephy_tabs_reloaded_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyTabsReloadedExtensionClass),
		NULL,	/* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tabs_reloaded_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTabsReloadedExtension),
		0,	/* n_preallocs */
		(GInstanceInitFunc) ephy_tabs_reloaded_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tabs_reloaded_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabsReloadedExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	
	return type;
}

static void
impl_attach_window (EphyExtension *extension,
			EphyWindow *window)
{
        g_print ("attach window");
}

static void
impl_detach_window (EphyExtension *ext,
			EphyWindow *window)
{
        g_print ("detach window");
}

static void
ephy_tabs_reloaded_extension_init (EphyTabsReloadedExtension *extension)
{
	LOG ("EphyTabsReloadedExtension initialising");
}

static void
ephy_tabs_reloaded_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tabs_reloaded_extension_class_init (EphyTabsReloadedExtensionClass *class)
{
	parent_class = (GObjectClass *) g_type_class_peek_parent (class);
}
