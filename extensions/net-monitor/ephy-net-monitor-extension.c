/*
 *  Copyright (C) 2005 Jean-François Rameau
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

#include "ephy-net-monitor-extension.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-dbus.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-session.h>
#include <epiphany/ephy-embed-single.h>

#include <NetworkManager/NetworkManager.h>

#include "ephy-debug.h"

#include <gmodule.h>

#define EPHY_NET_MONITOR_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_NET_MONITOR_EXTENSION, EphyNetMonitorExtensionPrivate))

struct _EphyNetMonitorExtensionPrivate
{
	DBusConnection *bus;
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

/* Network status */
typedef enum 
{
	NETWORK_UP,
	NETWORK_DOWN
} NetworkStatus;

/* ask Epiphany to toggle offline mode on or off */
static void
ephy_net_monitor_set_mode (EphyNetMonitorExtension *net_monitor, NetworkStatus status)
{
	EphyEmbedSingle *single;

	LOG ("EphyNetMonitorExtension turning Epiphany to %s mode",
	     status == NETWORK_UP ? "online" : "offline");

	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (embed_shell));
	ephy_embed_single_set_network_status (single, status != NETWORK_DOWN);
}

static gboolean 
ephy_net_monitor_check_for_active_device (EphyNetMonitorExtension *net_monitor,
					  char *all_path[],
					  int num)
{
	int i;
	DBusMessage *	message = NULL;
	DBusMessage *	reply = NULL;
	const char *	iface = NULL;
	char *		op = NULL;
	dbus_uint32_t	type = 0;
	const char *	udi = NULL;
	dbus_bool_t	active = FALSE;
	const char *	ip4_address = NULL;
	const char *	broadcast = NULL;
	const char *	subnetmask = NULL;
	const char *	hw_addr = NULL;
	const char *	route = NULL;
	const char *	primary_dns = NULL;
	const char *	secondary_dns = NULL;
	dbus_uint32_t	mode = 0;
	dbus_int32_t	strength = -1;
	char *		active_network_path = NULL;
	dbus_bool_t	link_active = FALSE;
	dbus_uint32_t	caps = NM_DEVICE_CAP_NONE;
	char **		networks = NULL;
	int		num_networks = 0;
	NMActStage	act_stage = NM_ACT_STAGE_UNKNOWN;

	for (i = 0; i < num; i++)
	{
		const char *path = all_path [i];
		DBusError error;

		if (!(message = dbus_message_new_method_call (
						NM_DBUS_SERVICE, 
						path, 
						NM_DBUS_INTERFACE_DEVICES, 
						"getProperties")))
		{
			g_warning ("EphyNetMonitorExtension: couldn't allocate the dbus message");
			active = TRUE;
			break;
		}
		reply = dbus_connection_send_with_reply_and_block (net_monitor->priv->bus, message, -1, NULL);
		dbus_message_unref (message);
		if (!reply)
		{
			g_warning ("EphyNetMonitorExtension: "
					"didn't get a reply from NetworkManager for device %s.\n", path);
			active = TRUE;
			break;
		}

		dbus_error_init (&error);
		if (dbus_message_get_args (reply, &error, DBUS_TYPE_OBJECT_PATH, &op,
					DBUS_TYPE_STRING, &iface,
					DBUS_TYPE_UINT32, &type,
					DBUS_TYPE_STRING, &udi,
					DBUS_TYPE_BOOLEAN,&active,
					DBUS_TYPE_UINT32, &act_stage,
					DBUS_TYPE_STRING, &ip4_address,
					DBUS_TYPE_STRING, &subnetmask,
					DBUS_TYPE_STRING, &broadcast,
					DBUS_TYPE_STRING, &hw_addr,
					DBUS_TYPE_STRING, &route,
					DBUS_TYPE_STRING, &primary_dns,
					DBUS_TYPE_STRING, &secondary_dns,
					DBUS_TYPE_UINT32, &mode,
					DBUS_TYPE_INT32,  &strength,
					DBUS_TYPE_BOOLEAN,&link_active,
					DBUS_TYPE_UINT32, &caps,
					DBUS_TYPE_STRING, &active_network_path,
					DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &networks, &num_networks,
					DBUS_TYPE_INVALID))
		{
			dbus_message_unref (reply);

			/* found one active device */
			if (active) break;
		}
		else
		{
			g_warning ("EphyNetMonitorExtension: "
				   "unexpected reply from NetworkManager for device %s.\n", path);
			if (dbus_error_is_set(&error))
			{
				g_warning ("EphyNetMonitorExtension: %s: %s", error.name, error.message);
				dbus_error_free(&error);
			}

			active = TRUE;

			break;
		}
	}
	return active;
}

