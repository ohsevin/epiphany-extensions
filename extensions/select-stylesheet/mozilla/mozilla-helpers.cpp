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

#include "mozilla-config.h"

#include "config.h"

#include "mozilla-helpers.h"

#include <string.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#define MOZILLA_STRICT_API
#include <nsEmbedString.h>
#undef MOZILLA_STRICT_API
#include <nsCOMPtr.h>
#include <nsICacheEntryDescriptor.h>
#include <nsICacheService.h>
#include <nsICacheSession.h>
#include <nsIDOMDocument.h>
#include <nsIDOMDocumentStyle.h>
#include <nsIDOMHTMLLinkElement.h>
#include <nsIDOMStyleSheet.h>
#include <nsIDOMStyleSheetList.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>
#include <nsIDOMMediaList.h>

#include <glib/gi18n-lib.h>

struct MozillaStyleSheet
{
	MozillaStyleSheet(StyleSheetType aType,
			  const char *aName,
			  nsIDOMStyleSheet *aStyleSheet)
	: mName(g_strdup (aName))
	, mType(aType)
	, mStyleSheet(aStyleSheet)
	{
	}

	~MozillaStyleSheet()
	{
		g_free (mName);
	}

	char *mName;
	StyleSheetType mType;
	nsCOMPtr<nsIDOMStyleSheet> mStyleSheet;
};

extern "C" StyleSheetType
mozilla_stylesheet_get_type (MozillaStyleSheet *aStyle)
{
	return aStyle->mType;
}

extern "C" const char *
mozilla_stylesheet_get_name (MozillaStyleSheet *aStyle)
{
	return aStyle->mName;
}

extern "C" void
mozilla_stylesheet_free (MozillaStyleSheet *aStyle)
{
	g_return_if_fail (aStyle != NULL);

	delete aStyle;
}

static int
stylesheet_find_func (gconstpointer a, gconstpointer b)
{
	const MozillaStyleSheet *sheet = (const MozillaStyleSheet *) a;
	const char *name = (const char *) b;

	return strcmp (sheet->mName, name);
}

static nsresult
GetStylesheets (EphyEmbed *aEmbed,
		nsIDOMStyleSheetList **aList)
{
	NS_ENSURE_TRUE (aEmbed, NS_ERROR_FAILURE);

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (aEmbed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NS_ERROR_FAILURE);

	nsCOMPtr<nsIDOMWindow> domWindow;
	browser->GetContentDOMWindow (getter_AddRefs (domWindow));
	NS_ENSURE_TRUE (domWindow, NS_ERROR_FAILURE);
	
	nsCOMPtr<nsIDOMDocument> doc;
	domWindow->GetDocument (getter_AddRefs (doc));

	nsCOMPtr<nsIDOMDocumentStyle> docstyle (do_QueryInterface (doc));
	NS_ENSURE_TRUE (docstyle, NS_ERROR_FAILURE);

	return docstyle->GetStyleSheets (aList);
}

static gboolean
IsAlternateStylesheet (nsIDOMStyleSheet *aStyleSheet)
{
	NS_ENSURE_TRUE (aStyleSheet, FALSE);

	nsCOMPtr<nsIDOMNode> node;
	aStyleSheet->GetOwnerNode (getter_AddRefs (node));

	nsCOMPtr<nsIDOMHTMLLinkElement> link (do_QueryInterface (node));

	gboolean ret = FALSE;
	if (link)
	{
		nsresult rv;
		nsEmbedString rel;
		rv = link->GetRel (rel);
		NS_ENSURE_SUCCESS (rv, ret);

		nsEmbedCString cRel;
		NS_UTF16ToCString (rel, NS_CSTRING_ENCODING_UTF8, cRel);

		ret = (g_ascii_strncasecmp (cRel.get(), "alternate", 9) == 0);
	}

	return ret;
}

