/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004 Christian Persch
 *  Copyright (C) 2004 Justin Wake
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-tab-grouper.h"
#include "ephy-debug.h"

#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-window.h>

#define EPHY_TAB_GROUPER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TAB_GROUPER, EphyTabGrouperPrivate))

struct _EphyTabGrouperPrivate
{
	GtkWidget *notebook;
	EphyTab *last_opened_tab;
};

static void ephy_tab_grouper_class_init (EphyTabGrouperClass *klass);
static void ephy_tab_grouper_init	(EphyTabGrouper *grouper);

enum
{
	PROP_0,
	PROP_NOTEBOOK
};

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_tab_grouper_get_type (void)
{
	return type;
}

GType
ephy_tab_grouper_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyTabGrouperClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_tab_grouper_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyTabGrouper),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_tab_grouper_init
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyTabGrouper",
					    &our_info, 0);
	
	return type;
}

/**
 * Clear the recently opened tab spot.
 */
static gboolean
reset_last_tab (EphyTabGrouper *grouper)
{
	if (grouper->priv->last_opened_tab != NULL)
	{
		g_object_unref (grouper->priv->last_opened_tab);
		grouper->priv->last_opened_tab = NULL;
	}

	return FALSE;
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyTab *tab,
	      EphyTabGrouper *grouper)
{
	guint position = -1;

	/* Calculate the appropriate position for the tab */
	if (grouper->priv->last_opened_tab != NULL)
	{
		position = gtk_notebook_page_num
			(GTK_NOTEBOOK (notebook),
			 GTK_WIDGET (grouper->priv->last_opened_tab)) + 1;
	}
	else
	{
		position = gtk_notebook_get_current_page
				(GTK_NOTEBOOK (notebook)) + 1;
	}

	gtk_notebook_reorder_child
		(GTK_NOTEBOOK (notebook), GTK_WIDGET (tab), position);
		
	/* Clear the last tab so we can set it to the new tab */
	reset_last_tab (grouper);

	grouper->priv->last_opened_tab = g_object_ref (tab);
}

static void
ephy_tab_grouper_set_notebook (EphyTabGrouper *grouper,
			       GtkWidget *notebook)
{
	g_return_if_fail (GTK_IS_NOTEBOOK (notebook));

	grouper->priv->notebook = notebook;

	/* Hook up the signals */
	g_signal_connect (notebook, "tab_added", 
			  G_CALLBACK (tab_added_cb), grouper);
	g_signal_connect_swapped (notebook, "tab_removed", 
				  G_CALLBACK (reset_last_tab), grouper);
	g_signal_connect_swapped (notebook, "tab_detached", 
				  G_CALLBACK (reset_last_tab), grouper);
	g_signal_connect_swapped (notebook, "switch_page", 
				  G_CALLBACK (reset_last_tab), grouper);
	g_signal_connect_swapped (notebook, "tabs_reordered", 
				  G_CALLBACK (reset_last_tab), grouper);	
	g_signal_connect_swapped (notebook, "tab_delete", 
				  G_CALLBACK (reset_last_tab), grouper);
}

static void
ephy_tab_grouper_init (EphyTabGrouper *grouper)
{
	grouper->priv = EPHY_TAB_GROUPER_GET_PRIVATE (grouper);

	LOG ("EphyTabGrouper initialised")
}

static void
ephy_tab_grouper_finalize (GObject *object)
{
	EphyTabGrouper *grouper = EPHY_TAB_GROUPER (object);

	reset_last_tab (grouper);

	/* Disconnect the signal handlers */
	g_signal_handlers_disconnect_matched
		(grouper->priv->notebook, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, grouper);

	LOG ("EphyTabGrouper finalised")

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_tab_grouper_set_property (GObject *object,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *pspec)
{
	EphyTabGrouper *grouper = EPHY_TAB_GROUPER (object);

	switch (prop_id)
	{
		case PROP_NOTEBOOK:
			ephy_tab_grouper_set_notebook (grouper, g_value_get_object (value));
			break;
	}
}

static void
ephy_tab_grouper_get_property (GObject *object,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *pspec)
{
	EphyTabGrouper *grouper = EPHY_TAB_GROUPER (object);

	switch (prop_id)
	{
		case PROP_NOTEBOOK:
			g_value_set_object (value, grouper->priv->notebook);
			break;
	}
}

static void
ephy_tab_grouper_class_init (EphyTabGrouperClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_tab_grouper_finalize;
	object_class->set_property = ephy_tab_grouper_set_property;
	object_class->get_property = ephy_tab_grouper_get_property;

	g_object_class_install_property (object_class,
					 PROP_NOTEBOOK,
					 g_param_spec_object ("notebook",
							      "Notebook",
							      "Notebook",
							      GTK_TYPE_NOTEBOOK,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));
	
	g_type_class_add_private (object_class, sizeof (EphyTabGrouperPrivate));
}

EphyTabGrouper *
ephy_tab_grouper_new (GtkWidget *notebook)
{
	g_return_val_if_fail (notebook != NULL, NULL);

	return EPHY_TAB_GROUPER (g_object_new (EPHY_TYPE_TAB_GROUPER,
					       "notebook", notebook,
					       NULL));
}
