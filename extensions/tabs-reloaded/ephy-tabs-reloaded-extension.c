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

#include "ephy-tabs-reloaded-extension.h"
#include "ephy-tabs-manager.h"
#include "ephy-debug.h"

#include <string.h>

#include <epiphany/epiphany.h>

static void ephy_tabs_reloaded_extension_class_init (EphyTabsReloadedExtensionClass *class);
static void ephy_tabs_reloaded_extension_iface_init (EphyExtensionIface *iface);
static void ephy_tabs_reloaded_extension_init	 (EphyTabsReloadedExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

static GQuark tabs_reloaded_quark;

GType
ephy_tabs_reloaded_extension_get_type (void)
{
	return type;
}

GType
ephy_tabs_reloaded_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyTabsReloadedExtensionClass),
		NULL,	/* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tabs_reloaded_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTabsReloadedExtension),
		0,	/* n_preallocs */
		(GInstanceInitFunc) ephy_tabs_reloaded_extension_init
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_tabs_reloaded_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabsReloadedExtension",
					    &our_info, 0);
	
	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	
	return type;
}

static void
sanitize_tree_view (GtkTreeView *view)
{
  static const struct {
    gboolean expand;
  } cell_info[] = {
    { TRUE }
  };
  GtkCellLayout *column;
  GtkCellRenderer *renderer;
  GList *renderers, *walk;
  guint column_id, renderer_id;

  renderer_id = 0;
  for (column_id = 0;; column_id++)
    {
      column = GTK_CELL_LAYOUT (gtk_tree_view_get_column (view, column_id));
      if (column == NULL)
        break;
      renderers = gtk_cell_layout_get_cells (column);
      g_list_foreach (renderers, (GFunc) g_object_ref, NULL);
      gtk_cell_layout_clear (column);
      for (walk = renderers; walk; walk = g_list_next (walk))
        {
          renderer = walk->data;
          gtk_cell_layout_pack_start (column, renderer, cell_info[renderer_id].expand);
          renderer_id++;
          gtk_cell_layout_set_attributes (column, renderer, "tab", 0, NULL);
        }
      g_list_foreach (renderers, (GFunc) g_object_unref, NULL);
      g_list_free (renderers);
    }

  /* 
   * If this fails, someone modified the ui file to contain different
   * cell renderers and we need to add or remove data functions from 
   * the funcs array above to match that change.
   */
  g_assert (renderer_id == G_N_ELEMENTS (cell_info));
}

static gboolean
tree_view_search_equal (GtkTreeModel *model,
                        int           column,
                        const char *  key,
                        GtkTreeIter * iter,
                        gpointer      unused)
{
  char *needle, *haystack, *tmp;
  gboolean not_found;

  /* 
   * FIXME: This search function should find the same way as the 
   * location bar.
   */
  
  tmp = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);
  needle = g_utf8_casefold (tmp, -1);
  g_free (tmp);

  tmp = g_utf8_normalize (
      ephy_web_view_get_title (
          ephy_embed_get_web_view (
              ephy_tabs_manager_get_tab (EPHY_TABS_MANAGER (model), iter))),
      -1, G_NORMALIZE_ALL);
  haystack = g_utf8_casefold (tmp, -1);
  g_free (tmp);

  not_found = strstr (haystack, needle) == NULL;

  g_print ("%s %s in %s\n", not_found ? "didn't find" : "found", key,
      ephy_web_view_get_title (
          ephy_embed_get_web_view (
              ephy_tabs_manager_get_tab (EPHY_TABS_MANAGER (model), iter))));

  g_free (needle);
  g_free (haystack);

  return not_found;
}

static void
tree_selection_changed (GtkTreeSelection *selection,
                        gpointer          notebook)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  EphyEmbed *embed;
  int page_id;

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    return;

  embed = ephy_tabs_manager_get_tab (EPHY_TABS_MANAGER (model), &iter);
  page_id = gtk_notebook_page_num (notebook, GTK_WIDGET (embed));
  if (page_id != gtk_notebook_get_current_page (notebook))
    gtk_notebook_set_current_page (notebook, page_id);
}

static void
notebook_selection_changed (GtkNotebook *     notebook,
                            GtkNotebookPage * page,
                            guint             page_num,
                            GtkTreeSelection *selection)
{
  GtkTreePath *path;

  /* FIXME: only works as long as we're not doing tab groups,
   * then we need to use _find_tab()
   */
  path = gtk_tree_path_new_from_indices (page_num, -1);
  gtk_tree_selection_select_path (selection, path);
  gtk_tree_path_free (path);
}

static void
force_no_tabs (GtkNotebook *notebook, GParamSpec *pspec, gpointer unused)
{
  if (gtk_notebook_get_show_tabs (notebook))
    gtk_notebook_set_show_tabs (notebook, FALSE);
}

