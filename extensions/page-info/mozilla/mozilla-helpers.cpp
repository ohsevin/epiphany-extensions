/*
 *  Copyright (C) 2004 Adam Hooper
 *  Copyright (C) 2004, 2005 Jean-François Rameau
 *  Copyright (C) 2004, 2005 Christian Persch
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

#include "mozilla-config.h"

#include "config.h"

#include "mozilla-helpers.h"
#include "PageInfoPrivate.h"

#include "EphyUtils.h"

#include <string.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#undef MOZILLA_INTERNAL_API
#include <nsEmbedString.h>
#define MOZILLA_INTERNAL_API 1
#include <nsCOMPtr.h>

#include <nsCompatibility.h>
#include <nsICacheEntryDescriptor.h>
#include <nsICacheService.h>
#include <nsICacheSession.h>
#include <nsIDocCharset.h>
#include <nsIDOM3Node.h>
#include <nsIDOMAbstractView.h>
#include <nsIDOMCSSStyleDeclaration.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentTraversal.h>
#include <nsIDOMDocumentView.h>
#include <nsIDOMElement.h>
#include <nsIDOMCSSStyleDeclaration.h>
#include <nsIDOMCSSPrimitiveValue.h>
#include <nsIDOMCSSValue.h>
#include <nsIDOMHTMLAnchorElement.h>
#include <nsIDOMHTMLAppletElement.h>
#include <nsIDOMHTMLAreaElement.h>
#include <nsIDOMHTMLCollection.h>
#include <nsIDOMHTMLDocument.h>
#include <nsIDOMHTMLEmbedElement.h>
#include <nsIDOMHTMLFormElement.h>
#include <nsIDOMHTMLFrameElement.h>
#include <nsIDOMHTMLIFrameElement.h>
#include <nsIDOMHTMLImageElement.h>
#include <nsIDOMHTMLInputElement.h>
#include <nsIDOMHTMLLinkElement.h>
#include <nsIDOMHTMLMetaElement.h>
#include <nsIDOMHTMLObjectElement.h>
#include <nsIDOMLocation.h>
#include <nsIDOMNodeFilter.h>
#include <nsIDOMNode.h>
#include <nsIDOMNodeList.h>
#include <nsIDOMNSDocument.h>
#include <nsIDOMNSHTMLDocument.h>
#include <nsIDOMTreeWalker.h>
#include <nsIDOMViewCSS.h>
#include <nsIDOMWindow.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIPrefBranch.h>
#include <nsIPrefService.h>
#include <nsIServiceManager.h>
#include <nsIURI.h>
#include <nsIWebBrowser.h>
#include <nsMemory.h>
#include <nsNetCID.h>
#include <nsTime.h>

#ifdef ALLOW_PRIVATE_API
#include <nsITextToSubURI.h>
#endif

/* page info helper class */

class PageInfoHelper
{
public:
  PageInfoHelper ();
  ~PageInfoHelper ();

  nsresult Init (EphyEmbed *aEmbed);
  EmbedPageInfo *GetInfo ();

private:
  EmbedPageProperties *GetProperties ();
  char *ToCString (const nsAString &aString);
  nsresult GetCacheEntryDescriptor (const nsAString &aUrl,
                                    nsICacheEntryDescriptor **aEntry);
  void ProcessNode (nsIDOMElement *aElement,
                    nsIDOMHTMLElement *aHTMLElement);
  void ProcessAppletNode (nsIDOMHTMLAppletElement *aElement);
  void ProcessAreaNode (nsIDOMHTMLAreaElement *aElement);
  void PageInfoHelper::ProcessEmbedNodeHelper (const nsEmbedString &aUrl, 
  				               nsIDOMHTMLEmbedElement *aElement);
  void ProcessEmbedNode (nsIDOMHTMLEmbedElement *aElement);
  void ProcessFormNode (nsIDOMHTMLFormElement *aElement);
  void ProcessImageNode (nsIDOMHTMLImageElement *aElement);
  void ProcessMetaNode (nsIDOMHTMLMetaElement *aElement);
  void ProcessInputNode (nsIDOMHTMLInputElement *aElement);
  template <class T> void ProcessLinkNode (nsIDOMNode *aNode);
  void ProcessObjectNode (nsIDOMHTMLObjectElement *aElement);
  void WalkTree (nsIDOMDocument *aDocument);
  void WalkFrame (nsIDOMDocument *aDocument);
  nsresult Resolve (const nsAString &aUrl,
                    nsACString &aResult);
  nsresult Unescape (const nsACString &aEscaped,
                     nsACString &aResult);