extern "C" GList *
mozilla_get_stylesheets (EphyEmbed *aEmbed,
			 MozillaStyleSheet **aSelected)
{
	*aSelected = NULL;

	nsCOMPtr<nsIDOMStyleSheetList> list;
	GetStylesheets (aEmbed, getter_AddRefs (list));
	NS_ENSURE_TRUE (list, NULL);

	nsresult rv;
	PRUint32 count = 0;
	rv = list->GetLength (&count);
	NS_ENSURE_SUCCESS (rv, NULL);

	GList *ret = NULL;
	PRUint32 numTotal = 0, numNamed = 0, i;
	for (i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMStyleSheet> item;
		list->Item(i, getter_AddRefs (item));
		if (!item) continue;

		++numTotal;

		/* skip stylesheets for media != screen or all */

		nsCOMPtr<nsIDOMMediaList> mediaList;
		item->GetMedia (getter_AddRefs (mediaList));
		if (mediaList)
		{
			nsEmbedString media;
			rv = mediaList->GetMediaText (media);
			if (NS_FAILED (rv)) continue;
	
			nsEmbedCString cMedia;
			NS_UTF16ToCString (media, NS_CSTRING_ENCODING_UTF8, cMedia);

			if (media.Length() > 0 &&
			    strstr (cMedia.get(), "screen") == NULL &&
			    strstr (cMedia.get(), "all") == NULL) continue;
		}

		nsEmbedString title;
		rv = item->GetTitle (title);
		if (NS_FAILED (rv) || !title.Length()) continue;

		nsEmbedCString cTitle;
		NS_UTF16ToCString (title, NS_CSTRING_ENCODING_UTF8, cTitle);

		/* check if it's already in the list */
		if (g_list_find_custom (ret, cTitle.get(), stylesheet_find_func)) continue;

		MozillaStyleSheet *sheet =
			new MozillaStyleSheet (STYLESHEET_NAMED,
					       cTitle.get(), item);

		if (!IsAlternateStylesheet (item))
		{
			numNamed++;
			if (aSelected) *aSelected = sheet;
		}

		ret = g_list_prepend (ret, sheet);
	}

	if (numTotal > 0 && numNamed == 0)
	{
		/* Add in the "Default" style if we found stylesheets but
		 * we didn't find any (non-alternate) named ones.
		 */

		MozillaStyleSheet *sheet =
			new MozillaStyleSheet (STYLESHEET_BASIC,
					       _("Default"), nsnull);
		if (aSelected) *aSelected = sheet;

		ret = g_list_prepend (ret, sheet);
	}

	ret = g_list_reverse (ret);

	if (numTotal > 0)
	{
		MozillaStyleSheet *sheet =
			new MozillaStyleSheet (STYLESHEET_NONE, _("None"), nsnull);

		ret = g_list_prepend (ret, sheet);
	}

	return ret;
}

extern "C" void
mozilla_set_stylesheet (EphyEmbed *aEmbed,
			MozillaStyleSheet *aSelected)
{
	nsCOMPtr<nsIDOMStyleSheetList> list;
	GetStylesheets (aEmbed, getter_AddRefs (list));
	NS_ENSURE_TRUE (list, );

	nsresult rv;
	PRUint32 count = 0;
	rv = list->GetLength(&count);
	NS_ENSURE_SUCCESS (rv, );

	PRUint32 i;
	for (i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMStyleSheet> item;
		list->Item(i, getter_AddRefs(item));
		if (!item) continue;

		/*
		 * if STYLESHEET_NONE, disable all
		 * if STYLESHEET_BASIC, enable only unnamed
		 * if STYLESHEET_NAMED, load unnamed and ones with given name
		 */

		if (aSelected->mType == STYLESHEET_NONE)
		{
			item->SetDisabled(PR_TRUE);
			continue;
		}

		nsEmbedString title;
		item->GetTitle (title);
		if (NS_FAILED (rv)) continue;

		if (title.Length() == 0)
		{
			item->SetDisabled(PR_FALSE);
			continue;
		}

		if (aSelected->mType == STYLESHEET_BASIC) continue;

		nsEmbedCString cTitle;
		NS_UTF16ToCString (title, NS_CSTRING_ENCODING_UTF8, cTitle);

		item->SetDisabled (strcmp (cTitle.get(), aSelected->mName) != 0);
	}
}
