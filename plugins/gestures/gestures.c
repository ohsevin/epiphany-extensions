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

static GHashTable *gestures_actions = NULL;

static void
tab_update_gestures (void)
{
	/* this should read a pref, parse it, setup a listener, etc */
	/* for now, just hardcode some */

	int i;
	static const struct
	{
		gchar *sequence;
		gchar *action;
	} default_gestures[] = {
                /* none */
                { "5"           , "none"          },

		/* down */
                { "258"         , "new_tab"       },

                /* up */
                { "852"         , "new_window"    },

                /* up-down */
                { "85258"       , "reload"        },
                { "8525"        , "reload"        },
                { "5258"        , "reload"        },

                /* up-down-up */
                { "8525852"     , "reload_bypass" },
                { "852585"      , "reload_bypass" },
                { "85252"       , "reload_bypass" },
                { "85852"       , "reload_bypass" },
                { "525852"      , "reload_bypass" },
                { "52585"       , "reload_bypass" },

                /* up-left-down */
                { "9632147"     , "homepage"      },
                { "963214"      , "homepage"      },
                { "632147"      , "homepage"      },
                { "962147"      , "homepage"      },
                { "963147"      , "homepage"      },

                /* down-up */
                { "25852"       , "clone_window"  },
                { "2585"        , "clone_window"  },
                { "5852"        , "clone_window"  },

                /* down-up-down */
                { "2585258"     , "clone_tab"     },
                { "258525"      , "clone_tab"     },
                { "25858"       , "clone_tab"     },
                { "25258"       , "clone_tab"     },
                { "585258"      , "clone_tab"     },
                { "58525"       , "clone_tab"     },

                /* up-left-up */
                { "96541"       , "up"            },
                { "9651"        , "up"            },
                { "9541"        , "up"            },

                /* right-left-right */
                { "4565456"     , "close"         },
                { "456545"      , "close"         },
                { "45656"       , "close"         },
                { "45456"       , "close"         },
                { "565456"      , "close"         },
                { "56545"       , "close"         },

                /* down-right */
                { "14789"       , "close"         },
                { "1489"        , "close"         },

                /* left */
                { "654"         , "back"          },

                /* right */
                { "456"         , "forward"       },

                /* down-left */
                { "36987"       , "fullscreen"    },
                { "3687"        , "fullscreen"    },

                /* up-right */
                { "74123"       , "next_tab"      },
                { "7423"        , "next_tab"      },

                /* up-left */
                { "96321"       , "prev_tab"      },
                { "9621"        , "prev_tab"      },

                /* s */
                { "321456987"   , "view_source"   },
                { "21456987"    , "view_source"   },
                { "32145698"    , "view_source"   },
                { "3256987"     , "view_source"   },
                { "3214587"     , "view_source"   },
                { "32145987"    , "view_source"   },
                { "32156987"    , "view_source"   },

                /* left-up */
                { "98741"       , "stop"          },
                { "9841"        , "stop"          },

		{ NULL		, NULL		  }

	};

	if (gestures_actions)
	{
		g_hash_table_destroy (gestures_actions);
	}

	gestures_actions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	for (i = 0; default_gestures[i].sequence; ++i)
	{
		g_hash_table_insert (gestures_actions,
				     g_strdup (default_gestures[i].sequence),
				     g_strdup (default_gestures[i].action));
	}
}

static void
tab_gesture_performed_cb (EphyGestures *eg, const gchar *sequence, EphyTab *tab)
{
	const char  *action;
	EphyWindow *window;
	EphyEmbed  *embed;

	if (!gestures_actions)
	{
		tab_update_gestures ();
	}

	action = g_hash_table_lookup (gestures_actions, sequence);

	if (!action)
	{
		return;
	}

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

		gul_gestures_start (eg, GTK_WIDGET (window), button, x, y);

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

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
			  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Gestures plugin exit")
}
