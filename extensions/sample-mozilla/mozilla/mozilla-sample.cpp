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

#include "mozilla-sample.h"

#include <glib.h>

#include <nsCOMPtr.h>
#include <nsIDOMEvent.h>
#include <nsIDOMMouseEvent.h>

void
mozilla_do_something (gpointer dom_event)
{
	nsCOMPtr<nsIDOMMouseEvent> ev = static_cast<nsIDOMMouseEvent*>(dom_event);
	NS_ENSURE_TRUE (ev,);

	nsresult rv;
	PRUint16 button;
	rv = ev->GetButton (&button);
	NS_ENSURE_SUCCESS (rv,);

	g_print ("Button %u\n", button);
	
}