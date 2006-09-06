/*
 *  Copyright (C) 2005 Mathias Hasselmann
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

#include "ephy-java-console-extension.h"
#include "java-console.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#define ACTION_NAME_SHOW_CONSOLE	"EphyJavaConsoleExtensionShowConsole"
#define WINDOW_DATA_KEY			"EphyJavaConsoleExtensionWindowData"

typedef struct
{
	GtkActionGroup *action_group;
	guint merge_id;
} WindowData;

static GType type = 0;

static const GtkActionEntry action_entries [] =
{
	{ ACTION_NAME_SHOW_CONSOLE,
		NULL,
		N_("_Java Console"),
		NULL, /* shortcut key */
		N_("Show Java Console"),
		G_CALLBACK (java_console_show) }
};

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	LOG ("EphyJavaConsoleExtension attach_window");

	if (java_console_is_available ())
	{
		GtkUIManager *manager;
		WindowData *data;

		data = g_new0 (WindowData, 1);

		g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
					(GDestroyNotify) g_free);

		data->action_group = gtk_action_group_new ("EphyJCExtActions");
		gtk_action_group_set_translation_domain (data->action_group, GETTEXT_PACKAGE);
		gtk_action_group_add_actions (data->action_group, action_entries,
				      	      G_N_ELEMENTS (action_entries), window);

		manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
		gtk_ui_manager_insert_action_group (manager, data->action_group, 0);
		g_object_unref (data->action_group);

		data->merge_id = gtk_ui_manager_new_merge_id (manager);

		gtk_ui_manager_add_ui (manager, data->merge_id, "/menubar/ToolsMenu",
				       ACTION_NAME_SHOW_CONSOLE, ACTION_NAME_SHOW_CONSOLE,
				       GTK_UI_MANAGER_MENUITEM, FALSE);
	}
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	WindowData *data;
	
	LOG ("EphyJavaConsoleExtension detach_window");

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

	if (data) 
	{
		GtkUIManager *manager;
	
		manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
		gtk_ui_manager_remove_ui (manager, data->merge_id);
		gtk_ui_manager_remove_action_group (manager, data->action_group);

		g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
	}
}

static void
ephy_java_console_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

GType
ephy_java_console_extension_get_type (void)
{
	return type;
}

GType
ephy_java_console_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyJavaConsoleExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		NULL, /* class_init */
		NULL,
		NULL, /* class_data */
		sizeof (EphyJavaConsoleExtension),
		0, /* n_preallocs */
		NULL /* instance_init */
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_java_console_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyJavaConsoleExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
