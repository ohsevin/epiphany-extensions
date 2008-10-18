/*
 *  Copyright © 2004 Jean-François Rameau
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

#include "smart-bookmarks-extension.h"
#include "mozilla-selection.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-embed-container.h>
#include <epiphany/ephy-command-manager.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-session.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-bookmarks.h>

#include "eel-gconf-extensions.h"
#include "ephy-debug.h"

#include <gmodule.h>
#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>

#include <string.h>

#define CONF_OPEN_IN_TAB  "/apps/epiphany/extensions/smart-bookmarks/dictionarysearch/open_in_tab"

#define SMART_BOOKMARKS_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_SMART_BOOKMARKS_EXTENSION, SmartBookmarksExtensionPrivate))

#define SMB_ACTION		"SmbExt%x"
#define SMB_ACTION_LENGTH	strlen (SMB_ACTION) + 14 + 1
#define LOOKUP_ACTION		"SmbExtLookup"
#define GDICT_ACTION		"SmbExtGDict"
#define NODE_ID_KEY		"EphyNodeId"
#define WINDOW_DATA_KEY		"SmartBookmarksWindowData"
#define LABEL_WIDTH_CHARS	32

struct SmartBookmarksExtensionPrivate
{
	int smart_bookmark_node_changed_id;
	int smart_bookmark_node_removed_id;
	int smart_bookmark_node_added_id;
};

typedef struct
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

static void smart_bookmarks_extension_class_init (SmartBookmarksExtensionClass *klass);
static void smart_bookmarks_extension_iface_init (EphyExtensionIface *iface);
static void smart_bookmarks_extension_init       (SmartBookmarksExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
smart_bookmarks_extension_get_type (void)
{
	return type;
}

GType
smart_bookmarks_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (SmartBookmarksExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) smart_bookmarks_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (SmartBookmarksExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) smart_bookmarks_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) smart_bookmarks_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "SmartBookmarksExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

/**
  * Gnome Dictionray launcher.
  * For now, just spawn an instance of Gnome Dictionary with
  * searched word added to command line
  * FIXME: what happens if user locate is != UTF-8
  * Will the word get mangled using char* ?
  *
 **/
static void
search_gnome_dict_cb (GtkAction *action,
		      EphyWindow *window)
{
	EphyEmbed *embed;
	char *argv[4] = { "gnome-dictionary", "--look-up", NULL, NULL };
	GError *error = NULL;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	g_return_if_fail (EPHY_IS_EMBED (embed));

	/* ask Mozilla the selection */
	argv[2] = mozilla_get_selected_text (embed);
	if (argv[2] == NULL) return;

	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);

	if (error != NULL)
	{
		g_warning ("Could not launch %s command: %s", argv[0], error->message);
		g_error_free (error);
	}

	g_free (argv[2]);
}

static void
search_smart_bookmark_cb (GtkAction *action,
			  EphyWindow *window)
{
	EphyNode *bmk;
	const char *bmk_url;
	char *text, *url;
	EphyBookmarks *bookmarks;
	EphyEmbed *embed;
	guint id;
	EphyNewTabFlags flags = EPHY_NEW_TAB_OPEN_PAGE;

	embed = ephy_embed_container_get_active_child (EPHY_EMBED_CONTAINER (window));
	g_return_if_fail (EPHY_IS_EMBED (embed));

	/* ask Mozilla the selection */
	text = mozilla_get_selected_text (embed);
	if (text == NULL) return;

	id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (action), NODE_ID_KEY));
	g_return_if_fail (id != 0);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell_get_default ());
	bmk = ephy_bookmarks_get_from_id (bookmarks, id);
	if (bmk == NULL) return;

	bmk_url =  ephy_node_get_property_string (bmk, EPHY_NODE_BMK_PROP_LOCATION);
	g_return_if_fail (bmk_url != NULL);

	/* Use smart bookmark solver to build definitive url */
	url = ephy_bookmarks_resolve_address (bookmarks, bmk_url, text);

	if (url != NULL)
	{
		if (eel_gconf_get_boolean (CONF_OPEN_IN_TAB))
		{
			flags |= EPHY_NEW_TAB_IN_EXISTING_WINDOW |
				 EPHY_NEW_TAB_JUMP;
		}
		else
		{
			flags |= EPHY_NEW_TAB_IN_NEW_WINDOW;
		}

		ephy_shell_new_tab (ephy_shell, window, NULL, url, flags);
	}
	else
	{
		g_warning("Smart Bookmarks extension: cannot solve smart url (url=%s, text=%s)", bmk_url, text);
	}

	g_free (url);
	g_free (text);
}

