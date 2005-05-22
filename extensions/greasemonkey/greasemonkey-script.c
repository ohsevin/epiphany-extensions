/*
 *  Copyright (C) 2005 Adam Hooper
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

#include "greasemonkey-script.h"
#include "ephy-debug.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <pcre.h>

#include <string.h>

#define GREASEMONKEY_SCRIPT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_GREASEMONKEY_SCRIPT, GreasemonkeyScriptPrivate))

struct _GreasemonkeyScriptPrivate
{
	char *filename;
	char *script;
	GList *include;
	GList *exclude;
};

enum
{
	PROP_0,
	PROP_FILENAME,
	PROP_SCRIPT
};

typedef struct
{
	char *str;
	pcre *re;
} UrlMatcher;

static GObjectClass *parent_class = NULL;

static GType type = 0;

static gint
matcher_url_cmp (const UrlMatcher *matcher,
		 const char *url)
{
	int res;

	res = pcre_exec (matcher->re, NULL, url, strlen (url), 0,
			 PCRE_NO_UTF8_CHECK, NULL, 0);

	if (res >= 0)
	{
		return 0;
	}

	return 1;
}

gboolean
greasemonkey_script_applies_to_url (GreasemonkeyScript *gs,
				    const char *url)
{
	GList *found;

	found = g_list_find_custom (gs->priv->include, url,
				    (GCompareFunc) matcher_url_cmp);

	if (found == NULL)
	{
		return FALSE;
	}

	found = g_list_find_custom (gs->priv->exclude, url,
				    (GCompareFunc) matcher_url_cmp);

	if (found == NULL)
	{
		return TRUE;
	}

	return FALSE;
}

static void
greasemonkey_script_get_property (GObject *object,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *pspec)
{
	GreasemonkeyScript *gs = GREASEMONKEY_SCRIPT (object);

	switch (prop_id)
	{
		case PROP_FILENAME:
			g_value_set_string (value, gs->priv->filename);
			break;
		case PROP_SCRIPT:
			g_value_set_string (value, gs->priv->script);
			break;
		default:
			g_return_if_reached ();
	}
}

static void
greasemonkey_script_set_property (GObject *object,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *pspec)
{
	GreasemonkeyScript *gs = GREASEMONKEY_SCRIPT (object);

	switch (prop_id)
	{
		case PROP_FILENAME:
			gs->priv->filename = g_strdup
				(g_value_get_string (value));
			break;
		default:
			g_return_if_reached ();
	}
}

static void
greasemonkey_script_init (GreasemonkeyScript *gs)
{
	gs->priv = GREASEMONKEY_SCRIPT_GET_PRIVATE (gs);

	LOG ("GreasemonkeyScript initialising");
}

GreasemonkeyScript *
greasemonkey_script_new (const char *filename)
{
	return g_object_new (TYPE_GREASEMONKEY_SCRIPT,
			     "filename", filename,
			     NULL);
}

/*
 * Returns a GList of char *'s
 *
 * For example, find_tag_values (script, "include") will return a list of all
 * [x] in "// @include [x]" in the header part of a Greasemonkey script.
 */
static GList *
find_tag_values (const char *script,
		 const char *tag)
{
	GList *ret;
	const char *pos;
	const char *end_tags;
	const char *begin_line;
	const char *end_line;
	char *commented_tag;

	pos = strstr (script, "// ==UserScript==");
	g_return_val_if_fail (pos != NULL, NULL);

	end_tags = strstr (pos, "// ==/UserScript==");
	g_return_val_if_fail (end_tags != NULL, NULL);

	commented_tag = g_strdup_printf ("// @%s", tag);

	ret = NULL;

	while (TRUE)
	{
		begin_line = strstr (pos, commented_tag);
		if (begin_line == NULL || begin_line > end_tags)
		{
			break;
		}

		begin_line += strlen (commented_tag);

		end_line = strstr (begin_line, "\n");
		if (end_line == NULL || end_line > end_tags)
		{
			break;
		}

		pos = end_line;

		while (begin_line < end_line && g_ascii_isspace (*begin_line))
		{
			begin_line++;
		}

		if (begin_line == end_line)
		{
			continue;
		}

		ret = g_list_prepend (ret, g_strndup (begin_line,
						      end_line - begin_line));
	}

	g_free (commented_tag);

	return ret;
}

