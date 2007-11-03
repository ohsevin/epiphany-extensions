/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003, 2004 Christian Persch
 *  Copyright © 2004 Justin Wake
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

#include "ephy-tab-grouper.h"
#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-notebook.h>

#define EPHY_TAB_GROUPER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_TAB_GROUPER, EphyTabGrouperPrivate))

struct _EphyTabGrouperPrivate
{
	GtkWidget *notebook;
	int last_pos;
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
	const GTypeInfo our_info =
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
static void
reset_last_tab (EphyTabGrouper *grouper)
{
	EphyTabGrouperPrivate *priv = grouper->priv;

	priv->last_pos = -1;
}

static void
notebook_page_added_cb (GtkNotebook *notebook,
			GtkWidget *tab,
			guint page,
			EphyTabGrouper *grouper)
{
	EphyTabGrouperPrivate *priv = grouper->priv;
	int position;

	/* Calculate the appropriate position for the tab */
	if (priv->last_pos != -1)
	{
		position = priv->last_pos + 1;
	}
	else
	{
		position = gtk_notebook_get_current_page (notebook) + 1;
	}

	gtk_notebook_reorder_child (notebook, tab, position);

	priv->last_pos = position;
}

static void
ephy_tab_grouper_set_notebook (EphyTabGrouper *grouper,
			       GtkWidget *notebook)
{
	EphyTabGrouperPrivate *priv = grouper->priv;

	g_return_if_fail (GTK_IS_NOTEBOOK (notebook));

	priv->notebook = notebook;

	/* Hook up the signals */
	g_signal_connect (notebook, "page-added",
			  G_CALLBACK (notebook_page_added_cb), grouper);
	g_signal_connect_swapped (notebook, "page-removed", 
				  G_CALLBACK (reset_last_tab), grouper);
	g_signal_connect_swapped (notebook, "page-reordered", 
				  G_CALLBACK (reset_last_tab), grouper);
	g_signal_connect_swapped (notebook, "switch-page", 
				  G_CALLBACK (reset_last_tab), grouper);
}

static void
ephy_tab_grouper_init (EphyTabGrouper *grouper)
{
	grouper->priv = EPHY_TAB_GROUPER_GET_PRIVATE (grouper);

	LOG ("EphyTabGrouper initialised");
}

static void
ephy_tab_grouper_finalize (GObject *object)
{
	EphyTabGrouper *grouper = EPHY_TAB_GROUPER (object);

	/* Disconnect the signal handlers */
	g_signal_handlers_disconnect_matched
		(grouper->priv->notebook, G_SIGNAL_MATCH_DATA,
		 0, 0, NULL, NULL, grouper);

	LOG ("EphyTabGrouper finalised");

	parent_class->finalize (object);
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