static void
impl_attach_window (EphyExtension *extension,
		    EphyWindow *   window)
{
	GtkBuilder *builder;
	GError *error = NULL;
        GtkWidget *tabs, *notebook, *parent;
        GValue position = { 0, };
        EphyTabsManager *manager;
        GtkTreeSelection *selection;

	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, SHARE_DIR "/glade/tabs-reloaded.ui", &error);
	if (error)
	{
		g_warning ("Unable to load UI file: %s", error->message);
		g_error_free (error);
		return;
	}

        notebook = ephy_window_get_notebook (window);
        g_signal_connect (notebook, "notify::show-tabs", G_CALLBACK (force_no_tabs), NULL);
        gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

	tabs = (GtkWidget *) gtk_builder_get_object (builder, "TabView");
        gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (tabs),
                                             tree_view_search_equal,
                                             NULL, NULL);
        sanitize_tree_view (GTK_TREE_VIEW (tabs));
        manager = ephy_tabs_manager_new ();
        ephy_tabs_manager_attach (manager, GTK_NOTEBOOK (notebook));
        gtk_tree_view_set_model (GTK_TREE_VIEW (tabs), GTK_TREE_MODEL (manager));
        g_object_unref (manager);

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tabs));
        g_signal_connect (selection, "changed",
                          G_CALLBACK (tree_selection_changed),
                          notebook);
        g_signal_connect_after (notebook, "switch-page",
                                G_CALLBACK (notebook_selection_changed),
                                selection);
        notebook_selection_changed (GTK_NOTEBOOK (notebook),
                                    NULL, 
                                    gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook)), 
                                    selection);
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

	tabs = (GtkWidget *) gtk_builder_get_object (builder, "TabBox");
        g_return_if_fail (GTK_IS_PANED (tabs));
        parent = gtk_widget_get_parent (notebook);

	/* Add the sidebar and the current notebook to a
	 * GtkHPaned, and add the hpaned to where the notebook
	 * currently is */
	g_value_init (&position, G_TYPE_INT);
	gtk_container_child_get_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);

	g_object_ref (notebook);
	gtk_container_remove (GTK_CONTAINER (parent), notebook);
	gtk_paned_add2 (GTK_PANED (tabs), notebook);
	g_object_unref (notebook);

	/* work-around for http://bugzilla.gnome.org/show_bug.cgi?id=169116 */
	gtk_widget_hide (notebook);
	gtk_widget_show (notebook);

	gtk_container_add (GTK_CONTAINER (parent), tabs);
	gtk_container_child_set_property (GTK_CONTAINER (parent),
					  tabs, "position", &position);
	g_value_unset (&position);

	g_object_unref (builder);
        g_object_set_qdata (G_OBJECT (window), tabs_reloaded_quark, tabs);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *   window)
{
        GtkWidget *tabs, *notebook, *parent;
        GValue position = { 0, };
        
        tabs = g_object_steal_qdata (G_OBJECT (window), tabs_reloaded_quark);
        if (tabs == NULL)
                return;

	notebook = gtk_paned_get_child2 (GTK_PANED (tabs));
	parent = gtk_widget_get_parent (tabs);

        g_signal_handlers_disconnect_matched (notebook,
                                              G_SIGNAL_MATCH_FUNC,
                                              0,
                                              0,
                                              NULL,
                                              notebook_selection_changed,
                                              NULL);
        g_signal_handlers_disconnect_matched (notebook,
                                              G_SIGNAL_MATCH_FUNC,
                                              0,
                                              0,
                                              NULL,
                                              force_no_tabs,
                                              NULL);
        ephy_notebook_set_show_tabs (EPHY_NOTEBOOK (notebook), TRUE);


	/* Remove the Sidebar, replacing our hpaned with the
	 * notebook itself */
	g_value_init (&position, G_TYPE_INT);
	gtk_container_child_get_property (GTK_CONTAINER (parent),
					  tabs, "position", &position);

	g_object_ref (notebook);
	gtk_container_remove (GTK_CONTAINER (tabs), notebook);
	gtk_container_remove (GTK_CONTAINER (parent), tabs);

	gtk_container_add (GTK_CONTAINER (parent), notebook);

	/* work-around for http://bugzilla.gnome.org/show_bug.cgi?id=169116 */
        gtk_widget_hide (notebook);
	gtk_widget_show (notebook);

	g_object_unref (notebook);

	gtk_container_child_set_property (GTK_CONTAINER (parent),
					  notebook, "position", &position);

	g_value_unset (&position);
}

static void
ephy_tabs_reloaded_extension_init (EphyTabsReloadedExtension *extension)
{
	LOG ("EphyTabsReloadedExtension initialising");

        tabs_reloaded_quark = g_quark_from_string ("ephy-tabs-reloaded");
}

static void
ephy_tabs_reloaded_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_tabs_reloaded_extension_class_init (EphyTabsReloadedExtensionClass *class)
{
	parent_class = (GObjectClass *) g_type_class_peek_parent (class);
}
