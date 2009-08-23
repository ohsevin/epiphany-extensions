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

enum {
  ICON_CLICKED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };


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

static gboolean
ephy_cell_renderer_tab_activate (GtkCellRenderer      *cell,
				 GdkEvent             *event,
				 GtkWidget            *widget,
				 const gchar          *path,
				 GdkRectangle         *background_area,
				 GdkRectangle         *cell_area,
				 GtkCellRendererState  flags)
{
  //EphyCellRendererTab *renderer = EPHY_CELL_RENDERER_TAB (cell);
  //EphyCellRendererTabPrivate *priv = renderer->priv;
  int x, y;
  int icon_width, icon_height;

  /* We only handle mouse events for the small buttons */
  if (event->type != GDK_BUTTON_PRESS)
    return FALSE;

  x = event->button.x - cell_area->x - cell->xpad;
  y = event->button.y - cell_area->y - cell->ypad;
  gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (widget),
                                     GTK_ICON_SIZE_MENU,
                                     &icon_width,
                                     &icon_height);
  /* clicked below icon */
  if (y >= icon_height)
    return FALSE;
  /* convert to number of icon that was clicked. */
  x = (cell_area->width - 2 * cell->xpad - x) / icon_width;
  if (x >= 1)
    return FALSE;

  g_signal_emit (cell, signals[ICON_CLICKED], 0, path, x);

  return TRUE;
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
  if (ephy_web_view_get_icon (view))
    pango_layout_set_indent (layout, icon_width * PANGO_SCALE);
  /* 
   * If wrapping words is not enough, wrap chars instead.
   * This way, as much title text as possible is visible.
   */
  if (pango_layout_get_line_count (layout) > n_columns)
    pango_layout_set_wrap (layout, PANGO_WRAP_CHAR);
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
ephy_marshal_VOID__STRING_UINT (GClosure     *closure,
                               GValue       *return_value G_GNUC_UNUSED,
                               guint         n_param_values,
                               const GValue *param_values,
                               gpointer      invocation_hint G_GNUC_UNUSED,
                               gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_UINT) (gpointer     data1,
                                                  const char * arg_2,
                                                  guint        arg_1,
                                                  gpointer     data2);
  register GMarshalFunc_VOID__STRING_UINT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_UINT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_string (param_values + 1),
            g_value_get_uint (param_values + 2),
            data2);
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
  cell_class->activate = ephy_cell_renderer_tab_activate;

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

  /**
   * EphyCellRendererTab::icon-clicked:
   * @cell_renderer: the object which received the signal
   * @path: string representation of #GtkTreePath describing the 
   *        event location
   * @icon: id of the icon that was clicked
   *
   * The signal is emitted when one of the icons at the top left of the renderer
   * is clicked.
   **/
  signals[ICON_CLICKED] = g_signal_new ("icon-clicked",
                                        G_OBJECT_CLASS_TYPE (object_class),
                                        G_SIGNAL_RUN_LAST,
                                        0, NULL, NULL,
                                        ephy_marshal_VOID__STRING_UINT,
                                        G_TYPE_NONE, 2,
                                        G_TYPE_STRING, G_TYPE_UINT);

  g_type_class_add_private (object_class, sizeof (EphyCellRendererTabPrivate));
}

static void
ephy_cell_renderer_tab_init (EphyCellRendererTab *renderer)
{
  EphyCellRendererTabPrivate *priv;

  priv = renderer->priv = EPHY_CELL_RENDERER_TAB_GET_PRIVATE (renderer);

  priv->n_columns = 1;

  GTK_CELL_RENDERER (renderer)->mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
}

GtkCellRenderer *
ephy_cell_renderer_tab_new (void)
{
  return g_object_new (EPHY_TYPE_CELL_RENDERER_TAB, NULL);
}

/**
 * ephy_cell_renderer_needs_invalidation:
 * @property_name: interned name of the property
 *
 * Checks if a #EphyCellRendererTab would need a repaint if the 
 * property with the given @property_name would have been changed 
 * on the #EphyWebView asociated to the #EphyEmbed of its tab. In 
 * that case, you should likely call gtk_tree_model_row_changed()
 * on the row that represents the node that changed to cause this 
 * repaint.
 *
 * Returns: %TRUE if a tab would need a repaint, %FALSE if not.
 **/
gboolean 
ephy_cell_renderer_needs_invalidation (const char *property_name)
{
  static struct {
    const char *property_name;
    const char *interned_name;
  } properties[] = {
    /* EphyWebView */
    { "embed-title", NULL },
    { "icon", NULL },
    /* WebKitWebView */
    { "progress", NULL },
    { "load-status", NULL },
  };
  guint i;

  g_return_val_if_fail (property_name != NULL, TRUE);

  if (properties[0].interned_name == NULL)
    {
      for (i = 0; i < G_N_ELEMENTS (properties); i++)
        {
          properties[i].interned_name = g_intern_string (properties[i].property_name);
        }
    }

  for (i = 0; i < G_N_ELEMENTS (properties); i++)
    {
      if (properties[i].interned_name == property_name)
        return TRUE;
    }

  return FALSE;
}

