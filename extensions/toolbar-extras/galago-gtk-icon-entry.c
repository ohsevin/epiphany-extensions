/**
 * @file libgalago-gtk/galago-gtk-icon-entry.c Entry widget
 *
 * @Copyright (C) 2004 Christian Hammond.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "config.h"

#include "galago-gtk-icon-entry.h"
#include <string.h>
#include <gtk/gtk.h>

#define ICON_MARGIN 4

struct _GalagoGtkIconEntryPriv
{
	GdkWindow *panel;

	GtkWidget *icon;

	gboolean icon_highlight;

	gboolean in_panel;
};

enum
{
	ICON_CLICKED,
	LAST_SIGNAL
};

static void galago_gtk_icon_entry_class_init(GalagoGtkIconEntryClass *klass);
static void galago_gtk_icon_entry_editable_init(GtkEditableClass *iface);
static void galago_gtk_icon_entry_init(GalagoGtkIconEntry *entry);
static void galago_gtk_icon_entry_finalize(GObject *obj);
static void galago_gtk_icon_entry_destroy(GtkObject *obj);
static void galago_gtk_icon_entry_map(GtkWidget *widget);
static void galago_gtk_icon_entry_unmap(GtkWidget *widget);
static void galago_gtk_icon_entry_realize(GtkWidget *widget);
static void galago_gtk_icon_entry_unrealize(GtkWidget *widget);
static void galago_gtk_icon_entry_size_request(GtkWidget *widget,
										  GtkRequisition *requisition);
static void galago_gtk_icon_entry_size_allocate(GtkWidget *widget,
										   GtkAllocation *allocation);
static gint galago_gtk_icon_entry_expose(GtkWidget *widget, GdkEventExpose *event);
static gint galago_gtk_icon_entry_enter_notify(GtkWidget *widget,
											   GdkEventCrossing *event);
static gint galago_gtk_icon_entry_leave_notify(GtkWidget *widget,
											   GdkEventCrossing *event);
static gint galago_gtk_icon_entry_button_press(GtkWidget *widget,
											   GdkEventButton *event);
static gint galago_gtk_icon_entry_button_release(GtkWidget *widget,
												 GdkEventButton *event);

static GtkEntryClass *parent_class = NULL;
static guint signals[LAST_SIGNAL] = {0};

GType
galago_gtk_icon_entry_get_type(void)
{
	static GType type = 0;

	if (!type)
	{
		static const GTypeInfo info =
		{
			sizeof(GalagoGtkIconEntryClass),
			NULL,
			NULL,
			(GClassInitFunc)galago_gtk_icon_entry_class_init,
			NULL,
			NULL,
			sizeof(GalagoGtkIconEntry),
			0,
			(GInstanceInitFunc)galago_gtk_icon_entry_init
		};

		static const GInterfaceInfo editable_info =
		{
			(GInterfaceInitFunc)galago_gtk_icon_entry_editable_init,
			NULL,
			NULL
		};

		type = g_type_register_static(GTK_TYPE_ENTRY, "GalagoGtkIconEntry",
									  &info, 0);

		g_type_add_interface_static(type, GTK_TYPE_EDITABLE, &editable_info);
	}

	return type;
}

static void
galago_gtk_icon_entry_class_init(GalagoGtkIconEntryClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkEntryClass *entry_class;

	parent_class = g_type_class_peek_parent(klass);

	gobject_class = G_OBJECT_CLASS(klass);
	object_class  = GTK_OBJECT_CLASS(klass);
	widget_class  = GTK_WIDGET_CLASS(klass);
	entry_class   = GTK_ENTRY_CLASS(klass);

	gobject_class->finalize = galago_gtk_icon_entry_finalize;

	object_class->destroy = galago_gtk_icon_entry_destroy;

	widget_class->map = galago_gtk_icon_entry_map;
	widget_class->unmap = galago_gtk_icon_entry_unmap;
	widget_class->realize = galago_gtk_icon_entry_realize;
	widget_class->unrealize = galago_gtk_icon_entry_unrealize;
	widget_class->size_request = galago_gtk_icon_entry_size_request;
	widget_class->size_allocate = galago_gtk_icon_entry_size_allocate;
	widget_class->expose_event = galago_gtk_icon_entry_expose;
	widget_class->enter_notify_event = galago_gtk_icon_entry_enter_notify;
	widget_class->leave_notify_event = galago_gtk_icon_entry_leave_notify;
	widget_class->button_press_event = galago_gtk_icon_entry_button_press;
	widget_class->button_release_event = galago_gtk_icon_entry_button_release;

	signals[ICON_CLICKED] =
		g_signal_new("icon_clicked",
					 G_TYPE_FROM_CLASS(gobject_class),
					 G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
					 G_STRUCT_OFFSET(GalagoGtkIconEntryClass, icon_clicked),
					 NULL, NULL,
					 g_cclosure_marshal_VOID__INT,
					 G_TYPE_NONE, 1,
					 G_TYPE_INT);
}

static void
galago_gtk_icon_entry_editable_init(GtkEditableClass *iface)
{
};

static void
galago_gtk_icon_entry_init(GalagoGtkIconEntry *entry)
{
	entry->priv = g_new0(GalagoGtkIconEntryPriv, 1);

	entry->priv->panel = NULL;
	entry->priv->icon  = NULL;
}

static void
galago_gtk_icon_entry_finalize(GObject *obj)
{
	GalagoGtkIconEntry *entry;

	g_return_if_fail(obj != NULL);
	g_return_if_fail(GALAGO_GTK_IS_ICON_ENTRY(obj));

	entry = GALAGO_GTK_ICON_ENTRY(obj);

	g_free(entry->priv);

	if (G_OBJECT_CLASS(parent_class)->finalize)
		G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
galago_gtk_icon_entry_destroy(GtkObject *obj)
{
	GalagoGtkIconEntry *entry;

	entry = GALAGO_GTK_ICON_ENTRY(obj);

	galago_gtk_icon_entry_set_icon(entry, NULL);

	if (GTK_OBJECT_CLASS(parent_class)->destroy)
		GTK_OBJECT_CLASS(parent_class)->destroy(obj);
}

static void
galago_gtk_icon_entry_map(GtkWidget *widget)
{
	if (GTK_WIDGET_REALIZED(widget) && !GTK_WIDGET_MAPPED(widget))
	{
		GTK_WIDGET_CLASS(parent_class)->map(widget);
		gdk_window_show(GALAGO_GTK_ICON_ENTRY(widget)->priv->panel);
	}
}

static void
galago_gtk_icon_entry_unmap(GtkWidget *widget)
{
	if (GTK_WIDGET_MAPPED(widget))
	{
		gdk_window_hide(GALAGO_GTK_ICON_ENTRY(widget)->priv->panel);
		GTK_WIDGET_CLASS(parent_class)->unmap(widget);
	}
}

static gint
get_icon_width(GalagoGtkIconEntry *entry)
{
	gint width;

	if (entry->priv->icon == NULL)
		return 0;

	if (entry->priv->panel != NULL)
	{
		GtkRequisition requisition;

		gtk_widget_size_request(entry->priv->icon, &requisition);

		width = requisition.width;
	}
	else
		width = 0;

	return width;
}

static void
galago_gtk_icon_entry_realize(GtkWidget *widget)
{
	GalagoGtkIconEntry *entry;
	GdkWindowAttr attributes;
	gint attributes_mask;
	guint real_width;
	gint icon_width;

	entry = GALAGO_GTK_ICON_ENTRY(widget);

	icon_width = get_icon_width(entry);

	real_width = widget->allocation.width;
//	widget->allocation.width -= icon_width + (icon_width > 0 ? ICON_MARGIN : 0);

	GTK_WIDGET_CLASS(parent_class)->realize(widget);

//	widget->allocation.width = real_width;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual(widget);
	attributes.colormap = gtk_widget_get_colormap(widget);
	attributes.event_mask = gtk_widget_get_events(widget);
	attributes.event_mask |=
		(GDK_EXPOSURE_MASK
		 | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		 | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	attributes.x = (widget->allocation.x +
					widget->allocation.width - icon_width -
					(icon_width > 0 ? ICON_MARGIN : 0));
	attributes.y = widget->allocation.y + (widget->allocation.height -
										   widget->requisition.height) / 2;
	attributes.width = icon_width + (icon_width > 0 ? ICON_MARGIN : 0);
	attributes.height = widget->requisition.height;

	entry->priv->panel = gdk_window_new(widget->window,
										&attributes, attributes_mask);
	gdk_window_set_user_data(entry->priv->panel, widget);

	gdk_window_set_background(entry->priv->panel,
							  &widget->style->base[GTK_WIDGET_STATE(widget)]);

	gtk_widget_queue_resize(widget);
}

static void
galago_gtk_icon_entry_unrealize(GtkWidget *widget)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);

	GTK_WIDGET_CLASS(parent_class)->unrealize(widget);

	if (entry->priv->panel != NULL)
	{
		gdk_window_set_user_data(entry->priv->panel, NULL);
		gdk_window_destroy(entry->priv->panel);
		entry->priv->panel = NULL;
	}
}

static void
get_borders(GalagoGtkIconEntry *entry, gint *xborder, gint *yborder)
{
	GtkWidget *widget = GTK_WIDGET(entry);
	gint focus_width;
	gboolean interior_focus;

	gtk_widget_style_get(widget,
						 "interior-focus", &interior_focus,
						 "focus-line-width", &focus_width,
						 NULL);

	if (gtk_entry_get_has_frame(GTK_ENTRY(entry)))
	{
		*xborder = widget->style->xthickness;
		*yborder = widget->style->ythickness;
	}
	else
	{
		*xborder = 0;
		*yborder = 0;
	}

	if (!interior_focus)
	{
		*xborder += focus_width;
		*yborder += focus_width;
	}
}

static void
get_text_area_size(GalagoGtkIconEntry *entry, gint *x, gint *y, gint *width,
				   gint *height)
{
	gint xborder, yborder;
	GtkRequisition requisition;
	GtkWidget *widget = GTK_WIDGET(entry);

	gtk_widget_get_child_requisition(widget, &requisition);

	get_borders(entry, &xborder, &yborder);

	if (x != NULL) *x = xborder;
	if (y != NULL) *y = yborder;

	if (width  != NULL) *width  = widget->allocation.width - xborder * 2;
	if (height != NULL) *height = requisition.height - yborder * 2;
}

static void
galago_gtk_icon_entry_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	GtkEntry *gtkentry;
	GalagoGtkIconEntry *entry;
	gint icon_width;

	gtkentry = GTK_ENTRY(widget);
	entry    = GALAGO_GTK_ICON_ENTRY(widget);

	icon_width = get_icon_width(entry);

	GTK_WIDGET_CLASS(parent_class)->size_request(widget, requisition);

	if (icon_width > requisition->width)
		requisition->width += icon_width + (icon_width > 0 ? ICON_MARGIN : 0);
}

static void
galago_gtk_icon_entry_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	GalagoGtkIconEntry *entry;
	GtkAllocation entry_allocation;
	GtkAllocation panel_allocation;
	gint icon_width;
	gint xborder, yborder, text_area_height;

	g_return_if_fail(GALAGO_GTK_IS_ICON_ENTRY(widget));
	g_return_if_fail(allocation != NULL);

	entry = GALAGO_GTK_ICON_ENTRY(widget);

	widget->allocation = *allocation;

	icon_width = get_icon_width(entry);

	get_text_area_size(entry, &xborder, &yborder, NULL, &text_area_height);

	entry_allocation.y = yborder;
	entry_allocation.width = allocation->width - icon_width - ICON_MARGIN;
	entry_allocation.height = text_area_height;

	panel_allocation.y = yborder;
	panel_allocation.width = icon_width;
	panel_allocation.height = text_area_height;

	if (gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL)
	{
		entry_allocation.x = xborder;
		panel_allocation.x = allocation->width -
		                     panel_allocation.width - xborder;
	}
	else
	{
		panel_allocation.x = xborder;
		entry_allocation.x = panel_allocation.x + panel_allocation.width;
	}

	GTK_WIDGET_CLASS(parent_class)->size_allocate(widget, allocation);

	if (GTK_WIDGET_REALIZED(widget))
	{
		gint x, y, width, height;

		gdk_window_get_position(GTK_ENTRY(widget)->text_area, &x, &y);
		gdk_drawable_get_size(GTK_ENTRY(widget)->text_area, &width, &height);

		gdk_window_move_resize(GTK_ENTRY(entry)->text_area,
							   entry_allocation.x,
							   entry_allocation.y,
							   entry_allocation.width,
							   entry_allocation.height);

		gdk_window_move_resize(entry->priv->panel,
							   panel_allocation.x,
							   panel_allocation.y,
							   panel_allocation.width,
							   panel_allocation.height);
	}
}

static GdkPixbuf *
get_pixbuf_from_icon(GalagoGtkIconEntry *entry)
{
	GdkPixbuf *pixbuf = NULL;
	gchar *stock_id;
	GtkIconSize size;

	switch (gtk_image_get_storage_type(GTK_IMAGE(entry->priv->icon)))
	{
		case GTK_IMAGE_PIXBUF:
			pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(entry->priv->icon));

			g_object_ref(pixbuf);
			break;

		case GTK_IMAGE_STOCK:
			gtk_image_get_stock(GTK_IMAGE(entry->priv->icon),
								&stock_id, &size);
			pixbuf = gtk_widget_render_icon(GTK_WIDGET(entry),
											stock_id, size, NULL);

			break;

		default:
			return NULL;
	}

	return pixbuf;
}

/* Kudos to the gnome-panel guys. */
static void
colorshift_pixbuf(GdkPixbuf *dest, GdkPixbuf *src, int shift)
{
	gint i, j;
	gint width, height, has_alpha, src_rowstride, dest_rowstride;
	guchar *target_pixels;
	guchar *original_pixels;
	guchar *pix_src;
	guchar *pix_dest;
	int val;
	guchar r, g, b;

	has_alpha       = gdk_pixbuf_get_has_alpha(src);
	width           = gdk_pixbuf_get_width(src);
	height          = gdk_pixbuf_get_height(src);
	src_rowstride   = gdk_pixbuf_get_rowstride(src);
	dest_rowstride  = gdk_pixbuf_get_rowstride(dest);
	original_pixels = gdk_pixbuf_get_pixels(src);
	target_pixels   = gdk_pixbuf_get_pixels(dest);

	for (i = 0; i < height; i++)
	{
		pix_dest = target_pixels   + i * dest_rowstride;
		pix_src  = original_pixels + i * src_rowstride;

		for (j = 0; j < width; j++)
		{
			r = *(pix_src++);
			g = *(pix_src++);
			b = *(pix_src++);

			val = r + shift;
			*(pix_dest++) = CLAMP(val, 0, 255);

			val = g + shift;
			*(pix_dest++) = CLAMP(val, 0, 255);

			val = b + shift;
			*(pix_dest++) = CLAMP(val, 0, 255);

			if (has_alpha)
				*(pix_dest++) = *(pix_src++);
		}
	}
}

