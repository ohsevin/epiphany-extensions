/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Adam Hooper
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

#include "ephy-gestures.h"
#include "ephy-debug.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <gmodule.h>
#include <glib-object.h>
#include <libxml/tree.h>

static GHashTable *gestures = NULL;

#define EPHY_GESTURES_XML_FILE		SHARE_DIR "/ephy-gestures.xml"
#define EPHY_GESTURES_XML_ROOT		"epiphany_gestures"
#define EPHY_GESTURES_XML_VERSION	"0.3"

static void
load_one_gesture (xmlNodePtr node)
{
	xmlNodePtr child;
	GSList *sequence = NULL, *cur = NULL;
	xmlChar *t = NULL, *action = NULL;

	g_return_if_fail (node != NULL);

	if (strcmp (node->name, "gesture") != 0) return;

	for (child = node->children; child != NULL; child = child->next)
	{
		if (xmlStrEqual (child->name, "sequence"))
		{
			t = xmlNodeGetContent (child);

			sequence = g_slist_prepend (sequence, t);
		}
		else if (xmlStrEqual (child->name, "action"))
		{
			g_return_if_fail (action == NULL);

			action = xmlNodeGetContent (child);
		}
	}

	g_return_if_fail (sequence != NULL && action != NULL);

	for (cur = sequence; cur != NULL ; cur = cur->next)
	{
		g_hash_table_insert (gestures, g_strdup (cur->data),
				     g_strdup (action));
		xmlFree (cur->data);
	}

	g_slist_free (sequence);
	xmlFree (action);
}

static void
load_gestures (void)
{
	xmlDocPtr doc;
	xmlNodePtr root, child;
	xmlChar *tmp;

	g_return_if_fail (g_file_test (EPHY_GESTURES_XML_FILE, G_FILE_TEST_EXISTS));

	doc = xmlParseFile (EPHY_GESTURES_XML_FILE);
	g_return_if_fail (doc != NULL);

	root = xmlDocGetRootElement (doc);
	g_return_if_fail (root && strcmp (root->name, EPHY_GESTURES_XML_ROOT) == 0);

	tmp = xmlGetProp (root, "version");
	g_return_if_fail (tmp && strcmp (tmp, EPHY_GESTURES_XML_VERSION) == 0);
	xmlFree (tmp);

	for (child = root->children; child != NULL; child = child->next)
	{
		load_one_gesture (child);
	}

	xmlFreeDoc (doc);
}

static void
tab_gesture_performed_cb (EphyGestures *eg, const char *sequence, EphyTab *tab)
{
	EphyWindow *window;
	const char *path;

	LOG ("Gesture: sequence '%s'", sequence)

	path = g_hash_table_lookup (gestures, sequence);
	if (path == NULL) return;

	LOG ("Gesture: path is '%s'", path)

	window = ephy_tab_get_window (tab);
	g_return_if_fail (EPHY_IS_WINDOW (window));

	if (strcmp (path, "fallback") == 0)
	{
		/* Fall back to normal click */
		EphyEmbed *embed;
		EphyEmbedEvent *event;
		gint handled = FALSE;
		guint type;

		embed = ephy_tab_get_embed (tab);

		event = g_object_get_data (G_OBJECT (eg), "embed_event");

		ephy_embed_event_get_event_type (event, &type);

		g_signal_emit_by_name (embed, "ge_dom_mouse_click", event,
				       &handled);

		if (handled == FALSE && type == EPHY_EMBED_EVENT_MOUSE_BUTTON3)
		{
			g_signal_emit_by_name (embed, "ge_context_menu",
					       event, &handled);
		}
	}
	else
	{
		GtkUIManager *manager;
		GtkAction *action = NULL;
		GList *action_groups, *l;

		manager = GTK_UI_MANAGER (window->ui_merge);
		action_groups = gtk_ui_manager_get_action_groups (manager);

//		action = gtk_ui_manager_get_action (manager, path);

		for (l = action_groups; l != NULL && action == NULL; l = l->next)
		{
			GtkActionGroup *group = GTK_ACTION_GROUP (l->data);

			action = gtk_action_group_get_action (group, path);
		}

		if (action != NULL)
		{
			gtk_action_activate (action);
		}
		else
		{
			g_warning ("Action for path '%s' not found!\n", path);
		}
	}
}

