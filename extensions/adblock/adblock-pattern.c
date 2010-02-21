/*
 *  Copyright © 2004 Adam Hooper
 *  Copyright © 2005, 2006 Jean-François Rameau
 *  Copyright © 2009 Xan Lopez <xan@gnome.org>
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
 */

#include "config.h"

#include "adblock-pattern.h"

#include <string.h>
#include <gio/gio.h>

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#define DEFAULT_BLACKLIST_FILENAME	"adblock-patterns"
#define BLACKLIST_FILENAME		"blacklist"
#define WHITELIST_FILENAME		"whitelist"

typedef enum
{
	OP_LOAD,
	OP_SAVE
} OpType;

void adblock_pattern_save (GSList *patterns, AdblockPatternType type);

static void
adblock_pattern_load_from_file (GHashTable *patterns,
				const char *filename)
{
	char *contents;
	char **lines;
	char **t;
	char *line;
	GRegex *regex;
	GError *error = NULL;

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

		regex = g_regex_new (line, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0, &error);

		if (regex == NULL)
		{
			g_warning ("Could not compile expression \"%s\"\n"
				   "Error: %s",
				   line, error->message);
			g_error_free (error);
			continue;
		}

		g_hash_table_insert (patterns, g_strdup (line), regex);
	}

	g_strfreev (lines);
	g_free (contents);
}

static char *
adblock_pattern_filename (AdblockPatternType type, OpType op)
{
	char *filename = NULL;

	switch (type)
	{
		case PATTERN_BLACKLIST:
			filename = g_build_filename (ephy_dot_dir (), "extensions", "data",
						     "adblock", BLACKLIST_FILENAME,
						     NULL);
			break;
		case PATTERN_WHITELIST:
			filename = g_build_filename (ephy_dot_dir (), "extensions", "data",
						     "adblock", WHITELIST_FILENAME,
						     NULL);
			break;
		case PATTERN_DEFAULT_BLACKLIST:
			/* We first try the user's one */
			filename = g_build_filename (ephy_dot_dir (), "extensions", "data",
						     "adblock", DEFAULT_BLACKLIST_FILENAME,
						     NULL);
			if (op == OP_LOAD && !g_file_test (filename, G_FILE_TEST_IS_REGULAR))
			{
				g_free (filename);
				/* Then, try the global one */
				filename = g_build_filename (SHARE_DIR,
							     DEFAULT_BLACKLIST_FILENAME,
							     NULL);
			}
			break;
	}

	return filename;
}

static gboolean
adblock_pattern_foreach_save (const char *pattern,
			      GIOChannel *channel)
{
	GIOStatus status;
	gsize bytes_written;

	status = g_io_channel_write_chars (channel, pattern, -1, &bytes_written, NULL);
	status = g_io_channel_write_chars (channel, "\n", -1, &bytes_written, NULL);

	return FALSE;
}

static char *
adblock_pattern_replace_dot (const char *line)
{
	char **sub;
	char *res;

	g_return_val_if_fail (line != NULL, NULL);

	sub = g_strsplit (line, ".", 0);
	res = g_strjoinv ("\\.", sub);

	g_strfreev (sub);

	return res;
}

static void
adblock_pattern_rewrite_patterns (const char *contents)
{
	char **lines, **t;
	char *line;
	GRegex *regex1, *regex2;
	GError *error = NULL;
	gboolean match;
	GSList *patterns = NULL;

	/* We don't care about some specific rules */
	regex1 = g_regex_new ("^\\[Adblock\\]", 0, 0, &error);
	if (regex1 == NULL)
	{
		g_warning ("Could not compile expression ^\\[Adblock]\n" "Error: %s",
			   error->message);
		g_error_free (error);
		return;
	}
	regex2 = g_regex_new ("^\\!Filterset", 0, 0, &error);
	if (regex2 == NULL)
	{
		g_warning ("Could not compile expression ^\\!Filterset\n" "Error: %s",
			   error->message);
		g_error_free (error);
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

		match = g_regex_match (regex1, line, 0, NULL);

		if (match) continue;

		match = g_regex_match (regex2, line, 0, NULL);

		if (match) continue;

		if (*line == '/')
		{
			*(line + strlen (line) -1) = '\0';
			line += 1;
			patterns = g_slist_prepend (patterns, g_strdup (line));
			continue;
		}
		line = adblock_pattern_replace_dot (line);

		patterns = g_slist_prepend (patterns, g_strdup (line));
	}

	g_strfreev (lines);
	g_regex_unref (regex1);
	g_regex_unref (regex2);

	adblock_pattern_save (patterns, PATTERN_DEFAULT_BLACKLIST);

	g_slist_foreach (patterns, (GFunc)g_free, NULL);
}

