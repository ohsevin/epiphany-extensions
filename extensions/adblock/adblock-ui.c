/*
 *  Copyright © 2004 Adam Hooper
 *  Copyright © 2005, 2006 Jean-François Rameau
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
#include "ephy-debug.h"

#include "adblock-ui.h"
#include "adblock-pattern.h"
#include "ephy-file-helpers.h"

#include <epiphany/ephy-adblock-manager.h>
#include <epiphany/ephy-embed-shell.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktextview.h>
#include <gtk/gtknotebook.h>

#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include <string.h>

#define ADBLOCK_UI_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), TYPE_ADBLOCK_UI, AdblockUIPrivate))

enum {
	WHITELIST_PAGE,
	BLACKLIST_PAGE,
	DEFAULT_BLACKLIST_PAGE,

	LAST_PAGE
};

typedef struct _InfoPage	InfoPage;

struct _InfoPage
{
	/* Methods */
	void	(* construct)	(InfoPage *info);
	void	(* fill)	(InfoPage *info);

	AdblockUI *dialog;

	/* Data */
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeView *treeview;

	/* Type of patterns it plays with */
	AdblockPatternType type;
};

struct _AdblockUIPrivate
{
	GtkWidget *dialog;
	GtkNotebook *notebook;

	/* Pages */
	InfoPage *pages[LAST_PAGE];
	InfoPage *current_page;

	/* The dialog buttons */
	GtkButton *add;
	GtkButton *modify;
	GtkButton *suppr;
	GtkButton *load;
	GtkButton *license;
	GtkFrame  *actions;

	/* The current pattern */
	GtkEntry *pattern;
	
	/* Page we want to show first */
	AdblockPatternType default_type;

	/* The uri tester */
	AdUriTester *tester;

	/* An address to be blocked/unblocked */
	char *url;
	gboolean url_allowed;
};

static void adblock_ui_class_init (AdblockUIClass *klass);
static void adblock_ui_init (AdblockUI *dialog);
static void adblock_ui_populate_store (InfoPage *page, AdblockPatternType type);

static GObjectClass *adblock_ui_parent_class = NULL;

static GType type = 0;

GType
adblock_ui_get_type (void)
{
	return type;
}

GType
adblock_ui_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (AdblockUIClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) adblock_ui_class_init,
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (AdblockUI),
		0, /* n_preallocs */
		(GInstanceInitFunc) adblock_ui_init
	};

	type = g_type_module_register_type (module,
					    EPHY_TYPE_DIALOG,
					    "AdblockUI",
					    &our_info, 0);

	return type;
}

enum
{
	PROP_WINDOW,
	PROP_TESTER,
	PROP_URL,
	PROP_URL_ALLOWED,
};

enum
{
	PROP_DIALOG,
	PROP_NOTEBOOK,
	PROP_WHITELIST,
	PROP_BLACKLIST,
	PROP_DEFAULTLIST,
	PROP_PATTERN,
	PROP_ADD,
	PROP_MODIFY,
	PROP_SUPPR,
	PROP_LOAD,
	PROP_LICENSE,
	PROP_ACTIONS
};

static const
EphyDialogProperty properties [] =
{
	{ "adblock-ui",		NULL, PT_NORMAL, 0 },
	{ "notebook",		NULL, PT_NORMAL, 0 },
	{ "white_treeview",	NULL, PT_NORMAL, 0 },
	{ "black_treeview",	NULL, PT_NORMAL, 0 },
	{ "default_treeview",	NULL, PT_NORMAL, 0 },
	{ "pattern",		NULL, PT_NORMAL, 0 },
	{ "add",		NULL, PT_NORMAL, 0 },
	{ "modify",		NULL, PT_NORMAL, 0 },
	{ "suppr",		NULL, PT_NORMAL, 0 },
	{ "load",		NULL, PT_NORMAL, 0 },
	{ "license",		NULL, PT_NORMAL, 0 },
	{ "action_rules_frame",	NULL, PT_NORMAL, 0 },
	{ NULL }
};

enum
{
	COL_PATTERN,
	N_COLUMNS
};

static gboolean
adblock_ui_foreach_save (GtkTreeModel *model,
              		 GtkTreePath  *path,
              		 GtkTreeIter  *iter,
              		 GSList       **patterns)
{
	char *pattern;

	gtk_tree_model_get (model, iter, COL_PATTERN, &pattern, -1);

	*patterns = g_slist_prepend (*patterns, pattern);

	return FALSE;
}

