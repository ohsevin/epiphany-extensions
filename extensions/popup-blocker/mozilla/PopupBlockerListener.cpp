/*
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

#include <nsCOMPtr.h>
#include <nsString.h>
#include <nsIDOMPopupBlockedEvent.h>
#include <nsIURI.h>

#include "PopupBlockerListener.h"

#include "ephy-popup-blocker-extension.h"

/* Implementation file */
NS_IMPL_ISUPPORTS1(PopupBlockerListener, nsIDOMEventListener)

PopupBlockerListener::PopupBlockerListener()
{
	/* member initializers and constructor code */
	mOwner = nsnull;
}

PopupBlockerListener::~PopupBlockerListener()
{
	 /* destructor code */
}

/* void handleEvent (in nsIDOMEvent event); */
NS_IMETHODIMP PopupBlockerListener::HandleEvent(nsIDOMEvent *event)
{
	nsresult rv;

	nsCOMPtr<nsIDOMPopupBlockedEvent> popupEvent =
		do_QueryInterface (event, &rv);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	nsCOMPtr<nsIURI> popupWindowURI;
	rv = popupEvent->GetPopupWindowURI (getter_AddRefs (popupWindowURI));
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	nsCAutoString popupWindowURIString;
	rv = popupWindowURI->GetSpec (popupWindowURIString);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

#if MOZILLA_SNAPSHOT >= 14
	nsAutoString popupWindowFeatures;
	rv = popupEvent->GetPopupWindowFeatures (popupWindowFeatures);
	NS_ENSURE_SUCCESS (rv, NS_ERROR_FAILURE);

	ephy_popup_blocker_extension_block (mOwner,
					    popupWindowURIString.get (),
					    NS_ConvertUCS2toUTF8 (popupWindowFeatures).get ());
#else
	ephy_popup_blocker_extension_block (mOwner, popupWindowURIString.get (),
					    NULL);
#endif
}

nsresult
PopupBlockerListener::Init(EphyEmbed *aOwner)
{
	mOwner = aOwner;
	return NS_OK;
}
