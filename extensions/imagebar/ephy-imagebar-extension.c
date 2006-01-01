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

#include "ephy-imagebar-extension.h"
#include "ephy-debug.h"
#include "mozilla-event.h"

#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-embed-factory.h>
#include <epiphany/ephy-embed-persist.h>

#include <string.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <gtkembedmoz/gtkmozembed.h>

#define EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_IMAGEBAR_EXTENSION, EphyImagebarExtensionPrivate))

struct _EphyImagebarExtensionPrivate
{
	guint			delay;
	GtkWidget *		popup;
	char *			src;
	EphyEmbed *		embed;
};

enum
{
	PROP_0
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
save_button_clicked_cb (GtkButton *button,
			EphyImagebarExtensionPrivate *priv)
{
	EphyEmbedPersist *persist;

	persist = EPHY_EMBED_PERSIST (ephy_embed_factory_new_object
						(EPHY_TYPE_EMBED_PERSIST));
	ephy_embed_persist_set_flags (persist,
				      EPHY_EMBED_PERSIST_NO_VIEW |
				      EPHY_EMBED_PERSIST_FROM_CACHE);

	ephy_embed_persist_set_embed (persist, priv->embed);
	ephy_embed_persist_set_source (persist, priv->src);
	ephy_embed_persist_save(persist);

	g_object_unref (persist);
}

static gboolean
leave_notify_event_cb (GtkWidget *widget,
		       GdkEventCrossing *event)
{
	/* XXX: hide if mouse is not over image element of GtkMozEmbed */
	gtk_widget_hide (widget);

	return FALSE; /* signal goes trigger on */
}

static void
ephy_imagebar_extension_init (EphyImagebarExtension *ext)
{
	EphyImagebarExtensionPrivate *priv;
	GtkWidget *toolbar;
	GtkToolItem *item;

	LOG ("EphyImagebarExtension initialising");

	priv = ext->priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (ext);

	priv->delay = 1; /* FIXME: configurable delay */
	priv->src = NULL;

	item = gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
	g_signal_connect (G_OBJECT (item), "clicked",
			  G_CALLBACK (save_button_clicked_cb), priv);
	gtk_widget_show (GTK_WIDGET (item));

	toolbar = gtk_toolbar_new ();
	/* gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU); */
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), FALSE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
	gtk_widget_show (toolbar);

	priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
	g_signal_connect (G_OBJECT (priv->popup), "leave-notify-event",
			  G_CALLBACK (leave_notify_event_cb), NULL);
	gtk_container_add (GTK_CONTAINER (priv->popup), toolbar);
}

static void
ephy_imagebar_extension_finalize (GObject *object)
{
	EphyImagebarExtension *ext;
	EphyImagebarExtensionPrivate *priv;

	LOG ("EphyImagebarExtension finalising");

	ext = EPHY_IMAGEBAR_EXTENSION (object);

	priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (ext);
/* FIXME: crash on unref. already done? */
/*
	g_object_unref (G_OBJECT (priv->popup));
*/
	g_free (priv->src);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
dom_mouse_over_cb (GtkMozEmbed *embed,
		   gpointer dom_event,
		   EphyImagebarExtensionPrivate *priv)
{
	GtkWidget *toplevel;
	gint32 x, y;
	gint tmp_x, tmp_y;
	gboolean is_image = FALSE, is_forced = FALSE, is_anchored = FALSE;
	char *img_src;

	evaluate_dom_event (dom_event, &is_image, &is_forced, &is_anchored,
				       &x, &y, &img_src);
	if (!is_image || (is_anchored && !is_forced)) return FALSE;

	if (priv->src != NULL) g_free (priv->src);
	priv->src = img_src;

	/* Popup an widget at explicit position, as gtk_menu_popup() does.
	 * GtkToolbar with tearoff handler might be the best way, if possible.
	 */

	/* FIXME: configurable offset */
	if (x < 0) x = 1;
	if (y < 0) y = 1;

	/* Immediately, forced */
	if (is_forced)
	{
		/* TODO: reset timeout delay */
	}

	/* Include window position without frame */
	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (embed));
	if (GTK_WIDGET_TOPLEVEL (toplevel))
	{
		gdk_window_get_position (toplevel->window,
					 &tmp_x, &tmp_y);
		x += tmp_x;
		y += tmp_y;
	}

	/* Include GtkMozEmbed position */
	gdk_window_get_position (GTK_WIDGET (embed)->window,
				 &tmp_x, &tmp_y);
	x += tmp_x;
	y += tmp_y;

	/* XXX: Avoid moving to same position */
	/* FIXME: first orrurance failed as gdk_window has not built yet */
	gdk_window_get_position (priv->popup->window,
				 &tmp_x, &tmp_y);
	if (x != tmp_x || y != tmp_y)
	{
		/* FIXME: configurable offset */
		x += 1;
		y += 1;

		gtk_window_move (GTK_WINDOW (priv->popup), x, y);
		gtk_window_present (GTK_WINDOW (priv->popup));
	}
	
	return FALSE; /* signal goes trigger on */
}

