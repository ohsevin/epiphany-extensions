/*
 *  Copyright (C) 2004 Adam Hooper
 *
 *  Ported from Galeon:
 *      Copyright (C) 2002 Ricardo Fernández Pascual
 *      Copyright (C) 2004 Crispin Flowerday
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

#include "ephy-select-stylesheet-extension.h"
#include "ephy-css-menu.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>

#include <glib/gi18n-lib.h>

#define WINDOW_DATA_KEY "EphySelectStylesheetExtensionWindowData"
#define CSS_MENU_KEY "EphySelectStyelesheetExtensionMenu"

#define EPHY_SELECT_STYLESHEET_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SELECT_STYLESHEET_EXTENSION, EphySelectStylesheetExtensionPrivate))

struct _EphySelectStylesheetExtensionPrivate
{
};

static void ephy_select_stylesheet_extension_class_init	(EphySelectStylesheetExtensionClass *klass);
static void ephy_select_stylesheet_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_select_stylesheet_extension_init	(EphySelectStylesheetExtension *extension);

static GObjectClass *parent_class = NULL;
static GType type = 0;

GType
ephy_select_stylesheet_extension_get_type (void)
{
	return type;
}

GType
ephy_select_stylesheet_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphySelectStylesheetExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_select_stylesheet_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySelectStylesheetExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_select_stylesheet_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_select_stylesheet_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphySelectStylesheetExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_select_stylesheet_extension_init (EphySelectStylesheetExtension *extension)
{
	LOG ("EphySelectStylesheetExtension initialising")

	/* extension->priv = EPHY_SELECT_STYLESHEET_EXTENSION_GET_PRIVATE (extension); */
}

static void
ephy_select_stylesheet_extension_finalize (GObject *object)
{
	EphySelectStylesheetExtension *extension = EPHY_SELECT_STYLESHEET_EXTENSION (object);

	LOG ("EphySelectStylesheetExtension finalizing")

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_select_stylesheet_extension_class_init (EphySelectStylesheetExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_select_stylesheet_extension_finalize;

/*	g_type_class_add_private (object_class, sizeof (EphySelectStylesheetExtensionPrivate)); */
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	LOG ("EphySelectStylesheetExtension attach_window")

	EphyCssMenu *menu;

	menu = ephy_css_menu_new (window);

	g_object_set_data_full (G_OBJECT (window), CSS_MENU_KEY, menu,
				(GDestroyNotify) g_object_unref);
}

static void
impl_detach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	g_object_set_data (G_OBJECT (window), CSS_MENU_KEY, NULL);
}

static void
ephy_select_stylesheet_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}
