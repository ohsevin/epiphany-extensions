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
 */

#include "mozilla-config.h"

#include "config.h"

#include "MozAdBlocker.h"

#include "ad-blocker.h"

#include "EphyUtils.h"

#include <epiphany/ephy-embed.h>

#include <nsCOMPtr.h>
#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API
#include <nsIDOMAbstractView.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentView.h>
#include <nsIDOMNode.h>
#include <nsIDOMWindow.h>
#include <nsIURI.h>

NS_IMPL_ISUPPORTS1(MozAdBlocker, nsIContentPolicy)

static EphyEmbed *
get_embed_from_context (nsISupports *aContext)
{
	GtkWidget *ret;

	/*
	 * aContext is either an nsIDOMWindow, an nsIDOMNode, or NULL. If it's
	 * an nsIDOMNode, we need the nsIDOMWindow to get the EphyEmbed.
	 */
	if (aContext == NULL) return NULL;

	nsCOMPtr<nsIDOMWindow> window;

	nsCOMPtr<nsIDOMNode> node (do_QueryInterface (aContext));
	if (node != NULL)
	{
		nsCOMPtr<nsIDOMDocument> domDocument;

		node->GetOwnerDocument (getter_AddRefs (domDocument));
		if (domDocument == NULL) return NULL; /* resource://... */

		nsCOMPtr<nsIDOMDocumentView> docView = 
			do_QueryInterface (domDocument);
		g_return_val_if_fail (docView != NULL, NULL);

		nsCOMPtr<nsIDOMAbstractView> view;
		
		docView->GetDefaultView (getter_AddRefs (view));
		g_return_val_if_fail (view != NULL, NULL);

		window = do_QueryInterface (view);
	}
	else
	{
		window = do_QueryInterface (aContext);
	}
	g_return_val_if_fail (window != NULL, NULL);

	ret = EphyUtils::FindEmbed (window);
	g_return_val_if_fail (EPHY_IS_EMBED (ret), NULL);

	return EPHY_EMBED (ret);
}

nsresult MozAdBlocker::ShouldLoadURI(nsIURI *uri,
				     nsISupports *aContext,
				     PRUint32 aContentType,
				     PRBool *_retval)
{
	EphyEmbed *embed;
	AdBlocker *blocker;
	AdUriCheckType content_type = (AdUriCheckType) aContentType;

	*_retval = PR_TRUE;

	embed = get_embed_from_context (aContext);
	if (embed == NULL) return NS_OK;

	blocker = AD_BLOCKER
		(g_object_get_data (G_OBJECT (embed), AD_BLOCKER_KEY));
	g_return_val_if_fail (blocker != NULL, NS_OK);

	nsEmbedCString s;
	uri->GetSpec(s);

	if (ad_blocker_test_uri (blocker, s.get(), content_type) == TRUE)
	{
		*_retval = PR_FALSE;
	}

	return NS_OK;
}

#if MOZ_NSICONTENTPOLICY_VARIANT == 2
NS_IMETHODIMP
MozAdBlocker::ShouldLoad(PRUint32 aContentType,
			 nsIURI *aContentLocation,
			 nsIURI *aRequestingLocation,
			 nsISupports *aContext,
			 const nsACString &aMimeTypeGuess,
			 nsISupports *aExtra,
			 PRInt16 *aDecision)
{
	NS_ENSURE_ARG (aContentLocation);

	PRBool accept;

	ShouldLoadURI(aContentLocation, aContext, aContentType, &accept);

	if (accept)
	{
		*aDecision = nsIContentPolicy::ACCEPT;
	}
	else
	{
		*aDecision = nsIContentPolicy::REJECT_REQUEST;
	}

	return NS_OK;
}

NS_IMETHODIMP
MozAdBlocker::ShouldProcess(PRUint32 aContentType,
			    nsIURI *aContentLocation,
			    nsIURI *aRequestingLocation,
			    nsISupports *aContext,
			    const nsACString &aMimeType,
			    nsISupports *aExtra,
			    PRInt16 *aDecision)
{
	*aDecision = nsIContentPolicy::ACCEPT;
	return NS_OK;
}

#else

/* boolean shouldLoad (in PRInt32 contentType, in nsIURI contentLocation, in nsISupports ctxt, in nsIDOMWindow window); */
NS_IMETHODIMP MozAdBlocker::ShouldLoad(PRInt32 contentType,
				       nsIURI *contentLocation,
				       nsISupports *ctxt,
				       nsIDOMWindow *window,
				       PRBool *_retval)
{
	NS_ENSURE_ARG (aContentLocation);

	return ShouldLoadURI(contentLocation, ctxt, _retval);
}

/* boolean shouldProcess (in PRInt32 contentType, in nsIURI documentLocation, in nsISupports ctxt, in nsIDOMWindow window); */
NS_IMETHODIMP MozAdBlocker::ShouldProcess(PRInt32 contentType,
					  nsIURI *documentLocation,
					  nsISupports *ctxt,
					  nsIDOMWindow *window,
					  PRBool *_retval)
{
	*_retval = PR_TRUE;
	return NS_OK;
}
#endif /* MOZ_NSICONTENTPOLICY_VARIANT_2 */