  nsCOMPtr<nsIDOMDocument> mDOMDocument;

  nsEmbedString mXLinkNS;
  nsEmbedString mBgImageAttr;
  nsEmbedString mHrefAttr;
  PRBool mJavaEnabled;
  nsCOMPtr<nsITextToSubURI> mTextToSubURI;

  GHashTable *mMediaHash;
  GHashTable *mLinkHash;
  GHashTable *mFormHash;
  GList      *mMetaTagsList;
  
  /* these two are used by the walker: */
  nsEmbedCString mDocCharset;
  nsCOMPtr<nsIURI> mBaseURI;
};

PageInfoHelper::PageInfoHelper ()
{
  mMediaHash     = g_hash_table_new (g_str_hash, g_str_equal);
  mLinkHash      = g_hash_table_new (g_str_hash, g_str_equal);
  mFormHash      = g_hash_table_new (g_str_hash, g_str_equal);
  /* list of (name,content) */
  mMetaTagsList  = NULL;
}

PageInfoHelper::~PageInfoHelper ()
{
  g_hash_table_destroy (mMediaHash);
  g_hash_table_destroy (mLinkHash);
  g_hash_table_destroy (mFormHash);
}

nsresult
PageInfoHelper::Init (EphyEmbed *aEmbed)
{
  NS_ENSURE_ARG (aEmbed);
  
  nsCOMPtr<nsIWebBrowser> browser;
  gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (aEmbed),
                                   getter_AddRefs (browser));
  NS_ENSURE_TRUE (browser, NS_ERROR_FAILURE);

  nsresult rv;
  nsCOMPtr<nsIDOMWindow> domWindow;
  rv = browser->GetContentDOMWindow (getter_AddRefs (domWindow));
  NS_ENSURE_SUCCESS (rv, rv);

  rv = domWindow->GetDocument (getter_AddRefs (mDOMDocument));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIDocCharset> docCharset (do_GetInterface (browser));
  NS_ENSURE_TRUE (docCharset, NS_ERROR_FAILURE);

  char *charset;
  docCharset->GetCharset (&charset);
  nsMemory::Free (charset);

  mJavaEnabled = PR_TRUE;
  nsCOMPtr<nsIPrefService> prefService
          (do_GetService (NS_PREFSERVICE_CONTRACTID, &rv));
  if (prefService)
    {
      nsCOMPtr<nsIPrefBranch> prefBranch;
      prefService->GetBranch ("", getter_AddRefs (prefBranch));
      if (prefBranch)
        {
          prefBranch->GetBoolPref ("security.enable_java", &mJavaEnabled);
        }
    }

  const PRUnichar kXLinkLiteral[] = { 'h', 't', 't', 'p', ':', '/', '/',
                                      'w', 'w', 'w', '.', 'w', '3', '.',
                                      'o', 'r', 'g', '/', '1', '9', '9',
                                      '9', '/', 'x', 'l', 'i', 'n', 'k',
                                      '\0' };
  const PRUnichar kBgImageLiteral[] = { 'b', 'a', 'c', 'k', 'g', 'r', 'o',
                                        'u', 'n', 'd', '-', 'i', 'm', 'a',
                                        'g', 'e', '\0' };
  const PRUnichar kHrefLiteral[] = { 'h', 'r', 'e', 'f', '\0' };

  mXLinkNS.Assign (kXLinkLiteral);
  mBgImageAttr.Assign (kBgImageLiteral);
  mHrefAttr.Assign (kHrefLiteral);

  return NS_OK;
}

static void
make_list (gpointer key,
           gpointer value,
           GList **list)
{
  *list = g_list_prepend (*list, value);
}

EmbedPageInfo *
PageInfoHelper::GetInfo ()
{
  NS_ENSURE_TRUE (mDOMDocument, NULL);

  WalkTree (mDOMDocument);

  EmbedPageInfo *info = g_new0 (EmbedPageInfo, 1);

  info->props = GetProperties ();
  g_hash_table_foreach (mMediaHash, (GHFunc) make_list, &info->media);
  g_hash_table_foreach (mLinkHash, (GHFunc) make_list, &info->links);
  g_hash_table_foreach (mFormHash, (GHFunc) make_list, &info->forms);
  info->metatags = mMetaTagsList;

  return info;
}

char *
PageInfoHelper::ToCString (const nsAString& aString)
{
  nsEmbedCString cString;
  NS_UTF16ToCString (aString, NS_CSTRING_ENCODING_UTF8, cString);

  return g_strdup (cString.get());
}

