/*
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

#include "ephy-popup-blocker-list.h"
#include "mozilla-helpers.h"

#include "ephy-debug.h"

#define EPHY_POPUP_BLOCKER_LIST_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_LIST, EphyPopupBlockerListPrivate))

struct EphyPopupBlockerListPrivate
{
	EphyEmbed *embed;
	GSList *popups;
};

typedef struct
{
	/* For open popups (or popups which were opened at one point) */
	EphyWindow *window;
	gint x;
	gint y;

	/* For never-opened popups */
	char *url;
	char *features;
} BlockedPopup;

static void ephy_popup_blocker_list_class_init	(EphyPopupBlockerListClass *klass);
static void ephy_popup_blocker_list_init	(EphyPopupBlockerList *popup);

enum
{
	PROP_0,
	PROP_COUNT
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_popup_blocker_list_get_type (void)
{
	return type;
}

GType
ephy_popup_blocker_list_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyPopupBlockerListClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_popup_blocker_list_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPopupBlockerList),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_popup_blocker_list_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPopupBlockerList",
					    &our_info, 0);

	return type;
}

EphyPopupBlockerList *
ephy_popup_blocker_list_new (EphyEmbed *embed)
{
	EphyPopupBlockerList *ret;

	ret = EPHY_POPUP_BLOCKER_LIST
		(g_object_new (EPHY_TYPE_POPUP_BLOCKER_LIST, NULL));

	ret->priv->embed = embed;

	return ret;
}

static void
ephy_popup_blocker_list_init (EphyPopupBlockerList *list)
{
	LOG ("EphyPopupBlockerList initialising")

	list->priv = EPHY_POPUP_BLOCKER_LIST_GET_PRIVATE (list);

	list->priv->popups = NULL;
	list->priv->embed = NULL;
}

void
ephy_popup_blocker_list_insert (EphyPopupBlockerList *list,
				const char *url,
				const char *features)
{
	BlockedPopup *popup;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	popup = g_new0 (BlockedPopup, 1);

	popup->window = NULL;
	popup->url = g_strdup (url);
	popup->features = g_strdup (features);

	list->priv->popups = g_slist_prepend (list->priv->popups, popup);

	LOG ("Added blocked popup to list %p: %s\n", list, popup->url);

	g_object_notify (G_OBJECT (list), "count");
}

static void
window_visible_cb (EphyWindow *widget,
		   GParamSpec *pspec,
		   EphyPopupBlockerList *list)
{
	g_return_if_fail (EPHY_IS_WINDOW (widget));
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	g_object_notify (G_OBJECT (list), "count");
}

/* FIXME: Have to declare a function here, the next three functions depend on
 * each other recursively */
static void ephy_popup_blocker_list_remove_window (EphyPopupBlockerList *list, EphyWindow *window);

static gboolean
window_destroy_cb (EphyWindow *window,
		   EphyPopupBlockerList *list)
{
	g_return_if_fail (EPHY_IS_WINDOW (window));
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	LOG ("window_destroy_cb on window %p with list %p\n", window, list)

	ephy_popup_blocker_list_remove_window (list, window);

	return FALSE;
}

static void
free_blocked_popup (BlockedPopup *popup)
{
	/* Destroy hidden popups */
	if (popup->window != NULL)
	{
		g_return_if_fail (EPHY_IS_WINDOW (popup->window));

		g_signal_handlers_disconnect_matched
			(popup->window, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
			 window_destroy_cb, NULL);
		g_signal_handlers_disconnect_matched
			(popup->window, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
			 window_visible_cb, NULL);

		if (GTK_WIDGET_VISIBLE (popup->window) == FALSE)
		{
			gtk_widget_destroy (GTK_WIDGET (popup->window));
		}
	}

	g_free (popup->url);
	g_free (popup->features);
	g_free (popup);
}

static void
ephy_popup_blocker_list_remove_window (EphyPopupBlockerList *list,
				       EphyWindow *window)
{
	GSList *t;
	BlockedPopup *popup;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));
	g_return_if_fail (EPHY_IS_WINDOW (window));

	for (t = list->priv->popups; t; t = g_slist_next (t))
	{
		popup = (BlockedPopup *) t->data;

		if (popup->window == window)
		{
			LOG ("Removing window %p from list %p\n", window, list)

			list->priv->popups = g_slist_delete_link
				(list->priv->popups, t);
			free_blocked_popup (popup);
			break;
		}
	}
}

