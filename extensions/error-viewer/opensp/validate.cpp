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

#include "validate.h"

#include <OpenSP/config.h> // Necessary for multi-byte support
#include <OpenSP/ParserEventGeneratorKit.h>

#include "error-viewer.h"

#include <sys/types.h>
#include <regex.h>

#include <glib/gi18n-lib.h>

#include <iostream>
#include <string>

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
	HtmlErrorFinder (SgmlValidator *validator, const char *location,
			 const char *filename);
	~HtmlErrorFinder (void);
	void error (const ErrorEvent &err);

private:
	void handle_line (const char *msg);
	regex_t *mErrRegex;
	SgmlValidator *mValidator;
	const char *mLocation;
	const char *mFilename;
};

HtmlErrorFinder::HtmlErrorFinder (SgmlValidator *validator,
				  const char *location,
				  const char *filename)
{
	int regex_ret;

	g_return_if_fail (IS_SGML_VALIDATOR (validator));
	g_return_if_fail (location != NULL);

	g_object_ref (validator);
	this->mValidator = validator;
	this->mLocation = location;
	this->mFilename = filename;

	this->mErrRegex = g_new0 (regex_t, 1);
	regex_ret = regcomp (this->mErrRegex,
			     "^(<URL>)?(.*):([0-9]+):[0-9]+:([A-Z]?):? (.*)$",
			     REG_EXTENDED);
	if (regex_ret != 0)
	{
		g_warning ("Could not compile HTML error regex. "
			   "This is bad.\n");
		g_free (this->mErrRegex);
		this->mErrRegex = NULL;
	}
}

HtmlErrorFinder::~HtmlErrorFinder (void)
{
	g_object_unref (this->mValidator);

	if (this->mErrRegex)
	{
		regfree (this->mErrRegex);
		g_free (this->mErrRegex);
	}
}

void
HtmlErrorFinder::handle_line (const char *msg)
{
	ErrorViewerErrorType error_type;
	char *display_filename;
	char *line_number;
	char *verbose_msg;
	const char *description;

	regmatch_t matches[6];
	int regex_ret;

	g_return_if_fail (IS_SGML_VALIDATOR (this->mValidator));
	g_return_if_fail (this->mErrRegex != NULL);

	regex_ret = regexec (this->mErrRegex, msg, G_N_ELEMENTS (matches),
			     matches, 0);
	if (regex_ret != 0)
	{
		g_warning ("Could not parse OpenSP string: %s\n", msg);
		sgml_validator_append (this->mValidator,
				       ERROR_VIEWER_ERROR,
				       msg);
		return;
	}

	display_filename = g_strndup (msg + matches[2].rm_so,
				      matches[2].rm_eo - matches[2].rm_so);

	/* If the error's in our temp file, display the "true" location */
	if (strcmp (display_filename, this->mFilename) == 0)
	{
		g_free (display_filename);
		display_filename = g_strdup (this->mLocation);
	}

	line_number = g_strndup (msg + matches[3].rm_so,
				 matches[3].rm_eo - matches[3].rm_so);

	switch (*(msg + matches[4].rm_so))
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

	description = msg + matches[5].rm_so;

	verbose_msg =
		g_strdup_printf (_("HTML error in “%s” on line %s:\n%s"),
				 display_filename, line_number, description);

	sgml_validator_append (this->mValidator, error_type, verbose_msg);

	g_free (display_filename);
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

	HtmlErrorFinder *app = new HtmlErrorFinder (validator, location,
						    filename);

	num_errors = egp->run (*app);

	delete egp;
	delete app;

	return num_errors;
}
