/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 *  Copyright © 2009 Igalia S.L.
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
 */

#include "config.h"

#include "ephy-debug.h"
#include "ephy-soup-fly-extension.h"
#include "soup-fly.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#define SOUP_FLY_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_SOUP_FLY, SoupFlyPrivate))

struct _SoupFlyPrivate {
  GtkTreeModel *model;
  GtkWidget *treeview;
  GtkWidget *clear_button;

  guint nth;
  gboolean clean_finished;
};

enum {
    COL_NUMBER,
    COL_STATE,
    COL_PROGRESS,
    COL_URL,
    N_COLUMNS
};

enum {
  STATE_QUEUED,
  STATE_SENDING,
  STATE_WAITING,
  STATE_RECEIVING,
  STATE_FINISHED
};

const struct {
  const char *name, *color;
} states[] = {
  { "Queued", "pink" },
  { "Sending", "green" },
  { "Waiting", "cyan" },
  { "Receiving", "blue" },
  { "Finished", "gray" }
};

static void soup_fly_class_init (SoupFlyClass *klass);
static void soup_fly_init       (SoupFly *logger);

static GObjectClass *parent_class = NULL;
static GType type = 0;

GType
soup_fly_get_type (void)
{
  return type;
}

GType
soup_fly_register_type (GTypeModule *module)
{
  static volatile gsize type_volatile = 0;

  if (g_once_init_enter (&type_volatile)) {
    const GTypeInfo our_info = {
        sizeof (SoupFlyClass),
        NULL, /* base_init */
        NULL, /* base_finalize */
        (GClassInitFunc) soup_fly_class_init,
        NULL,
        NULL, /* class_data */
        sizeof (SoupFly),
        0, /* n_preallocs */
        (GInstanceInitFunc) soup_fly_init
    };

    type = g_type_module_register_type (module,
                                        GTK_TYPE_WINDOW,
                                        "SoupFly",
                                        &our_info, 0);
    g_once_init_leave (&type_volatile, type);
  }

  return type_volatile;
}

static void
soup_fly_class_init (SoupFlyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  g_type_class_add_private (object_class, sizeof (SoupFlyPrivate));
}

static void
clear_button_clicked_cb (GtkButton *button, SoupFly *logger)
{
  GtkTreeIter iter;
  gboolean valid;
  SoupFlyPrivate *priv = logger->priv;

  valid = gtk_tree_model_get_iter_first (priv->model, &iter);

  while (valid) {
    int state;

    gtk_tree_model_get (priv->model, &iter,
                        COL_STATE, &state,
                        -1);

    if (state == STATE_FINISHED)
      valid = gtk_list_store_remove (GTK_LIST_STORE (priv->model), &iter);
    else
      valid = gtk_tree_model_iter_next (priv->model, &iter);
  }
}

static void
check_button_toggled_cb (GtkToggleButton *toggle, SoupFly *logger)
{
  SoupFlyPrivate *priv = logger->priv;

  priv->clean_finished = gtk_toggle_button_get_active (toggle);
}

static void
state_data_func (GtkTreeViewColumn *column, GtkCellRenderer *cell,
                 GtkTreeModel *model, GtkTreeIter *iter,
                 gpointer data)
{
  int state;

  gtk_tree_model_get (model, iter, COL_STATE, &state, -1);
  g_object_set (G_OBJECT (cell),
                "text", states[state].name,
                "background", states[state].color,
                NULL);
}

static void
construct_ui (SoupFly *logger)
{
  GtkListStore *store;
  GtkWidget *treeview;
  GtkCellRenderer *renderer;
  GtkWidget *scrolled, *hbox, *vbox, *button, *check_button;
  GtkTreeViewColumn *column;
  SoupFlyPrivate *priv = logger->priv;

  gtk_window_set_icon_name (GTK_WINDOW (logger),
                            GTK_STOCK_DIALOG_INFO);

  g_signal_connect (logger, "delete-event",
                    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  treeview = gtk_tree_view_new ();
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_UINT,
                              G_TYPE_INT,
                              G_TYPE_INT,
                              G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               COL_NUMBER, "#",
                                               renderer,
                                               "text", COL_NUMBER,
                                               NULL);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("State"));
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           state_data_func,
                                           NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column); 

  renderer = gtk_cell_renderer_progress_new ();
  g_object_set (G_OBJECT (renderer), "text-xalign", 0.0, NULL);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               COL_URL, "URL",
                                               renderer,
                                               "text", COL_URL,
                                               "value", COL_PROGRESS,
                                               NULL);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);

  hbox = gtk_hbox_new (FALSE, 10);
  button = gtk_button_new_with_label (_("Clear finished"));
  g_signal_connect (button, "clicked", G_CALLBACK (clear_button_clicked_cb), logger);
  gtk_container_add (GTK_CONTAINER (hbox), button);

  check_button = gtk_check_button_new_with_label (_("Automatically remove finished messages"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), priv->clean_finished);
  g_signal_connect (check_button, "toggled", G_CALLBACK (check_button_toggled_cb), logger);
  gtk_container_add (GTK_CONTAINER (hbox), check_button);

  vbox = gtk_vbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (logger), vbox);

  gtk_window_set_title (GTK_WINDOW (logger), "Soup Fly");
  gtk_container_set_border_width (GTK_CONTAINER (logger), 5);
  gtk_window_set_default_size (GTK_WINDOW (logger), 640, 480);

  priv->model = GTK_TREE_MODEL (store);
  priv->treeview = treeview;
  priv->clear_button = button;
}

