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

#include <stdio.h>
#include <string.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#define AD_BLOCKER_PATTERN_FILE "adblock-patterns"

#define AD_BLOCKER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_AD_BLOCKER, AdBlockerPrivate))

static void ad_blocker_class_init (AdBlockerClass *klass);
static void ad_blocker_init (AdBlocker *dialog);
static void ad_blocker_finalize (GObject *object);

static GObjectClass *parent_class = NULL;

static GType type = 0;

struct AdBlockerPrivate
{
	GSList *patterns;
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
ad_blocker_new (void)
{
	return g_object_new (TYPE_AD_BLOCKER, NULL);
}

static void
ad_blocker_class_init (AdBlockerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ad_blocker_finalize;

	g_type_class_add_private (object_class, sizeof (AdBlockerPrivate));
}

gboolean
ad_blocker_test_uri (AdBlocker *blocker,
		     const char *url)
{
	GSList *l;
	char *rev;
	guint len;
	gboolean ret;

	ret = TRUE;

	len = strlen (url);
	rev = g_utf8_strreverse (url, -1);

	for (l = blocker->priv->patterns; l != NULL; l = l->next)
	{
		if (g_pattern_match (l->data, len, url, rev))
		{
			LOG ("Blocking %s", url)

			ret = FALSE;
			break;
		}
	}

	g_free (rev);

	return ret;
}

static GSList *
load_patterns_file (const char *filename)
{
	GSList *ret = NULL;
	FILE *f;
	char *line = NULL;
	size_t buf_size = 0;

	LOG ("Loading patterns from %s", filename)

	f = fopen (filename, "r");

	g_return_val_if_fail (f != NULL, NULL);

	while (getline (&line, &buf_size, f) != -1)
	{
		if (*line == '#') continue; /* comment */

		g_strstrip (line);

		if (g_utf8_validate (line, -1, NULL) == FALSE)
		{
			g_warning ("Invalid UTF-8 data in adblock input: %s",
				   line);
			continue;
		}

		if (strlen (line) > 0)
		{
			/* TODO: use regex from libegg */
			ret = g_slist_prepend (ret, g_pattern_spec_new (line));
		}
	}

	fclose (f);

	return ret;
}

static GSList *
load_patterns (void)
{
	GSList *ret;
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
			return NULL;
		}
	}

	ret = load_patterns_file (filename);

	g_free (filename);

	return ret;
}

static void
ad_blocker_init (AdBlocker *blocker)
{
	LOG ("AdBlocker initializing %p", blocker)

	blocker->priv = AD_BLOCKER_GET_PRIVATE (blocker);

	blocker->priv->patterns = load_patterns ();
}

static void
ad_blocker_finalize (GObject *object)
{
	AdBlockerPrivate *priv = AD_BLOCKER_GET_PRIVATE (AD_BLOCKER (object));

	LOG ("AdBlocker finalizing %p", object)

	g_slist_foreach (priv->patterns, (GFunc) g_pattern_spec_free, NULL);
	g_slist_free (priv->patterns);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}
