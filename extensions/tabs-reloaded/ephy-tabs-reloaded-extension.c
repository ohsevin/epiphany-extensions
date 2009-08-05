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
pixbuf_renderer_cell_data_func (GtkCellLayout *  column,
                                GtkCellRenderer *renderer,
                                GtkTreeModel *   model,
                                GtkTreeIter *    iter,
                                gpointer         unused)
{
  EphyEmbed *embed;
  EphyWebView *view;
  
  embed = ephy_tabs_manager_get_tab (EPHY_TABS_MANAGER (model),
                                     iter);
  view = ephy_embed_get_web_view (embed);

  g_object_set (renderer,
                "pixbuf", ephy_web_view_get_icon (view),
                "visible", ephy_web_view_get_icon (view) != NULL,
                NULL);
}

static void
text_renderer_cell_data_func (GtkCellLayout *  column,
                              GtkCellRenderer *renderer,
                              GtkTreeModel *   model,
                              GtkTreeIter *    iter,
                              gpointer         unused)
{
  EphyEmbed *embed;
  EphyWebView *view;
  
  embed = ephy_tabs_manager_get_tab (EPHY_TABS_MANAGER (model),
                                     iter);
  view = ephy_embed_get_web_view (embed);

  g_object_set (renderer,
                "text", ephy_web_view_get_title (view),
                NULL);
}

static void
sanitize_tree_view (GtkTreeView *view)
{
  static const GtkCellLayoutDataFunc funcs[] = {
    pixbuf_renderer_cell_data_func,
    text_renderer_cell_data_func,
    NULL /* close button */
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
      for (walk = renderers; walk; walk = g_list_next (walk))
        {
          renderer = walk->data;
          if (funcs[renderer_id])
            gtk_cell_layout_set_cell_data_func (column,
                                                renderer,
                                                funcs[renderer_id],
                                                NULL,
                                                NULL);
          renderer_id++;
        }
    }

  /* 
   * If this fails, someone modified the ui file to contain different
   * cell renderers and we need to add or remove data functions from 
   * the funcs array above to match that change.
   */
  g_assert (renderer_id == G_N_ELEMENTS (funcs));
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

	tabs = (GtkWidget *) gtk_builder_get_object (builder, "TabView");
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

        tabs_reloaded_quark = g_quark_from_static_string ("ephy-tabs-reloaded");
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