static void
soup_fly_init (SoupFly *logger)
{
  logger->priv = SOUP_FLY_GET_PRIVATE (logger);
  
  logger->priv->nth = 0;

  construct_ui (logger);
}

typedef struct
{
  GtkTreeIter iter;
  SoupFly *logger;
  goffset content_length, received;
} FlyMessageData;

static void
message_finished_cb (SoupMessage *message, FlyMessageData *data)
{
  SoupFlyPrivate *priv = data->logger->priv;

  if (priv->clean_finished)
    gtk_list_store_remove (GTK_LIST_STORE (priv->model), &data->iter);
  else
    gtk_list_store_set (GTK_LIST_STORE (priv->model), &data->iter,
                        COL_PROGRESS, 0,
                        COL_STATE, STATE_FINISHED,
                        -1);
  g_slice_free (FlyMessageData, data);
}

static void
message_got_chunk_cb (SoupMessage *message, SoupBuffer *chunk, FlyMessageData *data)
{
  SoupFlyPrivate *priv = data->logger->priv;

  data->received += chunk->length;
  if (data->content_length && data->received) {
    gtk_list_store_set (GTK_LIST_STORE (priv->model), &data->iter,
                        COL_PROGRESS, (int)(data->received * 100 / data->content_length),
                        -1);
  }
}

static void
message_got_headers_cb (SoupMessage *message, FlyMessageData *data)
{
  SoupFlyPrivate *priv = data->logger->priv;

  data->content_length = soup_message_headers_get_content_length (message->response_headers);
  gtk_list_store_set (GTK_LIST_STORE (priv->model), &data->iter,
                      COL_STATE, STATE_RECEIVING,
                      COL_PROGRESS, 0,
                      -1);
}

static void
message_wrote_body_cb (SoupMessage *message, FlyMessageData *data)
{
  SoupFlyPrivate *priv = data->logger->priv;

  gtk_list_store_set (GTK_LIST_STORE (priv->model), &data->iter,
                      COL_STATE, STATE_WAITING,
                      -1);
}

static void
request_started_cb (SoupSession *session, SoupMessage *message,
                    SoupSocket *socket, SoupFly *logger)
{
  SoupFlyPrivate *priv = logger->priv;
  FlyMessageData *data = g_object_get_data (G_OBJECT (message), "FlyMessageData");

  if (!data)
    return;
  gtk_list_store_set (GTK_LIST_STORE (priv->model), &data->iter,
                      COL_STATE, STATE_SENDING,
                      -1);
}

static void
request_queued_cb (SoupSession *session, SoupMessage *message, SoupFly *logger)
{
  GtkTreeIter iter;
  SoupURI *uri;
  char *uri_string;
  FlyMessageData *data;
  SoupFlyPrivate *priv = logger->priv;

  uri = soup_message_get_uri (message);
  uri_string = soup_uri_to_string (uri, FALSE);

  gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (priv->model), &iter,
                      COL_NUMBER, priv->nth++,
                      COL_URL, uri_string,
                      COL_STATE, STATE_QUEUED,
                      -1);
  g_free (uri_string);
  
  data = g_slice_new (FlyMessageData);
  data->iter = iter;
  data->logger = logger;
  g_object_set_data (G_OBJECT (message), "FlyMessageData", data);
  g_signal_connect (message, "wrote-body", G_CALLBACK (message_wrote_body_cb), data);
  g_signal_connect (message, "got-headers", G_CALLBACK (message_got_headers_cb), data);
  g_signal_connect (message, "got-chunk", G_CALLBACK (message_got_chunk_cb), data);
  g_signal_connect (message, "finished", G_CALLBACK (message_finished_cb), data);
}

/* Public API */

/**
 * soup_fly_start:
 * @logger: a #SoupFly
 * 
 * 
 **/
void
soup_fly_start (SoupFly *logger)
{
  SoupSession *session;
  SoupFlyPrivate *priv;

  g_return_if_fail (IS_SOUP_FLY (logger));
  
  priv = logger->priv;
  session = webkit_get_default_session ();
  g_return_if_fail (session);
  g_signal_connect (session, "request-queued", G_CALLBACK (request_queued_cb), logger);
  g_signal_connect (session, "request-started", G_CALLBACK (request_started_cb), logger);
}

/**
 * soup_fly_stop:
 * @logger: a #SoupFly
 * 
 * 
 **/
void
soup_fly_stop (SoupFly *logger)
{
  SoupSession *session;

  g_return_if_fail (IS_SOUP_FLY (logger));
  
  session = webkit_get_default_session ();
  g_signal_handlers_disconnect_by_func (session, G_CALLBACK (request_queued_cb), logger);
}

/**
 * soup_fly_new:
 * @void: 
 * 
 * Returns: a new #SoupFly
 **/
SoupFly *
soup_fly_new (void)
{
  return SOUP_FLY (g_object_new (TYPE_SOUP_FLY, NULL));
}

