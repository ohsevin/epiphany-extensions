/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2003 Adam Hooper
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

#include "ephy-popup-blocker-extension.h"
#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-permission-manager.h>
#include <epiphany/ephy-embed-single.h>
#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-session.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "eel-gconf-extensions.h"

#include <gtk/gtktoggleaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstatusbar.h>

#include <glib/gi18n-lib.h>

#define EPHY_POPUP_BLOCKER_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_EXTENSION, EphyPopupBlockerExtensionPrivate))

struct EphyPopupBlockerExtensionPrivate
{
	guint gconf_cnxn_id;
};

static void action_activate_cb (GtkAction *action, EphyWindow *window);

static void clear_popup_permissions (void);

static GtkToggleActionEntry action_entries [] =
{
	{ "PopupBlocker", NULL, N_("Block _Popups from this Site"),
	  NULL, /* shortcut key */
	  N_("Block or unblock popup windows"),
	  NULL, TRUE }
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static void ephy_popup_blocker_extension_class_init	(EphyPopupBlockerExtensionClass *klass);
static void ephy_popup_blocker_extension_iface_init	(EphyExtensionClass *iface);
static void ephy_popup_blocker_extension_init		(EphyPopupBlockerExtension *extension);

static GObjectClass *parent_class = NULL;

GType type = 0;

GType
ephy_popup_blocker_extension_get_type (void)
{
	return type;
}

GType
ephy_popup_blocker_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyPopupBlockerExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_popup_blocker_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPopupBlockerExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_popup_blocker_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_popup_blocker_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPopupBlockerExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	return type;
}

static void
update_action (EphyWindow *window, const char *address)
{
	EphyPermissionManager *permission_manager;
	GtkAction *action;
	EphyPermission response;
	gboolean allow;

	LOG ("update_action: window %p, address %s", window, address)

	permission_manager = EPHY_PERMISSION_MANAGER (ephy_embed_shell_get_embed_single (EPHY_EMBED_SHELL (ephy_shell)));

	response = ephy_permission_manager_test (permission_manager, address,
						 EPT_POPUP);

	switch (response)
	{
		case EPHY_PERMISSION_ALLOWED:
			allow = TRUE;
			break;
		case EPHY_PERMISSION_DENIED:
			allow = FALSE;
			break;
		case EPHY_PERMISSION_DEFAULT:
		default:
			allow = get_gconf_allow_popups_pref ();
			break;
	}

	action = gtk_ui_manager_get_action (GTK_UI_MANAGER (window->ui_merge),
					    "/menubar/ViewMenu/PopupBlocker");

	g_signal_handlers_block_by_func (G_OBJECT (action),
					 G_CALLBACK (action_activate_cb),
					 window);

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
				      allow == FALSE);

	g_signal_handlers_unblock_by_func (G_OBJECT (action),
					   G_CALLBACK (action_activate_cb),
					   window);

	if (allow == TRUE)
	{
		LOG ("Popups are allowed from %s", address);
	}
	else
	{
		LOG ("Popups are blocked from %s", address);
	}
}

static void
update_action_without_address (EphyWindow *window)
{
	/*
	 * FIXME: The address used to update_action could be inaccurate.
	 * We can't use ephy_tab_get_embed() because of bug #119047
	 */

	EphyTab *tab;
	const char *address;

	tab = ephy_window_get_active_tab (window);

	if (tab == NULL) return; /* Happens on startup */

	g_return_if_fail (EPHY_IS_TAB (tab));

	address = ephy_tab_get_location (tab);
	g_return_if_fail (address != NULL);

	update_action (window, address);
}

static void
update_all_actions_foreach_func (gpointer data,
				 gpointer user_data)
{
	update_action_without_address (EPHY_WINDOW (data));
}

static void
update_all_actions (GConfClient *client,
		    guint cnxn_id,
		    GConfEntry *entry,
		    gpointer user_data)
{
	EphySession *session;
	GList *windows;
	EphyWindow *window;

	LOG ("update_all_actions")

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	windows = ephy_session_get_windows (session);

	g_list_foreach (windows, (GFunc) update_all_actions_foreach_func, NULL);
}

