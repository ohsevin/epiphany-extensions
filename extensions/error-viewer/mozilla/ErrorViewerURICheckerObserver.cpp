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
	link_checker_unuse (mChecker);
	g_object_unref (mChecker);
}

nsresult ErrorViewerURICheckerObserver::Init (LinkChecker *aChecker, const char *aFilename)
{
	g_return_val_if_fail (IS_LINK_CHECKER (aChecker), NS_ERROR_FAILURE);

	mChecker = aChecker;
	mFilename = g_strdup (aFilename);

	g_object_ref (mChecker);
	link_checker_use (mChecker);

	return NS_OK;
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

		link_checker_append (mChecker, ERROR_VIEWER_ERROR, msg);

		g_free (msg);

		mNumLinksInvalid++;
	}

	mNumLinksChecked++;

	link_checker_update_progress (mChecker,
				      mFilename, mNumLinksChecked,
				      mNumLinksInvalid, mNumLinksTotal);

	return NS_OK;
}
