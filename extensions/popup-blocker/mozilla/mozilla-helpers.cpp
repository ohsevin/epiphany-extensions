/*
 *  Copyright (C) 2000-2003 Marco Pesenti Gritti
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
#include "PopupBlockerListener.h"

#include <nsCOMPtr.h>
#include <nsIChromeEventHandler.h>
#include <nsIDOMEventReceiver.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMWindow.h>
#include <nsIDOMWindowInternal.h>
#include <nsIWebBrowser.h>
#include <nsPIDOMWindow.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

extern "C" PopupListenerFreeData *
mozilla_register_popup_listener (EphyEmbed *embed)
{
	nsresult rv;

	NS_ENSURE_TRUE (GTK_IS_MOZ_EMBED (embed), NULL);

	nsCOMPtr<nsIWebBrowser> browser;
	gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
					 getter_AddRefs (browser));
	NS_ENSURE_TRUE (browser, NULL);

	nsCOMPtr<nsIDOMWindow> domWindowExternal;
	rv = browser->GetContentDOMWindow (getter_AddRefs (domWindowExternal));
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCOMPtr<nsIDOMWindowInternal> domWindow;
	domWindow = do_QueryInterface (domWindowExternal, &rv);
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCOMPtr<nsPIDOMWindow> piWin(do_QueryInterface (domWindow, &rv));
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCOMPtr<nsIChromeEventHandler> chromeHandler;
	rv = piWin->GetChromeEventHandler (getter_AddRefs (chromeHandler));
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCOMPtr<nsIDOMEventReceiver> eventReceiver;
	eventReceiver = do_QueryInterface (chromeHandler, &rv);
	NS_ENSURE_SUCCESS (rv, NULL);

	nsCOMPtr<nsIDOMEventTarget> target;
	target = do_QueryInterface (eventReceiver, &rv);
	NS_ENSURE_SUCCESS (rv, NULL);

	PopupBlockerListener *listener = new PopupBlockerListener ();
	if (!listener)
	{
		g_warning ("Could not create popup listener\n");
		return NULL;
	}

	rv = listener->Init (embed);
	NS_ENSURE_SUCCESS (rv, NULL);

	rv = target->AddEventListener (NS_LITERAL_STRING ("DOMPopupBlocked"),
				       listener, PR_FALSE);
	NS_ENSURE_SUCCESS (rv, NULL);

	nsIDOMEventTarget *c_target = target.get();
	NS_ADDREF (c_target);

	PopupListenerFreeData *ret = g_new0 (PopupListenerFreeData, 1);
	ret->listener = listener;
	ret->eventTarget = target;

	return ret;
}

extern "C" void
mozilla_unregister_popup_listener (PopupListenerFreeData *data)
{
	/* FIXME: Error checking? */
	PopupBlockerListener *listener =
		(PopupBlockerListener *) data->listener;
	nsIDOMEventTarget *target = (nsIDOMEventTarget *) data->eventTarget;

	target->RemoveEventListener (NS_LITERAL_STRING ("DOMPopupBlocked"),
				     listener, PR_FALSE);

	NS_RELEASE (target);
	//delete listener;
}
