/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *  Copyright (C) 2004 Justin Wake
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

#include "ephy-tab-groups-extension.h"
#include "ephy-tab-grouper.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>

static void ephy_tab_groups_extension_class_init (EphyTabGroupsExtensionClass *class);
static void ephy_tab_groups_extension_iface_init (EphyExtensionIface *iface);
static void ephy_tab_groups_extension_init	 (EphyTabGroupsExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_tab_groups_extension_get_type (void)
{
	return type;
}

GType
ephy_tab_groups_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyTabGroupsExtensionClass),
		NULL,	/* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tab_groups_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTab),
		0,	/* n_preallocs */
		(GInstanceInitFunc) ephy_tab_groups_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tab_groups_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabGroupsExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	
	return type;
}

#define DATA_KEY "ephy-tab-grouper"

static void
impl_attach_window (EphyExtension *extension,
			EphyWindow *window)
{
	EphyTabGrouper *grouper;

	grouper = ephy_tab_grouper_new (ephy_window_get_notebook (window));
	g_object_set_data_full (G_OBJECT (window), DATA_KEY, grouper,
				(GDestroyNotify) g_object_unref);
}

static void
impl_detach_window (EphyExtension *ext,
			EphyWindow *window)
{
	/* Kill the tab grouper for this window */
	g_object_set_data (G_OBJECT (window), DATA_KEY, NULL);
}

static void
ephy_tab_groups_extension_init (EphyTabGroupsExtension *extension)
{
	LOG ("EphyTabGroupsExtension initialising")
}

static void
ephy_tab_groups_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tab_groups_extension_class_init (EphyTabGroupsExtensionClass *class)
{
	parent_class = (GObjectClass *) g_type_class_peek_parent (class);
}