static void
adblock_ui_save (InfoPage *page)
{
	GSList *patterns = NULL;

	gtk_tree_model_foreach (GTK_TREE_MODEL (page->store), 
				(GtkTreeModelForeachFunc)adblock_ui_foreach_save, 
				&patterns);

	adblock_pattern_save (patterns, page->type);

	g_slist_free (patterns);
}

static void
adblock_ui_sync_page (InfoPage *page)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	gtk_entry_set_text (page->dialog->priv->pattern, "");
	if (gtk_tree_selection_get_selected (page->selection, &model, &iter))
	{
		gchar *pattern;

		gtk_tree_model_get (model, &iter, COL_PATTERN, &pattern, -1);

		gtk_entry_set_text (page->dialog->priv->pattern, pattern);

		g_free (pattern);
	}
}

static void
adblock_ui_selection_changed_cb (GtkTreeSelection *selection,
				 InfoPage *page)
{	
	adblock_ui_sync_page (page);
}

static void
adblock_ui_response_cb (GtkWidget *widget,
		    	int response,
		    	AdblockUI *dialog)
{
	if (response == GTK_RESPONSE_CLOSE)
	{
		EphyAdBlockManager *manager;

		adblock_ui_save (dialog->priv->pages [WHITELIST_PAGE]);
		adblock_ui_save (dialog->priv->pages [BLACKLIST_PAGE]);

		/* Ask uri tester to reload all its patterns */
		ad_uri_tester_reload (dialog->priv->tester);

		/* Ask manager to emit a signal that rules have changed */
		manager = EPHY_ADBLOCK_MANAGER (
				ephy_embed_shell_get_adblock_manager (embed_shell));

		g_signal_emit_by_name (manager, "rules_changed", NULL);
	}

	g_object_unref (dialog);
}

static void
adblock_ui_show_license_cb (GtkButton *button, 
		   	    AdblockUI *dialog)
{
	GtkWidget *window;

	const char * const authors[] = {"Graham Pierce", NULL};
	const char *license = 
		N_("This list is Copyright © its original author(G). It is relicensed under "
		   "the GPL with permission.\n"
		   "The original list, Filterset.G, can be found at "
		   "http://www.pierceive.com/filtersetg/.\n"
		   "This version has been modified from its original. The changes have not been "
		   "approved by the original author;\n"
		   "direct any problems to Bugzilla and not to the original author (G).\n");

	window = ephy_dialog_get_parent (EPHY_DIALOG (dialog));
	gtk_show_about_dialog (GTK_WINDOW (window),
			       "copyright", license,
			       "authors", authors,
			       NULL);
}

static void
adblock_ui_add_cb (GtkButton *button, 
		   AdblockUI *dialog)
{
	GtkTreeIter iter;

	/* new pattern */
	const char *pattern = gtk_entry_get_text (dialog->priv->pattern);

	if (pattern != NULL)
	{
		GtkListStore *store = dialog->priv->current_page->store;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_PATTERN, pattern, -1);
	}
}

static void
adblock_ui_suppr_cb (GtkButton *button, 
		     AdblockUI *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = dialog->priv->current_page->selection;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_list_store_remove (GTK_LIST_STORE(model), &iter);
		gtk_entry_set_text (dialog->priv->pattern, "");
	}
}

static void
adblock_ui_load_cb (GtkButton *button, 
		    AdblockUI *dialog)
{
	adblock_pattern_get_filtersetg_patterns ();

	gtk_list_store_clear (dialog->priv->current_page->store);	

	adblock_ui_populate_store (dialog->priv->current_page, PATTERN_DEFAULT_BLACKLIST);
}

static void
adblock_ui_modify_cb (GtkButton *button, 
		      AdblockUI *dialog)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = dialog->priv->current_page->selection;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		GtkListStore *store = dialog->priv->current_page->store;

		/* new pattern */
		const char *pattern = gtk_entry_get_text (dialog->priv->pattern);
		if (strlen (pattern) == 0) return;

		gtk_list_store_set (store, &iter, COL_PATTERN, pattern, -1);
	}
}

static void 
foreach_key  (const char *key,
              const char *value,
              InfoPage   *page)
{
	GtkTreeIter iter;

	gtk_list_store_append (page->store, &iter);
	gtk_list_store_set (page->store, &iter, COL_PATTERN, key, -1);
}

static void
adblock_ui_populate_store (InfoPage *page, AdblockPatternType type)
{
	GHashTable *patterns;

	patterns = g_hash_table_new_full (g_str_hash,
					  g_str_equal,
					  g_free,
					  g_free);

	adblock_pattern_load (patterns, type);

	g_hash_table_foreach (patterns, (GHFunc)foreach_key, page);

	g_hash_table_destroy (patterns);
}

