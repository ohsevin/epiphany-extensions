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

#include <OpenSP/config.h> // Necessary for multi-byte support
#include <OpenSP/ParserEventGeneratorKit.h>

#include "error-viewer.h"

#include <iostream>

using namespace std;

string toString (SGMLApplication::CharString s)
{
	string r = "";

	for (size_t i = 0; i < s.len; i++)
	{
		r += char(s.ptr[i]);
	}

	return r;
}

class HtmlErrorFinder : public SGMLApplication
{
public:
	ErrorViewer *error_viewer;

	void error (const ErrorEvent &err)
	{
		ErrorViewerErrorType error_type;
		const char *message;

		if (!this->error_viewer) return;
		
		switch (err.type)
		{
			case err.info:
				error_type = ERROR_VIEWER_INFO;
				break;
			case err.warning:
				error_type = ERROR_VIEWER_WARNING;
				break;
			default:
				error_type = ERROR_VIEWER_ERROR;
		}

		message = toString(err.message).c_str();

		error_viewer_append (this->error_viewer, error_type, message);
	}
};

extern "C" void
validate (const char *filename,
	  ErrorViewer *error_viewer)
{
	ParserEventGeneratorKit parserKit;
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "valid");
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "non-sgml-char-ref");
	parserKit.setOption (ParserEventGeneratorKit::enableWarning,
			     "no-duplicate");
	// TODO: Figure out when it's XML
	parserKit.setOption (ParserEventGeneratorKit::enableWarning, "xml");

	EventGenerator *egp = parserKit.makeEventGenerator (1, (char* const*) &filename);
	egp->inhibitMessages (true);

	HtmlErrorFinder app;
	app.error_viewer = error_viewer;

	egp->run (app);

	delete egp;
}
