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

#include "config.h"

#include "ephy-find-extension.h"
#include "ephy-find-bar.h"
#include "mozilla-find.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>
#include "ephy-debug.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkvbox.h>

#include <gmodule.h>

#define EPHY_FIND_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_FIND_EXTENSION, EphyFindExtensionPrivate))

struct EphyFindExtensionPrivate
{
	gpointer dummy;
};

static void ephy_find_extension_class_init	(EphyFindExtensionClass *klass);
static void ephy_find_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_find_extension_init		(EphyFindExtension *extension);

enum
{
	PROP_0
};

#define TOOLBAR_DATA_KEY	"EphyFindToolbar"

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_find_extension_get_type (void)
{
	return type;
}

GType
ephy_find_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyFindExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_find_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyFindExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_find_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_find_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyFindExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_find_extension_init (EphyFindExtension *extension)
{
	extension->priv = EPHY_FIND_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyFindExtension initialising")
}

static void
ephy_find_extension_finalize (GObject *object)
{
/*
	EphyFindExtension *extension = EPHY_FIND_EXTENSION (object);
*/
	LOG ("EphyFindExtension finalising")

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
toolbar_weak_unref_cb (EphyWindow *window,
		       GtkWidget *zombie)
{
	LOG ("weak unref window %p", window)

	g_object_set_data (G_OBJECT (window), TOOLBAR_DATA_KEY, NULL);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *toolbar, *notebook, *parent;
//	GValue value = { 0, };

	LOG ("attach_window")

	/* create find toolbar */
	toolbar = ephy_find_bar_new (window);
	g_object_set_data (G_OBJECT (window), TOOLBAR_DATA_KEY, toolbar);
	g_object_weak_ref (G_OBJECT (toolbar), (GWeakNotify) toolbar_weak_unref_cb, window);

	/* add toolbar to the window */
	notebook = ephy_window_get_notebook (window);
	/* the notebook is not directly in the main vbox in case
	 * the sidebar extension has first dibs on the window
	 */
	parent = gtk_widget_get_ancestor (notebook, GTK_TYPE_VBOX);
#if 0
	g_value_init (&position, G_TYPE_INT);
	gtk_container_child_get_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);
#endif
	gtk_box_pack_start (GTK_BOX (parent), toolbar, FALSE, FALSE, 0);
#if 0
	gtk_container_child_set_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);
	g_value_unset (&position);
#endif
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *toolbar;

	LOG ("detach_window")

	/* remove the find toolbar */
	toolbar = g_object_get_data (G_OBJECT (window), TOOLBAR_DATA_KEY);
	if (toolbar != NULL)
	{
		gtk_container_remove (GTK_CONTAINER (toolbar->parent), toolbar);
	}
}

static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("detach_tab")

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (embed != NULL);

	mozilla_find_detach (embed);
}

static void
ephy_find_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_find_extension_class_init (EphyFindExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_find_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyFindExtensionPrivate));
}
