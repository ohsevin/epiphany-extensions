/*
 *  Copyright (C) 2000 Marco Pesenti Gritti
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

#include "error-viewer.h"

#include "ErrorViewerConsoleListener.h"
#include <nsCOMPtr.h>
#include <xpconnect/nsIScriptError.h>
#include <nsString.h>

#include <glib/gi18n-lib.h>

/* Implementation file */
NS_IMPL_ISUPPORTS1(ErrorViewerConsoleListener, nsIConsoleListener)

ErrorViewerConsoleListener::ErrorViewerConsoleListener()
{
  NS_INIT_ISUPPORTS();
  /* member initializers and constructor code */
}

ErrorViewerConsoleListener::~ErrorViewerConsoleListener()
{
  /* destructor code */
}

/* void observe (in nsIConsoleMessage aMessage); */
NS_IMETHODIMP ErrorViewerConsoleListener::Observe(nsIConsoleMessage *aMessage)
{
	PRUnichar *utmp;
	char *msg;
	ErrorViewerErrorType error_type = ERROR_VIEWER_ERROR;
	ErrorViewer *dialog;

	g_return_val_if_fail (IS_ERROR_VIEWER (this->dialog),
					       NS_ERROR_FAILURE);

	dialog = ERROR_VIEWER (this->dialog);

	aMessage->GetMessage (&utmp);

	msg = g_strchomp (g_strdup (NS_ConvertUCS2toUTF8(utmp).get()));
	nsMemory::Free (utmp);

	nsCOMPtr<nsIScriptError> js_error = do_QueryInterface (aMessage);
	if (js_error)
	{
		PRUint32 flags;
		char *t;
		char *source_name = NULL;
		char *category = NULL;
		PRUint32 line_number;

		js_error->GetFlags (&flags);
		if (flags == nsIScriptError::errorFlag ||
		    flags == nsIScriptError::exceptionFlag ||
		    flags == nsIScriptError::strictFlag)
		{
			error_type = ERROR_VIEWER_ERROR;
		}
		else if (flags == nsIScriptError::warningFlag)
		{
			error_type = ERROR_VIEWER_WARNING;
		}
		else
		{
			error_type = ERROR_VIEWER_INFO;
		}

		js_error->GetSourceName (&utmp);
		if (utmp)
		{
			source_name = g_strdup (NS_ConvertUCS2toUTF8(utmp).get());

			nsMemory::Free (utmp);
		}

		js_error->GetLineNumber (&line_number);

		if (source_name)
		{
			t = msg;

			msg = g_strdup_printf (_("Error in %s, Line %d:\n%s"),
					       source_name,
					       line_number,
					       t);

			g_free (t);
		}
	}

	error_viewer_append (dialog, error_type, msg);

	g_free (msg);

	return NS_OK;
}
