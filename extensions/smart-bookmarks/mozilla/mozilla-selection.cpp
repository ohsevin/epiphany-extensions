/*
 *  Copyright (C) 2000 Marco Pesenti Gritti
 *  Copyright (C) 2004 Jean-François Rameau
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

#include <glib.h>

#include <nsStringAPI.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>
#include <nsCOMPtr.h>
#include <nsIDOMElement.h>
#include <nsIDOMHTMLInputElement.h>
#include <nsIDOMHTMLTextAreaElement.h>
#include <nsIDOMNSHTMLInputElement.h>
#include <nsIDOMNSHTMLTextAreaElement.h>
#include <nsIDOMWindow.h>
#include <nsISelection.h>
#include <nsIWebBrowserFocus.h>
#include <nsIWebBrowser.h>
#include <nsMemory.h>

#include "mozilla-selection.h"

template <class NST, class T>
char * get_selection (nsIDOMElement *aElement)
{
	nsCOMPtr<NST> nsElement (do_QueryInterface(aElement));

	if (!nsElement) return NULL;

	// Boundaries
	PRInt32 start, end;
	nsElement->GetSelectionStart (&start);
	nsElement->GetSelectionEnd (&end);

	// Get full text in element
	nsString text; 
	nsCOMPtr<T> element (do_QueryInterface(aElement));
	element->GetValue(text);

	// Extract the selection
	text.Cut (end, text.Length());
	text.Cut (0, start);

	nsCString cText; 
	NS_UTF16ToCString(text, NS_CSTRING_ENCODING_UTF8, cText);

	if (cText.Length () == 0) return NULL;

	return g_strdup(cText.get());
}

extern "C" char *
mozilla_get_selected_text (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
			getter_AddRefs (browser));
	nsCOMPtr<nsIWebBrowserFocus> focus (do_QueryInterface(browser));
	NS_ENSURE_TRUE (focus, NULL);

	// First, guess the selection is not somewhere inside
	// a textarea nor an input 
	nsCOMPtr<nsIDOMWindow> domWindow;
	focus->GetFocusedWindow (getter_AddRefs (domWindow));
	NS_ENSURE_TRUE (domWindow, NULL);

	nsCOMPtr<nsISelection> nsSelection;
	domWindow->GetSelection (getter_AddRefs (nsSelection));
	NS_ENSURE_TRUE (nsSelection, NULL);

	PRUnichar *selText = nsnull; 
	nsSelection->ToString(&selText);
	nsString text(selText);

	if (text.Length () > 0)
	{
		nsCString cText; 
		NS_UTF16ToCString(text, NS_CSTRING_ENCODING_UTF8, cText);

		nsMemory::Free (selText);
		return g_strdup (cText.get());
	}

	// Then the selection is perhaps somewhere inside
	// a textarea or an input	
	nsCOMPtr<nsIDOMElement> domElement;
	focus->GetFocusedElement (getter_AddRefs(domElement));

	if (!domElement) return NULL;

	char *selection;

	selection = get_selection <nsIDOMNSHTMLTextAreaElement, 
		       		   nsIDOMHTMLTextAreaElement> (domElement);
	if (selection) return selection;

	// Take care of password fields
	nsCOMPtr<nsIDOMHTMLInputElement> input (do_QueryInterface(domElement));
	input->GetType (text);
	const PRUnichar *str = text.get ();
	if (!(str[0] == 't' && str[1] == 'e' && str[2] == 'x' && str[3] == 't' && str[4] == '\0')) return NULL;

	selection = get_selection <nsIDOMNSHTMLInputElement, 
		       		   nsIDOMHTMLInputElement> (domElement);
	if (selection) return selection;

	return NULL;
}
