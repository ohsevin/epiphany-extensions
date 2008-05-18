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

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>
#include <nsAutoPtr.h>
#include <nsCOMPtr.h>
#include <nsComponentManagerUtils.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLCollection.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMNode.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>
#include <nsNetCID.h>

#include "ErrorViewerURICheckerObserver.h"

#include "mozilla-link-checker.h"

extern "C" void
mozilla_check_links (LinkChecker *checker,
		     EphyEmbed *embed)
{
	nsresult rv;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (gtk_bin_get_child (GTK_BIN (embed))),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, );

	nsCOMPtr<nsIDOMWindow> dom_window;
	rv = browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_SUCCESS (rv, );

	nsCOMPtr<nsIDOMDocument> doc;
	rv = dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_SUCCESS (rv, );

	nsCOMPtr<nsIDOMHTMLDocument> html (do_QueryInterface (doc));
	NS_ENSURE_TRUE (html, );

	nsCOMPtr<nsIDOMHTMLCollection> links;
	rv = html->GetLinks (getter_AddRefs (links));
	NS_ENSURE_SUCCESS (rv, );

	nsRefPtr<ErrorViewerURICheckerObserver> observer (new ErrorViewerURICheckerObserver);
	NS_ENSURE_TRUE (observer, );

	char *url = ephy_embed_get_location (embed, FALSE);
	observer->Init (checker, url);
	g_free (url);

	PRUint32 numLinks;
	rv = links->GetLength (&numLinks);
	NS_ENSURE_SUCCESS (rv, );

	for (PRUint32 i = 0; i < numLinks; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = links->Item (i, getter_AddRefs (node));
		if (NS_FAILED (rv)) continue;

		observer->AddNode (node);
	}

	observer->DoneAdding ();
}