static gboolean
dom_mouse_out_cb (GtkMozEmbed *embed,
		  gpointer dom_event,
		  EphyImagebarExtensionPrivate *priv)
{
	gint x, y;

	/* XXX: Only hide when not into toolbar */
	gtk_widget_get_pointer (priv->popup, &x, &y);
	if (x < 0 || x > priv->popup->allocation.width
	 || y < 0 || y > priv->popup->allocation.height)
	{
		gtk_widget_hide (priv->popup);
	}

	return FALSE; /* signal goes trigger on */
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyImagebarExtensionPrivate *priv;
	EphyEmbed *embed;

	LOG ("attach_tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (GTK_IS_MOZ_EMBED (embed));

	priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (ext);
	priv->embed = embed;

	g_signal_connect (G_OBJECT (embed), "dom_mouse_over",
			  G_CALLBACK (dom_mouse_over_cb),
			  priv);
	g_signal_connect (G_OBJECT (embed), "dom_mouse_out",
			  G_CALLBACK (dom_mouse_out_cb),
			  priv);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyImagebarExtensionPrivate *priv;
	EphyEmbed *embed;

	LOG ("detach_tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (GTK_IS_MOZ_EMBED (embed));

	priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (embed);

	g_signal_handlers_disconnect_by_func (G_OBJECT (embed),
					      G_CALLBACK (dom_mouse_over_cb),
					      priv);
	g_signal_handlers_disconnect_by_func (G_OBJECT (embed),
					      G_CALLBACK (dom_mouse_out_cb),
					      priv);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyImagebarExtensionPrivate *priv;

	LOG ("attach_window");

	priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (ext);

	gtk_window_set_transient_for (GTK_WINDOW (priv->popup),
				      GTK_WINDOW (window));
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyImagebarExtensionPrivate *priv;
	
	LOG ("detach_window");

	priv = EPHY_IMAGEBAR_EXTENSION_GET_PRIVATE (ext);
}

static void
ephy_imagebar_extension_set_property (GObject *object,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *pspec)
{
/*
	EphyImagebarExtension *extension = EPHY_IMAGEBAR_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_imagebar_extension_get_property (GObject *object,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *pspec)
{
/*
	EphyImagebarExtension *extension = EPHY_IMAGEBAR_EXTENSION (object);

	switch (prop_id)
	{
	}
*/
}

static void
ephy_imagebar_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_imagebar_extension_class_init (EphyImagebarExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_imagebar_extension_finalize;
	object_class->get_property = ephy_imagebar_extension_get_property;
	object_class->set_property = ephy_imagebar_extension_set_property;

	g_type_class_add_private (object_class, sizeof (EphyImagebarExtensionPrivate));
}

GType
ephy_imagebar_extension_get_type (void)
{
	return type;
}

GType
ephy_imagebar_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyImagebarExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_imagebar_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyImagebarExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_imagebar_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_imagebar_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyImagebarExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}
