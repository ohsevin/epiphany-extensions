/*
 *  Copyright (C) 2003  Christian Persch
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

#include <epiphany/ephy-extension.h>

static void ephy_tabs_menu_extension_class_init	(EphyTabsMenuExtensionClass *class);
static void ephy_tabs_menu_extension_iface_init (EphyExtensionIface *iface);
static void ephy_tabs_menu_extension_init	(EphyTabsMenuExtension *menu);

GObjectClass *tabs_menu_extension_parent_class = NULL;

GType tabs_menu_extension_type = 0;

GType
ephy_tabs_menu_extension_get_type (void)
{
	return tabs_menu_extension_type;
}

GType
ephy_tabs_menu_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyTabsMenuExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tabs_menu_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTab),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_tabs_menu_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tabs_menu_extension_iface_init,
		NULL,
		NULL
	};

	tabs_menu_extension_type =
		g_type_module_register_type (module,
					     G_TYPE_OBJECT,
					     "EphyTabsMenuExtension",
					     &our_info, 0);

	g_type_module_add_interface (module,
				     tabs_menu_extension_type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return tabs_menu_extension_type;
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	EphyTabMoveMenu *menu;

	menu = ephy_tab_move_menu_new (window);
	g_object_set_data_full (G_OBJECT (window), "ephy-tab-move-menu", menu,
				(GDestroyNotify) g_object_unref);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	/* nothing to do */
}

static void
ephy_tabs_menu_extension_init (EphyTabsMenuExtension *extension)
{
	LOG ("EphyTabsMenuExtension initialising")
}

static void
ephy_tabs_menu_extension_finalize (GObject *object)
{
	EphyTabsMenuExtension *extension = EPHY_TABS_MENU_EXTENSION (object);

	LOG ("EphyTabsMenuExtension finalising")

	G_OBJECT_CLASS (tabs_menu_extension_parent_class)->finalize (object);
}

static void
ephy_tabs_menu_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tabs_menu_extension_class_init (EphyTabsMenuExtensionClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	tabs_menu_extension_parent_class  = (GObjectClass *) g_type_class_peek_parent (class);

	object_class->finalize = ephy_tabs_menu_extension_finalize;
}
