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
 * *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "mozilla-config.h"
#include "config.h"

#ifdef XPCOM_GLUE
#include <nsXPCOMGlue.h>
#include <gtkmozembed_glue.cpp>
#endif

#include <nsStringAPI.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>
#include <nsCOMPtr.h>
#include <nsIChannel.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentType.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMWindow.h>
#include <nsIHttpChannel.h>
#include <nsIWebBrowser.h>

#include "mozilla-helpers.h"

extern "C" char *
mozilla_get_doctype (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (gtk_bin_get_child (GTK_BIN (embed))),
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
	nsString name;
	rv = doctype->GetPublicId (name);
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCString cName;
	NS_UTF16ToCString (name, NS_CSTRING_ENCODING_UTF8, cName);

	return g_strdup (cName.get());
}

extern "C" char *
mozilla_get_content_type (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (gtk_bin_get_child (GTK_BIN (embed))),
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
	nsString contentType;
	rv = ns_doc->GetContentType (contentType);

	nsCString cType;
	NS_UTF16ToCString (contentType, NS_CSTRING_ENCODING_UTF8, cType);

	return g_strdup (cType.get());
}

gboolean
mozilla_glue_startup (void)
{
#ifdef XPCOM_GLUE
	static const GREVersionRange greVersion = {
	  "1.9a", PR_TRUE,
	  "2", PR_TRUE
	};
	char xpcomLocation[4096];

	if (NS_FAILED (GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0, xpcomLocation, sizeof (xpcomLocation))) ||
	    NS_FAILED (XPCOMGlueStartup (xpcomLocation)) ||
	    NS_FAILED (GTKEmbedGlueStartup ()) ||
	    NS_FAILED (GTKEmbedGlueStartupInternal()))
                return FALSE;

        return TRUE;
#else
#error hi there!
        return TRUE;
#endif /* XPCOM_GLUE */
}