nsresult
PageInfoHelper::GetCacheEntryDescriptor (const nsAString &aUrl,
                                         nsICacheEntryDescriptor **aEntry)
{
  nsresult rv;

  *aEntry = nsnull;
 
  nsCOMPtr<nsICacheService> cacheService =
          do_GetService(NS_CACHESERVICE_CONTRACTID);
  NS_ENSURE_TRUE (cacheService, NS_ERROR_FAILURE);

  nsEmbedCString cUrl;
  NS_UTF16ToCString (aUrl, NS_CSTRING_ENCODING_UTF8, cUrl);

  char *url = g_strdup (cUrl.get ());
  g_strdelimit (url, "#", '\0'); /* snip fragment, see bug #161201 */

  const char *cacheTypes[] = { "HTTP", "FTP" };
  for (unsigned int i = 0 ; i < G_N_ELEMENTS (cacheTypes); i++)
    {
      nsCOMPtr<nsICacheSession> cacheSession;
      cacheService->CreateSession (cacheTypes[i],
                                   nsICache::STORE_ANYWHERE,
                                   PR_TRUE,
                                   getter_AddRefs (cacheSession));
      NS_ENSURE_TRUE (cacheSession, NS_ERROR_FAILURE);
  
      cacheSession->SetDoomEntriesIfExpired (PR_FALSE);
  
      nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;
  
#ifdef MOZ_NSICACHESESSION_OPENCACHEENTRY_NSACSTRING_
      rv = cacheSession->OpenCacheEntry (nsEmbedCString(url), nsICache::ACCESS_READ,
                                         PR_FALSE, aEntry);
#else
      rv = cacheSession->OpenCacheEntry (url, nsICache::ACCESS_READ,
                                         PR_FALSE, aEntry);
#endif

      if (NS_SUCCEEDED (rv)) break;
    }

  g_free (url);

  return rv;
}

nsresult
PageInfoHelper::Resolve (const nsAString &aUrl,
                         nsACString &aResult)
{
  NS_ENSURE_TRUE (mBaseURI, NS_ERROR_FAILURE);

  nsEmbedCString cUrl;
  NS_UTF16ToCString (aUrl, NS_CSTRING_ENCODING_UTF8, cUrl);

  return mBaseURI->Resolve (cUrl, aResult);
}

nsresult
PageInfoHelper::Unescape (const nsACString &aEscaped,
                          nsACString &aResult)
{
  if (!aEscaped.Length()) return NS_ERROR_FAILURE;

  nsresult rv;
  if (!mTextToSubURI)
    {
      mTextToSubURI = do_CreateInstance ("@mozilla.org/intl/texttosuburi;1", &rv);
      NS_ENSURE_SUCCESS (rv, rv);
    }

  nsEmbedString unescaped;
  rv = mTextToSubURI->UnEscapeNonAsciiURI (mDocCharset, aEscaped, unescaped);
  NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && unescaped.Length(), rv);

  NS_UTF16ToCString (unescaped, NS_CSTRING_ENCODING_UTF8, aResult);

  return rv;
}

