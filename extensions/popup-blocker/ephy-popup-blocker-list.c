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

#include "ephy-debug.h"

#define EPHY_POPUP_BLOCKER_LIST_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_LIST, EphyPopupBlockerListPrivate))

typedef struct
{
	char *url;
	char *features;
} BlockedPopup;

struct EphyPopupBlockerListPrivate
{
	GSList *blocked_popups;
};

static void ephy_popup_blocker_list_class_init	(EphyPopupBlockerListClass *klass);
static void ephy_popup_blocker_list_init	(EphyPopupBlockerList *popup);

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
ephy_popup_blocker_list_new (void)
{
	EphyPopupBlockerList *ret;

	ret = EPHY_POPUP_BLOCKER_LIST
		(g_object_new (EPHY_TYPE_POPUP_BLOCKER_LIST, NULL));

	return ret;
}

static void
ephy_popup_blocker_list_init (EphyPopupBlockerList *list)
{
	LOG ("EphyPopupBlockerList initialising")

	list->priv = EPHY_POPUP_BLOCKER_LIST_GET_PRIVATE (list);

	list->priv->blocked_popups = NULL;
}

int
ephy_popup_blocker_list_length (EphyPopupBlockerList *list)
{
	g_return_val_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list), 0);

	return g_slist_length (list->priv->blocked_popups);
}

static void
free_blocked_popup (BlockedPopup *popup)
{
	g_free (popup->url);
	g_free (popup->features);
	g_free (popup);
}

void
ephy_popup_blocker_list_reset (EphyPopupBlockerList *list)
{
	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));

	g_slist_foreach (list->priv->blocked_popups,
			 (GFunc) free_blocked_popup, NULL);

	g_slist_free (list->priv->blocked_popups);

	list->priv->blocked_popups = NULL;
}

static int
popup_cmp (BlockedPopup *a,
	   BlockedPopup *b)
{
	return g_utf8_collate (a->url, b->url);
}

void
ephy_popup_blocker_list_insert (EphyPopupBlockerList *list,
				const char *url,
				const char *features)
{
	BlockedPopup *popup;

	g_return_if_fail (EPHY_IS_POPUP_BLOCKER_LIST (list));
	g_return_if_fail (url != NULL);

	popup = g_new0 (BlockedPopup, 1);

	popup->url = g_strdup (url);
	popup->features = g_strdup (features);

	list->priv->blocked_popups = g_slist_insert_sorted
		(list->priv->blocked_popups, popup, (GCompareFunc) popup_cmp);
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
ephy_popup_blocker_list_class_init (EphyPopupBlockerListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_popup_blocker_list_finalize;

	g_type_class_add_private (object_class, sizeof (EphyPopupBlockerListPrivate));
}
