/*
 *  Copyright © 2004 Adam Hooper
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

#include "ad-uri-tester.h"
#include "adblock-pattern.h"

#include <pcre.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#define AD_URI_TESTER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_AD_URI_TESTER, AdUriTesterPrivate))

static void ad_uri_tester_class_init (AdUriTesterClass *klass);
static void ad_uri_tester_init (AdUriTester *dialog);

static GObjectClass *parent_class = NULL;

static GType type = 0;

struct _AdUriTesterPrivate
{
	GHashTable *blacklist;
	GHashTable *whitelist;
};

typedef struct
{
	const char *uri;
	unsigned int len;
} UriWithLen;

GType
ad_uri_tester_get_type (void)
{
	return type;
}

GType
ad_uri_tester_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (AdUriTesterClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ad_uri_tester_class_init,
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (AdUriTester),
		0, /* n_preallocs */
		(GInstanceInitFunc) ad_uri_tester_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "AdUriTester",
					    &our_info, 0);

	return type;
}

AdUriTester *
ad_uri_tester_new (void)
{
	return g_object_new (TYPE_AD_URI_TESTER, NULL);
}

static void
load_patterns (AdUriTester *tester)
{
	adblock_pattern_load (tester->priv->blacklist, PATTERN_BLACKLIST);

	adblock_pattern_load (tester->priv->blacklist, PATTERN_DEFAULT_BLACKLIST);

	adblock_pattern_load (tester->priv->whitelist, PATTERN_WHITELIST);
}

static gboolean
match_uri (const char *pattern,
	   const pcre *preg,
	   const UriWithLen *uri_with_len)
{
	int ret;

	ret = pcre_exec (preg, NULL, uri_with_len->uri, uri_with_len->len,
			 0, PCRE_NO_UTF8_CHECK, NULL, 0);

	if (ret >= 0)
	{
		LOG ("Blocking '%s' with pattern '%s'",
		     uri_with_len->uri, pattern);

		return TRUE;
	}

	return FALSE;
}

static gboolean
ad_uri_tester_true (gpointer key,
                    gpointer value,
                    gpointer dummy)
{
	return TRUE;
}

/**
 * ad_uri_tester_reload:
 * @tester: an AdUriTester
 *
 * Reload patterns from zero
 */
void
ad_uri_tester_reload (AdUriTester *tester)
{
	g_hash_table_foreach_remove (tester->priv->blacklist,
				     ad_uri_tester_true,
				     NULL);		     
	g_hash_table_foreach_remove (tester->priv->whitelist,
				     ad_uri_tester_true,
				     NULL);		     
	load_patterns (tester);
}

/**
 * ad_uri_tester_test_uri:
 * @tester: an AdUriTester
 * @uri: an URL to test
 * @type: type of resource this URL is
 *
 * @return %TRUE if this URL should be blocked
 */
gboolean
ad_uri_tester_test_uri (AdUriTester *tester,
			const char *uri,
			AdUriCheckType type)
{
	const char *pattern;
	UriWithLen uri_with_len;

	if (type == AD_URI_CHECK_TYPE_DOCUMENT) 
	{
		return FALSE;
	}

	uri_with_len.uri = uri;
	uri_with_len.len = g_utf8_strlen (uri, -1);

	pattern = g_hash_table_find (tester->priv->blacklist,
				     (GHRFunc) match_uri, &uri_with_len);

	if (pattern == NULL)
	{
		return FALSE;
	}

	pattern = g_hash_table_find (tester->priv->whitelist,
				     (GHRFunc) match_uri, &uri_with_len);

	return pattern == NULL;
}

static void
ad_uri_tester_init (AdUriTester *tester)
{
	LOG ("AdUriTester initializing %p", tester);

	tester->priv = AD_URI_TESTER_GET_PRIVATE (tester);

	tester->priv->blacklist = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 g_free,
							 g_free);

	tester->priv->whitelist = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 g_free,
							 g_free);
	load_patterns (tester);
}

static void
ad_uri_tester_finalize (GObject *object)
{
	AdUriTesterPrivate *priv = AD_URI_TESTER_GET_PRIVATE (AD_URI_TESTER (object));

	LOG ("AdUriTester finalizing %p", object);

	g_hash_table_destroy (priv->blacklist);
	g_hash_table_destroy (priv->whitelist);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ad_uri_tester_class_init (AdUriTesterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ad_uri_tester_finalize;

	g_type_class_add_private (object_class, sizeof (AdUriTesterPrivate));
}
