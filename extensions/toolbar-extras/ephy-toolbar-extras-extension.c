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

#include "ephy-toolbar-extras-extension.h"
#include "ephy-multi-smart-action.h"
#include "ephy-debug.h"

#include "egg-editable-toolbar.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gmodule.h>
#include <glib/gi18n-lib.h>

#define EPHY_TOOLBAR_EXTRAS_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TOOLBAR_EXTRAS_EXTENSION, EphyToolbarExtrasExtensionPrivate))

struct EphyToolbarExtrasExtensionPrivate
{
};

enum
{
	PROP_0
};

static GObjectClass *ephy_toolbar_extras_parent_class = NULL;

static void ephy_toolbar_extras_extension_class_init	(EphyToolbarExtrasExtensionClass *klass);
static void ephy_toolbar_extras_extension_iface_init	(EphyExtensionClass *iface);
static void ephy_toolbar_extras_extension_init		(EphyToolbarExtrasExtension *extension);

static GType ephy_toolbar_extras_type = 0;

GType
ephy_toolbar_extras_extension_get_type (void)
{
	return ephy_toolbar_extras_type;
}

GType
ephy_toolbar_extras_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyToolbarExtrasExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_toolbar_extras_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyToolbarExtrasExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_toolbar_extras_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_toolbar_extras_extension_iface_init,
		NULL,
		NULL
	};

	ephy_toolbar_extras_type =
		g_type_module_register_type (module,
					     G_TYPE_OBJECT,
					     "EphyToolbarExtrasExtension",
					     &our_info, 0);

	g_type_module_add_interface (module,
				     ephy_toolbar_extras_type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return ephy_toolbar_extras_type;
}

static void
ephy_toolbar_extras_extension_init (EphyToolbarExtrasExtension *extension)
{
	extension->priv = EPHY_TOOLBAR_EXTRAS_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyToolbarExtrasExtension initialising")
}

static void
ephy_toolbar_extras_extension_finalize (GObject *object)
{
/*
	EphyToolbarExtrasExtension *extension = EPHY_TOOLBAR_EXTRAS_EXTENSION (object);
*/
	LOG ("EphyToolbarExtrasExtension finalising")

	G_OBJECT_CLASS (ephy_toolbar_extras_parent_class)->finalize (object);
}

static int
find_name (GtkActionGroup *action_group, const char *name)
{
	return strcmp (gtk_action_group_get_name (action_group), name);
}

static GtkActionGroup *
find_action_group (EphyWindow *window)
{
	GtkUIManager *manager;
	GList *list, *element;

	manager = GTK_UI_MANAGER (window->ui_merge);
	list = gtk_ui_manager_get_action_groups (manager);
	element = g_list_find_custom (list, "SpecialToolbarActions", (GCompareFunc) find_name);
	g_return_if_fail (element != NULL);
	g_return_if_fail (element->data != NULL);

	return GTK_ACTION_GROUP (element->data);
}

static void
action_request_cb (EggEditableToolbar *etoolbar,
                   char *action_name,
                   EphyToolbarExtrasExtension *extension)
{
	EphyWindow *window;
	GtkAction *action = NULL;

	LOG ("action_request_cb for %s", action_name)

	g_object_get (G_OBJECT (etoolbar), "window", &window, NULL);

	if (strcmp (action_name, "MultiSmartSearch") == 0)
	{
		action = g_object_new (EPHY_TYPE_MULTI_SMART_ACTION,
				       "name", "MultiSmartSearch",
				       "label", _("Smart Search"),
				       "window", window,
				       NULL);					
	}

	if (action != NULL)
	{
		GtkActionGroup *action_group;

		action_group = find_action_group (window);
		g_return_if_fail (action_group != NULL);

		gtk_action_group_add_action (action_group, action);
		g_object_unref (action);
	}

	g_object_unref (window);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyToolbarExtrasExtension *extension = EPHY_TOOLBAR_EXTRAS_EXTENSION (ext);
	GtkWidget *toolbar;

	LOG ("EphyToolbarExtrasExtension attach_window")

	toolbar = ephy_window_get_toolbar (window);
	g_return_if_fail (toolbar != NULL);

	g_signal_connect (toolbar, "action_request",
			  G_CALLBACK (action_request_cb),
			  extension);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
//	EphyToolbarExtrasExtension *extension = EPHY_TOOLBAR_EXTRAS_EXTENSION (ext);
	LOG ("EphyToolbarExtrasExtension detach_window")
}

static void
ephy_toolbar_extras_extension_set_property (GObject *object,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *pspec)
{
/*
	EphyToolbarExtrasExtension *extension = EPHY_TOOLBAR_EXTRAS_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_toolbar_extras_extension_get_property (GObject *object,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *pspec)
{
/*
	EphyToolbarExtrasExtension *extension = EPHY_TOOLBAR_EXTRAS_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_toolbar_extras_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_toolbar_extras_extension_class_init (EphyToolbarExtrasExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	ephy_toolbar_extras_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_toolbar_extras_extension_finalize;
	object_class->get_property = ephy_toolbar_extras_extension_get_property;
	object_class->set_property = ephy_toolbar_extras_extension_set_property;

	g_type_class_add_private (object_class, sizeof (EphyToolbarExtrasExtensionPrivate));
}
