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

#include "validate.h"

#include <OpenSP/config.h> // Necessary for multi-byte support
#include <OpenSP/ParserEventGeneratorKit.h>

#include "error-viewer.h"

#include <sys/types.h>
#include <regex.h>

#include <glib/gi18n-lib.h>

#include <iostream>

using namespace std;

static string
toString (SGMLApplication::CharString s)
{
	string r = "";

	for (size_t i = 0; i < s.len; i++)
	{
		r += (char) s.ptr[i];
	}

	return r;
}

class HtmlErrorFinder : public SGMLApplication
{
private:
	void handle_line (const char *msg);

public:
	ErrorViewer *error_viewer;
	const char *location;

	void error (const ErrorEvent &err);
};

void
HtmlErrorFinder::handle_line (const char *msg)
{
	ErrorViewerErrorType error_type;
	/* char *source_name; */
	char *line_number;
	char *verbose_msg;
	const char *description;

	regex_t *regex;
	regmatch_t matches[5];
	int regex_ret;

	if (!this->error_viewer) return;

	regex = g_new0 (regex_t, 1);
	regex_ret = regcomp (regex,
			     "([^/:]+):([0-9]+):[0-9]+:([A-Z]?):? (.*)$",
			     REG_EXTENDED);
	g_return_if_fail (regex_ret == 0);

	regex_ret = regexec (regex, msg, G_N_ELEMENTS (matches), matches, 0);
	if (regex_ret != 0)
	{
		g_warning ("Could not parse OpenSP string.: %s\n", msg);
		error_viewer_append (this->error_viewer,
				     ERROR_VIEWER_ERROR,
				     msg);

		regfree (regex);
		return;
	}

	/*
	source_name = g_strndup (msg + matches[1].rm_so,
				 matches[1].rm_eo - matches[1].rm_so);
	*/

	line_number = g_strndup (msg + matches[2].rm_so,
				 matches[2].rm_eo - matches[2].rm_so);

	switch (*(msg + matches[3].rm_so))
	{
		case 'W':
			error_type = ERROR_VIEWER_WARNING;
			break;
		case 'E':
			error_type = ERROR_VIEWER_ERROR;
			break;
		default:
			error_type = ERROR_VIEWER_INFO;
	}

	description = msg + matches[4].rm_so;

	verbose_msg =
		g_strdup_printf (_("HTML error in %s on line %s:\n%s"),
				 this->location, line_number, description);

	error_viewer_append (this->error_viewer, error_type,
			     verbose_msg);

	/* g_free (source_name); */
	g_free (line_number);
	g_free (verbose_msg);
	regfree (regex);
}

void
HtmlErrorFinder::error (const ErrorEvent &err)
{
	/* Don't listen to OpenSP's err.type. Each line comes with
	 * a ":W:" or ":E:" and err.type is redundant. Not to mention,
	 * two-line errors *don't* have a :W: or :E: on the second line
	 * and they *shouldn't* be errors because we want them to be "info"
	 */
	string msg = toString (err.message);

	const char *raw_msg = msg.c_str ();

	char **messages = g_strsplit (raw_msg, "\n", 0);

	char **i;

	for (i = messages; *i; i++)
	{
		if (**i == '\0') continue;

		handle_line (*i);
	}

	g_strfreev (messages);
}

extern "C" unsigned int
validate (const char *filename,
	  const char *location,
	  ErrorViewer *error_viewer,
	  gboolean is_xml)
{
	unsigned int num_errors;

	ParserEventGeneratorKit parserKit;
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "valid");
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "non-sgml-char-ref");
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "no-duplicate");

	if (is_xml)
	{
		parserKit.setOption (ParserEventGeneratorKit::enableWarning,
				     "xml");
	}

	EventGenerator *egp = parserKit.makeEventGenerator (1, (char* const*) &filename);
	egp->inhibitMessages (true);

	HtmlErrorFinder app;
	app.error_viewer = error_viewer;
	app.location = location;

	num_errors = egp->run (app);

	delete egp;

	return num_errors;
}