void
ephy_popup_blocker_list_insert_window (EphyPopupBlockerList *list,
				       EphyWindow *window)
{
	BlockedPopup *popup;

	g_return_if_fail (EPHY_IS_WINDOW (window));

	popup = g_new0 (BlockedPopup, 1);

	popup->window = window;
	popup->x = -1;
	popup->y = -1;
	popup->url = NULL;
	popup->features = NULL;

	list->priv->popups = g_slist_prepend (list->priv->popups, popup);

	g_signal_connect (window, "destroy",
			  G_CALLBACK (window_destroy_cb), list);
	g_signal_connect (window, "notify::visible",
			  G_CALLBACK (window_visible_cb), list);

	LOG ("Added allowed popup to list %p: window %p\n", list, window)

	g_object_notify (G_OBJECT (list), "count");
}

void
ephy_popup_blocker_list_reset (EphyPopupBlockerList *list)
{
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	g_slist_foreach (list->priv->popups,
			 (GFunc) free_blocked_popup, NULL);

	g_slist_free (list->priv->popups);

	list->priv->popups = NULL;

	g_object_notify (G_OBJECT (list), "count");
}

static void
ephy_popup_blocker_list_finalize (GObject *object)
{
	EphyPopupBlockerList *list = EPHY_POPUP_BLOCKER_LIST (object);

	LOG ("EphyPopupBlockerList finalizing")

	ephy_popup_blocker_list_reset (list);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_popup_blocker_list_set_property (GObject *object,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *pspec)
{
	EphyPopupBlockerList *list = EPHY_POPUP_BLOCKER_LIST (object);

	switch (prop_id)
	{
		case PROP_COUNT:
			/* read only */
			break;
	}
}

static guint
count_popups (EphyPopupBlockerList *list)
{
	GSList *t;
	BlockedPopup *popup;
	guint count = 0;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	for (t = list->priv->popups; t; t = g_slist_next (t))
	{
		popup = (BlockedPopup *) t->data;

		/* Hidden windows */
		if (popup->window != NULL
		    && EPHY_IS_WINDOW (popup->window)
		    && GTK_WIDGET_VISIBLE (popup->window) == FALSE)
		{
			count++;
			continue;
		}

		/* If there's no window, it can't be visible */
		if (popup->window == NULL)
		{
			count++;
			continue;
		}
	}

	return count;
}

static void
ephy_popup_blocker_list_get_property (GObject *object,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *pspec)
{
	EphyPopupBlockerList *list = EPHY_POPUP_BLOCKER_LIST (object);

	switch (prop_id)
	{
		case PROP_COUNT:
			g_value_set_uint (value, count_popups (list));
			break;
	}
}

static void
ephy_popup_blocker_list_class_init (EphyPopupBlockerListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_popup_blocker_list_finalize;
	object_class->get_property = ephy_popup_blocker_list_get_property;
	object_class->set_property = ephy_popup_blocker_list_set_property;

	g_object_class_install_property (object_class,
					 PROP_COUNT,
					 g_param_spec_uint ("count",
						 	    "Count",
							    "Number of popups",
							    0,
							    G_MAXUINT,
							    0,
							    G_PARAM_READABLE));

	g_type_class_add_private (object_class, sizeof (EphyPopupBlockerListPrivate));
}

void
ephy_popup_blocker_list_show_all (EphyPopupBlockerList *list)
{
	GSList *t;
	BlockedPopup *popup;
	EphyEmbed *embed;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	t = list->priv->popups;
	while (t != NULL)
	{
		popup = (BlockedPopup *) t->data;

		if (popup->window != NULL)
		{
			g_return_if_fail (EPHY_IS_WINDOW (popup->window));

			embed = ephy_window_get_active_embed (popup->window);
			g_return_if_fail (EPHY_IS_EMBED (embed));

			gtk_window_present (GTK_WINDOW (popup->window));
			if (popup->x != -1 && popup->y != -1)
			{
				gtk_window_move (GTK_WINDOW (popup->window),
						 popup->x, popup->y);
			}
			mozilla_enable_javascript (embed, TRUE);

			t = t->next;
		}
		else if (popup->url != NULL)
		{
			mozilla_open_popup (list->priv->embed, popup->url,
					    popup->features);

			t = g_slist_delete_link (list->priv->popups, t);

			free_blocked_popup (popup);
		}
		else
		{
			t = t->next;
		}
	}
}

void
ephy_popup_blocker_list_hide_all (EphyPopupBlockerList *list)
{
	GSList *t;
	BlockedPopup *popup;
	EphyEmbed *embed;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	for (t = list->priv->popups; t; t = g_slist_next (t))
	{
		popup = (BlockedPopup *) t->data;

		if (popup->window != NULL)
		{
			g_return_if_fail (EPHY_IS_WINDOW (popup->window));

			embed = ephy_window_get_active_embed (popup->window);
			g_return_if_fail (EPHY_IS_EMBED (embed));

			mozilla_enable_javascript (embed, FALSE);
			gtk_window_get_position (GTK_WINDOW (popup->window),
						 &popup->x, &popup->y);
			gtk_widget_hide (GTK_WIDGET (popup->window));
		}
	}
}
