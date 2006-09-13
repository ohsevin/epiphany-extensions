/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
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

#include "ephy-permissions-extension.h"
#include "ephy-permissions-dialog.h"

#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-extension.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#define EPHY_PERMISSIONS_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_PERMISSIONS_EXTENSION, EphyPermissionsExtensionPrivate))

struct _EphyPermissionsExtensionPrivate
{
	GtkWidget *dialog;
};

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

#define WINDOW_DATA_KEY "EphyPermissionsExtensionWindowData"

static void ephy_permissions_extension_class_init	 (EphyPermissionsExtensionClass *klass);
static void ephy_permissions_extension_iface_init	 (EphyExtensionIface *iface);
static void ephy_permissions_extension_init		 (EphyPermissionsExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_permissions_extension_get_type (void)
{
	return type;
}

GType
ephy_permissions_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyPermissionsExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_permissions_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPermissionsExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_permissions_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_permissions_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPermissionsExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	return type;
}

static void
ephy_permissions_extension_init (EphyPermissionsExtension *extension)
{
	extension->priv = EPHY_PERMISSIONS_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyPermissionsExtension initialising");
}

static void
ephy_permissions_extension_finalize (GObject *object)
{
	EphyPermissionsExtension *extension = EPHY_PERMISSIONS_EXTENSION (object);
	EphyPermissionsExtensionPrivate *priv = extension->priv;

	LOG ("EphyPermissionsExtension finalising");

	if (priv->dialog != NULL)
	{
		GtkWidget **dialog = &priv->dialog;

		g_object_remove_weak_pointer (G_OBJECT (priv->dialog),
					      (gpointer *) dialog);
		gtk_widget_destroy (priv->dialog);
		priv->dialog = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
manage_permissions_cb (GtkAction *action,
		       EphyPermissionsExtension *extension)
{
	GtkWidget **dialog;
	EphyPermissionsExtensionPrivate *priv = extension->priv;

	if (priv->dialog != NULL)
	{
		gtk_window_present (GTK_WINDOW (priv->dialog));
		return;
	}  

	priv->dialog = ephy_permissions_dialog_new ();
	dialog = &priv->dialog;
	gtk_window_present (GTK_WINDOW (priv->dialog));

	g_object_add_weak_pointer (G_OBJECT (priv->dialog),
				   (gpointer *) dialog);
}

static const GtkActionEntry action_entries [] =
{
	{ "EditPermissionsManager",
	  NULL /* stock icon */,
	  N_("Site _Permissions"),
	  NULL, /* shortcut key */
	  N_("Manage web site permissions"),
	  G_CALLBACK (manage_permissions_cb) },
};

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
	WindowData *win_data;

	LOG ("EphyPermissionsExtension attach_window");

	/* add UI */
	win_data = g_new (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	win_data->action_group = action_group = gtk_action_group_new
		("PermissionsExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), ext);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (win_data->action_group);

	win_data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/EditMenu/EditPersonalDataMenu",
			       "EditPermissionsManagerItem",
			       "EditPermissionsManager",
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, win_data,
				(GDestroyNotify) g_free);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *win_data;

	LOG ("EphyPermissionsExtension detach_window");

	/* remove UI */
	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	win_data = (WindowData *) g_object_get_data (G_OBJECT (window),
						     WINDOW_DATA_KEY);
	g_return_if_fail (win_data != NULL);

	gtk_ui_manager_remove_ui (manager, win_data->ui_id);
	gtk_ui_manager_remove_action_group (manager, win_data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
ephy_permissions_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_permissions_extension_class_init (EphyPermissionsExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_permissions_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyPermissionsExtensionPrivate));
}
