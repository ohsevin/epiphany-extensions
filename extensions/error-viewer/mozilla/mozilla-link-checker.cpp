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

#include "mozilla-link-checker.h"

#include "ErrorViewerURICheckerObserver.h"

#include <gtkembedmoz/gtkmozembed.h>
#include <gtkembedmoz/gtkmozembed_internal.h>

#include <nsCOMPtr.h>
#include <nsIComponentManager.h>
#include <nsIComponentRegistrar.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLCollection.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMNode.h>
#include <nsIDOMWindow.h>
#include <nsIGenericFactory.h>
#include <nsIWebBrowser.h>
#include <nsNetCID.h>

NS_GENERIC_FACTORY_CONSTRUCTOR(ErrorViewerURICheckerObserver)

/* Globals are evil. This code is evil. */
static gboolean is_registered = FALSE;
static nsCOMPtr<nsIGenericFactory> factory = 0;

static const nsModuleComponentInfo sAppComp =
{
	G_ERRORVIEWERURICHECKEROBSERVER_CLASSNAME,
	G_ERRORVIEWERURICHECKEROBSERVER_CID,
	G_ERRORVIEWERURICHECKEROBSERVER_CONTRACTID,
	ErrorViewerURICheckerObserverConstructor
};

extern "C" void
mozilla_register_link_checker_component (void)
{
	nsresult rv;

	g_return_if_fail (is_registered == FALSE);

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = NS_NewGenericFactory (getter_AddRefs (factory),
				   &sAppComp);
	if (NS_FAILED (rv) || !factory)
	{
		g_warning ("Failed to make a factory for %s\n",
			   sAppComp.mDescription);
		g_return_if_fail (NS_SUCCEEDED (rv));
	}

	rv = cr->RegisterFactory (sAppComp.mCID, sAppComp.mDescription,
				  sAppComp.mContractID, factory);
	if (NS_FAILED (rv))
	{
		g_warning ("Failed to register %s\n", sAppComp.mDescription);
		g_return_if_fail (NS_SUCCEEDED (rv));
	}

	is_registered = TRUE;
}

extern "C" void
mozilla_unregister_link_checker_component (void)
{
	nsresult rv;

	g_return_if_fail (is_registered == TRUE);

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = cr->UnregisterFactory (sAppComp.mCID, factory);
	g_return_if_fail (NS_SUCCEEDED (rv));

	is_registered = FALSE;
}

extern "C" void
mozilla_check_links (LinkChecker *checker,
		     EphyEmbed *embed)
{
	nsresult rv;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	g_return_if_fail (browser != NULL);

	nsCOMPtr<nsIDOMWindow> dom_window;
	rv = browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<nsIDOMDocument> doc;
	rv = dom_window->GetDocument (getter_AddRefs (doc));
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<nsIDOMHTMLDocument> html;
	html = do_QueryInterface (doc, &rv);
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<nsIDOMHTMLCollection> links;
	rv = html->GetLinks (getter_AddRefs (links));
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<ErrorViewerURICheckerObserver> observer = do_CreateInstance
		(G_ERRORVIEWERURICHECKEROBSERVER_CONTRACTID);
	char *url = ephy_embed_get_location (embed, FALSE);
	observer->Init (checker, url);
	g_free (url);

	PRUint32 numLinks;
	rv = links->GetLength (&numLinks);
	g_return_if_fail (NS_SUCCEEDED (rv));

	for (PRUint32 i = 0; i < numLinks; i++)
	{
		nsCOMPtr<nsIDOMNode> node;
		rv = links->Item (i, getter_AddRefs (node));
		g_return_if_fail (NS_SUCCEEDED (rv));

		observer->AddNode (node);
	}

	observer->DoneAdding ();
}
