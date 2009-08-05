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

#include "ephy-tabs-manager.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>

G_DEFINE_DYNAMIC_TYPE (EphyTabsManager, ephy_tabs_manager, GTK_TYPE_TREE_STORE)

static void
ephy_tabs_manager_detach (EphyTabsManager *manager)
{
  if (manager->notebook == NULL)
    return;

  g_signal_handlers_disconnect_matched (manager->notebook,
                                        G_SIGNAL_MATCH_DATA,
                                        0,
                                        0,
                                        NULL,
                                        NULL,
                                        manager);

  gtk_tree_store_clear (GTK_TREE_STORE (manager));

  g_object_unref (manager->notebook);
  manager->notebook = NULL;
}

static void
ephy_tabs_manager_finalize (GObject *object)
{
  EphyTabsManager *manager = EPHY_TABS_MANAGER (object);

  ephy_tabs_manager_detach (manager);

  G_OBJECT_CLASS (ephy_tabs_manager_parent_class)->finalize (object);
}

static void
ephy_tabs_manager_class_init (EphyTabsManagerClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = ephy_tabs_manager_finalize;
}

static void
ephy_tabs_manager_class_finalize (EphyTabsManagerClass *class)
{
}

static void
ephy_tabs_manager_init (EphyTabsManager *extension)
{
  GType types[] = { EPHY_TYPE_EMBED };

  gtk_tree_store_set_column_types (GTK_TREE_STORE (extension),
                                   G_N_ELEMENTS (types),
                                   types);
}

void
ephy_tabs_manager_register (GTypeModule *module)
{
  ephy_tabs_manager_register_type (module);
}

EphyTabsManager *
ephy_tabs_manager_new (void)
{
    return g_object_new (EPHY_TYPE_TABS_MANAGER, NULL);
}

static void
ephy_tabs_manager_view_changed (GObject *view, GParamSpec *psepc, EphyTabsManager *manager)
{
  GtkTreeModel *model = GTK_TREE_MODEL (manager);
  GtkTreeIter iter;
  EphyEmbed *embed;
  GtkTreePath *path;

  embed = EPHY_EMBED (gtk_widget_get_parent (GTK_WIDGET (view)));
  if (!ephy_tabs_manager_find_tab (manager, &iter, embed))
    {
      g_assert_not_reached ();
    }

  path = gtk_tree_model_get_path (model, &iter);
  gtk_tree_model_row_changed (model, path, &iter);
  gtk_tree_path_free (path);
}

/**
 * ephy_tabs_manager_add_tab:
 * @manager: the tab manager
 * @embed: the tab to add
 * @position: 0-indexed position to add the tab at or -1 for 
 *            adding at the end.
 *
 * Adds a new tab to the manager.
 **/
void
ephy_tabs_manager_add_tab (EphyTabsManager *manager,
                           EphyEmbed *      embed,
                           int              position)
{
  g_return_if_fail (EPHY_IS_TABS_MANAGER (manager));
  g_return_if_fail (EPHY_IS_EMBED (embed));
  g_return_if_fail (position >= -1);

  gtk_tree_store_insert_with_values (GTK_TREE_STORE (manager),
                                     NULL,
                                     NULL,
                                     position,
                                     0, embed,
                                     -1);

  g_signal_connect (ephy_embed_get_web_view (embed),
                    "notify",
                    G_CALLBACK (ephy_tabs_manager_view_changed),
                    manager);
}

/**
 * ephy_tabs_manager_remove_tab:
 * @manager: the tab manager
 * @embed: the tab to remove
 *
 * Removes the given tab from the manager. The tab must be managed by
 * the @manager.
 **/
void
ephy_tabs_manager_remove_tab (EphyTabsManager *manager,
                              EphyEmbed *      embed)
{
  GtkTreeIter iter;

  g_return_if_fail (EPHY_IS_TABS_MANAGER (manager));
  g_return_if_fail (EPHY_IS_EMBED (embed));

  if (!ephy_tabs_manager_find_tab (manager, &iter, embed))
    {
      g_assert_not_reached ();
    }

  gtk_tree_store_remove (GTK_TREE_STORE (manager), &iter);
}