static void
add_popup_gconf_notification (EphyPopupBlockerExtension *extension)
{
	guint cnxn_id;

	cnxn_id = eel_gconf_notification_add ("/apps/epiphany/web/allow_popups",
					      (GConfClientNotifyFunc) update_all_actions,
					      NULL);

	extension->priv->gconf_cnxn_id = cnxn_id;
}

static void
remove_popup_gconf_notification (EphyPopupBlockerExtension *extension)
{
	if (extension->priv->gconf_cnxn_id != 0)
	{
		eel_gconf_notification_remove (extension->priv->gconf_cnxn_id);
		extension->priv->gconf_cnxn_id = 0;
	}
}

static void
ephy_popup_blocker_extension_init (EphyPopupBlockerExtension *extension)
{
	LOG ("EphyPopupBlockerExtension initialising")

	extension->priv = EPHY_POPUP_BLOCKER_EXTENSION_GET_PRIVATE (extension);

	add_popup_gconf_notification (extension);
}

static void
ephy_popup_blocker_extension_finalize (GObject *object)
{
	EphyPopupBlockerExtension *extension = EPHY_POPUP_BLOCKER_EXTENSION (object);

	LOG ("EphyPopupBlockerExtension finalizing")

	remove_popup_gconf_notification (extension);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

gboolean
get_gconf_allow_popups_pref (void)
{
	return eel_gconf_get_boolean ("/apps/epiphany/web/allow_popups");
}

static void
clear_one_permission_info (gpointer data,
			   gpointer user_data)
{
	EphyPermissionInfo *info = (EphyPermissionInfo *) data;
	EphyPermissionManager *manager = (EphyPermissionManager *) user_data;

	ephy_permission_manager_remove (manager, info->host, EPT_POPUP);

	ephy_permission_info_free (info);
}

static void
clear_popup_permissions (void)
{
	EphyPermissionManager *manager;
	GList *list;

	manager = EPHY_PERMISSION_MANAGER (ephy_embed_shell_get_embed_single (EPHY_EMBED_SHELL (ephy_shell)));

	list = ephy_permission_manager_list (manager, EPT_POPUP);

	g_list_foreach (list, (GFunc) clear_one_permission_info, manager);

	g_list_free (list);
}

static void
increment_num_popups (EphyEmbed *embed)
{
	guint n;

	n = GPOINTER_TO_UINT(g_object_get_data (G_OBJECT (embed), "popup-blocker-num-blocked"));

	g_object_set_data (G_OBJECT (embed), "popup-blocker-num-blocked",
			   GUINT_TO_POINTER (++n));
}

static void
reset_num_popups (EphyEmbed *embed)
{
	g_object_set_data (G_OBJECT (embed), "popup-blocker-num-blocked",
			   GUINT_TO_POINTER (0));
}

static char *
get_popups_blocked_string (EphyEmbed *embed)
{
	guint n;
	char *str;

	n = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (embed), "popup-blocker-num-blocked"));

	if (n == 0) return NULL;

	str = g_strdup_printf (ngettext ("%d popup blocked",
					 "%d popups blocked", n), n);

	return str;
}

static void
update_statusbar (EphyWindow *window)
{
	GtkStatusbar *statusbar;
	guint context_id;
	char *statusbar_message;
	EphyEmbed *embed;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	statusbar = GTK_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (GTK_IS_STATUSBAR (statusbar));

	context_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (window), "popup-blocker-statusbar-context-id"));
	g_return_if_fail (context_id != 0);

	gtk_statusbar_pop (statusbar, context_id);

	statusbar_message = get_popups_blocked_string (embed);

	if (statusbar_message == NULL) return;

	gtk_statusbar_push (statusbar, context_id, statusbar_message);

	g_free (statusbar_message);
}

static void
location_cb (EphyEmbed *embed,
	     char *address,
	     EphyTab *tab)
{
	EphyWindow *window;

	window = ephy_tab_get_window (tab);
	g_return_if_fail (EPHY_IS_WINDOW (window));

	update_action (window, address);

	reset_num_popups (embed);

	update_statusbar (window);
}

static void
popup_blocked_cb (EphyEmbed *embed,
		  EphyTab *tab)
{
	EphyWindow *window;

	increment_num_popups (embed);

	window = ephy_tab_get_window (tab);
	g_return_if_fail (EPHY_IS_WINDOW (window));

	update_statusbar (window);
}