static gint
dom_mouse_down_cb  (EphyEmbed *embed,
		    EphyEmbedEvent *event,
		    EphyTab *tab)
{
        EmbedEventContext context;
	EphyEmbedEventType type;
	gint handled = FALSE;

	ephy_embed_event_get_event_type (event, &type);
        ephy_embed_event_get_context (event, &context);

	if (type == EPHY_EMBED_EVENT_MOUSE_BUTTON2 &&
            !(context & EMBED_CONTEXT_INPUT))
	{
		EphyGestures *eg;
		EphyWindow *window;
		guint x, y;

		window = ephy_tab_get_window (tab);

		ephy_embed_event_get_coords (event, &x, &y);

		eg = ephy_gestures_new ();

		g_object_set_data_full (G_OBJECT(eg), "embed_event",
					g_object_ref (event),
					g_object_unref);

		g_signal_connect (eg, "gesture-performed",
				  G_CALLBACK (tab_gesture_performed_cb), tab);

		ephy_gestures_start (eg, GTK_WIDGET (window), /* button */ 1, x, y);

		g_object_unref (eg);

		handled = TRUE;
	}

	return handled;
}

static void
tab_added_cb (GtkWidget *nb, GtkWidget *child)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (child), "EphyTab"));

	g_signal_connect (G_OBJECT (child), "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), tab);
}

static void
tab_removed_cb (GtkWidget *nb, GtkWidget *child)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (child), "EphyTab"));

	g_signal_handlers_disconnect_by_func (G_OBJECT (child),
					      G_CALLBACK (dom_mouse_down_cb),
					      tab);
}

static void
reload_bypass_cb (GtkAction *action, EphyWindow *window)
{
	EphyEmbed *embed;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	ephy_embed_reload (embed, EMBED_RELOAD_BYPASSCACHE);
}

static void
clone_window_cb (GtkAction *action, EphyWindow *window)
{
	EphyTab *tab;

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	ephy_shell_new_tab (ephy_shell, window, tab, NULL,
			    EPHY_NEW_TAB_CLONE_PAGE |
			    EPHY_NEW_TAB_IN_NEW_WINDOW);
}

static void
clone_tab_cb (GtkAction *action, EphyWindow *window)
{
	EphyTab *tab;

	tab = ephy_window_get_active_tab (window);
	g_return_if_fail (EPHY_IS_TAB (tab));

	ephy_shell_new_tab (ephy_shell, window, tab, NULL,
			    EPHY_NEW_TAB_CLONE_PAGE |
			    EPHY_NEW_TAB_IN_EXISTING_WINDOW);
}

static GtkActionEntry action_entries [] =
{
	{ "EphyGesturesPluginReloadBypass", "", NULL, NULL, NULL,
	  G_CALLBACK (reload_bypass_cb)
	},
	{ "EphyGesturesPluginCloneWindow", "", NULL, NULL, NULL,
	  G_CALLBACK (clone_window_cb)
	},
	{ "EphyGesturesPluginCloneTab", "", NULL, NULL, NULL,
	  G_CALLBACK (clone_tab_cb)
	}
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static void
setup_actions (EphyWindow *window)
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;

	manager = GTK_UI_MANAGER (window->ui_merge);

	action_group = gtk_action_group_new ("EphyGesturesPluginActions");

	gtk_action_group_add_actions (action_group, action_entries,
				      n_action_entries, window);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	g_object_unref (action_group);
}

static void
new_window_cb (Session *session, EphyWindow *window)
{
	GtkWidget *notebook;

	setup_actions (window);

	notebook = ephy_window_get_notebook (window);

	g_signal_connect (notebook, "tab_added",
			  G_CALLBACK (tab_added_cb), NULL);
	g_signal_connect (notebook, "tab_removed",
			  G_CALLBACK (tab_removed_cb), NULL);
}

G_MODULE_EXPORT void
plugin_init (GTypeModule *module)
{
	Session *session;

	LOG ("Gestures plugin initialising")

	gestures = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	g_assert (gestures != NULL);

	load_gestures ();

	session = EPHY_SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
			  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Gestures plugin exiting")

	g_hash_table_destroy (gestures);
	gestures = NULL;
}