static gboolean
context_menu_cb (EphyEmbed *embed,
		 EphyEmbedEvent *event,
		 EphyWindow *window)
{
	gboolean can_copy;
	GtkAction  *action;
	WindowData *data;

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_val_if_fail (data != NULL, FALSE);

	/* Is there some selection? */
	can_copy = ephy_command_manager_can_do_command
		(EPHY_COMMAND_MANAGER (embed), "cmd_copy");

	action = gtk_action_group_get_action (data->action_group, LOOKUP_ACTION);
	g_return_val_if_fail (action != NULL, FALSE);

	gtk_action_set_sensitive (action, can_copy);
	gtk_action_set_visible (action, can_copy);

	return FALSE;
}

/* copied from ephy-bookmarks-menu.c */
static int
sort_bookmarks (gconstpointer a,
		gconstpointer b)
{
	EphyNode *node_a = (EphyNode *) a;
	EphyNode *node_b = (EphyNode *) b;
	const char *title1, *title2;
	int retval;

	title1 = ephy_node_get_property_string (node_a, EPHY_NODE_BMK_PROP_TITLE);
	title2 = ephy_node_get_property_string (node_b, EPHY_NODE_BMK_PROP_TITLE);

	if (title1 == NULL)
	{
		retval = -1;
	}
	else if (title2 == NULL)
	{
		retval = 1;
	}
	else
	{
		char *str_a, *str_b;

		str_a = g_utf8_casefold (title1, -1);
		str_b = g_utf8_casefold (title2, -1);
		retval = g_utf8_collate (str_a, str_b);
		g_free (str_a);
		g_free (str_b);
	}

	return retval;
}

static void
rebuild_ui (WindowData *data)
{
	GtkUIManager *manager = data->manager;
	guint ui_id;
	GList *bmks = NULL, *l;
	EphyBookmarks *bookmarks;
	GPtrArray *children;
	EphyNode *smart_bmks, *bmk;
	char verb[SMB_ACTION_LENGTH];
	int i;

	LOG ("Rebuilding UI");

	/* clean UI */
	if (data->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (manager, data->ui_id);
		gtk_ui_manager_ensure_update (manager);
	}

	data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	/* Add bookmarks to popup context (normal document) */
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyDocumentPopup",
			       "SmbExtSep0", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyDocumentPopup",
			       LOOKUP_ACTION "Menu", LOOKUP_ACTION,
			       GTK_UI_MANAGER_MENU, FALSE);

	/* Add bookmarks to input popup context */
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyInputPopup",
			       "SmbExtSep0", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/EphyInputPopup",
			       LOOKUP_ACTION "Menu", LOOKUP_ACTION,
			       GTK_UI_MANAGER_MENU, FALSE);

	/* get smart bookmarks and sort them */
	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	smart_bmks = ephy_bookmarks_get_smart_bookmarks (bookmarks);
	children = ephy_node_get_children (smart_bmks);

	for (i=0; i < children->len; i++)
	{
		bmks = g_list_prepend (bmks, g_ptr_array_index (children, i));
	}

	bmks = g_list_sort (bmks, (GCompareFunc) sort_bookmarks);

	for (l = bmks; l != NULL; l = l->next)
	{
		bmk = (EphyNode *) l->data;

		g_snprintf (verb, sizeof (verb), SMB_ACTION, ephy_node_get_id (bmk));

		gtk_ui_manager_add_ui (manager, ui_id,
				       "/EphyDocumentPopup/" LOOKUP_ACTION "Menu",
				       verb, verb,
				       GTK_UI_MANAGER_MENUITEM, FALSE);
		gtk_ui_manager_add_ui (manager, ui_id,
				       "/EphyInputPopup/" LOOKUP_ACTION "Menu",
				       verb, verb,
				       GTK_UI_MANAGER_MENUITEM, FALSE);
	}

	g_list_free (bmks);

	/* See action argument */
	gtk_ui_manager_add_ui (manager, ui_id,
			       "/EphyDocumentPopup/" LOOKUP_ACTION "Menu",
			       GDICT_ACTION "IDP", GDICT_ACTION,
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id,
			       "/EphyInputPopup/" LOOKUP_ACTION "Menu",
			       GDICT_ACTION "INPUTP", GDICT_ACTION,
			       GTK_UI_MANAGER_MENUITEM, FALSE);

	gtk_ui_manager_ensure_update (manager);
}

