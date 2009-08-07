/*
 *  Copyright © 2009 Benjamin Otte
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
 */


#include "config.h"

#include "ephy-cell-renderer-tab.h"

enum {
  PROP_0,
  PROP_TAB,
  PROP_COLUMNS
};


#define EPHY_CELL_RENDERER_TAB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EPHY_TYPE_CELL_RENDERER_TAB, EphyCellRendererTabPrivate))

struct _EphyCellRendererTabPrivate
{
  EphyEmbed *   tab;            /* the tab we render */
  guint         n_columns;      /* number of columns we occupy */
};

G_DEFINE_DYNAMIC_TYPE (EphyCellRendererTab, ephy_cell_renderer_tab, GTK_TYPE_CELL_RENDERER)

void
ephy_cell_renderer_tab_register (GTypeModule *module)
{
  ephy_cell_renderer_tab_register_type (module);
}

static guint
ephy_cell_renderer_get_row_height (GtkWidget *widget)
{
  PangoFontMetrics *metrics;
  guint height;

  metrics = pango_context_get_metrics (gtk_widget_get_pango_context (widget),
                                       widget->style->font_desc,
                                       NULL);
  height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
                         pango_font_metrics_get_descent (metrics));

  pango_font_metrics_unref (metrics);

  return height;
}

static void
ephy_cell_renderer_tab_get_size (GtkCellRenderer *cell,
				 GtkWidget       *widget,
				 GdkRectangle    *cell_area,
				 gint            *x_offset,
				 gint            *y_offset,
				 gint            *width,
				 gint            *height)
{
  EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (cell);
  EphyCellRendererTabPrivate *priv = renderer->priv;

  priv = EPHY_CELL_RENDERER_TAB_GET_PRIVATE (cell);

  if (x_offset) *x_offset = 0;
  if (y_offset) *y_offset = 0;

  /*
   * We don't request any width as the width is determined by the user.
   * This way, we allow shrinking as much as the user wishes */
  if (width)
    *width = 2 * cell->xpad + 1;
  
  if (height)
    *height = 2 * cell->ypad + priv->n_columns * ephy_cell_renderer_get_row_height (widget);
}

static void
ephy_cell_renderer_tab_render (GtkCellRenderer      *cell,
                               GdkWindow            *window,
                               GtkWidget            *widget,
                               GdkRectangle         *background_area,
                               GdkRectangle         *cell_area,
                               GdkRectangle         *expose_area,
                               GtkCellRendererState  flags)

{
  EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (cell);
  EphyCellRendererTabPrivate *priv = renderer->priv;
  EphyWebView *view;
  WebKitWebView *webview;
  GdkPixbuf *pixbuf;
  cairo_t *cr;
  guint x, y, width, height, row_height, n_columns;
  int icon_width, icon_height;
  PangoLayout *layout;

  /* FIXME: render "add tab" button here */
  if (priv->tab == NULL)
    return;

  view = ephy_embed_get_web_view (priv->tab);
  webview = WEBKIT_WEB_VIEW (view);
  row_height = ephy_cell_renderer_get_row_height (widget);
  x = cell_area->x + cell->xpad;
  y = cell_area->y + cell->ypad;
  width = cell_area->width - 2 * cell->xpad;
  height = cell_area->height - 2 * cell->ypad;
  gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (widget),
                                     GTK_ICON_SIZE_MENU,
                                     &icon_width,
                                     &icon_height);
  n_columns = priv->n_columns;

  /* setup cairo context:
   * - clip to exposed area
   * - set origin correctly
   * - clip to cell area (minus padding)
   */
  cr = gdk_cairo_create (window);
  gdk_cairo_rectangle (cr, expose_area);
  cairo_clip (cr);
  cairo_translate (cr, x, y);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_clip (cr);

  /* render the progressbar */
  if (webkit_web_view_get_load_status (webview) != WEBKIT_LOAD_FINISHED)
    {
      GdkRectangle clip;
      gtk_paint_box (widget->style,
                     window,
                     GTK_STATE_NORMAL, GTK_SHADOW_IN,
                     NULL, widget, NULL,
                     x, y + height - row_height, width, row_height);
      clip.x = x;
      clip.y = y + height - row_height;
      clip.width = width * webkit_web_view_get_progress (webview) + 0.5;
      clip.height = row_height;
      gtk_paint_box (widget->style,
                     window,
                     GTK_STATE_SELECTED, GTK_SHADOW_OUT,
                     &clip, widget, "bar",
                     clip.x, clip.y,
                     clip.width, clip.height);
      if (n_columns > 1)
        n_columns--;
    }

  /* render the title */
  layout = gtk_widget_create_pango_layout (widget,
                                           ephy_web_view_get_title (view));
  pango_layout_set_width (layout, width * PANGO_SCALE);
  if (pango_layout_get_line_count (layout) >= n_columns)
    {
      if (ephy_web_view_get_icon (view))
        pango_layout_set_indent (layout, icon_width * PANGO_SCALE);
    }
  pango_layout_set_height (layout, -(int)n_columns);
  pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
  gtk_paint_layout (widget->style,
                    window,
                    GTK_STATE_NORMAL, /* FIXME */
		    TRUE,
                    expose_area,
                    widget,
                    "cellrenderertext",
                    x,
                    y,
                    layout);
  g_object_unref (layout);

  /* render the icons */
  cairo_translate (cr, cell_area->width, 0);
  pixbuf = gtk_widget_render_icon (widget, "gtk-close", GTK_ICON_SIZE_MENU, NULL);
  cairo_translate (cr, -icon_width, 0);
  gdk_cairo_set_source_pixbuf (cr, 
                               pixbuf, 
                               (icon_width - gdk_pixbuf_get_width (pixbuf)) / 2,
                               (icon_height - gdk_pixbuf_get_height (pixbuf)) / 2);
  cairo_paint_with_alpha (cr, flags & GTK_CELL_RENDERER_PRELIT ? 1.0 : 0.3);
  g_object_unref (pixbuf);

  cairo_destroy (cr);
}