static void
draw_icon(GtkWidget *widget)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);
	GdkPixbuf *pixbuf;
	gint x, y, width, height;

	if (entry->priv->icon == NULL)
		return;

	if ((pixbuf = get_pixbuf_from_icon(entry)) == NULL)
		return;

	gdk_drawable_get_size(entry->priv->panel, &width, &height);

	if (gdk_pixbuf_get_height(pixbuf) > height)
	{
		GdkPixbuf *temp_pixbuf;
		int scale;

		scale = height - (2 * ICON_MARGIN);

		temp_pixbuf = gdk_pixbuf_scale_simple(pixbuf, scale, scale,
											  GDK_INTERP_BILINEAR);

		g_object_unref(pixbuf);

		pixbuf = temp_pixbuf;
	}

	x = (width  - gdk_pixbuf_get_height(pixbuf)) / 2;
	y = (height - gdk_pixbuf_get_height(pixbuf)) / 2;

	if (entry->priv->in_panel)
	{
		GdkPixbuf *temp_pixbuf;

		temp_pixbuf = gdk_pixbuf_copy(pixbuf);

		colorshift_pixbuf(temp_pixbuf, pixbuf, 30);

		g_object_unref(pixbuf);

		pixbuf = temp_pixbuf;
	}

	gdk_draw_pixbuf(entry->priv->panel, widget->style->black_gc, pixbuf,
					0, 0, x, y, -1, -1,
					GDK_RGB_DITHER_NORMAL, 0, 0);

	g_object_unref(pixbuf);
}