static void
foreach_window (GFunc func,
		gpointer user_data)
{
	EphySession *session;
	GList *windows;

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));

	windows = ephy_session_get_windows (session);
	g_list_foreach (windows, func, user_data);
	g_list_free (windows);
}

static void
sync_bookmark_properties (GtkAction *action,
			  EphyNode *bmk)
{
	const char *title;

	title = ephy_node_get_property_string (bmk, EPHY_NODE_BMK_PROP_TITLE);
	g_object_set (action, "label", title, NULL);
}

static void
add_action_for_smart_bookmark (EphyWindow *window,
			       WindowData *data,
			       EphyNode *bmk)
{
	GtkAction *action;
	char verb[SMB_ACTION_LENGTH];
	guint id;

	id = ephy_node_get_id (bmk);
	g_snprintf (verb, sizeof (verb), SMB_ACTION, id);

	action = g_object_new (GTK_TYPE_ACTION,
			       "name", verb,
			       NULL);
	g_object_set_data (G_OBJECT (action), NODE_ID_KEY, GUINT_TO_POINTER (id));

	gtk_action_group_add_action (data->action_group, action);
	g_signal_connect (action, "activate",
			  G_CALLBACK (search_smart_bookmark_cb), window);

	sync_bookmark_properties (action, bmk);

	g_object_unref (action);
}

static void
add_bookmark_to_window (EphyWindow *window,
			EphyNode *bmk)
{
	WindowData *data;

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	add_action_for_smart_bookmark (window, data, bmk);

	rebuild_ui (data);
}

static void
remove_bookmark_from_window (EphyWindow *window,
			     EphyNode *bmk)
{
	GtkAction *action;
	WindowData *data;
	char verb[SMB_ACTION_LENGTH];

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	g_snprintf (verb, sizeof (verb), SMB_ACTION, ephy_node_get_id (bmk));
	action = gtk_action_group_get_action (data->action_group, verb);
	g_return_if_fail (action != NULL);

	/* first rebuild the UI so that it doesn't reference the action anymore */
	rebuild_ui (data);

	/* now we can remove the action */
	gtk_action_group_remove_action (data->action_group, action);
}

static void
sync_bookmark_properties_in_window (EphyWindow *window,
				    EphyNode *bmk)
{
	GtkAction *action;
	WindowData *data;
	char verb[SMB_ACTION_LENGTH];

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	g_snprintf (verb, sizeof (verb), SMB_ACTION, ephy_node_get_id (bmk));
	action = gtk_action_group_get_action (data->action_group, verb);
	g_return_if_fail (action != NULL);

	sync_bookmark_properties (action, bmk);
}

static void
smart_bookmark_added_cb (EphyNode *keywords,
			 EphyNode *bmk,
			 SmartBookmarksExtension *extension)
{
	LOG ("SmartBookmarksExtension smart bookmarks added");

	foreach_window ((GFunc) add_bookmark_to_window, bmk);
}

static void
smart_bookmark_removed_cb (EphyNode *keywords,
			   EphyNode *bmk,
			   guint old_index,
			   SmartBookmarksExtension *extension)
{
	LOG ("SmartBookmarksExtension smart bookmarks removed");

	foreach_window ((GFunc) remove_bookmark_from_window, bmk);
}

static void
smart_bookmark_changed_cb (EphyNode *node,
			   EphyNode *bmk,
			   guint property_id,
			   SmartBookmarksExtension *extension)
{
	LOG ("SmartBookmarksExtension smart bookmarks changed");

	if (property_id == EPHY_NODE_BMK_PROP_TITLE)
	{
		foreach_window ((GFunc) sync_bookmark_properties_in_window, bmk);
	}
}