static void
adblock_ui_switch_page (GtkNotebook *notebook,
               		GtkNotebookPage *page,
               		guint page_num,
               		AdblockUI *dialog)
{
	/* set the new current page */
	dialog->priv->current_page = dialog->priv->pages [page_num];

	/* Sync entry with page */
	if (page_num != DEFAULT_BLACKLIST_PAGE)
	{
		gtk_widget_show (GTK_WIDGET (dialog->priv->actions));
		adblock_ui_sync_page (dialog->priv->current_page);	
	}
	else
	{
		gtk_widget_hide (GTK_WIDGET (dialog->priv->actions));
	}
}

static GObject *
adblock_ui_constructor (GType type,
		        guint n_construct_properties,
		        GObjectConstructParam *construct_params)
{
	GObject *object;
	AdblockUI *dialog;
	AdblockUIPrivate *priv;
	EphyDialog *edialog;
	int i;

	object = G_OBJECT_CLASS (adblock_ui_parent_class)->constructor (
			type, n_construct_properties, construct_params);

	dialog = ADBLOCK_UI (object);
	edialog = EPHY_DIALOG (object);

	priv = dialog->priv;

	ephy_dialog_construct (EPHY_DIALOG (edialog),
			properties,
			SHARE_DIR "/glade/adblock.glade",
			"adblock-ui",
			GETTEXT_PACKAGE);

	ephy_dialog_get_controls (edialog,
			properties[PROP_DIALOG].id, &priv->dialog,
			properties[PROP_NOTEBOOK].id, &priv->notebook,
			properties[PROP_ADD].id, &priv->add,
			properties[PROP_MODIFY].id, &priv->modify,
			properties[PROP_SUPPR].id, &priv->suppr,
			properties[PROP_LOAD].id, &priv->load,
			properties[PROP_PATTERN].id, &priv->pattern,
			properties[PROP_LICENSE].id, &priv->license,
			properties[PROP_ACTIONS].id, &priv->actions,
			NULL);

	g_signal_connect (priv->dialog, "response",
			G_CALLBACK (adblock_ui_response_cb), dialog);

	g_signal_connect_after (priv->notebook, "switch_page",
				G_CALLBACK (adblock_ui_switch_page), dialog);

	g_signal_connect (priv->add, "clicked",
			  G_CALLBACK (adblock_ui_add_cb), dialog);
	g_signal_connect (priv->suppr, "clicked",
			  G_CALLBACK (adblock_ui_suppr_cb), dialog);
	g_signal_connect (priv->modify, "clicked",
			  G_CALLBACK (adblock_ui_modify_cb), dialog);
	g_signal_connect (priv->load, "clicked",
			  G_CALLBACK (adblock_ui_load_cb), dialog);
	g_signal_connect (priv->license, "clicked",
			  G_CALLBACK (adblock_ui_show_license_cb), dialog);

	/* Build all the pages */
	for (i = WHITELIST_PAGE; i < LAST_PAGE; i++)
	{
		InfoPage *page = priv->pages [i];

		page->construct (page);
		page->fill (page);
	}

	/* If the ui is asked to edit a specific url */
	if (priv->url != NULL)
	{
		gtk_notebook_set_current_page (
			priv->notebook,
			priv->url_allowed ? WHITELIST_PAGE : BLACKLIST_PAGE);
		/* Be careful, changing page changes selection 
		   and then the rule entry. So, change entry value *after*
		   page switching */
		gtk_entry_set_text (priv->pattern, priv->url);
	}

	return object;
}

static void
adblock_ui_set_treeview_commons (InfoPage *page)
{
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (page->treeview,
			COL_PATTERN, _("Pattern"),
			renderer,
			"text", COL_PATTERN,
			NULL);

	gtk_tree_sortable_set_sort_column_id (
			GTK_TREE_SORTABLE (page->store),
			COL_PATTERN, 
			GTK_SORT_ASCENDING);

	gtk_tree_view_set_model (page->treeview, GTK_TREE_MODEL (page->store));
	gtk_tree_view_set_search_column (page->treeview, COL_PATTERN);

	g_object_unref (page->store);

	page->selection = gtk_tree_view_get_selection (page->treeview);
	gtk_tree_selection_set_mode (page->selection, GTK_SELECTION_SINGLE);

	g_signal_connect (page->selection, "changed",
			  G_CALLBACK (adblock_ui_selection_changed_cb),
			  page);
}

