/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
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

#include "ephy-tab-key-tab-navigate-extension.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>
#include <gdk/gdkkeysyms.h>

#include <libpeas/peas.h>

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType	ephy_tab_key_tab_navigate_extension_register_type	(GTypeModule *module);

static void set_tab_offset (EphyWindow *window, gint offset)
{
	GtkWidget *notebook;
	gint index, count, new_index;

	notebook = ephy_window_get_notebook (window);
	index = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));
	count = gtk_notebook_get_n_pages (GTK_NOTEBOOK(notebook));
	new_index = (index + offset) % count;

	gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook), new_index);
}

static gboolean key_press_cb (EphyWindow *window, GdkEventKey *event, gpointer data)
{
	if (event->state & GDK_CONTROL_MASK) {
		if (event->keyval == GDK_KEY_Tab) {
			set_tab_offset (window, 1);
		} else if (event->keyval == GDK_KEY_ISO_Left_Tab) {
			set_tab_offset (window, -1);
		} else {
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

static void
ephy_tab_key_tab_navigate_extension_init (EphyTabKeyTabNavigateExtension *extension)
{
	LOG ("EphyTabKeyTabNavigateExtension initialising");
}

static void
ephy_tab_key_tab_navigate_extension_finalize (GObject *object)
{
	LOG ("EphyTabKeyTabNavigateExtension finalising");

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
impl_attach_window (EphyExtension *ext,
		EphyWindow *window)
{
	LOG ("EphyTabKeyTabNavigateExtension attach_window");
	g_signal_connect (GTK_WIDGET(window), "key-press-event", G_CALLBACK (key_press_cb), NULL);
}

static void
impl_detach_window (EphyExtension *ext,
		EphyWindow *window)
{
	LOG ("EphyTabKeyTabNavigateExtension detach_window");
	g_signal_handlers_disconnect_by_func (GTK_WIDGET(window), G_CALLBACK (key_press_cb), NULL);
}

static void
ephy_tab_key_tab_navigate_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tab_key_tab_navigate_extension_class_init (EphyTabKeyTabNavigateExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tab_key_tab_navigate_extension_finalize;
}

GType
ephy_tab_key_tab_navigate_extension_get_type (void)
{
	return type;
}

GType
ephy_tab_key_tab_navigate_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyTabKeyTabNavigateExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tab_key_tab_navigate_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTabKeyTabNavigateExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_tab_key_tab_navigate_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tab_key_tab_navigate_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
			G_TYPE_OBJECT,
			"EphyTabKeyTabNavigateExtension",
			&our_info, 0);

	g_type_module_add_interface (module,
			type,
			EPHY_TYPE_EXTENSION,
			&extension_info);

	return type;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	ephy_tab_key_tab_navigate_extension_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    EPHY_TYPE_EXTENSION,
						    EPHY_TYPE_TAB_KEY_TAB_NAVIGATE_EXTENSION);
}
