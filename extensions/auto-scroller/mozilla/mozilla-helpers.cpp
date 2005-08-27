/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#include <glib.h>

#include <nsCOMPtr.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserFocus.h>
#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIDOMWindow.h>

gboolean
mozilla_helper_fine_scroll (EphyEmbed *embed, gint step_x, gint step_y )
{
	nsresult rv;

        nsCOMPtr<nsIWebBrowser> webBrowser;

	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs(webBrowser));
	if (!webBrowser) return FALSE;

	nsCOMPtr<nsIWebBrowserFocus> focus = do_GetInterface (webBrowser);
	NS_ENSURE_TRUE (focus, FALSE);

        nsCOMPtr<nsIDOMWindow> focusedDOMWindow;
	rv = focus->GetFocusedWindow (getter_AddRefs (focusedDOMWindow));
	if (NS_FAILED(rv))
	{
		rv = webBrowser->GetContentDOMWindow (getter_AddRefs (focusedDOMWindow));
	}
	if (!focusedDOMWindow) return FALSE;

	rv = focusedDOMWindow->ScrollBy (step_x, step_y);
	NS_ENSURE_SUCCESS (rv, FALSE);

	return TRUE;
}
