/*
 *  Copyright © 2000 Marco Pesenti Gritti
 *  Copyright © 2004 Adam Hooper
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

#include "mozilla-config.h"
#include "config.h"

#include <string.h>

#include <glib/gi18n-lib.h>

#include <nsStringAPI.h>

#include <nsCOMPtr.h>
#include <nsMemory.h>
#include <nsIScriptError.h>

#include "ephy-debug.h"
#include "error-viewer.h"

#include "ErrorViewerConsoleListener.h"

NS_IMPL_ISUPPORTS1(ErrorViewerConsoleListener, nsIConsoleListener)

ErrorViewerConsoleListener::ErrorViewerConsoleListener()
{
}

ErrorViewerConsoleListener::~ErrorViewerConsoleListener()
{
	LOG ("ErrorViewerConsoleListener dtor %p", this);
}

nsresult
ErrorViewerConsoleListener::GetMessageFromError (nsIScriptError *aError,
						 char **aMessage)
{
	NS_ENSURE_ARG (aError);
	NS_ENSURE_ARG_POINTER (aMessage);

	nsresult rv;
	char *category = nsnull;
	rv = aError->GetCategory (&category);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && category, NS_ERROR_FAILURE);

	nsString message;
	rv = aError->GetErrorMessage (message);
	NS_ENSURE_SUCCESS (rv, rv);

	nsCString cMessage;
	NS_UTF16ToCString (nsString (message),
				NS_CSTRING_ENCODING_UTF8, cMessage);

#ifdef HAVE_GECKO_1_8
	nsString sourceName;
	rv = aError->GetSourceName (sourceName);
	NS_ENSURE_SUCCESS (rv, rv);

	nsCString cSourceName;
	NS_UTF16ToCString (sourceName, NS_CSTRING_ENCODING_UTF8, cSourceName);
#else
	PRUnichar *sourceName = nsnull;
	rv = aError->GetSourceName (&sourceName);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && sourceName, NS_ERROR_FAILURE);

	nsCString cSourceName;
	NS_UTF16ToCString (nsString (sourceName),
			   NS_CSTRING_ENCODING_UTF8, cSourceName);
	nsMemory::Free (sourceName);
#endif
	
	if (strstr (category, "javascript") || 
		!strcmp (category, "CSS Parser"))
	{
		PRUint32 lineNumber;
		rv = aError->GetLineNumber (&lineNumber);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);
	
		PRUint32 columnNumber;
		rv = aError->GetColumnNumber (&columnNumber);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);
		
		if (lineNumber && columnNumber)
		{
			*aMessage = g_strdup_printf (
					_("%s error in %s on line %d and column %d:\n%s"),
					category, cSourceName.get(), lineNumber, columnNumber, cMessage.get());
		}
		else
		{
			*aMessage = g_strdup_printf (
					_("%s error in %s:\n%s"),
					category, cSourceName.get(), cMessage.get());
		}
	}
	else if (!strcmp (category, "DOM::HTML") ||
			 !strcmp (category, "XBL Content Sink"))
	{
		PRUint32 lineNumber;
		rv = aError->GetLineNumber (&lineNumber);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);
		
		if (lineNumber)
		{
			*aMessage = g_strdup_printf (
					_("%s error in %s on line %d:\n%s"),
					category, cSourceName.get(), lineNumber, cMessage.get());
		}
		else
		{
			*aMessage = g_strdup_printf (
					_("%s error in %s:\n%s"),
					category, cSourceName.get(), cMessage.get());
		}
	}
	else if	(!strcmp (category, "HTML") ||
			 !strcmp (category, "XUL Document") ||
			 !strcmp (category, "ImageMap") ||
			 !strcmp (category, "CSS Loader") ||
			 !strcmp (category, "XForms"))
	{
			*aMessage = g_strdup_printf (
					_("%s error in %s:\n%s"),
				category, cSourceName.get(), cMessage.get());
	}

	nsMemory::Free (category);						 
	
	return NS_OK;
}

/* void observe (in nsIConsoleMessage aMessage); */
NS_IMETHODIMP ErrorViewerConsoleListener::Observe(nsIConsoleMessage *aMessage)
{
	nsresult rv;
	ErrorViewer *dialog;

	NS_ENSURE_ARG (aMessage);

	g_return_val_if_fail (IS_ERROR_VIEWER (this->mDialog),
					       NS_ERROR_FAILURE);

	dialog = ERROR_VIEWER (this->mDialog);

	nsCOMPtr<nsIScriptError> error = do_QueryInterface (aMessage, &rv);
	/* Mozilla at this point will *always* give a nsIScriptError */
	if (NS_FAILED (rv) || !error)
	{
		PRUnichar* message;

		g_warning ("Could not get nsIScriptError");

		rv = aMessage->GetMessage (&message);
		NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && message, NS_ERROR_FAILURE);

		nsCString cMessage;
		NS_UTF16ToCString (nsString (message),
				   NS_CSTRING_ENCODING_UTF8, cMessage);

		error_viewer_append (dialog, ERROR_VIEWER_ERROR, cMessage.get());

		nsMemory::Free (message);

		return NS_OK;
	}

	PRUint32 flags;
	rv = error->GetFlags (&flags);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	ErrorViewerErrorType errorType;
	if (flags == nsIScriptError::errorFlag ||
	    flags == nsIScriptError::exceptionFlag ||
	    flags == nsIScriptError::strictFlag)
	{
		errorType = ERROR_VIEWER_ERROR;
	}
	else if (flags == nsIScriptError::warningFlag)
	{
		errorType = ERROR_VIEWER_WARNING;
	}
	else
	{
		errorType = ERROR_VIEWER_INFO;
	}

	char *msg = nsnull;
	rv = GetMessageFromError (error, &msg);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && msg, NS_ERROR_FAILURE);

	error_viewer_append (dialog, errorType, msg);

	g_free (msg);

	return NS_OK;
}
