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

#include "ad-blocker.h"

#include "ephy-debug.h"

#define AD_BLOCKER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_AD_BLOCKER, AdBlockerPrivate))

static void ad_blocker_class_init (AdBlockerClass *klass);
static void ad_blocker_init (AdBlocker *dialog);

struct _AdBlockerPrivate
{
	AdUriTester *uri_tester;
};

enum
{
	PROP_0,
	PROP_URI_TESTER
};

enum
{
	AD_BLOCKED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ad_blocker_get_type (void)
{
	return type;
}

GType
ad_blocker_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (AdBlockerClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ad_blocker_class_init,
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (AdBlocker),
		0, /* n_preallocs */
		(GInstanceInitFunc) ad_blocker_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "AdBlocker",
					    &our_info, 0);

	return type;
}

AdBlocker *
ad_blocker_new (AdUriTester *uri_tester)
{
	return g_object_new (TYPE_AD_BLOCKER,
			     "uri-tester", uri_tester,
			     NULL);
}

gboolean
ad_blocker_test_uri (AdBlocker *blocker,
		     const char *uri,
		     AdUriCheckType type)
{
	gboolean ret;

	ret = ad_uri_tester_test_uri (blocker->priv->uri_tester, uri, type);

	if (ret == TRUE)
	{
		g_signal_emit (G_OBJECT (blocker), signals[AD_BLOCKED], 0, uri);
	}

	return ret;
}

static void
ad_blocker_get_property (GObject *object,
			 guint prop_id,
			 GValue *value,
			 GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
ad_blocker_set_property (GObject *object,
			 guint prop_id,
			 const GValue *value,
			 GParamSpec *pspec)
{
	AdBlocker *blocker = AD_BLOCKER (object);

	switch (prop_id)
	{
		case PROP_URI_TESTER:
			blocker->priv->uri_tester = g_value_get_object (value);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
ad_blocker_init (AdBlocker *blocker)
{
	LOG ("AdBlocker initializing %p", blocker)

	blocker->priv = AD_BLOCKER_GET_PRIVATE (blocker);
}

static void
ad_blocker_finalize (GObject *object)
{
	/*
	AdBlockerPrivate *priv = AD_BLOCKER_GET_PRIVATE (AD_BLOCKER (object));
	*/

	LOG ("AdBlocker finalizing %p", object)

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ad_blocker_class_init (AdBlockerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ad_blocker_finalize;
	object_class->get_property = ad_blocker_get_property;
	object_class->set_property = ad_blocker_set_property;

	g_object_class_install_property
		(object_class,
		 PROP_URI_TESTER,
		 g_param_spec_object ("uri-tester",
				      "URI Tester",
				      "URI Tester",
				      G_TYPE_OBJECT,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	signals[AD_BLOCKED] =
		g_signal_new ("ad_blocked",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (AdBlockerClass, ad_blocked),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	g_type_class_add_private (object_class, sizeof (AdBlockerPrivate));
}