/* This is the heart of Net Monitor extension */
/* ATM, if there is an active device, we say that network is up: that's all ! */
static gboolean
ephy_net_monitor_network_status (EphyNetMonitorExtension *net_monitor)
{
	DBusMessage *message, *reply;
	DBusError error;
	NetworkStatus net_status;

	/* ask to Network Manager if there is at least one active device */
	message = dbus_message_new_method_call (NM_DBUS_SERVICE, 
						NM_DBUS_PATH, 
						NM_DBUS_INTERFACE, 
						"getDevices");

	if (message == NULL)
	{
		g_warning ("Couldn't allocate the dbus message");
		/* fallbak: let's Epiphany roll */
		return NETWORK_UP;
	}
	dbus_error_init (&error);
	reply = dbus_connection_send_with_reply_and_block (net_monitor->priv->bus, 
						 	   message, 
							   -1, 
							   &error);
	if (dbus_error_is_set (&error))
	{
		if (dbus_error_has_name (&error, NM_DBUS_NO_DEVICES_ERROR))
		{
			LOG ("EphyNetMonitorExtension: Network Manager says - no available network devices -");

			net_status = NETWORK_DOWN;
		}
		else
		{
			LOG ("EphyNetMonitorExtension can't talk to Network Manager: %s: %s", 
			     error.name, error.message);

			/* fallback */
			net_status = NETWORK_UP;
		}
		dbus_error_free(&error);
	}
	else
	{
		if (reply == NULL)
		{
			g_warning("EphyNetMonitorExtension got NULL reply");

			/* fallback */			
			net_status = NETWORK_UP;
		}
		else
		{
			/* check if we have at least one active device */
			gboolean active_device;
			char **paths = NULL;
			int num = -1;

			if (!dbus_message_get_args (reply, 
						    NULL, 
						    DBUS_TYPE_ARRAY, 
						    DBUS_TYPE_OBJECT_PATH, 
						   &paths, 
						   &num, 
						    DBUS_TYPE_INVALID))
			{
				g_warning ("EphyNetMonitorExtension: unexpected reply from NetworkManager");
				dbus_message_unref (reply);
				net_status = NETWORK_UP;
			}
			else
			{
				active_device = 
					ephy_net_monitor_check_for_active_device (
							net_monitor,
							paths,
							num);

				net_status = active_device ? NETWORK_UP : NETWORK_DOWN;
			}
		}
	}

	if (reply)
	{
		dbus_message_unref (reply);
	}
	
	if (message)
	{
		dbus_message_unref (message);
	}

	return net_status;
}

static void
ephy_net_monitor_check_network (EphyNetMonitorExtension *net_monitor)
{
	NetworkStatus net_status = ephy_net_monitor_network_status (net_monitor);

	LOG ("EphyNetMonitorExtension guesses the network is %s", 
	     net_status == NETWORK_UP ? "up" : "down");

	ephy_net_monitor_set_mode (net_monitor, net_status);
}

