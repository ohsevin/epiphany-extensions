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

#include <string.h>

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

static char *
get_message_from_error (nsIScriptError *ns_error)
{
	char *ret;
	PRUnichar* message;
	char* category;
	PRUnichar* source_name;
	PRUint32 line_number;

	ns_error->GetMessage (&message);

	ns_error->GetCategory (&category);
	/*
	 * No docs on category, but some are listed in:
	 * http://lxr.mozilla.org/seamonkey/source/dom/src/base/nsJSEnvironment.cpp#208
	 *
	 * XUL javascript
	 * content javascript
	 * component javascript
	 *
	 * Some errors are none of the above. GNOME Bugzilla #134438
	 */
	if (strstr (category, "javascript") == NULL)
	{
		/* Don't bother looking for source lines -- they're not there */
		ret = g_strdup_printf (_("Error:\n%s"),
				       NS_ConvertUCS2toUTF8(message).get());

		nsMemory::Free (message);
		g_free (category);

		return ret;
	}

	ns_error->GetLineNumber (&line_number);

	ns_error->GetSourceName (&source_name);
	g_return_val_if_fail (source_name != NULL, NS_OK);

	ret = g_strdup_printf (
		_("Javascript error in %s on line %d:\n%s"),
		NS_ConvertUCS2toUTF8(source_name).get(),
		line_number,
		NS_ConvertUCS2toUTF8(message).get());

	nsMemory::Free (message);
	nsMemory::Free (source_name);
	g_free (category);

	return ret;
}

/* void observe (in nsIConsoleMessage aMessage); */
NS_IMETHODIMP ErrorViewerConsoleListener::Observe(nsIConsoleMessage *aMessage)
{
	nsresult rv;
	PRUint32 flags;
	ErrorViewerErrorType error_type = ERROR_VIEWER_ERROR;
	ErrorViewer *dialog;
	char *msg;

	g_return_val_if_fail (IS_ERROR_VIEWER (this->mDialog),
					       NS_ERROR_FAILURE);

	dialog = ERROR_VIEWER (this->mDialog);

	nsCOMPtr<nsIScriptError> ns_error = do_QueryInterface (aMessage, &rv);
	/* Mozilla at this point will *always* give a nsIScriptError */
	if (NS_FAILED (rv) || aMessage == NULL)
	{
		PRUnichar* ns_message;

		g_warning ("Could not get nsIScriptError");

		aMessage->GetMessage (&ns_message);

		error_viewer_append (dialog, error_type,
				     NS_ConvertUCS2toUTF8(ns_message).get());

		nsMemory::Free (ns_message);

		return NS_OK;
	}

	ns_error->GetFlags (&flags);
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

	msg = get_message_from_error (ns_error);

	error_viewer_append (dialog, error_type, msg);

	g_free (msg);

	return NS_OK;
}
