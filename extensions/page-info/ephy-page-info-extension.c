/*
 *  Copyright (C) 2004 Adam Hooper
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

#include "ephy-page-info-extension.h"
#include "ephy-debug.h"
#include "page-info-dialog.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-notebook.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstock.h>

#include <glib/gi18n-lib.h>

#define WINDOW_DATA_KEY "EphyPageInfoExtensionWindowData"
#define MENU_PATH "/menubar/ToolsMenu"

#define EPHY_PAGE_INFO_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_PAGE_INFO_EXTENSION, EphyPageInfoExtensionPrivate))

struct _EphyPageInfoExtensionPrivate
{
};

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

static void ephy_page_info_extension_class_init	(EphyPageInfoExtensionClass *klass);
static void ephy_page_info_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_page_info_extension_init	(EphyPageInfoExtension *extension);

static void ephy_page_info_extension_display_cb (GtkAction *action,
						 EphyWindow *window);

static GtkActionEntry action_entries [] =
{
	{ "PageInfo",
	  NULL,
	  N_("_Page Info"),
	  NULL,
	  N_("Display page information in a dialog"),
	  G_CALLBACK (ephy_page_info_extension_display_cb) },
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static GObjectClass *parent_class = NULL;
static GType type = 0;

GType
ephy_page_info_extension_get_type (void)
{
	return type;
}

GType
ephy_page_info_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyPageInfoExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_page_info_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPageInfoExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_page_info_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_page_info_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPageInfoExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_page_info_extension_init (EphyPageInfoExtension *extension)
{
	LOG ("EphyPageInfoExtension initialising");

/*	extension->priv = EPHY_PAGE_INFO_EXTENSION_GET_PRIVATE (extension);*/
}

static void
ephy_page_info_extension_finalize (GObject *object)
{
//	EphyPageInfoExtension *extension = EPHY_PAGE_INFO_EXTENSION (object);

	LOG ("EphyPageInfoExtension finalizing");

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_page_info_extension_class_init (EphyPageInfoExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_page_info_extension_finalize;

/*	g_type_class_add_private (object_class, sizeof (EphyPageInfoExtensionPrivate)); */
}

static void
ephy_page_info_extension_display_cb (GtkAction *action,
				     EphyWindow *window)
{
	EphyEmbed *embed;
	PageInfoDialog *dialog;

	LOG ("Creating page info dialog");

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	dialog = page_info_dialog_new (window, embed);

	ephy_dialog_show (EPHY_DIALOG (dialog));
}

static void
update_action (EphyWindow *window,
	       EphyTab *tab)
{
	GtkAction *action;
	gboolean loading = TRUE;

	action = gtk_ui_manager_get_action (GTK_UI_MANAGER (ephy_window_get_ui_manager (window)),
					    MENU_PATH "/PageInfo");

	g_object_get (G_OBJECT (tab), "load-status", &loading, NULL);
	g_object_set (G_OBJECT (action), "sensitive", !loading, NULL);
}

static void
load_status_cb (EphyTab *tab,
		GParamSpec *pspec,
		EphyWindow *window)
{
	if (tab == ephy_window_get_active_tab (window))
	{
		update_action (window, tab);
	}
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyWindow *window)
{
	if (GTK_WIDGET_REALIZED (window) == FALSE) return; /* on startup */

	update_action (window, ephy_window_get_active_tab (window));
}

static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	g_signal_connect_after (tab, "notify::load-status",
				G_CALLBACK (load_status_cb), window);
}

static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	g_signal_handlers_disconnect_by_func
		(tab, G_CALLBACK (load_status_cb), window);
}

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_object_unref (data->action_group);
	g_free (data);
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkActionGroup *action_group;
	GtkUIManager *manager;
	guint ui_id;
	WindowData *data;
	GtkWidget *notebook;

	LOG ("EphyPageInfoExtension attach_window");

	data = g_new (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data->action_group = action_group =
		gtk_action_group_new ("EphyPageInfoExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      n_action_entries, window);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);

	data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "PageInfoSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "PageInfo", "PageInfo",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, MENU_PATH,
			       "PageInfoSep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), window);
}

static void
impl_detach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	GtkWidget *notebook;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (switch_page_cb), window);
}

static void
ephy_page_info_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}
