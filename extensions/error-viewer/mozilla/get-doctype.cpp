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

#include "get-doctype.h"

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <nsCOMPtr.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentType.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>
#include <nsString.h>

extern "C" char *
mozilla_get_doctype (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMDocumentType> doctype;
	doc->GetDoctype (getter_AddRefs (doctype));
	NS_ENSURE_TRUE (doctype, NULL);

	nsAutoString name;
	doctype->GetPublicId (name);

	return ToNewUTF8String (name);
}

extern "C" char *
mozilla_get_content_type (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMNSDocument> ns_doc = do_QueryInterface (doc);
	NS_ENSURE_TRUE (ns_doc, NULL);

	nsAutoString content_type;

	ns_doc->GetContentType (content_type);

	return ToNewUTF8String (content_type);
}
