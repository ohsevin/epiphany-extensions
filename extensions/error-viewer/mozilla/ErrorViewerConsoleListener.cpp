/*
 *  Copyright (C) 2000 Marco Pesenti Gritti
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
 */

#include "error-viewer.h"

#include "ErrorViewerConsoleListener.h"
#include <nsCOMPtr.h>
/*
#include <xpconnect/nsIScriptError.h>
*/
#include <nsString.h>

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
	PRUint32 itmp;
	GString *code_ref;
	gchar *msg;
	gchar *error_message;
	ErrorViewer *dialog;

	g_return_val_if_fail (IS_ERROR_VIEWER (this->dialog),
					       NS_ERROR_FAILURE);

	dialog = ERROR_VIEWER (this->dialog);

	aMessage->GetMessage (&utmp);

	msg = g_strchomp (g_strdup (NS_ConvertUCS2toUTF8(utmp).get()));
	nsMemory::Free (utmp);

	error_viewer_append (dialog, ERROR_VIEWER_ERROR, msg);

	g_free (msg);

	return NS_OK;

	/* The following needs cleanup */
#if 0
	nsCOMPtr<nsIScriptError> error = do_QueryInterface (aMessage);
	if (error) /* best interface, more infos */
	{
		/* build a message */
		PRUint32 flags;
		/*
		JSConsoleMessageType type;
		error->GetFlags (&flags);

		if (flags == nsIScriptError::errorFlag ||
		    flags == nsIScriptError::exceptionFlag ||
		    flags == nsIScriptError::strictFlag)
		{
			type = JS_CONSOLE_ERROR;
		}
		else if (flags == nsIScriptError::warningFlag)
		{
			type = JS_CONSOLE_WARNING;
		}
		else
		{
			type = JS_CONSOLE_MESSAGE;
		}
		*/

		code_ref = g_string_new(NULL);

		error->GetSourceName (&utmp);
		if (utmp) 
		{
			char *tmp;
			tmp = g_strchomp (g_strdup (NS_ConvertUCS2toUTF8(utmp).get()));

			if (strcmp(tmp,"") != 0)
			{
				code_ref = g_string_append (code_ref, "In ");
				code_ref = g_string_append (code_ref, tmp);
				code_ref = g_string_append (code_ref, ", ");
			}

			g_free (tmp);
			nsMemory::Free (utmp);
		}

		error->GetLineNumber (&itmp);
		if (itmp)
		{
			/* FIXME: i18n */
			char *tmp;
			tmp = g_strdup_printf ("%d", itmp);
			code_ref = g_string_append (code_ref, "Line ");
			code_ref = g_string_append (code_ref, tmp);
			code_ref = g_string_append (code_ref, ", ");
			g_free (tmp);
		}

		error->GetSourceLine (&utmp);
		if (utmp) 
		{
			char *tmp;
			tmp = g_strchomp (g_strdup (NS_ConvertUCS2toUTF8(utmp).get()));

			code_ref = g_string_append (code_ref, tmp);
			if (strlen (tmp) > 0)
				code_ref = g_string_append (code_ref, "\n");
			g_free (tmp);
			nsMemory::Free (utmp);
		}
		
		error->GetColumnNumber (&itmp);
		error_message = g_strconcat (msg, 
					     "\n",
					     code_ref->str,
					     NULL);

		error_viewer_append (dialog,
				     error_message);

		g_free (error_message);

		g_string_free (code_ref, TRUE);
	}
	else
	{
		error_viewer_append (dialog, msg);
	}

	g_free (msg);

	return NS_OK;
#endif
}
