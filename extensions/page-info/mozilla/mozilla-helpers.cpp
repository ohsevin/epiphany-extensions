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
#include <EphyUtils.h>

#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API
#include <nsCOMPtr.h>
#include <nsICacheEntryDescriptor.h>
#include <nsICacheService.h>
#include <nsICacheSession.h>
#include <nsIDOMDocument.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLInputElement.h>
#include <nsIDOMHTMLAreaElement.h>
#include <nsIDOMHTMLCollection.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMHTMLFormElement.h>
#include <nsIDOMHTMLImageElement.h>
#include <nsIDOMHTMLLinkElement.h>
#include <nsIDOMHTMLFrameElement.h>
#include <nsIDOMHTMLIFrameElement.h>
#include <nsIDOMLocation.h>
#include <nsIDOMNode.h>
#include <nsIDOMNodeList.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMNSHTMLDocument.h>
#include <nsIDOMWindow.h>
#include <nsMemory.h>
#include <nsNetCID.h>
#include <nsIServiceManagerUtils.h>
#include <nsIURI.h>
#include <nsIWebBrowser.h>
#include <nsTime.h>
#include <nsIHTMLDocument.h>
#include <nsCompatibility.h>
#include <nsIDOMTreeWalker.h>
#include <nsIDOMDocumentTraversal.h>
#include <nsIDOMNodeFilter.h>
#include <nsIDOMCSSStyleDeclaration.h>
#include <nsIDOMDocumentView.h>
#include <nsIDOMAbstractView.h>
#include <nsIDOMViewCSS.h>
#include <nsIURI.h>
#include <nsIDOM3Node.h>
#include <nsIDocCharset.h>
#include <nsIInterfaceRequestorUtils.h>

#ifdef ALLOW_PRIVATE_API
#include <nsITextToSubURI.h>
#endif

static char *
embed_string_to_c_string (const nsEmbedString& embed_string)
{
	nsEmbedCString c_string;
	NS_UTF16ToCString (embed_string, NS_CSTRING_ENCODING_UTF8, c_string);

	return g_strdup (c_string.get());
}

