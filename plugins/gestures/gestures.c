/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-gestures.h"
#include "ephy-debug.h"

#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>

#include <gmodule.h>
#include <glib-object.h>
#include <gconf/gconf.h>
#include <libxml/tree.h>

static GHashTable *gestures = NULL;

#define EPHY_GESTURES_XML_FILE		SHARE_DIR "/ephy-gestures.xml"
#define EPHY_GESTURES_XML_ROOT		"epiphany_gestures"
#define EPHY_GESTURES_XML_VERSION	"0.1"

static void
load_one_gesture (xmlNodePtr node)
{
	xmlNodePtr child;
	xmlChar *stroke = NULL, *action = NULL;

	g_return_if_fail (node != NULL);

	if (strcmp (node->name, "gesture") != 0) return;

	for (child = node->children; child != NULL; child = child->next)
	{
		if (strcmp (child->name, "stroke") == 0)
		{
			g_return_if_fail (stroke == NULL);

			stroke = xmlNodeGetContent (child);
		}
		else if (strcmp (child->name, "action") == 0)
		{
			g_return_if_fail (action == NULL);

			action = xmlNodeGetContent (child);
		}
	}

	g_return_if_fail (stroke != NULL && action != NULL);

	g_hash_table_insert (gestures, g_strdup (stroke), g_strdup (action));

	LOG ("added gesture: stroke '%s' action '%s'", stroke, action)

	xmlFree (stroke);
	xmlFree (action);
}

static void
load_gestures (void)
{
	xmlDocPtr doc;
	xmlNodePtr root, child;
	char *tmp;

	g_return_if_fail (g_file_test (EPHY_GESTURES_XML_FILE, G_FILE_TEST_EXISTS));

	doc = xmlParseFile (EPHY_GESTURES_XML_FILE);
	g_return_if_fail (doc != NULL);

	root = xmlDocGetRootElement (doc);
	g_return_if_fail (root && strcmp (root->name, EPHY_GESTURES_XML_ROOT) == 0);

	tmp = xmlGetProp (root, "version");
	if (tmp != NULL && strcmp (tmp, EPHY_GESTURES_XML_VERSION) != 0)
	{
		g_warning ("Unsupported gestures file version detected\n");

		xmlFreeDoc (doc);
		return;
	}
	g_free (tmp);

	for (child = root->children; child != NULL; child = child->next)
	{
		load_one_gesture (child);
	}

	xmlFreeDoc (doc);
}

