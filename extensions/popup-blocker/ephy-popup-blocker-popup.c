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

#include "ephy-popup-blocker-popup.h"

#include "ephy-debug.h"

#define EPHY_POPUP_BLOCKER_POPUP_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_POPUP_BLOCKER_POPUP, EphyPopupBlockerPopupPrivate))

struct EphyPopupBlockerPopupPrivate
{
	char *url;
	char *features;
};

static void ephy_popup_blocker_popup_class_init	(EphyPopupBlockerPopupClass *klass);
static void ephy_popup_blocker_popup_init	(EphyPopupBlockerPopup *popup);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_popup_blocker_popup_get_type (void)
{
	return type;
}

GType
ephy_popup_blocker_popup_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyPopupBlockerPopupClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_popup_blocker_popup_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyPopupBlockerPopup),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_popup_blocker_popup_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyPopupBlockerPopup",
					    &our_info, 0);

	return type;
}

EphyPopupBlockerPopup *
ephy_popup_blocker_popup_new (const char *url,
			      const char *features)
{
	EphyPopupBlockerPopup *ret;

	ret = EPHY_POPUP_BLOCKER_POPUP
		(g_object_new (EPHY_TYPE_POPUP_BLOCKER_POPUP, NULL));

	ret->priv->url = g_strdup (url);
	ret->priv->features = g_strdup (features);

	return ret;
}

const char *
ephy_popup_blocker_popup_get_url (EphyPopupBlockerPopup *popup)
{
	return popup->priv->url;
}

const char *
ephy_popup_blocker_popup_get_features (EphyPopupBlockerPopup *popup)
{
	return popup->priv->features;
}

static void
ephy_popup_blocker_popup_init (EphyPopupBlockerPopup *popup)
{
	LOG ("EphyPopupBlockerPopup initialising")

	popup->priv = EPHY_POPUP_BLOCKER_POPUP_GET_PRIVATE (popup);
}

static void
ephy_popup_blocker_popup_finalize (GObject *object)
{
	EphyPopupBlockerPopup *popup = EPHY_POPUP_BLOCKER_POPUP (object);

	LOG ("EphyPopupBlockerPopup finalizing")

	g_free (popup->priv->url);
	g_free (popup->priv->features);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_popup_blocker_popup_class_init (EphyPopupBlockerPopupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_popup_blocker_popup_finalize;

	g_type_class_add_private (object_class, sizeof (EphyPopupBlockerPopupPrivate));
}