extern "C" void
mozilla_free_page_properties (EmbedPageProperties *props)
{
	g_free (props->content_type);
	g_free (props->encoding);
	g_free (props->referring_url);
	g_free (props);
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
	EmbedPageProperties *props;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> domWindow;
	browser->GetContentDOMWindow (getter_AddRefs (domWindow));
	NS_ENSURE_TRUE (domWindow, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	domWindow->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	nsCOMPtr<nsIDOMNSDocument> domNSDoc = do_QueryInterface (doc);
	NS_ENSURE_TRUE (domNSDoc, NULL);

	nsEmbedString value;

	props = g_new0 (EmbedPageProperties, 1);

	nsresult rv;
	rv = domNSDoc->GetLastModified (value);
	NS_ENSURE_SUCCESS (rv, props);

	char *c_time = embed_string_to_c_string (value);
	nsTime last_modified (c_time, PR_TRUE);
	LL_DIV (props->modification_time,
		NS_STATIC_CAST(PRTime, last_modified), PR_USEC_PER_SEC);
	g_free (c_time);

	rv = domNSDoc->GetContentType (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->content_type = embed_string_to_c_string (value);

	rv = domNSDoc->GetCharacterSet (value);
	NS_ENSURE_SUCCESS (rv, props;);
	props->encoding = embed_string_to_c_string (value);

	/* Might not work on XUL pages */
	nsCOMPtr<nsIDOMHTMLDocument> domHtmlDoc = do_QueryInterface (doc);
	if (domHtmlDoc)
	{
		domHtmlDoc->GetReferrer (value);
		if (value.Length())
		{
			props->referring_url = embed_string_to_c_string (value);
		}
	}

	/* Might not work on XUL pages */
	nsCOMPtr<nsIHTMLDocument> htmlDoc = do_QueryInterface (doc);
	if (htmlDoc)
	{
		props->rendering_mode =
			(EmbedPageRenderMode) htmlDoc->GetCompatibilityMode();
	}

	/* Get the URL so we can look in the cache for this page */
	nsCOMPtr<nsIDOMLocation> domLocation;
	domNSDoc->GetLocation (getter_AddRefs (domLocation));
	NS_ENSURE_TRUE (domLocation, props);

	nsEmbedString url;
	domLocation->ToString (url);

	nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;
	char *c_url = embed_string_to_c_string (url);
	get_cache_entry_descriptor (c_url, getter_AddRefs (cacheEntryDescriptor));
	g_free (c_url);

	if (cacheEntryDescriptor)
	{
		PRUint32 expiry = 0, dataSize = 0;
		char *source = nsnull;

		cacheEntryDescriptor->GetExpirationTime (&expiry);
		cacheEntryDescriptor->GetDataSize (&dataSize);
		cacheEntryDescriptor->GetDeviceID (&source);
		NS_ENSURE_TRUE (source, props);

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

static void
mozilla_free_embed_page_image (EmbedPageImage *image)
{
	g_free (image->url);
	g_free (image->alt);
	g_free (image->title);
	g_free (image);
}

static void
process_image_node (nsIDOMHTMLImageElement *element, GHashTable *hash, GList **ret)
{
	nsresult rv;
	EmbedPageImage *image = NULL;

	nsEmbedString tmp;
	rv = element->GetSrc(tmp);
	if (NS_SUCCEEDED(rv) && tmp.Length ())
	{
		char *c = embed_string_to_c_string (tmp);

		if (g_hash_table_lookup(hash, c)) { g_free (c); return; }

		image = g_new0 (EmbedPageImage, 1);
		image->url = c;
		g_hash_table_insert(hash, image->url, GINT_TO_POINTER(TRUE));

		rv = element->GetAlt(tmp);
		if (NS_SUCCEEDED(rv))
		{
			image->alt = embed_string_to_c_string (tmp);
		}
		rv = element->GetTitle(tmp);
		if (NS_SUCCEEDED(rv))
		{
			image->title = embed_string_to_c_string (tmp);
		}
		rv = element->GetWidth(&(image->width));
		rv = element->GetHeight(&(image->height));

		if (image)
		{
			*ret = g_list_prepend(*ret, image);
		}
	}
}

static nsresult
mozilla_get_absolute_url (nsIDOMDocument *doc, const nsAString &relurl, nsACString &absurl)
{
	nsresult rv;

	nsCOMPtr<nsIDOM3Node> node(do_QueryInterface (doc));
	nsEmbedString spec;
	node->GetBaseURI (spec);

	nsCOMPtr<nsIURI> base;
	rv = EphyUtils::NewURI (getter_AddRefs(base), spec);
	if (NS_SUCCEEDED(rv))
	{
		nsEmbedCString crelurl;
		NS_UTF16ToCString (relurl, NS_CSTRING_ENCODING_UTF8, crelurl);	

		rv = base->Resolve (crelurl, absurl);
	}

	return rv;
}

static void
process_input_node (nsIDOMHTMLInputElement *element, 
		    nsIDOMDocument *doc,
		    GHashTable *hash, 
		    GList **ret)
{
	/* We are searching for input of type image only */
	nsresult rv;
	nsEmbedString tmp;
	rv = element->GetType (tmp);
	if (NS_SUCCEEDED(rv))
	{
		nsEmbedCString c_tmp;
		NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, c_tmp);

		/* Is it an image ? */
		if (g_ascii_strcasecmp (c_tmp.get (), "image") > 0) return;

		/* Get url */
		rv = element->GetSrc (tmp);
		if (!NS_SUCCEEDED(rv) || tmp.Length () == 0) return;

		char *c_url = embed_string_to_c_string (tmp);

		/* Relative url ? */
		if (c_url && c_url[0] == '/')
		{
			rv = mozilla_get_absolute_url (doc, tmp, c_tmp);
			if (NS_SUCCEEDED(rv) && c_tmp.Length ())
			{
				g_free (c_url);
				c_url = strdup (c_tmp.get ());
			}
		}

		/* is already listed ? */		
		if (g_hash_table_lookup(hash, c_url)) return;

		EmbedPageImage *image = g_new0 (EmbedPageImage, 1);

		image->url = c_url;

		rv = element->GetAlt(tmp);
		if (NS_SUCCEEDED(rv))
		{
			image->alt = embed_string_to_c_string (tmp);
		}

		*ret = g_list_prepend (*ret, image);

		g_hash_table_insert(hash, image->url, GINT_TO_POINTER(TRUE));
	}
}

static void
mozilla_free_embed_page_link (EmbedPageLink *link)
{
	g_free (link->url);
	g_free (link->title);
	g_free (link->rel);
	g_free (link);
}

static nsresult 
mozilla_unescape (const nsACString &aEscaped, 
                  nsACString &aUnescaped, 
                  const nsACString &encoding)
{
	nsresult rv;

	if (!aEscaped.Length()) return NS_ERROR_FAILURE;

	nsCOMPtr<nsITextToSubURI> escaper
		(do_CreateInstance ("@mozilla.org/intl/texttosuburi;1"));
	NS_ENSURE_TRUE (escaper, NS_ERROR_FAILURE);

	nsEmbedString unescaped;
	rv = escaper->UnEscapeNonAsciiURI (encoding, aEscaped, unescaped);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && unescaped.Length(), NS_ERROR_FAILURE);

	NS_UTF16ToCString (unescaped, NS_CSTRING_ENCODING_UTF8, aUnescaped);

	return NS_OK;
}

template <class T>
static nsresult
process_link_node (nsIDOMNode *node,
		   GHashTable *hash,
		   GList **links,
		   GList **imgs,
		   const nsEmbedCString &encoding)
{
	nsresult rv;

	nsCOMPtr<T> element (do_QueryInterface(node));
	NS_ENSURE_TRUE (element, NS_ERROR_FAILURE);

	nsEmbedString tmp;

	/* HREF */
	rv = element->GetHref(tmp);
	NS_ENSURE_SUCCESS (rv, rv);

	/* Check for mailto scheme */
	nsCOMPtr<nsIURI> uri;
	rv = EphyUtils::NewURI (getter_AddRefs (uri), tmp);
	NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && uri, NS_ERROR_FAILURE);

	PRBool isMailto = PR_FALSE;
	rv = uri->SchemeIs ("mailto", &isMailto);

	char* url = NULL;

	/* We need to unescape mailto link */
	if (isMailto)
	{
		nsEmbedCString unescapedHref, escapedHref;

		NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, escapedHref);

		rv = mozilla_unescape (escapedHref, unescapedHref, encoding);
		if (NS_SUCCEEDED (rv) && unescapedHref.Length())
		{
			url = strdup (unescapedHref.get ());
		}
		else
		{
			url = embed_string_to_c_string (tmp);
		}
	}
	else
	{
		url = embed_string_to_c_string (tmp);
	}

	/* REL */	
	rv = element->GetRel(tmp);
	NS_ENSURE_SUCCESS (rv, rv);

	char *rel = NULL;

	if (tmp.Length())
	{
		rel = embed_string_to_c_string (tmp);
	}
	if (!rel)
	{
		rv = element->GetRev(tmp);
		NS_ENSURE_SUCCESS (rv, rv);
		if (tmp.Length())
		{
			rel = embed_string_to_c_string (tmp);
		}
	}
	else
	{
		/* Special case : rel = icon */
		if (strcmp (rel, "icon") == 0)
		{
			EmbedPageImage *image = g_new0 (EmbedPageImage, 1);

			image->url = url;

			*imgs = g_list_prepend (*imgs, image);

			return NS_OK;
		}
	}

	/* This is really a link */

	/* Already listed ? */
	if (g_hash_table_lookup(hash, url)) return NS_OK;

	g_hash_table_insert(hash, url, GINT_TO_POINTER(TRUE));

	EmbedPageLink *link = g_new0 (EmbedPageLink, 1);

	link->url = url;
	link->rel = rel;

	/* TITLE */
	rv = element->GetTitle(tmp);
	NS_ENSURE_SUCCESS (rv, rv);
	if (tmp.Length())
	{
		link->title = embed_string_to_c_string (tmp);
	}

	if (isMailto)
		/* The list is reversed on end to get the mailto links on head */
		*links = g_list_append(*links, link);
	else
		*links = g_list_prepend(*links, link);

	return NS_OK;
}

