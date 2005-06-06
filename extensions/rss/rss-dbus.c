/*
*  Copyright (C) 2005 Raphaël Slinckx <raphael@slinckx.net>
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

#include "config.h"

#include "rss-dbus.h"

#include "ephy-debug.h"

#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-dbus.h>

gboolean
rss_dbus_subscribe_feed (const char *address)
{
	DBusMessage *message, *reply;
	DBusError error;
	gboolean success;
	DBusConnection *bus;
#ifndef HAVE_NEW_DBUS
	DBusMessageIter iter;
#endif

	bus = ephy_dbus_get_bus (
				EPHY_DBUS (ephy_shell_get_dbus_service (ephy_shell_get_default ())),
				EPHY_DBUS_SESSION);
		
	if (address == NULL || bus == NULL)
	{	
		LOG ("Feed is NULL, or no connection to dbus");
		return FALSE;
	}
	
	LOG ("Performing dbus remote call");
	
	/* ask the feed reader to register the given feed */
	message = dbus_message_new_method_call (RSS_DBUS_SERVICE, 
						RSS_DBUS_OBJECT_PATH, 
						RSS_DBUS_INTERFACE, 
						RSS_DBUS_SUBSCRIBE);

	if (message == NULL)
	{
		LOG ("Couldn't allocate the dbus rss message");
		return FALSE;
	}
	
	/* Build the dbus mesage containing the url a a string */
#ifdef HAVE_NEW_DBUS
	dbus_message_append_args (message, DBUS_TYPE_STRING, &address);
#else
	dbus_message_iter_init (message, &iter);
	dbus_message_iter_append_string (&iter, address);
#endif

	dbus_error_init (&error);
	reply = dbus_connection_send_with_reply_and_block (bus, message, 1000, &error);
	dbus_message_unref (message);
	
	if (dbus_error_is_set (&error))
	{
		LOG ("Failed to connect to a Feed Reader: %s: %s", error.name, error.message);
		dbus_error_free (&error);
		return FALSE;
	}

	if (reply == NULL)
	{
		LOG ("Failed to retreive dbus reply, got NULL");
		dbus_error_free (&error);
		return FALSE;
	}

	/* We got an answer */
	LOG ("Received a subscription confirmation");
	
	dbus_message_get_args (reply, &error, DBUS_TYPE_BOOLEAN, &success, DBUS_TYPE_INVALID);
	dbus_message_unref (reply);
	
	if (dbus_error_is_set (&error))
	{
		LOG ("Error while retreiveing method answer: %s: %s", error.name, error.message);
		dbus_error_free (&error);
		return FALSE;
	}

	LOG ("Successfully retreived boolean answer");
	dbus_error_free (&error);

	return success;
}
