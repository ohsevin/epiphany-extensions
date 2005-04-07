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

#include "mozilla-selection.h"

#include <glib.h>

#undef MOZILLA_INTERNAL_API
#include <nsEmbedString.h>
#define MOZILLA_INTERNAL_API 1
#include <nsCOMPtr.h>
#include <nsISelection.h>
#include <gtkmozembed.h>
#include <nsIWebBrowser.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowserFocus.h>
#include <gtkmozembed_internal.h>
#include <nsMemory.h>

extern "C" char *
mozilla_get_selected_text (EphyEmbed *embed)
{
	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	nsCOMPtr<nsIWebBrowserFocus> focus (do_QueryInterface(browser));
	NS_ENSURE_TRUE (focus, NULL);

	nsCOMPtr<nsIDOMWindow> domWindow;
	focus->GetFocusedWindow (getter_AddRefs(domWindow));
	NS_ENSURE_TRUE (domWindow, NULL);

	nsCOMPtr<nsISelection> selection;
	domWindow->GetSelection(getter_AddRefs(selection));
	NS_ENSURE_TRUE (selection, NULL);
	
	PRUnichar *selText = nsnull; 
	selection->ToString(&selText);
	if (selText != nsnull)
	{
		nsEmbedCString cText; 
		NS_UTF16ToCString(nsEmbedString(selText), NS_CSTRING_ENCODING_UTF8, cText);

		nsMemory::Free (selText);
		return g_strdup (cText.get());
	}

	return NULL;
}