static void
tab_gesture_performed_cb (EphyGestures *eg, const char *sequence, EphyTab *tab)
{
	const char  *action;
	EphyWindow *window;
	EphyEmbed  *embed;

	action = g_hash_table_lookup (gestures, sequence);

	if (action == NULL) return;

	LOG ("Sequence: %s; Action: %s", sequence, action)

	window = ephy_tab_get_window (tab);
	embed = ephy_tab_get_embed (tab);

	if (!strcmp (action, "none"))
	{
		/* Fall back to normal click */
		gint return_val;
		EphyEmbedEvent *event = g_object_get_data(G_OBJECT(eg), "embed_event");
		g_signal_emit_by_name (embed, "ge_dom_mouse_click", event,
				       &return_val);
	}
	else if (!strcmp (action, "new_tab"))
	{
		ephy_shell_new_tab (ephy_shell, window, tab, NULL,
				    EPHY_NEW_TAB_NEW_PAGE |
				    EPHY_NEW_TAB_IN_EXISTING_WINDOW |
				    EPHY_NEW_TAB_JUMP);
	}
	else if (!strcmp (action, "new_window"))
	{
		ephy_shell_new_tab (ephy_shell, NULL, tab, NULL,
				    EPHY_NEW_TAB_NEW_PAGE |
				    EPHY_NEW_TAB_IN_NEW_WINDOW);
	}
	else if (!strcmp (action, "reload"))
	{
		ephy_embed_reload (embed, EMBED_RELOAD_NORMAL);
	}
	else if (!strcmp (action, "reload_bypass"))
	{
		ephy_embed_reload (embed,
				   EMBED_RELOAD_BYPASSCACHE |
				   EMBED_RELOAD_BYPASSPROXY);
	}
	else if (!strcmp (action, "homepage"))
	{
		GConfEngine *engine;
		char *location;

		engine = gconf_engine_get_default ();
		location = gconf_engine_get_string (engine,
						    "/apps/epiphany/general/homepage",
						    NULL);

		ephy_window_load_url (window,
				      location ? location : "about:blank");

		gconf_engine_unref(engine);
		g_free (location);
	}
	else if (!strcmp (action, "clone_window"))
	{
		ephy_shell_new_tab (ephy_shell, window, tab, NULL,
				    EPHY_NEW_TAB_CLONE_PAGE |
				    EPHY_NEW_TAB_IN_NEW_WINDOW);
	}
	else if (!strcmp (action, "clone_tab"))
	{
		ephy_shell_new_tab (ephy_shell, window, tab, NULL,
				    EPHY_NEW_TAB_CLONE_PAGE |
				    EPHY_NEW_TAB_IN_EXISTING_WINDOW);
	}
	else if (!strcmp (action, "up"))
	{
		ephy_embed_go_up (embed);
	}
	else if (!strcmp (action, "close"))
	{
		ephy_window_remove_tab (window, tab);
	}
	else if (!strcmp (action, "back"))
	{
		ephy_embed_go_back (embed);
	}
	else if (!strcmp (action, "forward"))
	{
		ephy_embed_go_forward (embed);
	}
	else if (!strcmp (action, "fullscreen"))
	{
		if (gdk_window_get_state (GTK_WIDGET (window)->window) & GDK_WINDOW_STATE_FULLSCREEN)
		{
			gtk_window_unfullscreen (GTK_WINDOW (window));
		}
		else
		{
			gtk_window_fullscreen (GTK_WINDOW (window));
		}

	}
	else if (!strcmp (action, "next_tab"))
	{
		GtkNotebook *nb;
		gint page;

		nb = GTK_NOTEBOOK (ephy_window_get_notebook (window));
		g_return_if_fail (nb != NULL);

		page = gtk_notebook_get_current_page (nb);
		g_return_if_fail (page != -1);

		if (page < gtk_notebook_get_n_pages (nb) - 1)
		{
			gtk_notebook_set_current_page (nb, page + 1);
		}
	}
	else if (!strcmp (action, "prev_tab"))
	{
		GtkNotebook *nb;
		gint page;

		nb = GTK_NOTEBOOK (ephy_window_get_notebook (window));
		g_return_if_fail (nb != NULL);

		page = gtk_notebook_get_current_page (nb);
		g_return_if_fail (page != -1);

		if (page > 0)
		{
			gtk_notebook_set_current_page (nb, page - 1);
		}
	}
	else if (!strcmp (action, "view_source"))
	{
		ephy_shell_new_tab (ephy_shell, window, tab, NULL,
				    EPHY_NEW_TAB_CLONE_PAGE |
				    EPHY_NEW_TAB_SOURCE_MODE);
	}
	else if (!strcmp (action, "stop"))
	{
		ephy_embed_stop_load (embed);
	}
	else
	{
		/* unrecognized */
		LOG ("Unrecognized gesture: %s", sequence)
	}
}

static gint
dom_mouse_down_cb  (EphyEmbed *embed,
		    EphyEmbedEvent *event,
		    EphyTab *tab)
{
	guint button;
        EmbedEventContext context;

	ephy_embed_event_get_event_type (event, &button);
        ephy_embed_event_get_context (event, &context);

	if (button == EPHY_EMBED_EVENT_MOUSE_BUTTON2 &&
            !(context & EMBED_CONTEXT_INPUT)) {
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

		ephy_gestures_start (eg, GTK_WIDGET (window), button, x, y);

		g_object_unref (eg);
	}
}

static void
tab_added_cb (GtkWidget *nb, GtkWidget *child)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (child), "EphyTab"));

	g_signal_connect (EPHY_EMBED (child), "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb),
			  tab);
}

static void
new_window_cb (Session *session, EphyWindow *window)
{
	GtkWidget *nb;

	nb = ephy_window_get_notebook (window);

	g_signal_connect (nb, "tab_added",
			  G_CALLBACK (tab_added_cb), NULL);
}

G_MODULE_EXPORT void
plugin_init (GTypeModule *module)
{
	Session *session;

	gestures = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	g_assert (gestures != NULL);

	load_gestures ();

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
			  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Gestures plugin exit")

	g_hash_table_destroy (gestures);
	gestures = NULL;
}
