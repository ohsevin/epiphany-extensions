/*
 *  Copyright © 2006 Jean-François Rameau
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

#include "mozilla-helper.h"
#include "LiveHTTPHeadersListener.h"

#include <nsIObserverService.h>
#include <nsCOMPtr.h>
#include <nsIServiceManager.h>
#include <nsIHttpProtocolHandler.h>
#include <nsServiceManagerUtils.h>

static LiveHTTPHeadersListener *listener;

extern "C" void
livehttpheaders_register_listener (void)
{
	nsresult rv;

	nsCOMPtr<nsIObserverService> observerService = do_GetService ("@mozilla.org/observer-service;1", &rv);
	g_return_if_fail (NS_SUCCEEDED (rv));

	listener = new LiveHTTPHeadersListener ();

	rv = observerService->AddObserver (listener, NS_HTTP_ON_MODIFY_REQUEST_TOPIC, PR_FALSE);
	rv = observerService->AddObserver (listener, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC, PR_FALSE);
}

extern "C" void
livehttpheaders_unregister_listener (void)
{
	nsresult rv;

	nsCOMPtr<nsIObserverService> observerService = do_GetService ("@mozilla.org/observer-service;1", &rv);
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = observerService->RemoveObserver (listener, NS_HTTP_ON_MODIFY_REQUEST_TOPIC);
	rv = observerService->RemoveObserver (listener, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC);
}

extern "C" GSList *
get_frames (void)
{
	return listener->GetFrames ();
}

extern "C" void
clear_frames (void)
{
	return listener->ClearFrames ();
}
