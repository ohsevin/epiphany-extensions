/*
 *  Copyright © 2006 Raphaël Slinckx
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

#include "ephy-auto-reload-extension.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>

#include <gmodule.h>

#define EPHY_AUTO_RELOAD_EXTENSION_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_AUTO_RELOAD_EXTENSION, EphyAutoReloadExtensionPrivate))

/* This is the time to wait before the first reload, the maximal reload rate and the time increment after each unchanged reload */
#define RELOAD_RATE 		3*60*1000
#define WINDOW_DATA_KEY		"EphyAutoReloadExtensionWindowData"
#define TIMEOUT_DATA_KEY 	"EphyAutoReloadExtensionTimeout"

typedef struct
{
	GtkActionGroup *action_group;
	GtkToggleAction *action;
	guint ui_id;
	GtkAction *popup;
} WindowData;

typedef struct
{
	guint source;
	guint timeout;
} TimeoutData;

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
ephy_auto_reload_extension_init (EphyAutoReloadExtension *extension)
{
}

static void
ephy_auto_reload_extension_finalize (GObject *object)
{
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_auto_reload_activate_cb (GtkToggleAction *action, EphyWindow *window);

static gboolean
ephy_auto_reload_timeout (EphyEmbed *embed);

static const GtkToggleActionEntry action_entries [] =
{
	{ "AutoReload",
	  GTK_STOCK_REFRESH,
	  N_("_Auto Reload"),
	  NULL,
	  N_("Reload the page periodically"),
	  G_CALLBACK (ephy_auto_reload_activate_cb),
	  FALSE
	},
};

static void
ephy_auto_reload_remove_timeout (TimeoutData *data)
{
	g_source_remove (data->source);
	g_free (data);
}

static void
ephy_auto_reload_create (EphyEmbed *embed, guint new_timeout)
{
	/* We have a new timeout,discard the old one */
	g_object_set_data (G_OBJECT (embed), TIMEOUT_DATA_KEY, NULL);

	/* Check the new_timeout sanity */
	new_timeout = (new_timeout < RELOAD_RATE) ? RELOAD_RATE : new_timeout;

	LOG ("AutoReload reloading embed: %s in %d msecs", ephy_web_view_get_title (ephy_embed_get_web_view (embed)), new_timeout);

	/* Create the new one */
	TimeoutData *timeout = g_new (TimeoutData, 1);

	g_object_set_data_full (G_OBJECT (embed), TIMEOUT_DATA_KEY, timeout, (GDestroyNotify) ephy_auto_reload_remove_timeout);
	timeout->source = g_timeout_add (new_timeout, (GSourceFunc) ephy_auto_reload_timeout, embed);
	timeout->timeout = new_timeout;
}

static gboolean
ephy_auto_reload_timeout (EphyEmbed *embed)
{
	guint new_timeout;
	WebKitWebView *view;
	/* See below for these
	TimeoutData *timeout;
	guint old_timeout;
	*/

	g_return_val_if_fail (embed != NULL, FALSE);

	/* Reload the page */
	view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);
	LOG ("AutoReload tab: %s", ephy_web_view_get_title (EPHY_WEB_VIEW (view)));
	webkit_web_view_reload_bypass_cache (view);

	/* Retreive the old timeout value (if we want to do something relative to it
	timeout = (TimeoutData *) g_object_get_data (G_OBJECT (tab), TIMEOUT_DATA_KEY);
	old_timeout = timeout->timeout;
	*/
	new_timeout = RELOAD_RATE;

	ephy_auto_reload_create (embed, new_timeout);

	return FALSE;
}

static void
ephy_auto_reload_activate_cb (GtkToggleAction *action,
			      EphyWindow *window)
{
	EphyEmbed *embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));

	/* Invalidate the current timeout */
	g_object_set_data (G_OBJECT (embed), TIMEOUT_DATA_KEY, NULL);

	if (gtk_toggle_action_get_active (action))
	{
		LOG("Activated action, reloading in %d msecs", RELOAD_RATE);
		ephy_auto_reload_create (embed, RELOAD_RATE);
	}
}

static void
update_auto_reload_menu_cb (GtkAction *action,
			    EphyWindow *window)
{
	EphyEmbed *embed;
	WindowData *data;
	TimeoutData *timeout;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	timeout = (TimeoutData *) g_object_get_data (G_OBJECT (embed), TIMEOUT_DATA_KEY);

	LOG ("Displaying menu item ? %s", (timeout != NULL) ? "yes" : "no");
	g_signal_handlers_block_by_func (data->action, update_auto_reload_menu_cb, window);

	gtk_toggle_action_set_active (data->action, timeout != NULL);

	g_signal_handlers_block_by_func (data->action, update_auto_reload_menu_cb, window);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	/* The window data */
	data = g_new (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data, (GDestroyNotify) g_free);

	manager =  GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	data->action_group = gtk_action_group_new ("EphyAutoReloadExtensionActions");
	gtk_action_group_set_translation_domain (data->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_toggle_actions (data->action_group, action_entries, G_N_ELEMENTS (action_entries), window);
	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);
	g_object_unref (data->action_group);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);
	gtk_ui_manager_add_ui (manager, data->ui_id, "/EphyNotebookPopup",
			       "AutoReload", "AutoReload",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	data->action = GTK_TOGGLE_ACTION (gtk_action_group_get_action (data->action_group, "AutoReload"));

	data->popup = gtk_ui_manager_get_action (manager, "/EphyNotebookPopup");
	g_return_if_fail (data->popup != NULL);
	g_signal_connect (data->popup, "activate",
			  G_CALLBACK (update_auto_reload_menu_cb), window);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	WindowData *data;
	GtkUIManager *manager;

	manager =  GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	/* Remove the menu item */
	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	/* Remove callbacks */
	g_signal_handlers_disconnect_by_func
		(data->popup, G_CALLBACK (update_auto_reload_menu_cb), window);

	/* Destroy data */
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
}

static void
ephy_auto_reload_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_auto_reload_extension_class_init (EphyAutoReloadExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_auto_reload_extension_finalize;
}

GType
ephy_auto_reload_extension_get_type (void)
{
	return type;
}

GType
ephy_auto_reload_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyAutoReloadExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_auto_reload_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAutoReloadExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_auto_reload_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_auto_reload_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAutoReloadExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