static void
process_area_node (nsIDOMHTMLAreaElement *node,
		   GHashTable *hash,
		   GList **links)
{
	nsresult rv;

	nsEmbedString tmp;

	/* HREF */
	rv = node->GetHref(tmp);
	if (!NS_SUCCEEDED (rv) || tmp.Length() == 0) return;

	char* url = embed_string_to_c_string (tmp);

	/* Already listed ? */
	if (g_hash_table_lookup(hash, url)) return;

	g_hash_table_insert(hash, url, GINT_TO_POINTER(TRUE));

	EmbedPageLink *link = g_new0 (EmbedPageLink, 1);

	link->url = url;

	/* TITLE */
	rv = node->GetTitle(tmp);
	if (NS_SUCCEEDED (rv) && tmp.Length() == 0)
	{
		link->title = embed_string_to_c_string (tmp);
	}

	*links = g_list_prepend(*links, link);
}

static void
mozilla_free_embed_page_form (EmbedPageForm *form)
{
	g_free (form->name);
	g_free (form->method);
	g_free (form->action);
	g_free (form);
}

static void
process_form_node (nsIDOMHTMLFormElement *element,
		   GList **ret)
{
	nsresult rv;
	EmbedPageForm *form = g_new0 (EmbedPageForm, 1);

	nsEmbedString tmp;

	rv = element->GetAction(tmp);
	if (NS_SUCCEEDED(rv) && tmp.Length())
	{
		/*
		   nsCOMPtr<nsIDocument> document;
		   document = do_QueryInterface(doc, &rv);
		   if (NS_FAILED(rv))
		   {
		   g_free(form);
		   continue;
		   }

		   nsIURI *uri = document->GetDocumentURI();

		   const nsACString &s = NS_ConvertUCS2toUTF8(tmp);
		   nsCAutoString c;
		   rv = uri->Resolve(s, c);

		   form->action = c.Length() ?
		   g_strdup (c.get()) :
		   g_strdup (PromiseFlatCString(s).get());
		 */
		form->action = embed_string_to_c_string (tmp);
	}

	rv = element->GetName(tmp);
	if (NS_SUCCEEDED(rv) && tmp.Length())
	{
		form->name = embed_string_to_c_string (tmp);
	}

	rv = element->GetMethod(tmp);
	if (NS_SUCCEEDED(rv) && tmp.Length())
	{
		form->method = embed_string_to_c_string (tmp);
	}

	*ret = g_list_prepend (*ret, form);
}