static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_context_menu",
			  G_CALLBACK (context_menu_cb), window);
}

static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (context_menu_cb), window);
}

static void
connect_proxy_cb (GtkActionGroup *action_group,
                  GtkAction *action,
                  GtkWidget *proxy)
{
	/* If NODE_ID_KEY has NULL data, we have the main menu entry, or the
	 * context menu main entry. Allow underscores for these.
	 */
	if (GTK_IS_MENU_ITEM (proxy) &&
	    g_object_get_data (G_OBJECT (action),NODE_ID_KEY) != NULL)
	{
		GtkLabel *label;

		label = (GtkLabel *) ((GtkBin *) proxy)->child;

		gtk_label_set_use_underline (label, FALSE);
		gtk_label_set_ellipsize (label, PANGO_ELLIPSIZE_END);
		gtk_label_set_max_width_chars (label, LABEL_WIDTH_CHARS);
	}
}

static const GtkActionEntry action_entries [] =
{
	{ LOOKUP_ACTION,
	  NULL,
	  N_("_Look Up"),
	  NULL,
	  NULL,
	  NULL
	},
	{ GDICT_ACTION,
	  NULL,
	  N_("_GNOME Dictionary"),
	  NULL,
	  NULL,
	  G_CALLBACK (search_gnome_dict_cb)
	},
};

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkActionGroup *action_group;
	WindowData *data;
	EphyBookmarks *bookmarks;
	GPtrArray *children;
	EphyNode *smart_bmks, *bmk;
	int i;

	LOG ("SmartBookmarksExtension attach_window %p", window);

	/* Attach ui infos to the window */
	data = g_new0 (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
				(GDestroyNotify) g_free);

	/* Create new action group for this extension */
	action_group = gtk_action_group_new ("SmbExtActions");
	g_signal_connect (action_group, "connect-proxy",
			  G_CALLBACK (connect_proxy_cb), NULL);
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), window);

	data->manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	data->action_group = action_group;

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	smart_bmks = ephy_bookmarks_get_smart_bookmarks (bookmarks);
	children = ephy_node_get_children (smart_bmks);

	for (i=0; i < children->len; i++)
	{
		bmk = g_ptr_array_index (children, i);

		add_action_for_smart_bookmark (window, data, bmk);
	}

	/* Action group completed */
	gtk_ui_manager_insert_action_group (data->manager, action_group, -1);
	g_object_unref (action_group);

	/* now add the UI to the window */
	rebuild_ui (data);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	WindowData *data;

	LOG ("SmartBookmarksExtension detach_window");

	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (data->manager, data->ui_id);
	gtk_ui_manager_ensure_update (data->manager);
	gtk_ui_manager_remove_action_group (data->manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
smart_bookmarks_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
smart_bookmarks_extension_init (SmartBookmarksExtension *extension)
{
	EphyBookmarks *bookmarks;
	EphyNode *smart_bmks;

	LOG ("SmartBookmarksExtension initialising");

	extension->priv = SMART_BOOKMARKS_EXTENSION_GET_PRIVATE (extension);

	bookmarks = ephy_shell_get_bookmarks (ephy_shell);
	smart_bmks = ephy_bookmarks_get_smart_bookmarks (bookmarks);

	extension->priv->smart_bookmark_node_added_id =
		ephy_node_signal_connect_object
			(smart_bmks, EPHY_NODE_CHILD_ADDED,
			 (EphyNodeCallback) smart_bookmark_added_cb,
			 G_OBJECT (extension));
	extension->priv->smart_bookmark_node_removed_id =
		ephy_node_signal_connect_object
			(smart_bmks, EPHY_NODE_CHILD_REMOVED,
			 (EphyNodeCallback) smart_bookmark_removed_cb,
			 G_OBJECT (extension));
	extension->priv->smart_bookmark_node_changed_id =
		ephy_node_signal_connect_object
			(smart_bmks, EPHY_NODE_CHILD_CHANGED,
			 (EphyNodeCallback) smart_bookmark_changed_cb,
			 G_OBJECT (extension));
}

static void
smart_bookmarks_extension_class_init (SmartBookmarksExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	g_type_class_add_private (object_class, sizeof (SmartBookmarksExtensionPrivate));
}