static void
ephy_cell_renderer_tab_get_property (GObject        *object,
                                     guint           param_id,
                                     GValue         *value,
                                     GParamSpec     *pspec)
{
  EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (object);
  EphyCellRendererTabPrivate *priv = renderer->priv;
  
  switch (param_id)
    {
    case PROP_TAB:
      g_value_set_object (value, priv->tab);
      break;
    case PROP_COLUMNS:
      g_value_set_uint (value, priv->n_columns);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
ephy_cell_renderer_tab_set_property (GObject      *object,
                                     guint         param_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (object);
  EphyCellRendererTabPrivate *priv = renderer->priv;
  
  switch (param_id)
    {
    case PROP_TAB:
      if (priv->tab)
        g_object_unref (priv->tab);
      priv->tab = g_value_dup_object (value);
      break;
    case PROP_COLUMNS:
      priv->n_columns = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
ephy_cell_renderer_tab_finalize (GObject *object)
{
  EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (object);
  EphyCellRendererTabPrivate *priv = renderer->priv;

  if (priv->tab)
    g_object_unref (priv->tab);

  G_OBJECT_CLASS (ephy_cell_renderer_tab_parent_class)->finalize (object);
}

static void
ephy_cell_renderer_tab_class_finalize (EphyCellRendererTabClass *class)
{
}

static void
ephy_cell_renderer_tab_class_init (EphyCellRendererTabClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

  object_class->finalize = ephy_cell_renderer_tab_finalize;

  object_class->get_property = ephy_cell_renderer_tab_get_property;
  object_class->set_property = ephy_cell_renderer_tab_set_property;

  cell_class->get_size = ephy_cell_renderer_tab_get_size;
  cell_class->render = ephy_cell_renderer_tab_render;

  g_object_class_install_property (object_class,
				   PROP_TAB,
				   g_param_spec_object ("tab",
							"tab",
                                                        "the tab to render",
							EPHY_TYPE_EMBED,
							G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
				   PROP_COLUMNS,
				   g_param_spec_uint   ("columns",
							"columns",
                                                        "number of text columns to span",
                                                        1,
                                                        G_MAXUINT,
                                                        1,
							G_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (EphyCellRendererTabPrivate));
}

static void
ephy_cell_renderer_tab_init (EphyCellRendererTab *renderer)
{
  EphyCellRendererTabPrivate *priv;

  priv = renderer->priv = EPHY_CELL_RENDERER_TAB_GET_PRIVATE (renderer);

  priv->n_columns = 1;
}

GtkCellRenderer *
ephy_cell_renderer_tab_new (void)
{
  return g_object_new (EPHY_TYPE_CELL_RENDERER_TAB, NULL);
}