static nsresult 
mozilla_get_encoding (nsIWebBrowser *browser, nsACString &encoding)
{
	nsCOMPtr<nsIDocCharset> docCharset = do_GetInterface (browser);
	NS_ENSURE_TRUE (docCharset, NS_ERROR_FAILURE);
	char *charset;
	docCharset->GetCharset (&charset);
	encoding = charset;
	nsMemory::Free (charset);

	return NS_OK;
}

static void
mozilla_walk_tree (nsIDOMDocument *doc, 
		   const nsEmbedCString &encoding,
		   EmbedPageInfo *page_info,
		   GHashTable *img_hash,
		   GHashTable *lnk_hash)
{
	nsresult rv;

	nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(doc);
	if (htmlDoc == NULL) return;

	nsCOMPtr<nsIDOMTreeWalker> walker;
	nsCOMPtr<nsIDOMDocumentTraversal> trav = do_QueryInterface(doc, &rv);
	if (trav == NULL) return;

	rv = trav->CreateTreeWalker(htmlDoc, 
			nsIDOMNodeFilter::SHOW_ELEMENT, 
			nsnull, 
			PR_TRUE, 
			getter_AddRefs(walker));
	if (!NS_SUCCEEDED(rv)) return;

	nsCOMPtr<nsIDOMCSSStyleDeclaration> computedStyle;
	nsCOMPtr<nsIDOMDocumentView> docView = do_QueryInterface(doc);
	nsCOMPtr<nsIDOMViewCSS> defaultCSSView = nsnull;
	if (docView) 
	{
		nsCOMPtr<nsIDOMAbstractView> defaultView;
		docView->GetDefaultView(getter_AddRefs(defaultView));
		defaultCSSView = do_QueryInterface(defaultView);
	}

	nsCOMPtr<nsIDOMNode> aNode;
	walker->GetCurrentNode(getter_AddRefs(aNode));
	while (aNode)
	{
		nsCOMPtr<nsIDOMHTMLElement> nodeAsElement = do_QueryInterface(aNode);
		if (nodeAsElement)
		{
			if (defaultCSSView)
			{
				nsEmbedString EmptyStr;
				defaultCSSView->GetComputedStyle(nodeAsElement,
						EmptyStr,
						getter_AddRefs(computedStyle));
			}

			if (computedStyle)
			{
				nsEmbedString bgr;
				const PRUnichar bgimage[] = {'b', 'a', 'c', 'k', 'g', 
					'r', 'o', 'u', 'n', 'd', 
					'-', 'i', 'm', 'a', 'g', 'e', '\0'};
				computedStyle->GetPropertyValue(
						nsEmbedString(bgimage), 
						bgr);
				nsEmbedCString cValue;
				NS_UTF16ToCString (bgr, NS_CSTRING_ENCODING_UTF8, cValue);
				if (!g_ascii_strcasecmp (cValue.get(), "none") == 0)
				{
					/* Format is url(http://...) */
					gchar **tab = g_strsplit_set (cValue.get(), "()", 0);

					/* FIXME : should use g_strv_length() with glib 2.5.x */
					guint counter = 0;
					while (tab[counter] != NULL) counter++;

					if (counter > 2 && !g_hash_table_lookup(img_hash, tab[1]))
					{
						EmbedPageImage *image = g_new0 (EmbedPageImage, 1);

						/* Sorry, only the url is available */	
						image->url = g_strdup (tab[1]);

						page_info->images = g_list_prepend(page_info->images, image);

						g_hash_table_insert(img_hash, 
								    image->url, 
								    GINT_TO_POINTER(TRUE));
					}
					g_strfreev (tab);
				}
			}
		}

		nsCOMPtr<nsIDOMHTMLImageElement> nodeAsImage = do_QueryInterface(aNode);
		if (nodeAsImage)
		{
			process_image_node (nodeAsImage, img_hash, &page_info->images);
		}

		nsCOMPtr<nsIDOMHTMLLinkElement> nodeAsLink = do_QueryInterface(aNode);
		if (nodeAsLink)
		{
			process_link_node<nsIDOMHTMLLinkElement>(
					nodeAsLink, 
					lnk_hash, 
					&page_info->links,
					&page_info->images,
					encoding);
		}

		nsCOMPtr<nsIDOMHTMLAnchorElement> nodeAsAnchor = do_QueryInterface(aNode);
		if (nodeAsAnchor)
		{
			process_link_node<nsIDOMHTMLAnchorElement>(
					nodeAsAnchor, 
					lnk_hash, 
					&page_info->links,
					&page_info->images,
					encoding);
		}

		nsCOMPtr<nsIDOMHTMLAreaElement> nodeAsArea = do_QueryInterface(aNode);
		if (nodeAsArea)
		{
			process_area_node (nodeAsArea, lnk_hash, &page_info->links);
		}

		nsCOMPtr<nsIDOMHTMLFormElement> nodeAsForm = do_QueryInterface(aNode);
		if (nodeAsForm)
		{
			process_form_node(nodeAsForm, &page_info->forms);
		}

		nsCOMPtr<nsIDOMHTMLInputElement> nodeAsInput = do_QueryInterface(aNode);
		if (nodeAsInput)
		{
			process_input_node(nodeAsInput, doc, img_hash, &page_info->images);
		}

		nsCOMPtr<nsIDOMHTMLFrameElement> nodeAsFrame = do_QueryInterface(aNode);
		if (nodeAsFrame)
		{
			nsCOMPtr<nsIDOMDocument> frameDoc;
			nodeAsFrame->GetContentDocument (getter_AddRefs (frameDoc));
			if (frameDoc)
			{
				mozilla_walk_tree (frameDoc, encoding, page_info, img_hash, lnk_hash);
			}
		}

		nsCOMPtr<nsIDOMHTMLIFrameElement> nodeAsIFrame = do_QueryInterface(aNode);
		if (nodeAsIFrame)
		{
			nsCOMPtr<nsIDOMDocument> frameDoc;
			nodeAsIFrame->GetContentDocument (getter_AddRefs (frameDoc));
			if (frameDoc)
			{
				mozilla_walk_tree (frameDoc, encoding, page_info, img_hash, lnk_hash);
			}
		}

		walker->NextNode(getter_AddRefs(aNode));
	}
}

