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

#include <nsEmbedString.h>
#include <nsIURI.h>

AdBlocker *MozAdBlocker::mBlocker = NULL;

NS_IMPL_ISUPPORTS1(MozAdBlocker, nsIContentPolicy)

MozAdBlocker::MozAdBlocker()
{
}

void
MozAdBlocker::SetAdBlocker (AdBlocker* blocker)
{
	if (mBlocker)
	{
		g_object_unref (mBlocker);
	}
	mBlocker = blocker;
	if (mBlocker)
	{
		g_object_ref (mBlocker);
	}
}

nsresult MozAdBlocker::ShouldLoadURI(nsIURI *uri,
				     PRBool *_retval)
{
	*_retval = PR_TRUE;

	if (mBlocker)
	{
		nsEmbedCString s;
		uri->GetSpec(s);

		if (ad_blocker_test_uri (mBlocker, s.get()) == FALSE)
		{
			*_retval = PR_FALSE;
		}
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
	PRBool accept;

	ShouldLoadURI(aContentLocation, &accept);

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
	return ShouldLoadURI(contentLocation, _retval);
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
