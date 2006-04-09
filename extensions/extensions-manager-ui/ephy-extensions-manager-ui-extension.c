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

#include "config.h"

#include "ephy-extensions-manager-ui-extension.h"
#include "extensions-manager-ui.h"

#include <epiphany/ephy-extension.h>

#include "ephy-gui.h"
#include "ephy-debug.h"

#include <gmodule.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <glib/gi18n-lib.h>

#define WINDOW_DATA_KEY "EEMUIEWindowData"

#define EPHY_EXTENSIONS_MANAGER_UI_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_EXTENSIONS_MANAGER_UI_EXTENSION, EphyExtensionsManagerUIExtensionPrivate))

struct EphyExtensionsManagerUIExtensionPrivate
{
	ExtensionsManagerUI *dialog;
};

static void ephy_extensions_manager_ui_extension_class_init	(EphyExtensionsManagerUIExtensionClass *klass);
static void ephy_extensions_manager_ui_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_extensions_manager_ui_extension_init		(EphyExtensionsManagerUIExtension *extension);

typedef struct
{
	EphyExtensionsManagerUIExtension *extension;
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

static void display_cb (GtkAction *action,
			EphyWindow *window);
static void help_cb    (GtkAction *action,
			EphyWindow *window);

static const GtkActionEntry action_entries [] =
{
	{ "ExtensionsManagerUI",
	  NULL,
	  N_("_Extensions..."),
	  NULL,
	  N_("Load and unload extensions"),
	  G_CALLBACK (display_cb) },
	{ "ExtensionsManagerHelp",
	  NULL,
	  N_("_Extensions"),
	  NULL,
	  N_("Help about extensions"),
	  G_CALLBACK (help_cb)
	}
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_extensions_manager_ui_extension_get_type (void)
{
	return type;
}

GType
ephy_extensions_manager_ui_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyExtensionsManagerUIExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_extensions_manager_ui_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyExtensionsManagerUIExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_extensions_manager_ui_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_extensions_manager_ui_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyExtensionsManagerUIExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	return type;
}

static void
ephy_extensions_manager_ui_extension_init (EphyExtensionsManagerUIExtension *extension)
{
	LOG ("EphyExtensionsManagerUIExtension initialising");

	extension->priv = EPHY_EXTENSIONS_MANAGER_UI_EXTENSION_GET_PRIVATE (extension);

	extension->priv->dialog = NULL;
}

static void
ephy_extensions_manager_ui_extension_finalize (GObject *object)
{
	EphyExtensionsManagerUIExtension *extension = EPHY_EXTENSIONS_MANAGER_UI_EXTENSION (object);

	LOG ("EphyExtensionsManagerUIExtension finalising");

	if (extension->priv->dialog != NULL)
	{
		g_object_unref (extension->priv->dialog);
	}

	parent_class->finalize (object);
}

static void
display_cb (GtkAction *action,
	    EphyWindow *window)
{
	EphyExtensionsManagerUIExtensionPrivate *priv;
	WindowData *data;

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	priv = data->extension->priv;

	if (priv->dialog == NULL)
	{
		ExtensionsManagerUI **dialog;

		priv->dialog = extensions_manager_ui_new ();
		dialog = &priv->dialog;
		g_object_add_weak_pointer (G_OBJECT (priv->dialog),
					   (gpointer *) dialog);
	}

	ephy_dialog_show (EPHY_DIALOG (priv->dialog));
}

static void
help_cb (GtkAction *action,
	 EphyWindow *window)
{
	ephy_gui_help_with_doc_id (GTK_WINDOW (window),
				   "epiphany-extensions",
				   "epiphany-extensions",
				   "");
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyExtensionsManagerUIExtension *extension = EPHY_EXTENSIONS_MANAGER_UI_EXTENSION (ext);
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint ui_id;
	WindowData *data;

	data = g_new (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data->extension = extension;

	data->action_group = action_group = gtk_action_group_new ("EEMUIEActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (action_group);

	data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) g_free);

	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ExtensionsManagerUISep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ExtensionsManagerUI", "ExtensionsManagerUI",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ExtensionsManagerUISep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);

	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/HelpMenu/HelpContentsMenu",
			       "ExtensionsManagerHelp", "ExtensionsManagerHelp",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_assert (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
ephy_extensions_manager_ui_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_extensions_manager_ui_extension_class_init (EphyExtensionsManagerUIExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_extensions_manager_ui_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyExtensionsManagerUIExtensionPrivate));
}
