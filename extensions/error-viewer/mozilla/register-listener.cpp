/*
 *  Copyright (C) 2001,2002,2003 Philip Langdale
 *  Copyright (C) 2003 Marco Pesenti Gritti
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

#include "ErrorViewerConsoleListener.h"

#include <nsIServiceManager.h>
#include <nsIConsoleService.h>
#include <nsCOMPtr.h>
#include <nsXPCOM.h>
#include <nsNetCID.h>

#include <glib-object.h>

extern "C" void *
mozilla_register_error_listener (GObject *dialog)
{
	ErrorViewerConsoleListener *listener;
	nsresult rv;

	nsCOMPtr<nsIConsoleService> consoleService =
		do_GetService (NS_CONSOLESERVICE_CONTRACTID, &rv);
	g_return_val_if_fail (NS_SUCCEEDED (rv), NULL);

	listener = new ErrorViewerConsoleListener ();
	consoleService->RegisterListener (listener);

	listener->mDialog = dialog;

	return (void *) listener;
}

extern "C" void
mozilla_unregister_error_listener (void *listener)
{
	nsresult rv;

	nsCOMPtr<nsIConsoleService> consoleService =
		do_GetService (NS_CONSOLESERVICE_CONTRACTID, &rv);
	g_return_if_fail (NS_SUCCEEDED (rv));

	consoleService->UnregisterListener ((ErrorViewerConsoleListener *) listener);

	/* FIXME: Are we leaking the listener? */
}