static void
adblock_ui_page_fill (InfoPage *page)
{
	adblock_ui_populate_store (page, page->type);
}

static void
adblock_ui_page_construct (InfoPage *page)
{
	EphyDialog *edialog;

	edialog = EPHY_DIALOG (page->dialog);

	switch (page->type)
	{
		case PATTERN_WHITELIST:
			ephy_dialog_get_controls (edialog,
					  	  properties[PROP_WHITELIST].id, 
						  &page->treeview,
					  	  NULL);
			break;
		case PATTERN_BLACKLIST:
			ephy_dialog_get_controls (edialog,
					  	  properties[PROP_BLACKLIST].id, 
						  &page->treeview,
					  	  NULL);
			break;
		case PATTERN_DEFAULT_BLACKLIST:
			ephy_dialog_get_controls (edialog,
					  	  properties[PROP_DEFAULTLIST].id, 
						  &page->treeview,
					  	  NULL);
			break;
		default:
			g_return_if_reached ();
	}
	page->store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING);

	adblock_ui_set_treeview_commons (page);
}

static InfoPage *
adblock_ui_page_new (AdblockUI *dialog, AdblockPatternType type)
{
	InfoPage *page = g_new0 (InfoPage, 1);

	page->dialog = dialog;
	page->construct = adblock_ui_page_construct;
	page->fill = adblock_ui_page_fill;
	page->type = type;

	return page;
}

static void
adblock_ui_init (AdblockUI *dialog)
{
	LOG ("AdblockUI initialising");
	
	dialog->priv = ADBLOCK_UI_GET_PRIVATE (dialog);

	dialog->priv->pages[WHITELIST_PAGE] = 
		adblock_ui_page_new (dialog, PATTERN_WHITELIST);
	dialog->priv->pages[BLACKLIST_PAGE] = 
		adblock_ui_page_new (dialog, PATTERN_BLACKLIST);
	dialog->priv->pages[DEFAULT_BLACKLIST_PAGE] = 
		adblock_ui_page_new ( dialog, PATTERN_DEFAULT_BLACKLIST);

	/* Default page */
	dialog->priv->current_page = dialog->priv->pages[WHITELIST_PAGE];

	/* Init attributes */	
	dialog->priv->url = NULL;
}

static void
adblock_ui_finalize (GObject *object)
{
	AdblockUI *dialog = ADBLOCK_UI (object);

	g_free (dialog->priv->url);

	G_OBJECT_CLASS(adblock_ui_parent_class)->finalize (object);
}

static void
adblock_ui_get_property (GObject *object,
			 guint prop_id,
			 GValue *value,
			 GParamSpec *pspec)
{
	/* no readable properties */
	g_return_if_reached ();
}

static void
adblock_ui_set_property (GObject *object,
			 guint prop_id,
			 const GValue *value,
			 GParamSpec *pspec)
{
	AdblockUI *dialog = ADBLOCK_UI (object);

	switch (prop_id)
	{
		case PROP_TESTER:
			dialog->priv->tester = g_value_get_object (value);
			break;
		case PROP_URL:
			dialog->priv->url = g_strdup (g_value_get_string (value));
			break;
		case PROP_URL_ALLOWED:
			dialog->priv->url_allowed = g_value_get_boolean (value);
			break;
	}
}

static void
adblock_ui_class_init (AdblockUIClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	adblock_ui_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = adblock_ui_finalize;
	object_class->constructor = adblock_ui_constructor;
	object_class->get_property = adblock_ui_get_property;
	object_class->set_property = adblock_ui_set_property;

	g_object_class_install_property
		(object_class,
		 PROP_TESTER,
		 g_param_spec_object ("tester",
				      "AdUriTester",
				      "AdUriTester",
				      TYPE_AD_URI_TESTER,
				      G_PARAM_WRITABLE |
				      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property
		(object_class,
		 PROP_URL,
		 g_param_spec_string ("url",
				      "URL",
				      "url to be blocked/unblocked",
				      NULL,
				      G_PARAM_WRITABLE |
				      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (
		object_class,
		PROP_URL_ALLOWED,
		g_param_spec_boolean ("url_allowed",
			 	      "URL Allowed",
				      "Whether url is to be allowed",
				       TRUE,
				       G_PARAM_READWRITE | 
				       G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof (AdblockUIPrivate));
}

AdblockUI *
adblock_ui_new (AdUriTester *tester,
		const char *url,
		gboolean allowed)
{
	return g_object_new (TYPE_ADBLOCK_UI,
			     "tester"     , tester,
			     "url"        , url,
			     "url_allowed", allowed,
			     NULL);
}
