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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-sample2-extension.h"
#include "mozilla-sample.h"
#include "ephy-debug.h"

#include <epiphany/ephy-extension.h>

#include <gmodule.h>

#define EPHY_SAMPLE2_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SAMPLE2_EXTENSION, EphySample2ExtensionPrivate))

struct EphySample2ExtensionPrivate
{
};

static void ephy_sample2_extension_class_init	(EphySample2ExtensionClass *klass);
static void ephy_sample2_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_sample2_extension_init		(EphySample2Extension *extension);

enum
{
	PROP_0
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_sample2_extension_get_type (void)
{
	return type;
}

GType
ephy_sample2_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphySample2ExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sample2_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySample2Extension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sample2_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_sample2_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphySample2Extension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_sample2_extension_init (EphySample2Extension *extension)
{
	extension->priv = EPHY_SAMPLE2_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphySample2Extension initialising")
}

static void
ephy_sample2_extension_finalize (GObject *object)
{
/*
	EphySample2Extension *extension = EPHY_SAMPLE2_EXTENSION (object);
*/
	LOG ("EphySample2Extension finalising")

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphySample2Extension *extension)
{
	gpointer dom_event;

	dom_event = ephy_embed_event_get_dom_event (event);
	LOG ("DOM Event %p", dom_event)

	mozilla_do_something (dom_event);
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyEmbed *embed,
	      EphySample2Extension *extension)
{
	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), extension);
}

static void
tab_removed_cb (EphyEmbed *notebook,
		EphyEmbed *embed,
		EphySample2Extension *extension)
{
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), extension);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	LOG ("EphySample2Extension attach_window")

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_added_cb), ext);
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_removed_cb), ext);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;

	LOG ("EphySample2Extension detach_window")

	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_added_cb), ext);
	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (tab_removed_cb), ext);
}

static void
ephy_sample2_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_sample2_extension_class_init (EphySample2ExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_sample2_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphySample2ExtensionPrivate));
}