static gint
galago_gtk_icon_entry_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GalagoGtkIconEntry *entry;

	g_return_val_if_fail(GALAGO_GTK_IS_ICON_ENTRY(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	entry = GALAGO_GTK_ICON_ENTRY(widget);

	if (GTK_WIDGET_DRAWABLE(widget))
	{
		if (event->window == entry->priv->panel)
		{
			gint width, area_height;

			get_text_area_size(entry, NULL, NULL, NULL, &area_height);

			gdk_drawable_get_size(entry->priv->panel, &width, NULL);

			gtk_paint_flat_box(widget->style, entry->priv->panel,
							   GTK_WIDGET_STATE(widget), GTK_SHADOW_NONE,
							   NULL, widget, "entry_bg",
							   0, 0, width, area_height);

			draw_icon(widget);
		}
		else
			GTK_WIDGET_CLASS(parent_class)->expose_event(widget, event);
	}

	return FALSE;
}

static void
update_icon(GObject *obj, GParamSpec *param, GalagoGtkIconEntry *entry)
{
	if (param != NULL)
	{
		const char *name = g_param_spec_get_name(param);

		if (strcmp(name, "pixbuf")   && strcmp(name, "stock")  &&
			strcmp(name, "image")    && strcmp(name, "pixmap") &&
			strcmp(name, "icon_set") && strcmp(name, "pixbuf_animation"))
		{
			return;
		}
	}

	gtk_widget_queue_resize(GTK_WIDGET(entry));
}

static gint
galago_gtk_icon_entry_enter_notify(GtkWidget *widget, GdkEventCrossing *event)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);

	if (event->window == entry->priv->panel)
	{
		if (galago_gtk_icon_entry_get_icon_highlight(entry))
		{
			entry->priv->in_panel = TRUE;

			update_icon(NULL, NULL, entry);
		}
	}

	return FALSE;
}

