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

#include "ephy-soup-fly-extension.h"
#include "soup-fly.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <string.h>
#include <libpeas/peas.h>

#define WINDOW_DATA_KEY "EphySoupFlyExtWindowData"

#define EPHY_SOUP_FLY_EXTENSION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SOUP_FLY_EXTENSION, EphySoupFlyExtensionPrivate))

typedef struct
{
  EphySoupFlyExtension *extension;
  EphyWindow *window;
} SoupFlyCBData;

typedef struct
{
  GtkActionGroup *action_group;
  guint ui_id;
} WindowData;

struct _EphySoupFlyExtensionPrivate
{
  SoupFly *fly;

  gboolean is_logging;
};

static void ephy_soup_fly_extension_class_init  (EphySoupFlyExtensionClass *klass);
static void ephy_soup_fly_extension_iface_init  (EphyExtensionIface *iface);
static void ephy_soup_fly_extension_init    (EphySoupFlyExtension *extension);

static void ephy_soup_fly_extension_show_viewer (GtkAction *action,
                                                 SoupFlyCBData *data);

static const GtkActionEntry action_entries [] = {
  { "SoupFly",
    GTK_STOCK_DIALOG_INFO,
    N_("_Soup Fly"),
    NULL, /* shortcut key */
    N_("Show information about the SoupSession used by the browser"),
    G_CALLBACK (ephy_soup_fly_extension_show_viewer) }
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_soup_fly_extension_get_type (void)
{
  return type;
}

GType
ephy_soup_fly_extension_register_type (GTypeModule *module)
{
  static volatile gsize type_volatile = 0;

  if (g_once_init_enter (&type_volatile)) {
    const GTypeInfo our_info = {
      sizeof (EphySoupFlyExtensionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ephy_soup_fly_extension_class_init,
      NULL,
      NULL, /* class_data */
      sizeof (EphySoupFlyExtension),
      0, /* n_preallocs */
      (GInstanceInitFunc) ephy_soup_fly_extension_init
    };

    const GInterfaceInfo extension_info = {
      (GInterfaceInitFunc) ephy_soup_fly_extension_iface_init,
      NULL,
      NULL
    };

    type = g_type_module_register_type (module,
                                        G_TYPE_OBJECT,
                                        "EphySoupFlyExtension",
                                        &our_info, 0);

    g_type_module_add_interface (module,
                                 type,
                                 EPHY_TYPE_EXTENSION,
                                 &extension_info);

    g_once_init_leave (&type_volatile, type);
  }

  return type_volatile;
}

static void
ephy_soup_fly_extension_init (EphySoupFlyExtension *extension)
{
  LOG ("EphySoupFlyExtension initialising");

  extension->priv = EPHY_SOUP_FLY_EXTENSION_GET_PRIVATE (extension);

  extension->priv->fly = soup_fly_new ();
}

static void
ephy_soup_fly_extension_finalize (GObject *object)
{
  EphySoupFlyExtension *extension =
    EPHY_SOUP_FLY_EXTENSION (object);

  LOG ("EphySoupFlyExtension finalizing");

  gtk_widget_destroy (GTK_WIDGET (extension->priv->fly));

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_soup_fly_extension_class_init (EphySoupFlyExtensionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = ephy_soup_fly_extension_finalize;

  g_type_class_add_private (object_class, sizeof (EphySoupFlyExtensionPrivate));
}

static void
ephy_soup_fly_extension_show_viewer (GtkAction *action,
                                     SoupFlyCBData *data)
{
  gtk_widget_show_all (GTK_WIDGET (data->extension->priv->fly));
}

static void
free_window_data (WindowData *data)
{
  if (data) {
    g_object_unref (data->action_group);
    g_free (data);
  }
}

static void
impl_attach_window (EphyExtension *extension,
                    EphyWindow *window)
{
  SoupFlyCBData *cb_data;
  GtkActionGroup *action_group;
  GtkUIManager *manager;
  guint merge_id;
  WindowData *data;
  EphySoupFlyExtensionPrivate *priv = EPHY_SOUP_FLY_EXTENSION (extension)->priv;

  LOG ("EphySoupFlyExtension attach_window");

  cb_data = g_new (SoupFlyCBData, 1);
  cb_data->extension = EPHY_SOUP_FLY_EXTENSION (extension);
  cb_data->window = window;

  data = g_new (WindowData, 1);

  data->action_group = action_group =
    gtk_action_group_new ("EphySoupFlyExtensionActions");
  gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions_full (action_group, action_entries,
                                     G_N_ELEMENTS (action_entries), cb_data,
                                     g_free);

  manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  data->ui_id = merge_id = gtk_ui_manager_new_merge_id (manager);

  g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, data,
                          (GDestroyNotify) free_window_data);

  gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
                         "SoupFlySep", NULL,
                         GTK_UI_MANAGER_SEPARATOR, FALSE);
  gtk_ui_manager_add_ui (manager, merge_id, "/menubar/ToolsMenu",
                         "SoupFly", "SoupFly",
                         GTK_UI_MANAGER_MENUITEM, FALSE);

  if (priv->is_logging == FALSE) {
    soup_fly_start (priv->fly);
    priv->is_logging = TRUE;
  }
}

static void
impl_detach_window (EphyExtension *extension,
                    EphyWindow *window)
{
  GtkUIManager *manager;
  WindowData *data;

  /* Remove UI */
  manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

  data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
  g_return_if_fail (data != NULL);

  gtk_ui_manager_remove_ui (manager, data->ui_id);
  gtk_ui_manager_remove_action_group (manager, data->action_group);

  g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);

}

static void
ephy_soup_fly_extension_iface_init (EphyExtensionIface *iface)
{
  iface->attach_window = impl_attach_window;
  iface->detach_window = impl_detach_window;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	soup_fly_register_type (G_TYPE_MODULE (module));
	ephy_soup_fly_extension_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    EPHY_TYPE_EXTENSION,
						    EPHY_TYPE_SOUP_FLY_EXTENSION);
}
