/*
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

#include <glib/gi18n-lib.h>

#include <nsStringAPI.h>

#include <nsCOMPtr.h>
#include <nsComponentManagerUtils.h>
#include <nsIChannel.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLAreaElement.h>
#include <nsIDOMNode.h>
#include <nsIURIChecker.h>
#include <nsIURI.h>
#include <nsNetError.h>
#include <nsServiceManagerUtils.h>

#include "EphyUtils.h"

#include "link-checker.h"

#include "ErrorViewerURICheckerObserver.h"

NS_IMPL_ISUPPORTS1(ErrorViewerURICheckerObserver, nsIRequestObserver)

ErrorViewerURICheckerObserver::ErrorViewerURICheckerObserver()
  : mNumLinksChecked (0)
  , mNumLinksInvalid (0)
  , mNumLinksTotal (0)
{
}

ErrorViewerURICheckerObserver::~ErrorViewerURICheckerObserver()
{
	if (mNumLinksTotal > 0)
	{
		char *msg, *part1, *part2;

		part1 = g_strdup_printf ("Link check of “%s” complete",
					 mFilename);
		part2 = g_strdup_printf (ngettext ("Found %d invalid link",
						   "Found %d invalid links",
						   mNumLinksInvalid),
					 mNumLinksInvalid);

		msg = g_strconcat (part1, "\n", part2, (char *) NULL);

		link_checker_append (mChecker, ERROR_VIEWER_INFO, msg);

		g_free (msg);
		g_free (part1);
		g_free (part2);
	}

	link_checker_unuse (mChecker);
	g_object_unref (mChecker);
	g_free (mFilename);
}

nsresult ErrorViewerURICheckerObserver::Init (LinkChecker *aChecker, const char *aFilename)
{
	NS_ENSURE_TRUE (IS_LINK_CHECKER (aChecker), NS_ERROR_FAILURE);

	g_object_ref (aChecker);
	link_checker_use (aChecker);
	mChecker = aChecker;

	mFilename = g_strdup (aFilename);

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
	mNumLinksChecked++;

	if (aStatusCode == NS_BINDING_SUCCEEDED)
	{
	}
	else
	{
		nsresult rv;

		nsCString uri;
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

	return NS_OK;
}

nsresult ErrorViewerURICheckerObserver::AddNode (nsIDOMNode *node)
{
	nsresult rv;
	nsString href;

	nsCOMPtr<nsIDOMHTMLAnchorElement> anchor;
	anchor = do_QueryInterface (node, &rv);
	if (NS_SUCCEEDED (rv))
	{
		rv = anchor->GetHref (href);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);
	}
	else
	{
		nsCOMPtr<nsIDOMHTMLAreaElement> area;
		area = do_QueryInterface (node, &rv);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

		rv = area->GetHref (href);
		NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);
	}

	/* Get the URI from the href string. Ignore mailto:, etc URIs */
	nsCOMPtr<nsIURI> uri;
	rv = EphyUtils::NewURI (getter_AddRefs (uri), href);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && uri, NS_ERROR_FAILURE);

	PRBool isHttp, isHttps, isFtp;
	rv = uri->SchemeIs ("http", &isHttp);
	rv |= uri->SchemeIs ("https", &isHttps);
	rv |= uri->SchemeIs ("ftp", &isFtp);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	if (!isHttp && !isHttps && !isFtp) return NS_OK;

	/* Check the URI */
	nsCOMPtr<nsIURIChecker> uriChecker = do_CreateInstance
		("@mozilla.org/network/urichecker;1");
	NS_ENSURE_TRUE (uriChecker, NS_ERROR_FAILURE);

	rv = uriChecker->Init (uri);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	rv = uriChecker->AsyncCheck (this, NULL);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	mNumLinksTotal++;

	return NS_OK;
}

nsresult ErrorViewerURICheckerObserver::DoneAdding (void)
{
	char *msg;

	if (mNumLinksChecked == mNumLinksTotal && mNumLinksTotal > 0)
	{
		/* No links on page */
		return NS_OK;
	}

	if (mNumLinksTotal == 0)
	{
		msg = g_strdup_printf ("No links to check on “%s”", mFilename);
	}
	else
	{
		msg = g_strdup_printf
		        (ngettext("Checking %d Link on “%s”",
		                  "Checking %d Links on “%s”",
		                  mNumLinksTotal),
		         mNumLinksTotal, mFilename);
	}

	link_checker_append (mChecker, ERROR_VIEWER_INFO, msg);

	g_free (msg);

	return NS_OK;
}
