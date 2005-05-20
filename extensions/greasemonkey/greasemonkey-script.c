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
	pcre *include_sites;
};

enum
{
	PROP_0,
	PROP_FILENAME,
	PROP_SCRIPT
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

gboolean
greasemonkey_script_applies_to_url (GreasemonkeyScript *gs,
				    const char *url)
{
	int res;

	g_return_val_if_fail (gs->priv->include_sites != NULL, FALSE);

	LOG ("Trying to match %s", url);

	res = pcre_exec (gs->priv->include_sites, NULL, url, strlen (url),
			 0, PCRE_NO_UTF8_CHECK, NULL, 0);

	if (res >= 0)
	{
		LOG ("Applying '%s' to page '%s'", gs->priv->filename, url);
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

static char *
find_include_sites (const char *script)
{
	const char *begin_tags;
	const char *end_tags;
	const char *begin_include;
	const char *end_include;

	begin_tags = strstr (script, "// ==UserScript==");
	g_return_val_if_fail (begin_tags != NULL, NULL);

	end_tags = strstr (begin_tags, "// ==/UserScript==");
	g_return_val_if_fail (end_tags != NULL, NULL);

	begin_include = strstr (begin_tags, "// @include");
	g_return_val_if_fail (begin_include != NULL, NULL);
	g_return_val_if_fail (begin_include < end_tags, NULL);
	begin_include += 11;

	end_include = strstr (begin_include, "\n");
	g_return_val_if_fail (end_include != NULL, NULL);
	g_return_val_if_fail (end_include < end_tags, NULL);

	return g_strndup (begin_include, end_include - begin_include);
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

static void
load_script_file (GreasemonkeyScript *gs)
{
	gboolean success;
	char *include_sites;

	g_return_if_fail (gs->priv->filename != NULL);

	success = g_file_get_contents (gs->priv->filename,
				       &gs->priv->script,
				       NULL, NULL);
	g_return_if_fail (success);

	include_sites = find_include_sites (gs->priv->script);
	/* FIXME: this *will* occur, yet the user must find out somehow */
	g_return_if_fail (include_sites != NULL);

	gs->priv->include_sites = build_preg (include_sites);
	g_return_if_fail (gs->priv->include_sites != NULL);
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
greasemonkey_script_finalize (GObject *object)
{
	GreasemonkeyScript *gs = GREASEMONKEY_SCRIPT (object);

	LOG ("GreasemonkeyScript finalising");

	g_free (gs->priv->filename);
	g_free (gs->priv->script);
	g_free (gs->priv->include_sites);

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