static void
action_activate_cb (GtkAction *action,
		    EphyWindow *window)
{
	const char *address;
	EphyTab *tab;
	EphyPermissionManager *permission_manager;
	EphyPermission allow;

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	address = ephy_tab_get_location (tab);
	g_return_if_fail (address != NULL);

	permission_manager = EPHY_PERMISSION_MANAGER (ephy_embed_shell_get_embed_single (EPHY_EMBED_SHELL (ephy_shell)));

	if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) == TRUE)
	{
		allow = EPHY_PERMISSION_DENIED;
	}
	else
	{
		allow = EPHY_PERMISSION_ALLOWED;
	}

	LOG ("from now on, address '%s' will %s popups", address,
	     allow == EPHY_PERMISSION_ALLOWED ? "allow" : "block")

	ephy_permission_manager_add (permission_manager, address,
				     EPT_POPUP, allow); 
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyEmbed *embed,
	      EphyPopupBlockerExtension *extension)
{
	EphyTab *tab;

	tab = ephy_tab_for_embed (embed);

	g_signal_connect (embed, "ge_location",
			  G_CALLBACK (location_cb), tab);

	g_signal_connect (embed, "ge_popup_blocked",
			  G_CALLBACK (popup_blocked_cb), tab);
}

static void
tab_removed_cb (GtkWidget *notebook,
		EphyEmbed *embed,
		EphyPopupBlockerExtension *extension)
{
	EphyTab *tab;

	tab = ephy_tab_for_embed (embed);

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (location_cb), tab);

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (popup_blocked_cb), tab);
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyPopupBlockerExtension *extension)
{
	GtkWidget *toplevel;
	EphyWindow *window;
	const char *address;

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (notebook));
	g_return_if_fail (EPHY_IS_WINDOW (toplevel));
	if (GTK_WIDGET_REALIZED (toplevel) == FALSE) return; /* happens on startup */

	window = EPHY_WINDOW (toplevel);

	update_action_without_address (window);

	update_statusbar (window);
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkActionGroup *action_group;
	GtkAction *action;
	GtkWidget *notebook;
	GtkStatusbar *statusbar;
	guint context_id;
	GtkUIManager *manager;
	guint merge_id;

	/* Get a context_id for the statusbar */
	statusbar = GTK_STATUSBAR (ephy_window_get_statusbar (window));

	context_id = gtk_statusbar_get_context_id (statusbar,
						   "Popup blocker extension");

	g_object_set_data (G_OBJECT (window),
			   "popup-blocker-statusbar-context-id",
			   GUINT_TO_POINTER (context_id));

	/* Merge the UI */
	manager = GTK_UI_MANAGER (window->ui_merge);

	action_group = gtk_action_group_new ("EphyPopupBlockerExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_toggle_actions (action_group, action_entries,
					     n_action_entries, window);


	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	g_object_unref (action_group);

	merge_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ViewMenu",
			       "PopupBlockerSep", NULL,
			       GTK_UI_MANAGER_SEPARATOR, TRUE);
	gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ViewMenu",
			       "PopupBlocker", "PopupBlocker",
			       GTK_UI_MANAGER_MENUITEM, TRUE);

	/*
	 * Connect the GtkToggleAction's activate signal
	 * We can't use the toggled callback because it'll get fired when we
	 * want to change the action's value through code.
	 */

	action = gtk_ui_manager_get_action (GTK_UI_MANAGER (window->ui_merge),
					    "/menubar/ViewMenu/PopupBlocker");

	g_signal_connect_after (action, "activate",
				G_CALLBACK (action_activate_cb), window);

	/* Notebook signals */

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_added_cb), extension);
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_removed_cb), extension);
	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), extension);
}

static void
impl_detach_window (EphyExtension *extension,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_added_cb), extension);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_removed_cb), extension);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (switch_page_cb), extension);
}

static void
ephy_popup_blocker_extension_iface_init (EphyExtensionClass *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_popup_blocker_extension_class_init (EphyPopupBlockerExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_popup_blocker_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyPopupBlockerExtensionPrivate));
}