static gint
galago_gtk_icon_entry_leave_notify(GtkWidget *widget, GdkEventCrossing *event)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);

	if (event->window == entry->priv->panel)
	{
		if (galago_gtk_icon_entry_get_icon_highlight(entry))
		{
			entry->priv->in_panel = FALSE;

			update_icon(NULL, NULL, entry);
		}
	}

	return FALSE;
}

static gint
galago_gtk_icon_entry_button_press(GtkWidget *widget, GdkEventButton *event)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);

	if (event->window == entry->priv->panel)
	{
		if (event->button == 1)
		{
			if (galago_gtk_icon_entry_get_icon_highlight(entry))
			{
				entry->priv->in_panel = FALSE;

				update_icon(NULL, NULL, entry);
			}
		}

		return TRUE;
	}
	else
	{
		if (GTK_WIDGET_CLASS(parent_class)->button_press_event)
			return GTK_WIDGET_CLASS(parent_class)->button_press_event(widget,
																	  event);
	}

	return FALSE;
}

static gint
galago_gtk_icon_entry_button_release(GtkWidget *widget, GdkEventButton *event)
{
	GalagoGtkIconEntry *entry = GALAGO_GTK_ICON_ENTRY(widget);

	if (event->window == entry->priv->panel)
	{
		if (event->button == 1)
		{
			int width, height;

			gdk_drawable_get_size(entry->priv->panel, &width, &height);

			if (galago_gtk_icon_entry_get_icon_highlight(entry) &&
				event->x >= 0     && event->y >= 0 &&
				event->x <= width && event->y <= height)
			{
				entry->priv->in_panel = TRUE;

				update_icon(NULL, NULL, entry);
			}
		}

		g_signal_emit(entry, signals[ICON_CLICKED], 0, event->button);

		return TRUE;
	}
	else
	{
		if (GTK_WIDGET_CLASS(parent_class)->button_release_event)
			return GTK_WIDGET_CLASS(parent_class)->button_release_event(widget,
																		event);
	}

	return FALSE;
}

