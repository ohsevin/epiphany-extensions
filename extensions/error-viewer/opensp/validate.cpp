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
	char utf8_buf[7];
	int len;

	for (size_t i = 0; i < s.len; i++)
	{
		len = g_unichar_to_utf8 ((gunichar) s.ptr[i], utf8_buf);

		utf8_buf[len] = 0;

		r += utf8_buf;
	}

	return r;
}

class HtmlErrorFinder : public SGMLApplication
{
public:
	HtmlErrorFinder (SgmlValidator *validator, const char *location);
	~HtmlErrorFinder (void);
	void error (const ErrorEvent &err);

private:
	void handle_line (const char *msg);
	regex_t *err_regex;
	SgmlValidator *validator;
	const char *location;
};

HtmlErrorFinder::HtmlErrorFinder (SgmlValidator *validator,
				  const char *location)
{
	int regex_ret;

	g_return_if_fail (IS_SGML_VALIDATOR (validator));
	g_return_if_fail (location != NULL);

	g_object_ref (validator);
	this->validator = validator;
	this->location = location;

	this->err_regex = g_new0 (regex_t, 1);
	regex_ret = regcomp (this->err_regex,
			     "^[^:]+:([0-9]+):[0-9]+:([A-Z]?):? (.*)$",
			     REG_EXTENDED);
	if (regex_ret != 0)
	{
		g_warning ("Could not compile HTML error regex. "
			   "This is bad.\n");
		g_free (this->err_regex);
		this->err_regex = NULL;
	}
}

HtmlErrorFinder::~HtmlErrorFinder (void)
{
	g_object_unref (this->validator);

	if (this->err_regex)
	{
		regfree (this->err_regex);
	}
}

void
HtmlErrorFinder::handle_line (const char *msg)
{
	ErrorViewerErrorType error_type;
	char *line_number;
	char *verbose_msg;
	const char *description;

	regmatch_t matches[4];
	int regex_ret;

	g_return_if_fail (IS_SGML_VALIDATOR (this->validator));
	g_return_if_fail (this->err_regex != NULL);

	regex_ret = regexec (err_regex, msg, G_N_ELEMENTS (matches), matches,
			     0);
	if (regex_ret != 0)
	{
		g_warning ("Could not parse OpenSP string.: %s\n", msg);
		sgml_validator_append (this->validator,
				       ERROR_VIEWER_ERROR,
				       msg);
		return;
	}

	line_number = g_strndup (msg + matches[1].rm_so,
				 matches[1].rm_eo - matches[1].rm_so);

	switch (*(msg + matches[2].rm_so))
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

	description = msg + matches[3].rm_so;

	verbose_msg =
		g_strdup_printf (_("HTML error in %s on line %s:\n%s"),
				 this->location, line_number, description);

	sgml_validator_append (this->validator, error_type, verbose_msg);

	g_free (line_number);
	g_free (verbose_msg);
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
	  SgmlValidator *validator,
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

	EventGenerator *egp = parserKit.makeEventGenerator
		(1, (char* const*) &filename);

	egp->inhibitMessages (true);

	HtmlErrorFinder *app = new HtmlErrorFinder (validator, location);

	num_errors = egp->run (*app);

	delete egp;
	delete app;

	return num_errors;
}
