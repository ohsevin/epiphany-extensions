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
 * *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mozilla-helpers.h"

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API
#include <nsCOMPtr.h>
#include <nsIChannel.h>
#include <nsIHttpChannel.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentType.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>

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

	nsresult rv;
	nsEmbedString name;
	rv = doctype->GetPublicId (name);
	NS_ENSURE_SUCCESS (rv, NULL);

	nsEmbedCString cName;
	NS_UTF16ToCString (name, NS_CSTRING_ENCODING_UTF8, cName);

	return g_strdup (cName.get());
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

	nsresult rv;
	nsEmbedString contentType;
	rv = ns_doc->GetContentType (contentType);

	nsEmbedCString cType;
	NS_UTF16ToCString (contentType, NS_CSTRING_ENCODING_UTF8, cType);

	return g_strdup (cType.get());
}