EmbedPageProperties *
PageInfoHelper::GetProperties ()
{
  NS_ENSURE_TRUE (mDOMDocument, NULL);

  nsCOMPtr<nsIDOMNSDocument> domNSDoc = do_QueryInterface (mDOMDocument);
  NS_ENSURE_TRUE (domNSDoc, NULL);

  EmbedPageProperties *props = g_new0 (EmbedPageProperties, 1);

  nsresult rv;
  nsEmbedString value;
  rv = domNSDoc->GetLastModified (value);
  NS_ENSURE_SUCCESS (rv, props);

  nsEmbedCString cTmp;
  NS_UTF16ToCString (value, NS_CSTRING_ENCODING_UTF8, cTmp);

  nsTime last_modified (cTmp.get(), PR_TRUE);
  LL_DIV (props->modification_time,
          NS_STATIC_CAST(PRTime, last_modified), PR_USEC_PER_SEC);

  rv = domNSDoc->GetContentType (value);
  NS_ENSURE_SUCCESS (rv, props);
  props->content_type = ToCString (value);

  rv = domNSDoc->GetCharacterSet (value);
  NS_ENSURE_SUCCESS (rv, props;);
  props->encoding = ToCString (value);

  /* Might not work on XUL pages */
  nsCOMPtr<nsIDOMHTMLDocument> domHtmlDoc (do_QueryInterface (mDOMDocument));
  if (domHtmlDoc)
    {
      rv = domHtmlDoc->GetReferrer (value);
      if (NS_SUCCEEDED (rv) && value.Length ())
      	{
          props->referring_url = ToCString (value);
	}
    }

  /* Until https://bugzilla.mozilla.org/show_bug.cgi?id=154359 is fixed */
  props->rendering_mode = PageInfoPrivate::GetRenderMode (mDOMDocument);

  /* Get the URL so we can look in the cache for this page */
  nsCOMPtr<nsIDOMLocation> domLocation;
  domNSDoc->GetLocation (getter_AddRefs (domLocation));
  NS_ENSURE_TRUE (domLocation, props);

  nsEmbedString url;
  domLocation->ToString (url);

  nsCOMPtr<nsICacheEntryDescriptor> cacheEntryDescriptor;
  GetCacheEntryDescriptor (url, getter_AddRefs (cacheEntryDescriptor));
  if (cacheEntryDescriptor)
    {
      PRUint32 expiry = 0, dataSize = 0;
      char *source = nsnull;

      cacheEntryDescriptor->GetExpirationTime (&expiry);
      cacheEntryDescriptor->GetDataSize (&dataSize);
      cacheEntryDescriptor->GetDeviceID (&source);

      props->expiration_time = expiry;
      props->size = dataSize;

      if (source && strcmp (source, "disk") == 0)
        {
          props->page_source = EMBED_SOURCE_DISK_CACHE;
        }
      else if (source && strcmp (source, "memory") == 0)
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

void
PageInfoHelper::ProcessNode (nsIDOMElement *aElement,
                             nsIDOMHTMLElement *aHTMLElement)
{
  nsresult rv;
  nsEmbedString value;

  rv = aElement->GetAttributeNS (mXLinkNS, nsEmbedString (mHrefAttr), value);
  if (NS_FAILED (rv) || !value.Length ()) return;

  nsEmbedCString cUrl;
  rv = Resolve (value, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  /* check if we have it already */
  if (g_hash_table_lookup (mLinkHash, cUrl.get())) return;
      
  EmbedPageLink *link = g_new0 (EmbedPageLink, 1);
  link->url = g_strdup (cUrl.get());
  g_hash_table_insert (mLinkHash, link->url, link);

  if (aHTMLElement)
    {
      rv = aHTMLElement->GetTitle (value);
      if (NS_SUCCEEDED(rv) && value.Length())
        {
          link->title = ToCString (value);
        }
    }
}

void
PageInfoHelper::ProcessAppletNode (nsIDOMHTMLAppletElement *aElement)
{
  nsEmbedString tmp;

  nsresult rv;
  rv = aElement->GetCode (tmp);
  if (NS_FAILED (rv) || !tmp.Length())
    {
      rv = aElement->GetObject (tmp);
      if (NS_FAILED (rv) || !tmp.Length()) return;
    }

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;
  
  if (g_hash_table_lookup (mMediaHash, cUrl.get())) return;

  /* FIXME: link or medium('media') ? */
  EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
  medium->type = MEDIUM_APPLET;
  medium->url = g_strdup (cUrl.get());
  g_hash_table_insert (mMediaHash, medium->url, medium);

  rv = aElement->GetAlt (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->alt = ToCString (tmp);
    }

  rv = aElement->GetTitle (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->title = ToCString (tmp);
    }
}

void
PageInfoHelper::ProcessAreaNode (nsIDOMHTMLAreaElement *aElement)
{
  nsEmbedString tmp;

  nsresult rv;
  rv = aElement->GetHref (tmp);
  if (NS_FAILED (rv) || !tmp.Length()) return;

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  /* Already listed ? */
  if (g_hash_table_lookup (mLinkHash, cUrl.get())) return;

  EmbedPageLink *link = g_new0 (EmbedPageLink, 1);
  link->url = g_strdup (cUrl.get());
  g_hash_table_insert (mLinkHash, link->url, link);

  rv = aElement->GetTitle (tmp);
  if (NS_SUCCEEDED (rv) && tmp.Length())
    {
      link->title = ToCString (tmp);
    }
}

void
PageInfoHelper::ProcessEmbedNodeHelper (const nsEmbedString &aUrl, 
  				        nsIDOMHTMLEmbedElement *aElement)
{
  nsresult rv;

  nsEmbedCString cUrl;
  rv = Resolve (aUrl, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  if (g_hash_table_lookup (mMediaHash, cUrl.get())) return;

  EmbedPageMedium *medium;

  medium = g_new0 (EmbedPageMedium, 1);
  medium->type = MEDIUM_EMBED;
  medium->url = g_strdup (cUrl.get());
  g_hash_table_insert (mMediaHash, medium->url, medium);

  nsEmbedString tmp;
  rv = aElement->GetTitle (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->title = ToCString (tmp);
    }
}

void
PageInfoHelper::ProcessEmbedNode (nsIDOMHTMLEmbedElement *aElement)
{
  nsresult rv;
  nsEmbedString tmp;

  // Look at src tag
  rv = aElement->GetSrc (tmp);
  if (NS_SUCCEEDED (rv) && tmp.Length())
    {
      ProcessEmbedNodeHelper (tmp, aElement);
    }

  // Look at href tag
  aElement->GetAttribute(mHrefAttr, tmp);
  if (NS_SUCCEEDED (rv) && tmp.Length())
    { 
      ProcessEmbedNodeHelper (tmp, aElement);
    }
}

void
PageInfoHelper::ProcessFormNode (nsIDOMHTMLFormElement *aElement)
{
  nsEmbedString tmp;

  nsresult rv;
  rv = aElement->GetAction (tmp);
  if (NS_FAILED (rv) || !tmp.Length ()) return;

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  EmbedPageForm *form = g_new0 (EmbedPageForm, 1);
  form->action = cUrl.Length() ? g_strdup (cUrl.get()) : ToCString (tmp);
  g_hash_table_insert (mFormHash, form->action, form);

  rv = aElement->GetName (tmp);
  if (NS_SUCCEEDED(rv) && tmp.Length())
    {
      form->name = ToCString (tmp);
    }

  rv = aElement->GetMethod (tmp);
  if (NS_SUCCEEDED(rv) && tmp.Length())
    {
      form->method = ToCString (tmp);
    }
}

void
PageInfoHelper::ProcessImageNode (nsIDOMHTMLImageElement *aElement)
{
  nsresult rv;
  nsEmbedString tmp;
  rv = aElement->GetSrc (tmp);
  if (NS_FAILED (rv) || !tmp.Length()) return;

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  if (g_hash_table_lookup (mMediaHash, cUrl.get())) return;

  EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
  medium->type = MEDIUM_IMAGE;
  medium->url = g_strdup (cUrl.get());
  g_hash_table_insert (mMediaHash, medium->url, medium);

  rv = aElement->GetAlt (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->alt = ToCString (tmp);
    }

  rv = aElement->GetTitle (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->title = ToCString (tmp);
    }

  aElement->GetWidth (&medium->width);
  aElement->GetHeight (&medium->height);
}

void
PageInfoHelper::ProcessObjectNode (nsIDOMHTMLObjectElement *aElement)
{
  nsresult rv;
  nsEmbedString tmp;
  rv = aElement->GetData (tmp);
  if (NS_FAILED (rv) || !tmp.Length()) return;

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  if (g_hash_table_lookup (mMediaHash, cUrl.get())) return;

  EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
  medium->type = MEDIUM_OBJECT;
  medium->url = g_strdup (cUrl.get());
  g_hash_table_insert (mMediaHash, medium->url, medium);

  rv = aElement->GetTitle (tmp);
  if (NS_SUCCEEDED (rv))
    {
      medium->title = ToCString (tmp);
    }
}

void
PageInfoHelper::ProcessMetaNode (nsIDOMHTMLMetaElement *aElement)
{
  nsresult rv;
  nsEmbedString key;
  /* if we have a http-equiv in the page */
  rv = aElement->GetHttpEquiv (key);
  if (NS_FAILED (rv) || !key.Length())
  {
    rv = aElement->GetName (key);
    if (NS_FAILED (rv) || !key.Length()) return;
  }

  nsEmbedString content;
  rv = aElement->GetContent (content);
  if (NS_FAILED (rv) || !content.Length ()) return;

  EmbedPageMetaTag *tag = g_new0 (EmbedPageMetaTag, 1);
  tag->name = ToCString (key);
  tag->content =  ToCString (content);

  mMetaTagsList = g_list_prepend (mMetaTagsList, tag);
}

void
PageInfoHelper::ProcessInputNode (nsIDOMHTMLInputElement *aElement)
{
  /* We are searching for input of type medium only */
  nsresult rv;
  nsEmbedString tmp;
  rv = aElement->GetType (tmp);
  if (NS_FAILED (rv) || !tmp.Length()) return;

  nsEmbedCString cTmp;
  NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, cTmp);

  /* Is it an image ? */
  if (g_ascii_strcasecmp (cTmp.get (), "image") != 0) return;

  /* Get url */
  rv = aElement->GetSrc (tmp);
  if (NS_FAILED (rv) || !tmp.Length ()) return;

  nsEmbedCString cUrl;
  rv = Resolve (tmp, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  /* is already listed ? */
  if (g_hash_table_lookup (mMediaHash, cUrl.get())) return;

  EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
  medium->type = MEDIUM_IMAGE;
  medium->url = g_strdup (cUrl.get());
  g_hash_table_insert (mMediaHash, medium->url, medium);

  rv = aElement->GetAlt(tmp);
  if (NS_SUCCEEDED(rv))
    {
      medium->alt = ToCString (tmp);
    }
}

template <class T>
void
PageInfoHelper::ProcessLinkNode (nsIDOMNode *aNode)
{
  nsCOMPtr<T> element (do_QueryInterface (aNode));
  if (!element) return;

  nsEmbedString tmp;

  nsresult rv;
  rv = element->GetHref(tmp);
  NS_ENSURE_SUCCESS (rv, );

  /* Check for mailto scheme */
  nsCOMPtr<nsIURI> uri;
  rv = EphyUtils::NewURI (getter_AddRefs (uri), tmp, mDocCharset.get(), mBaseURI);
  if (NS_FAILED (rv) || !uri) return;

  PRBool isMailto = PR_FALSE;
  uri->SchemeIs ("mailto", &isMailto);

#if 0
  /* We need to unescape mailto link, see bug #144462 */
  nsEmbedCString cTmp;
  NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, cTmp);

  if (isMailto)
  {
          nsEmbedCString unescapedHref;

          rv = mozilla_unescape (cTmp, unescapedHref, encoding);
          if (NS_SUCCEEDED (rv) && unescapedHref.Length())
          {
                  url = g_strdup (unescapedHref.get ());
          }
          else
          {
                  url = g_strdup (cTmp.get());
          }
  }
  else
  {
          url = g_strdup (cTmp.get());
  }
#endif

  nsEmbedCString spec;
  rv = uri->GetSpec (spec);
  if (NS_FAILED (rv)) return;

  nsEmbedCString cUrl;
  rv = Unescape (spec, cUrl);
  if (NS_FAILED (rv) || !cUrl.Length()) return;

  /* REL */
  element->GetRel (tmp);

  nsEmbedCString rel;
  NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, rel);

  if (rel.Length() && (g_ascii_strcasecmp (rel.get(), "icon") == 0 ||
      g_ascii_strcasecmp (rel.get(), "shortcut icon") == 0))
    {
      EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
      medium->type = MEDIUM_ICON;
      medium->url = g_strdup (cUrl.get());
      g_hash_table_insert (mMediaHash, medium->url, medium);

      return;
    }

  if (!rel.Length())
    {
      element->GetRev (tmp);

      NS_UTF16ToCString (tmp, NS_CSTRING_ENCODING_UTF8, rel);
    }

  /* This is really a link */

  /* Already listed ? */
  if (!cUrl.Length () || g_hash_table_lookup (mLinkHash, cUrl.get())) return;

  EmbedPageLink *link = g_new0 (EmbedPageLink, 1);
  link->url = g_strdup (cUrl.get());
  link->rel = g_strdup (rel.get());
  link->type = isMailto ? EMBED_LINK_TYPE_MAIL : EMBED_LINK_TYPE_NORMAL;
  g_hash_table_insert (mLinkHash, link->url, link);

  /* TITLE */
  rv = element->GetTitle (tmp);
  if (NS_SUCCEEDED (rv) && tmp.Length())
    {
      link->title = ToCString (tmp);
    }
}

void
PageInfoHelper::WalkTree (nsIDOMDocument *aDocument)
{
  nsCOMPtr<nsIDOMHTMLDocument> htmlDoc (do_QueryInterface (aDocument));
  nsCOMPtr<nsIDOMNSDocument> nsDoc (do_QueryInterface (aDocument));
  nsCOMPtr<nsIDOMDocumentTraversal> trav (do_QueryInterface (aDocument));
  if (!htmlDoc || !nsDoc || !trav) return;

  nsresult rv;
  nsEmbedString charset;
  rv = nsDoc->GetCharacterSet (charset);
  if (NS_FAILED (rv)) return;

  NS_UTF16ToCString (charset, NS_CSTRING_ENCODING_UTF8, mDocCharset);

  nsCOMPtr<nsIDOM3Node> dom3Node (do_QueryInterface (aDocument));
  if (!dom3Node) return;
  nsEmbedString spec;
  rv = dom3Node->GetBaseURI (spec);
  if (NS_FAILED (rv)) return;

  nsCOMPtr<nsIURI> base;
  rv = EphyUtils::NewURI (getter_AddRefs (mBaseURI), spec, mDocCharset.get(), nsnull);
  if (NS_FAILED (rv) || !mBaseURI) return;

  nsCOMPtr<nsIDOMNode> rootNode (do_QueryInterface (htmlDoc));
  if (!rootNode) return;

  nsCOMPtr<nsIDOMTreeWalker> walker;
  rv = trav->CreateTreeWalker(rootNode,
                              nsIDOMNodeFilter::SHOW_ELEMENT,
                              nsnull,
                              PR_TRUE,
                              getter_AddRefs (walker));
  if (NS_FAILED (rv) || !walker) return;

  nsCOMPtr<nsIDOMDocumentView> docView (do_QueryInterface (aDocument));
  nsCOMPtr<nsIDOMViewCSS> defaultCSSView;
  if (docView)
    {
      nsCOMPtr<nsIDOMAbstractView> defaultView;
      docView->GetDefaultView (getter_AddRefs (defaultView));
      defaultCSSView = do_QueryInterface(defaultView);
    }

  nsEmbedString EmptyString;

  nsCOMPtr<nsIDOMNode> aNode;
  nsCOMPtr<nsIDOMCSSStyleDeclaration> computedStyle;
  
  walker->GetCurrentNode(getter_AddRefs(aNode));
  while (aNode)
    {
      PRBool hasAttributes = PR_FALSE;
      aNode->HasAttributes (&hasAttributes);

      nsCOMPtr<nsIDOMHTMLElement> nodeAsElement (do_QueryInterface (aNode));
      if (nodeAsElement)
        {
          if (defaultCSSView)
            {
              defaultCSSView->GetComputedStyle(nodeAsElement, EmptyString,
                                               getter_AddRefs (computedStyle));
            }
  
          if (computedStyle)
            {
	      nsCOMPtr<nsIDOMCSSValue> value;
	      computedStyle->GetPropertyCSSValue(mBgImageAttr, getter_AddRefs (value));

	      nsCOMPtr<nsIDOMCSSPrimitiveValue> primitiveValue (do_QueryInterface (value));
	      if (primitiveValue)
	      	{
		  PRUint16 primitiveType = 0;
		  rv = primitiveValue->GetPrimitiveType (&primitiveType);
		  if (NS_SUCCEEDED (rv) && primitiveType == nsIDOMCSSPrimitiveValue::CSS_URI)
		    {
		      nsEmbedString stringValue;
		      rv = primitiveValue->GetStringValue (stringValue);
		      if (NS_SUCCEEDED (rv) && stringValue.Length())
		      	{
                          EmbedPageMedium *medium = g_new0 (EmbedPageMedium, 1);
                          medium->type = MEDIUM_IMAGE;
                          medium->url = ToCString (stringValue);
                          g_hash_table_insert(mMediaHash, medium->url, medium);
			}
		    }
		  
		}
            }
        }

      nsCOMPtr<nsIDOMElement> nodeAsDOMElement (do_QueryInterface (aNode));
      if (hasAttributes && nodeAsDOMElement)
        {
          ProcessNode (nodeAsDOMElement, nodeAsElement);
        }

      nsCOMPtr<nsIDOMHTMLImageElement> nodeAsImage (do_QueryInterface(aNode));
      if (nodeAsImage)
        {
          ProcessImageNode (nodeAsImage);
        }

      ProcessLinkNode<nsIDOMHTMLLinkElement> (aNode);
      ProcessLinkNode<nsIDOMHTMLAnchorElement> (aNode);

      nsCOMPtr<nsIDOMHTMLAreaElement> nodeAsArea (do_QueryInterface(aNode));
      if (nodeAsArea)
        {
          ProcessAreaNode (nodeAsArea);
        }

      nsCOMPtr<nsIDOMHTMLFormElement> nodeAsForm (do_QueryInterface(aNode));
      if (nodeAsForm)
        {
          ProcessFormNode (nodeAsForm);
        }

      nsCOMPtr<nsIDOMHTMLMetaElement> nodeAsMeta (do_QueryInterface(aNode));
      if (nodeAsMeta)
        {
          ProcessMetaNode (nodeAsMeta);
        }

      nsCOMPtr<nsIDOMHTMLInputElement> nodeAsInput (do_QueryInterface(aNode));
      if (nodeAsInput)
        {
          ProcessInputNode (nodeAsInput);
        }

      /* When Java is enabled, the DOM model for <APPLET> is broken.
       * See https://bugzilla.mozilla.org/show_bug.cgi?id=59686
       * Also, some reports of a crash with Java in Media tab
       * (https://bugzilla.mozilla.org/show_bug.cgi?id=136535),
       * and mixed content from two hosts
       * (https://bugzilla.mozilla.org/show_bug.cgi?id=136539),
       * so just drop applets from Page Info when Java is on.
       */
      if (!mJavaEnabled)
        {
          nsCOMPtr<nsIDOMHTMLAppletElement> nodeAsApplet (do_QueryInterface (aNode));
          if (nodeAsApplet)
            {
              ProcessAppletNode (nodeAsApplet);
            }
        }

      nsCOMPtr<nsIDOMHTMLObjectElement> nodeAsObject (do_QueryInterface (aNode));
      if (nodeAsObject)
        {
          ProcessObjectNode (nodeAsObject);
        }

      nsCOMPtr<nsIDOMHTMLEmbedElement> nodeAsEmbed (do_QueryInterface (aNode));
      if (nodeAsEmbed)
        {
          ProcessEmbedNode (nodeAsEmbed);
        }

      nsCOMPtr<nsIDOMHTMLFrameElement> nodeAsFrame (do_QueryInterface (aNode));
      if (nodeAsFrame)
        {
          nsCOMPtr<nsIDOMDocument> frameDoc;
          nodeAsFrame->GetContentDocument (getter_AddRefs (frameDoc));
          if (frameDoc)
            {
              WalkFrame (frameDoc);
            }
        }

      nsCOMPtr<nsIDOMHTMLIFrameElement> nodeAsIFrame (do_QueryInterface(aNode));
      if (nodeAsIFrame)
        {
          nsCOMPtr<nsIDOMDocument> frameDoc;
          nodeAsIFrame->GetContentDocument (getter_AddRefs (frameDoc));
          if (frameDoc)
            {
              WalkFrame (frameDoc);
            }
        }

      walker->NextNode (getter_AddRefs (aNode));
    }
}

void
PageInfoHelper::WalkFrame (nsIDOMDocument *aDocument)
{
  nsEmbedCString saveCharset (mDocCharset);
  nsCOMPtr<nsIURI> uri (mBaseURI);
  
  WalkTree (aDocument);

  mDocCharset = saveCharset;
  mBaseURI = uri;
}

/* external API */

static void
free_embed_page_link (EmbedPageLink *link)
{
  g_free (link->url);
  g_free (link->title);
  g_free (link->rel);
  g_free (link);
}

static void
free_embed_page_medium (EmbedPageMedium *medium)
{
  g_free (medium->url);
  g_free (medium->alt);
  g_free (medium->title);
  g_free (medium);
}

static void
free_embed_page_form (EmbedPageForm *form)
{
  g_free (form->name);
  g_free (form->method);
  g_free (form->action);
  g_free (form);
}

static void
free_embed_page_metatags (EmbedPageMetaTag *meta)
{
  g_free (meta->name);
  g_free (meta->content);
  g_free (meta);
}

static void
mozilla_free_page_properties (EmbedPageProperties *props)
{
  g_free (props->content_type);
  g_free (props->encoding);
  g_free (props->referring_url);
  g_free (props);
}

extern "C" void
mozilla_free_embed_page_info (EmbedPageInfo *info)
{
  g_list_foreach (info->forms, (GFunc) free_embed_page_form, NULL);
  g_list_free (info->forms);

  g_list_foreach (info->links, (GFunc) free_embed_page_link, NULL);
  g_list_free (info->links);

  g_list_foreach (info->media, (GFunc) free_embed_page_medium, NULL);
  g_list_free (info->media);

  g_list_foreach (info->metatags, (GFunc) free_embed_page_metatags, NULL);
  g_list_free (info->metatags);

  mozilla_free_page_properties (info->props);

  g_free (info);
}

extern "C" EmbedPageInfo *
mozilla_get_page_info (EphyEmbed *embed)
{
  PageInfoHelper *helper = new PageInfoHelper ();
  if (!helper) return NULL;

  nsresult rv;
  rv = helper->Init (embed);
  if (NS_FAILED (rv)) return NULL;

  EmbedPageInfo *info;
  info = helper->GetInfo ();

  delete helper;

  return info;
}