static pcre *
build_preg (const char *s)
{
	GString *preg_str;
	pcre *preg;
	const char *err;
	int erroffset;

	preg_str = g_string_new (NULL);

	/* Copied from Greasemonkey's copy from AdBlock */
	/* FIXME: Handle ".tld" */
	for (; *s; s++)
	{
		/* TODO: handle Unicode */
		if (g_ascii_isspace (*s))
		{
			/* Strip spaces from URLs */
			continue;
		}

		switch (*s)
		{
			case '*':
				g_string_append (preg_str, ".*");
				break;
			case '.':
			case '?':
			case '^':
			case '$':
			case '+':
			case '{':
			case '[':
			case '|':
			case '(':
			case ')':
			case ']':
			case '\\':
				g_string_append_c (preg_str, '\\');
				g_string_append_c (preg_str, *s);
				break;
			default:
				g_string_append_c (preg_str, *s);
				break;
		}
	}

	LOG ("Matching against %s", preg_str->str);

	preg = pcre_compile (preg_str->str, PCRE_UTF8, &err, &erroffset, NULL);

	if (preg == NULL)
	{
		g_warning ("Could not compile expression \"%s\"\n"
			   "Error at column %d: %s",
			   preg_str->str, erroffset, err);
	}

	g_string_free (preg_str, TRUE);

	return preg;
}

static GList *
matchers_for_patterns (const GList *patterns)
{
	GList *ret;
	pcre *re;
	UrlMatcher *matcher;

	ret = NULL;

	while (patterns != NULL)
	{
		re = build_preg (patterns->data);
		if (re == NULL)
		{
			continue;
		}

		matcher = g_new (UrlMatcher, 1);
		matcher->str = g_strdup (patterns->data);
		matcher->re = re;

		ret = g_list_prepend (ret, matcher);

		patterns = patterns->next;
	}

	return ret;
}

static void
load_script_file (GreasemonkeyScript *gs)
{
	gboolean success;
	GList *patterns;

	g_return_if_fail (gs->priv->filename != NULL);

	success = g_file_get_contents (gs->priv->filename,
				       &gs->priv->script,
				       NULL, NULL);
	g_return_if_fail (success);

	patterns = find_tag_values (gs->priv->script, "include");
	gs->priv->include = matchers_for_patterns (patterns);
	g_list_foreach (patterns, (GFunc) g_free, NULL);
	g_list_free (patterns);

	patterns = find_tag_values (gs->priv->script, "exclude");
	gs->priv->exclude = matchers_for_patterns (patterns);
	g_list_foreach (patterns, (GFunc) g_free, NULL);
	g_list_free (patterns);
}

static GObject *
greasemonkey_script_constructor (GType type,
				 guint n_construct_properties,
				 GObjectConstructParam *construct_properties)
{
	GObject *object;
	GreasemonkeyScript *gs;

	object = parent_class->constructor (type, n_construct_properties,
					    construct_properties);
	gs = GREASEMONKEY_SCRIPT (object);

	load_script_file (gs);
	g_return_val_if_fail (gs->priv->script != NULL, NULL);

	return object;
}

static void
url_matcher_free (UrlMatcher *matcher)
{
	g_free (matcher->str);
	g_free (matcher->re);
	g_free (matcher);
}

static void
greasemonkey_script_finalize (GObject *object)
{
	GreasemonkeyScript *gs = GREASEMONKEY_SCRIPT (object);

	LOG ("GreasemonkeyScript finalising");

	g_free (gs->priv->filename);
	g_free (gs->priv->script);

	g_list_foreach (gs->priv->include, (GFunc) url_matcher_free, NULL);
	g_list_free (gs->priv->include);
	g_list_foreach (gs->priv->exclude, (GFunc) url_matcher_free, NULL);
	g_list_free (gs->priv->exclude);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
greasemonkey_script_class_init (GreasemonkeyScriptClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->get_property = greasemonkey_script_get_property;
	object_class->set_property = greasemonkey_script_set_property;
	object_class->constructor = greasemonkey_script_constructor;
	object_class->finalize = greasemonkey_script_finalize;

	g_object_class_install_property
		(object_class,
		 PROP_FILENAME,
		 g_param_spec_string ("filename",
				      "Filename",
				      "Filename of script",
				      NULL,
				      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property
		(object_class,
		 PROP_SCRIPT,
		 g_param_spec_string ("script",
				      "Script",
				      "Script contents",
				      NULL,
				      G_PARAM_READABLE));

	g_type_class_add_private (object_class, sizeof (GreasemonkeyScriptPrivate));
}

GType
greasemonkey_script_get_type (void)
{
	return type;
}

GType
greasemonkey_script_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (GreasemonkeyScriptClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) greasemonkey_script_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (GreasemonkeyScript),
		0, /* n_preallocs */
		(GInstanceInitFunc) greasemonkey_script_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "GreasemonkeyScript",
					    &our_info, 0);

	return type;
}