extern "C" EmbedPageInfo *
mozilla_get_page_info (EphyEmbed *embed)
{
	nsresult rv;
	GHashTable *img_hash = g_hash_table_new(g_str_hash, g_str_equal);
	GHashTable *lnk_hash = g_hash_table_new(g_str_hash, g_str_equal);
	EmbedPageInfo *page_info = g_new0 (EmbedPageInfo, 1);

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
			getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	/* Get charset */
	nsEmbedCString encoding;
	rv = mozilla_get_encoding (browser, encoding);
	if (!NS_SUCCEEDED(rv)) return NULL;

	nsCOMPtr<nsIDOMWindow> dom_window;
	browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_TRUE (dom_window, NULL);

	nsCOMPtr<nsIDOMDocument> doc;
	dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_TRUE (doc, NULL);

	mozilla_walk_tree (doc, encoding, page_info, img_hash, lnk_hash);

	g_hash_table_destroy (img_hash);
	g_hash_table_destroy (lnk_hash);

	/* Ensure that mailto links are on head */
	page_info->links = g_list_reverse (page_info->links);

	return page_info;
}

extern "C" void
mozilla_free_embed_page_info (EmbedPageInfo *page_info)
{
	g_list_foreach (page_info->forms, (GFunc) mozilla_free_embed_page_form, NULL);
	g_list_free (page_info->forms);

	g_list_foreach (page_info->links, (GFunc) mozilla_free_embed_page_link, NULL);
	g_list_free (page_info->links);

	g_list_foreach (page_info->images, (GFunc) mozilla_free_embed_page_image, NULL);
	g_list_free (page_info->images);

	g_free (page_info);
}