/* Filters all the messages from Network Manager */
static DBusHandlerResult
filter_func (DBusConnection *connection,
	     DBusMessage *message,
	     void *extension)
{
	EphyNetMonitorExtension *net_monitor;

	g_return_val_if_fail (extension != NULL, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

	net_monitor = EPHY_NET_MONITOR_EXTENSION (extension);

	if (dbus_message_is_signal (message,
				    NM_DBUS_INTERFACE,
				    "DeviceNoLongerActive"))
	{
		LOG ("EphyNetMonitorExtension catches DeviceNoLongerActive signal");

		ephy_net_monitor_check_network (net_monitor);

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	if (dbus_message_is_signal (message,
				    NM_DBUS_INTERFACE,
				    "DeviceNowActive"))
	{
		LOG ("EphyNetMonitorExtension catches DeviceNowActive signal");

		ephy_net_monitor_set_mode (net_monitor, NETWORK_UP);

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
ephy_net_monitor_attach_to_dbus (EphyNetMonitorExtension *net_monitor)
{
	DBusError error;
	EphyDbus *dbus;
	DBusGConnection *g_connection;
	
	LOG ("EphyNetMonitorExtension is trying to attach to SYSTEM bus");
	
	dbus = EPHY_DBUS (ephy_shell_get_dbus_service (ephy_shell));

	g_connection = ephy_dbus_get_bus (dbus, EPHY_DBUS_SYSTEM);
	g_return_if_fail (g_connection != NULL);
	
	net_monitor->priv->bus = dbus_g_connection_get_connection (g_connection);

	if (net_monitor->priv->bus != NULL)
	{
		dbus_connection_add_filter (net_monitor->priv->bus, 
					    filter_func, 
					    net_monitor, 
					    NULL);
		dbus_error_init (&error);
		dbus_bus_add_match (net_monitor->priv->bus, 
				    "type='signal',interface='" NM_DBUS_INTERFACE "'", 
				    &error);
		if (dbus_error_is_set(&error)) 
		{
			g_warning("EphyNetMonitorExtension cannot register signal handler: %s: %s", 
				  error.name, error.message);
			dbus_error_free(&error);
		}
		LOG ("EphyNetMonitorExtension attached to SYSTEM bus");
	}
}

static void
connect_to_system_bus_cb (EphyDbus *dbus,
			  EphyDbusBus kind,
			  EphyNetMonitorExtension *net_monitor)
{
	if (kind == EPHY_DBUS_SYSTEM)
	{
		LOG ("EphyNetMonitorExtension connecting to SYSTEM bus");

		ephy_net_monitor_attach_to_dbus (net_monitor);
	}
}

static void
disconnect_from_system_bus_cb (EphyDbus *dbus,
			       EphyDbusBus kind,
			       EphyNetMonitorExtension *net_monitor)
{
	if (kind == EPHY_DBUS_SYSTEM)
	{
		LOG ("EphyNetMonitorExtension disconnected from SYSTEM bus");

		/* no bus anymore */
		net_monitor->priv->bus = NULL;
	}
}


static void
ephy_net_monitor_startup (EphyNetMonitorExtension *net_monitor)
{
	EphyDbus *bus = EPHY_DBUS (ephy_shell_get_dbus_service (ephy_shell));

	LOG ("EphyNetMonitorExtension starting up");

	ephy_net_monitor_attach_to_dbus (net_monitor);

	/* DBUS may disconnect us at any time. So listen carefully to it */
	g_signal_connect (bus, 
			  "connected",  
			  G_CALLBACK (connect_to_system_bus_cb), 
			  net_monitor);
	g_signal_connect (bus, 
			  "disconnected",  
			  G_CALLBACK (disconnect_from_system_bus_cb), 
			  net_monitor);

	ephy_net_monitor_check_network (net_monitor);
}

static void
ephy_net_monitor_shutdown (EphyNetMonitorExtension *net_monitor)
{
	if (ephy_shell != NULL)
	{
		EphyDbus *bus = EPHY_DBUS (ephy_shell_get_dbus_service (ephy_shell));

		if (bus != NULL)
		{
			g_signal_handlers_disconnect_by_func(net_monitor, 
					G_CALLBACK (connect_to_system_bus_cb),
					bus);
			g_signal_handlers_disconnect_by_func(net_monitor, 
					G_CALLBACK (disconnect_from_system_bus_cb),
					bus);
		}
		net_monitor->priv->bus = NULL;
	}
	LOG ("EphyNetMonitorExtension down");
}

static void
ephy_net_monitor_extension_init (EphyNetMonitorExtension *extension)
{
	extension->priv = EPHY_NET_MONITOR_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyNetMonitorExtensionExtension initialising");

	ephy_net_monitor_startup (extension);
}

static void
ephy_net_monitor_extension_finalize (GObject *object)
{
	LOG ("EphyNetMonitorExtensionExtension finalising");

	EphyNetMonitorExtension *net_monitor = EPHY_NET_MONITOR_EXTENSION (object);

	ephy_net_monitor_shutdown (net_monitor);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_net_monitor_extension_class_init (EphyNetMonitorExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_net_monitor_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyNetMonitorExtensionPrivate));
}

GType
ephy_net_monitor_extension_get_type (void)
{
	return type;
}

GType
ephy_net_monitor_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyNetMonitorExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_net_monitor_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyNetMonitorExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_net_monitor_extension_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyNetMonitorExtension",
					    &our_info, 0);

	return type;
}
