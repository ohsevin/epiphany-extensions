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

#include <glib/gi18n.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#include <nsEmbedString.h>
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

struct MozillaEmbedStyleSheet : public EmbedStyleSheet
{
	MozillaEmbedStyleSheet()
	{
		name = NULL; sheet = NULL; type = STYLESHEET_NONE;
	}
	~MozillaEmbedStyleSheet()
	{
	}

	nsCOMPtr<nsIDOMStyleSheet> domStyle;
};

static char *
embed_string_to_c_string (const nsEmbedString& embed_string)
{
	nsEmbedCString c_string;
	NS_UTF16ToCString (embed_string, NS_CSTRING_ENCODING_UTF8, c_string);

	return g_strdup (c_string.get());
}

extern "C" void
mozilla_free_stylesheet (EmbedStyleSheet *sheet)
{
	MozillaEmbedStyleSheet *mess = NS_STATIC_CAST(MozillaEmbedStyleSheet*, sheet);

	if (mess)
	{
		g_free (mess->name);
		delete mess;
	}
}

static gint
stylesheet_find_func (gconstpointer a, gconstpointer b)
{
	const EmbedStyleSheet *sheet = (const EmbedStyleSheet *) a;
	const char *name = (const char *) b;

	return strcmp (sheet->name, name);
}

static nsresult
get_raw_stylesheets (EphyEmbed *embed,
		     nsIDOMStyleSheetList **list)
{
	nsresult rv;

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NS_ERROR_FAILURE);

	nsCOMPtr<nsIDOMWindow> dom_window;
	rv = browser->GetContentDOMWindow (getter_AddRefs (dom_window));
	NS_ENSURE_SUCCESS (rv, rv);

	nsCOMPtr<nsIDOMDocument> doc;
	rv = dom_window->GetDocument (getter_AddRefs (doc));
	NS_ENSURE_SUCCESS (rv, rv);

	nsCOMPtr<nsIDOMDocumentStyle> docstyle = do_QueryInterface (doc);
	NS_ENSURE_TRUE (docstyle, NS_ERROR_FAILURE);

	return docstyle->GetStyleSheets(list);
}

static gboolean
stylesheet_is_alternate (nsIDOMStyleSheet *item)
{
	nsresult rv;
	gboolean ret = FALSE;

	nsCOMPtr<nsIDOMNode> node;
	rv = item->GetOwnerNode (getter_AddRefs (node));
	NS_ENSURE_SUCCESS (rv, FALSE);

	nsCOMPtr<nsIDOMHTMLLinkElement> link = do_QueryInterface (node);

	if (link)
	{
		nsEmbedString str;
		link->GetRel(str);

		char *t = embed_string_to_c_string (str);

		ret = (g_ascii_strncasecmp (t, "alternate", 9) == 0);

		g_free (t);
	}

	return ret;
}

extern "C" GList *
mozilla_get_stylesheets (EphyEmbed *embed,
			 EmbedStyleSheet **selected)
{
	nsresult rv;
	GList *ret = NULL;
	int num_total = 0;
	int num_named = 0;

	*selected = NULL;

	nsCOMPtr<nsIDOMStyleSheetList> list;
	rv = get_raw_stylesheets (embed, getter_AddRefs (list));
	NS_ENSURE_SUCCESS (rv, NULL);

	PRUint32 count(0);
	rv = list->GetLength(&count);
	NS_ENSURE_SUCCESS (rv, NULL);

	for (PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMStyleSheet> item;
		rv = list->Item(i, getter_AddRefs (item));
		NS_ENSURE_SUCCESS (rv, ret);

		num_total++;

		nsEmbedString string;
		rv = item->GetTitle(string);
		NS_ENSURE_SUCCESS (rv, ret);

		if (string.Length() == 0) continue;

		char *name = embed_string_to_c_string (string);

		if (g_list_find_custom (ret, name, stylesheet_find_func))
		{
			g_free (name);
			continue;
		}

		MozillaEmbedStyleSheet *sheet = new MozillaEmbedStyleSheet ();
		sheet->name = name;
		sheet->sheet = item;
		sheet->type = STYLESHEET_NAMED;
		sheet->domStyle = item;

		if (!stylesheet_is_alternate (item))
		{
			num_named++;
			if (selected) *selected = sheet;
		}

		ret = g_list_prepend (ret, sheet);
	}

	if (num_total > 0 && num_named == 0)
	{
		/* Add in the "Default" style if we found stylesheets but
		 * we didn't find any (non-alternate) named ones) */
		MozillaEmbedStyleSheet *sheet = new MozillaEmbedStyleSheet();
		sheet->name = g_strdup (_("Default"));
		sheet->sheet = NULL;
		sheet->type = STYLESHEET_BASIC;

		if (selected) *selected = sheet;

		ret = g_list_prepend (ret, sheet);
	}

	ret = g_list_reverse (ret);

	if (num_total > 0)
	{
		MozillaEmbedStyleSheet *sheet = new MozillaEmbedStyleSheet();
		sheet->name = g_strdup (_("None"));
		sheet->sheet = NULL;
		sheet->type = STYLESHEET_NONE;

		ret = g_list_prepend (ret, sheet);
	}

	return ret;
}

extern "C" void
mozilla_set_stylesheet (EphyEmbed *embed,
			EmbedStyleSheet *selected)
{
	nsresult rv;

	nsCOMPtr<nsIDOMStyleSheetList> list;
	rv = get_raw_stylesheets (embed, getter_AddRefs (list));
	g_return_if_fail (NS_SUCCEEDED (rv));

	PRUint32 count(0);
	rv = list->GetLength(&count);
	g_return_if_fail (NS_SUCCEEDED (rv));

	for (PRUint32 i = 0; i < count; i++)
	{
		nsCOMPtr<nsIDOMStyleSheet> item;
		rv = list->Item(i, getter_AddRefs(item));
		if (NS_FAILED (rv) || !item) continue;

		/*
		 * if STYLESHEET_NONE, disable all
		 * if STYLESHEET_BASIC, enable only unnamed
		 * if STYLESHEET_NAMED, load unnamed and ones with given name
		 */
		if (selected->type == STYLESHEET_NONE)
		{
			item->SetDisabled(PR_TRUE);
			continue;
		}

		nsEmbedString string;
		item->GetTitle(string);
		if (NS_FAILED (rv)) continue;

		if (selected->type == STYLESHEET_BASIC)
		{
			item->SetDisabled(string.Length());
			continue;
		}

		char *name = embed_string_to_c_string (string);

		item->SetDisabled(strcmp (name, selected->name) != 0);

		g_free (name);
	}
}
