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

#include "mozilla-config.h"

#include "config.h"

#include "error-viewer.h"

#include "ErrorViewerConsoleListener.h"

#undef MOZILLA_INTERNAL_API
#include <nsEmbedString.h>
#define MOZILLA_INTERNAL_API 1
#include <nsCOMPtr.h>
#include <nsMemory.h>

#include <xpconnect/nsIScriptError.h>

#include <glib/gi18n-lib.h>

#include <string.h>

#include "ephy-debug.h"

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

	nsEmbedString message;
#ifdef MOZ_NSISCRIPTERROR_NSASTRING_
	rv = aError->GetErrorMessage (message);
#else
	PRUnichar *msg = nsnull;
	rv = aError->GetMessage (&msg);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && msg, NS_ERROR_FAILURE);
	message.Assign (msg);
	nsMemory::Free (msg);
#endif
	NS_ENSURE_SUCCESS (rv, rv);

	nsEmbedCString cMessage;
	NS_UTF16ToCString (nsEmbedString (message),
				NS_CSTRING_ENCODING_UTF8, cMessage);

	/*
	 * FIXME: file bug to document all of these in
	 * http://lxr.mozilla.org/seamonkey/source/js/src/xpconnect/idl/nsIScriptError.idl
	 *
	 * With line and/or col # info:
	 * "xbl javascript" (nsXBLDocumentInfo.cpp:)
	 * "chrome javascript" and "content javascript" (nsJSEnvironment.cpp)
	 * "CSS Parser" (nsCSSScanner.cpp)
	 * "DOM::HTML" (nsDOMClassInfo.cpp: line # only)
	 * "XBL Content Sink" (nsXBLContentSink.cpp: line # only)
	 *
	 * Without line and col # info:
	 * "HTML" (nsFormSubmission.cpp)
	 * "XUL Document" (nsXULDocument.cpp)
	 * "ImageMap" (nsImageMap.cpp: has source line info)
	 * "CSS Loader" (nsCSSLoader.cpp)
	 * "XForms" (nsXFormsUtils.cpp)
	 */
	/* FIXME FIXME FIXME */
	if (strstr (category, "javascript") == NULL &&
	    strcmp (category, "CSS Parser") != 0 &&
	    strcmp (category, "DOM::HTML") != 0 &&
	    strcmp (category, "XBL Content Sink") != 0)
	{
		/* Don't bother looking for source lines -- they're not there */
		*aMessage = g_strdup_printf (_("Error:\n%s"), cMessage.get());

		nsMemory::Free (category);

		return NS_OK;
	}

	PRUint32 lineNumber;
	rv = aError->GetLineNumber (&lineNumber);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

#ifdef MOZ_NSISCRIPTERROR_NSASTRING_
	nsEmbedString sourceName;
	rv = aError->GetSourceName (sourceName);
	NS_ENSURE_SUCCESS (rv, rv);

	nsEmbedCString cSourceName;
	NS_UTF16ToCString (sourceName, NS_CSTRING_ENCODING_UTF8, cSourceName);
#else
	PRUnichar *sourceName = nsnull;
	rv = aError->GetSourceName (&sourceName);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && sourceName, NS_ERROR_FAILURE);

	nsEmbedCString cSourceName;
	NS_UTF16ToCString (nsEmbedString (sourceName),
			   NS_CSTRING_ENCODING_UTF8, cSourceName);
	nsMemory::Free (sourceName);
#endif

	*aMessage = g_strdup_printf (
			_("Javascript error in %s on line %d:\n%s"),
			cSourceName.get(), lineNumber, cMessage.get());

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

		nsEmbedCString cMessage;
		NS_UTF16ToCString (nsEmbedString (message),
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
