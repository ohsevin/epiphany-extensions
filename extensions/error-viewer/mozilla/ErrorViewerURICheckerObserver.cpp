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

#include "error-viewer.h"
#include "link-checker.h"

#include "ErrorViewerURICheckerObserver.h"

#include <nsCOMPtr.h>
#include <nsIChannel.h>
#include <nsIURI.h>
#include <nsIURIChecker.h>
#include <nsNetError.h>
#include <nsString.h>

#include <glib/gi18n-lib.h>

/* Implementation file */
NS_IMPL_ISUPPORTS1(ErrorViewerURICheckerObserver, nsIRequestObserver)

ErrorViewerURICheckerObserver::ErrorViewerURICheckerObserver()
{
	  /* member initializers and constructor code */
	mNumLinksChecked = 0;
	mNumLinksInvalid = 0;
}
                                                                                
ErrorViewerURICheckerObserver::~ErrorViewerURICheckerObserver()
{
	  /* destructor code */
}
                                                                                
/* void onStartRequest (in nsIRequest aRequest, in nsISupports aContext); */
NS_IMETHODIMP ErrorViewerURICheckerObserver::OnStartRequest(nsIRequest *aRequest, nsISupports *aContext)
{
	return NS_OK;
}
                                                                                
/* void onStopRequest (in nsIRequest aRequest, in nsISupports aContext, in nsresult aStatusCode); */
NS_IMETHODIMP ErrorViewerURICheckerObserver::OnStopRequest(nsIRequest *aRequest, nsISupports *aContext, nsresult aStatusCode)
{
	if (aStatusCode == NS_BINDING_SUCCEEDED)
	{
	}
	else
	{
		nsresult rv;

		nsCAutoString uri;
		rv = aRequest->GetName(uri);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

		char *msg;

		msg = g_strdup_printf (
			_("Link error in %s:\n%s is unavailable."),
			mFilename, uri.get());

		link_checker_append (LINK_CHECKER (mChecker),
				     ERROR_VIEWER_ERROR, msg);

		g_free (msg);

		mNumLinksInvalid++;
	}

	mNumLinksChecked++;

	link_checker_update_progress (LINK_CHECKER (mChecker),
				      mNumLinksChecked, mNumLinksTotal);

	if (mNumLinksChecked >= mNumLinksTotal)
	{
		char *msg = g_strdup_printf (
			ngettext("Link check of %s complete\n"
				 "Found %d invalid link",
				 "Link check of %s complete\n"
				 "Found %d invalid links",
				 mNumLinksInvalid),
			mFilename, mNumLinksInvalid);

		link_checker_append (LINK_CHECKER (mChecker),
				     ERROR_VIEWER_INFO, msg);

		g_free (msg);
	}

	return NS_OK;
}