/**
 * ephy_tabs_manager_get_tab:
 * @manager: the manager
 * @iter: a valid tree iter
 *
 * Gets the #EphyEmbed of the tab associated with @iter.
 *
 * Returns: the tab associated with @iter
 **/
EphyEmbed *
ephy_tabs_manager_get_tab (EphyTabsManager *manager,
                           GtkTreeIter *    iter)
{
  GValue value = { 0, };
  EphyEmbed *embed;

  g_return_val_if_fail (EPHY_IS_TABS_MANAGER (manager), NULL);
  g_return_val_if_fail (iter != NULL, NULL);

  gtk_tree_model_get_value (GTK_TREE_MODEL (manager), iter, 0, &value);
  embed = EPHY_EMBED (g_value_get_object (&value));
  g_value_unset (&value);

  return embed;
}

/**
 * ephy_tabs_manager_find_tab:
 * @manager: the tab manager
 * @iter: the iterator to initialize to point to @embed
 * @embed: the tab to find
 *
 * Tries to find @embed in @manager. If successful, @iter is initialized
 * to point to @embed and %TRUE is returned. If @embed is not managed by
 * @manager, %FALSE is returned.
 *
 * Returns: %TRUE if @embed was found and @iter was initialized.
 **/
gboolean
ephy_tabs_manager_find_tab (EphyTabsManager *manager,
                            GtkTreeIter *    iter,
                            EphyEmbed *      embed)
{
  GtkTreeModel *model;
  gboolean valid;

  g_return_val_if_fail (EPHY_IS_TABS_MANAGER (manager), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (EPHY_IS_EMBED (embed), FALSE);

  model = GTK_TREE_MODEL (manager);
  for (valid = gtk_tree_model_get_iter_first (model, iter);
       valid;
       valid = gtk_tree_model_iter_next (model, iter))
    {
        EphyEmbed *check = ephy_tabs_manager_get_tab (manager, iter);
        if (check == embed)
          return TRUE;
    }

  return FALSE;
}

/*** NOTEBOOK MONITORING CODE ***/

static void
ephy_tabs_manager_page_added_cb (GtkNotebook *    notebook,
                                 EphyEmbed *      child,
                                 guint            page_num,
                                 EphyTabsManager *manager)
{
  ephy_tabs_manager_add_tab (manager, child, page_num);
}

static void
ephy_tabs_manager_page_removed_cb (GtkNotebook *    notebook,
                                   EphyEmbed *      child,
                                   guint            page_num,
                                   EphyTabsManager *manager)
{
  ephy_tabs_manager_remove_tab (manager, child);
}

static void
ephy_tabs_manager_page_reordered_cb (GtkNotebook *    notebook,
                                     EphyEmbed *      child,
                                     guint            page_num,
                                     EphyTabsManager *manager)
{
  g_message ("implement reordered signal");
}

void
ephy_tabs_manager_attach (EphyTabsManager *manager,
                          GtkNotebook *    notebook)
{
  int i;

  g_return_if_fail (EPHY_IS_TABS_MANAGER (manager));
  g_return_if_fail (EPHY_IS_TABS_MANAGER (manager));

  g_object_ref (notebook);
  ephy_tabs_manager_detach (manager);
  manager->notebook = notebook;

  g_signal_connect_after (notebook, "page-added",
                          G_CALLBACK (ephy_tabs_manager_page_added_cb),
                          manager);
  g_signal_connect_after (notebook, "page-removed",
                          G_CALLBACK (ephy_tabs_manager_page_removed_cb),
                          manager);
  g_signal_connect_after (notebook, "page-reordered",
                          G_CALLBACK (ephy_tabs_manager_page_reordered_cb),
                          manager);

  for (i = 0; i < gtk_notebook_get_n_pages (notebook); i++)
    {
      ephy_tabs_manager_add_tab (manager,
                                 EPHY_EMBED (gtk_notebook_get_nth_page (notebook, i)),
                                 -1);
    }
}

