/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003, 2005 Christian Persch
 *  Copyright © 2005 Crispin Flowerday
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License scrollerpublished by
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
 */

#include "config.h"

#include "ephy-auto-scroller-extension.h"
#include "ephy-auto-scroller.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <libpeas/peas.h>

#define WINDOW_DATA_KEY	"EphyAutoScrollerExtension::WindowData"

static GType type = 0;

static void
ephy_auto_scroller_extension_init (EphyAutoScrollerExtension *extension)
{
}

static EphyAutoScroller *
ensure_auto_scroller (EphyWindow *window)
{
	EphyAutoScroller *scroller;

	scroller = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	if (scroller == NULL)
	{
		scroller = ephy_auto_scroller_new (window);
		g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY,
					scroller,
					(GDestroyNotify) g_object_unref);
	}

	return scroller;
}

static gboolean
dom_mouse_down_cb (EphyWebView *view,
		   GdkEventButton *event,
		   EphyWindow *window)
{
	EphyAutoScroller *scroller;
	guint context;
	guint button;
        WebKitHitTestResult *hit_test;

	button = event->button;
	hit_test = webkit_web_view_get_hit_test_result (WEBKIT_WEB_VIEW (view), event);
	g_object_get (hit_test, "context", &context, NULL);
	g_object_unref (hit_test);

	if (button != 2 ||
	    event->type != GDK_BUTTON_PRESS ||
	    (context & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE) ||
	    (context & WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK))
	{
		return FALSE;
	}

	scroller = ensure_auto_scroller (window);
	g_return_val_if_fail (scroller != NULL, FALSE);

	ephy_auto_scroller_start (scroller,
				  EPHY_GET_EMBED_FROM_EPHY_WEB_VIEW (view),
				  event->x_root, event->y_root);

	return TRUE;
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	WebKitWebView* web_view;
	LOG ("impl_attach_tab");

	g_return_if_fail (embed != NULL);

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);
	g_signal_connect_object (web_view, "button-press-event",
				 G_CALLBACK (dom_mouse_down_cb), window, 0);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
	WebKitWebView *web_view;
	LOG ("impl_detach_tab");

	g_return_if_fail (embed != NULL);

	web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);
	g_signal_handlers_disconnect_by_func
		(web_view, G_CALLBACK (dom_mouse_down_cb), window);
}

static void
ephy_auto_scroller_extension_iface_init (EphyExtensionIface *iface)
{
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

GType
ephy_auto_scroller_extension_get_type (void)
{
	return type;
}

GType
ephy_auto_scroller_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyAutoScrollerExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		NULL, /* class_init */
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (EphyAutoScrollerExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_auto_scroller_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_auto_scroller_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAutoScrollerExtension",
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
	ephy_auto_scroller_register_type (G_TYPE_MODULE (module));
	ephy_auto_scroller_extension_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    EPHY_TYPE_EXTENSION,
						    EPHY_TYPE_AUTO_SCROLLER_EXTENSION);
}