GtkWidget *
galago_gtk_icon_entry_new(void)
{
	return GTK_WIDGET(g_object_new(GALAGO_GTK_TYPE_ICON_ENTRY, NULL));
}

void
galago_gtk_icon_entry_set_icon(GalagoGtkIconEntry *entry, GtkWidget *icon)
{
	g_return_if_fail(entry != NULL);
	g_return_if_fail(GALAGO_GTK_IS_ICON_ENTRY(entry));
	g_return_if_fail(icon == NULL || GTK_IS_IMAGE(icon));

	if (icon == entry->priv->icon)
		return;

	if (icon == NULL)
	{
		if (entry->priv->icon != NULL)
		{
			gtk_widget_destroy(entry->priv->icon);
			entry->priv->icon = NULL;
		}
	}
	else
	{
		g_signal_connect(G_OBJECT(icon), "notify",
						 G_CALLBACK(update_icon), entry);

		entry->priv->icon = icon;
	}

	update_icon(NULL, NULL, entry);
}

void
galago_gtk_icon_entry_set_icon_highlight(GalagoGtkIconEntry *entry,
										 gboolean highlight)
{
	g_return_if_fail(entry != NULL);
	g_return_if_fail(GALAGO_GTK_IS_ICON_ENTRY(entry));

	if (entry->priv->icon_highlight == highlight)
		return;

	entry->priv->icon_highlight = highlight;
}

GtkWidget *
galago_gtk_icon_entry_get_icon(const GalagoGtkIconEntry *entry)
{
	g_return_val_if_fail(entry != NULL, NULL);
	g_return_val_if_fail(GALAGO_GTK_IS_ICON_ENTRY(entry), NULL);

	return entry->priv->icon;
}

gboolean
galago_gtk_icon_entry_get_icon_highlight(const GalagoGtkIconEntry *entry)
{
	g_return_val_if_fail(entry != NULL, FALSE);
	g_return_val_if_fail(GALAGO_GTK_IS_ICON_ENTRY(entry), FALSE);

	return entry->priv->icon_highlight;
}