static char *
adblock_pattern_get_filterg_patterns (const char *date)
{
	gsize size;
	char *contents = NULL, *url = NULL;
	GFile *file;
	GFileInfo *info;
	GFileInputStream *input_stream;
	gboolean res = TRUE;

	url = g_strdup_printf ("http://www.pierceive.com/filtersetg/%s", date);
	file = g_file_new_for_uri (url);
	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_STANDARD_SIZE,
				  0, 0, NULL);
	if (info == NULL)
	{
		g_warning ("Could not get rules file from filterg site");
		goto out;
	}
	else
	{
		size = (gsize) g_file_info_get_size (info);
		g_object_unref (info);
	}

	/* First, get the changelog so we can build the url pointing to the last rules */
	input_stream = g_file_read (file, NULL, NULL);
	if (input_stream != NULL)
	{
		gsize bytes_read;

		contents = g_malloc (size);
		res = g_input_stream_read_all (G_INPUT_STREAM (input_stream),
					       contents,
					       size,
					       &bytes_read,
					       NULL, NULL);
		if (!res)
		{	
			g_warning ("Could not get rules file from filterg site");
		}
		g_object_unref (input_stream);
	}
	else
	{
		g_warning ("Could not get rules file from filterg site");
	}

out:
	g_free (url);
	g_object_unref (file);

	return contents;
}

static char *
adblock_pattern_get_filterg_date (void)
{
	gsize size;
	char *contents = NULL;
	char **lines;
	char *date = NULL;
	GFile *file;
	GFileInfo *info;
	GFileInputStream *input_stream;
	gboolean res = TRUE;

	file = g_file_new_for_uri ("http://www.pierceive.com/filtersetg/latest.txt");
	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_STANDARD_SIZE,
				  0, 0, NULL);
	if (info == NULL)
	{
		g_warning ("Could not get latest.txt file from filterg site");
		goto out;
	}
	else
	{
		size = (gsize) g_file_info_get_size (info);
		g_object_unref (info);
	}

	/* First, get the changelog so we can build the url pointing to the last rules */
	input_stream = g_file_read (file, NULL, NULL);
	if (input_stream != NULL)
	{
		gsize bytes_read;

		contents = g_malloc (size);
		res = g_input_stream_read_all (G_INPUT_STREAM (input_stream),
					       contents,
					       size,
					       &bytes_read,
					       NULL, NULL);
		if (!res)
		{	
			g_warning ("Could not get latest.txt file from filterg site");
			goto out;
		}
		g_object_unref (input_stream);
		lines = g_strsplit (contents, "\n", 0);
		date = g_strdup (lines [0]);

		g_free (contents);
		g_strfreev (lines);
	}
	else
	{
		g_warning ("Could not get latest.txt file from filterg site");
		goto out;
	}

out:
	g_object_unref (file);

	return date;
}

/* Public */

void
adblock_pattern_load (GHashTable *patterns,
		      AdblockPatternType type)
{
	char *filename = NULL;

	filename = adblock_pattern_filename (type, OP_LOAD);

	if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
	{
		LOG ("Loading patterns from %s", filename);

		adblock_pattern_load_from_file (patterns, filename);
	}
	g_free (filename);
}


void
adblock_pattern_save (GSList *patterns, AdblockPatternType type)
{
	GError *error = NULL;
	char *filename = NULL;

	filename = adblock_pattern_filename (type, OP_SAVE);

	GIOChannel *channel = g_io_channel_new_file (filename, "w", NULL);

	g_slist_foreach (patterns,
			 (GFunc)adblock_pattern_foreach_save,
			 channel);

	g_io_channel_shutdown (channel, TRUE, &error);
}

void
adblock_pattern_get_filtersetg_patterns (void)
{
	char *date, *patterns;

	date = adblock_pattern_get_filterg_date ();
	if (date == NULL)
	{
		g_warning ("Could not get the last update");
		return;
	}

	patterns = adblock_pattern_get_filterg_patterns (date);
	if (patterns == NULL)
	{
		g_warning ("Could not get content from last update");
		return;
	}

	adblock_pattern_rewrite_patterns (patterns);

	g_free (date);
	g_free (patterns);
}
