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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ad-blocker.h"

#include <sys/types.h>
#include <regex.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#define AD_BLOCKER_PATTERN_FILE "adblock-patterns"
#define BUFFER_SIZE		1024

#define AD_BLOCKER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_AD_BLOCKER, AdBlockerPrivate))

static void ad_blocker_class_init (AdBlockerClass *klass);
static void ad_blocker_init (AdBlocker *dialog);

static GObjectClass *parent_class = NULL;

static GType type = 0;

struct _AdBlockerPrivate
{
	EphyEmbedSingle *embed_single;
	guint signal;
	GHashTable *patterns;
};

enum
{
	PROP_0,
	PROP_EMBED_SINGLE
};

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
ad_blocker_new (EphyEmbedSingle *embed_single)
{
	return g_object_new (TYPE_AD_BLOCKER,
			     "embed-single", embed_single,
			     NULL);
}

static void
handle_reg_error (const regex_t *preg,
		  int err)
{
	size_t len;
	char *s;

	len = regerror (err, preg, NULL, 0);
	s = g_malloc (len);

	regerror (err, preg, s, len);

	g_warning ("%s", s);

	g_free (s);
}

static gboolean
match_uri (const char *pattern,
	   const regex_t *preg,
	   const char *uri)
{
	int ret;

	ret = regexec (preg, uri, 0, NULL, 0);

	if (ret == 0)
	{
		LOG ("Blocking '%s' with pattern '%s'", uri, pattern)

		return TRUE;
	}

	if (ret != REG_NOMATCH)
	{
		handle_reg_error (preg, ret);
	}

	return FALSE;
}

static gboolean
ad_blocker_test_uri (AdBlocker *blocker,
		     EphyContentCheckType type,
		     char *uri,
		     char *referrer,
		     char *mime_type,
		     EphyEmbedSingle *single)
{
	const char *pattern;

	pattern = g_hash_table_find (blocker->priv->patterns,
				     (GHRFunc) match_uri, uri);

	return pattern != NULL;
}

static void
load_patterns_file (GHashTable *patterns,
		    const char *filename)
{
	char *contents;
	char **lines;
	char **t;
	char *line;
	regex_t *preg;
	int err;

	if (!g_file_get_contents (filename, &contents, NULL, NULL))
	{
		g_warning ("Could not read from file '%s'", filename);
		return;
	}

	t = lines = g_strsplit (contents, "\n", 0);

	while (TRUE)
	{
		line = *t++;
		if (line == NULL) break;

		if (*line == '#') continue; /* comment */

		g_strstrip (line);

		if (*line == '\0') continue; /* empty line */

		if (g_utf8_validate (line, -1, NULL) == FALSE)
		{
			g_warning ("Invalid UTF-8 data in adblock input: %s",
				   line);
			continue;
		}

		preg = g_malloc (sizeof (regex_t));

		err = regcomp (preg, line, REG_EXTENDED | REG_NOSUB);

		if (err != 0)
		{
			handle_reg_error (preg, err);
			regfree (preg);
			g_free (preg);
			continue;
		}

		g_hash_table_insert (patterns, g_strdup (line), preg);
	}

	g_strfreev (lines);
	g_free (contents);
}

static void
load_patterns (AdBlocker *blocker)
{
	char *filename;

	filename = g_build_filename (ephy_dot_dir (), "extensions", "data",
				     AD_BLOCKER_PATTERN_FILE,
				     NULL);
	if (g_file_test (filename, G_FILE_TEST_IS_REGULAR) == FALSE)
	{
		g_free (filename);

		filename = g_build_filename (SHARE_DIR,
					     AD_BLOCKER_PATTERN_FILE, NULL);

		if (g_file_test (filename, G_FILE_TEST_IS_REGULAR) == FALSE)
		{
			g_warning ("Could not find " AD_BLOCKER_PATTERN_FILE);
			g_free (filename);
			return;
		}
	}

	load_patterns_file (blocker->priv->patterns, filename);

	g_free (filename);
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
		case PROP_EMBED_SINGLE:
			blocker->priv->embed_single = g_value_get_object (value);
			break;
	}
}

static void
free_regex (gpointer preg)
{
	if (preg != NULL)
	{
		regfree (preg);
		g_free (preg);
	}
}

static void
ad_blocker_init (AdBlocker *blocker)
{
	LOG ("AdBlocker initializing %p", blocker)

	blocker->priv = AD_BLOCKER_GET_PRIVATE (blocker);

	blocker->priv->patterns = g_hash_table_new_full (g_str_hash,
							 g_str_equal,
							 g_free,
							 free_regex);
	load_patterns (blocker);
}

static GObject *
ad_blocker_constructor (GType type,
			guint n_construct_properties,
			GObjectConstructParam *construct_params)
{
	GObject *object;
	AdBlocker *blocker;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_params);

	blocker = AD_BLOCKER (object);

	blocker->priv->signal =
		g_signal_connect_object (blocker->priv->embed_single,
					 "check-content",
					 (GCallback) ad_blocker_test_uri,
					 blocker, G_CONNECT_SWAPPED);

	return object;
}

static void
ad_blocker_finalize (GObject *object)
{
	AdBlockerPrivate *priv = AD_BLOCKER_GET_PRIVATE (AD_BLOCKER (object));

	LOG ("AdBlocker finalizing %p", object)

	if (g_signal_handler_is_connected (priv->embed_single, priv->signal))
	{
		g_signal_handler_disconnect (priv->embed_single, priv->signal);
	}

	g_hash_table_destroy (priv->patterns);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ad_blocker_class_init (AdBlockerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ad_blocker_finalize;
	object_class->constructor = ad_blocker_constructor;
	object_class->get_property = ad_blocker_get_property;
	object_class->set_property = ad_blocker_set_property;

	g_object_class_install_property
		(object_class,
		 PROP_EMBED_SINGLE,
		 g_param_spec_object ("embed-single",
				      "Embed Single",
				      "Embed Single",
				      G_TYPE_OBJECT,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (AdBlockerPrivate));
}
