/*
 *  Copyright (C) 2004 Adam Hooper
 *
 *  Ripped from GaleonWrapper.cpp, which has no copyright info besides Marco...
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

#include "mozilla-helpers.h"

#include <string.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <nsEmbedString.h>
#include <nsCOMPtr.h>
#include <nsICacheEntryDescriptor.h>
#include <nsICacheService.h>
#include <nsICacheSession.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMLocation.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMNSHTMLDocument.h>
#include <nsIDOMWindow.h>
#include <nsMemory.h>
#include <nsNetCID.h>
#include <nsIServiceManagerUtils.h>
#include <nsIWebBrowser.h>
#include <nsTime.h>

extern "C" void
mozilla_free_page_properties (EmbedPageProperties *props)
{
	g_free (props->content_type);
	g_free (props->encoding);
	g_free (props->referring_url);
	g_free (props);
}

static char *
embed_string_to_c_string (nsEmbedString embed_string)
{
	nsEmbedCString c_string;
	NS_UTF16ToCString (embed_string, NS_CSTRING_ENCODING_UTF8, c_string);

	return g_strdup (c_string.get());
}

static nsresult
get_cache_entry_descriptor (const char *url,
			    nsICacheEntryDescriptor **aCacheEntryDescriptor)
{
	nsresult rv;

        nsCOMPtr<nsICacheService> cacheService =
                do_GetService(NS_CACHESERVICE_CONTRACTID);
        NS_ENSURE_TRUE (cacheService, NS_ERROR_FAILURE);

        const char *cacheTypes[] = { "HTTP", "FTP" };
        for (unsigned int i = 0 ; i < G_N_ELEMENTS (cacheTypes); i++)
        {
                nsCOMPtr<nsICacheSession> cacheSession;
                cacheService->CreateSession(cacheTypes[i],
                                            nsICache::STORE_ANYWHERE,
                                            PR_TRUE,
                                            getter_AddRefs(cacheSession));
                NS_ENSURE_TRUE (cacheSession, NS_ERROR_FAILURE);

                cacheSession->SetDoomEntriesIfExpired(PR_FALSE);

                nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;

                rv = cacheSession->OpenCacheEntry(url,
                                                  nsICache::ACCESS_READ,
						  PR_FALSE,
                                                  aCacheEntryDescriptor);

                if (NS_SUCCEEDED (rv)) return NS_OK;
        }
	*aCacheEntryDescriptor = NULL;
        return NS_ERROR_FAILURE;
}

extern "C" EmbedPageProperties *
mozilla_get_page_properties (EphyEmbed *embed)
{
	nsresult rv;
	EmbedPageProperties *props;

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

	nsEmbedString value;

	props = g_new0 (EmbedPageProperties, 1);

	rv = ns_doc->GetLastModified (value);
	NS_ENSURE_SUCCESS (rv, props);
	char *c_time = embed_string_to_c_string (value);
	nsTime last_modified (c_time, PR_TRUE);
	LL_DIV (props->modification_time,
		NS_STATIC_CAST(PRTime, last_modified), PR_USEC_PER_SEC);
	g_free (c_time);

	rv = ns_doc->GetContentType (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->content_type = embed_string_to_c_string (value);

	rv = ns_doc->GetCharacterSet (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->encoding = embed_string_to_c_string (value);

	/* Might not work on XUL pages */
	nsCOMPtr<nsIDOMHTMLDocument> html_doc = do_QueryInterface (doc);
	if (html_doc)
	{
		rv = html_doc->GetReferrer (value);
		NS_ENSURE_SUCCESS (rv, props);
		if (value.Length())
		{
			props->referring_url = embed_string_to_c_string (value);
		}
	}

	/* Might not work on XUL pages */
	nsCOMPtr<nsIDOMNSHTMLDocument> ns_html_doc = do_QueryInterface (doc);
	if (ns_html_doc)
	{
		rv = ns_html_doc->GetCompatMode (value);
		char *c_value = embed_string_to_c_string (value);
		props->rendering_mode = (strcmp (c_value, "CSS1Compat") == 0 ?
					 EMBED_RENDER_STANDARDS :
					 EMBED_RENDER_QUIRKS);
		g_free (c_value);
	}

	/* Get the URL so we can look in the cache for this page */
	nsCOMPtr<nsIDOMLocation> dom_location;
	rv = ns_doc->GetLocation (getter_AddRefs (dom_location));
	NS_ENSURE_SUCCESS (rv, props);

	nsEmbedString url;
	dom_location->ToString (url);

	nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;
	char *c_url = embed_string_to_c_string (url);
	rv = get_cache_entry_descriptor (c_url,
					 getter_AddRefs (cacheEntryDescriptor));
	g_free (c_url);
	NS_ENSURE_SUCCESS (rv, props);

        if (cacheEntryDescriptor)
        {
                PRUint32 expiry = 0, dataSize = 0;
		char *source;

                cacheEntryDescriptor->GetExpirationTime (&expiry);
                cacheEntryDescriptor->GetDataSize (&dataSize);
                cacheEntryDescriptor->GetDeviceID (&source);

                props->expiration_time = expiry;
                props->size = dataSize;

		if (strcmp (source, "disk") == 0)
                {
                        props->page_source = EMBED_SOURCE_DISK_CACHE;
                }
		else if (strcmp (source, "memory") == 0)
                {
                        props->page_source = EMBED_SOURCE_MEMORY_CACHE;
                }
                else
                {
                        props->page_source = EMBED_SOURCE_UNKNOWN_CACHE;
                }

		nsMemory::Free (source);
        }
        else
        {
                props->page_source = EMBED_SOURCE_NOT_CACHED;
                props->size = -1;
                props->expiration_time = 0;
        }

	return props;
}
