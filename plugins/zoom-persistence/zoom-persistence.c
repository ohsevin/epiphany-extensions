/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-node.h>
#include <epiphany/ephy-node-db.h>
#include <epiphany/ephy-history.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/session.h>
#include <epiphany/ephy-types.h>

#include <gmodule.h>
#include <glib-object.h>
#include <libxml/tree.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <string.h>
#include <time.h>

#define EPHY_ZOOM_PERSISTENCE_XML_FILE		"ephy-zoom-persistence.xml"
#define EPHY_ZOOM_PERSISTENCE_XML_ROOT		"epiphany_zoom_persistence"
#define EPHY_ZOOM_PERSISTENCE_XML_VERSION	"0.1"

#define EPHY_NODE_DB_ZOOM_PERSISTENCE_HOSTS	"EphyZoomPersistenceHosts"
#define HOSTS_NODE_ID				19

/* how often to save the data, in millisecond */
#define HISTORY_SAVE_INTERVAL		(5 * 60 * 1000)

#define HISTORY_HOST_OBSOLETE_DAYS	180

enum
{
	EPHY_NODE_HOST_PROP_HOST	= 0,
	EPHY_NODE_HOST_PROP_LAST_VISIT	= 1,
	EPHY_NODE_HOST_PROP_ZOOM	= 2
};

typedef struct
{
	char *xml_file;
	EphyNodeDb *db;
	EphyNode *hosts;
	GHashTable *hosts_hash;
	GStaticRWLock *hosts_hash_lock;
	int autosave_timeout;
}
EphyZoomPersistencePrivate;

static EphyZoomPersistencePrivate *priv = NULL;

static void
load_hosts_db (void)
{
	xmlDocPtr doc;
	xmlNodePtr root, child;
	xmlChar *tmp;

	if (g_file_test (priv->xml_file, G_FILE_TEST_EXISTS) == FALSE) return;

	LOG ("Loading hosts db")

	doc = xmlParseFile (priv->xml_file);
	g_return_if_fail (doc != NULL);

	root = xmlDocGetRootElement (doc);
	g_return_if_fail (root && strcmp (root->name, EPHY_ZOOM_PERSISTENCE_XML_ROOT) == 0);

	tmp = xmlGetProp (root, "version");
	g_return_if_fail (tmp && strcmp (tmp, EPHY_ZOOM_PERSISTENCE_XML_VERSION) == 0);
	xmlFree (tmp);

	for (child = root->children; child != NULL; child = child->next)
	{
		EphyNode *node;

		node = ephy_node_new_from_xml (priv->db, child);
	}

	xmlFreeDoc (doc);
}

static void
save_hosts_db (void)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	GPtrArray *children;
	int i;

	LOG ("Saving hosts db")

	/* save nodes to xml */
	xmlIndentTreeOutput = TRUE;
	doc = xmlNewDoc ("1.0");

	root = xmlNewDocNode (doc, NULL, EPHY_ZOOM_PERSISTENCE_XML_ROOT, NULL);
	xmlSetProp (root, "version", EPHY_ZOOM_PERSISTENCE_XML_VERSION);
	xmlDocSetRootElement (doc, root);

	children = ephy_node_get_children (priv->hosts);
	for (i = 0; i < children->len; i++)
	{
		EphyNode *kid;

		kid = g_ptr_array_index (children, i);

		ephy_node_save_to_xml (kid, root);
	}
	ephy_node_thaw (priv->hosts);

	ephy_file_save_xml (priv->xml_file, doc);

	xmlFreeDoc(doc);
}

static gboolean
host_is_obsolete (EphyNode *node, GDate *now)
{
	int last_visit;
	GDate date;

	last_visit = ephy_node_get_property_int
		(node, EPHY_NODE_HOST_PROP_LAST_VISIT);

        g_date_clear (&date, 1);
        g_date_set_time (&date, last_visit);

	return (g_date_days_between (&date, now) >=
		HISTORY_HOST_OBSOLETE_DAYS);
}

static void
remove_obsolete_hosts (void)
{
	GPtrArray *children;
	int i;
	GTime now;
	GDate current_date;

	now = time (NULL);
        g_date_clear (&current_date, 1);
        g_date_set_time (&current_date, time (NULL));

	children = ephy_node_get_children (priv->hosts);
	ephy_node_thaw (priv->hosts);
	for (i = 0; i < children->len; i++)
	{
		EphyNode *kid;

		kid = g_ptr_array_index (children, i);

		if (host_is_obsolete (kid, &current_date))
		{
			ephy_node_unref (kid);
		}
	}
}

static gboolean
periodic_save_cb (void)
{
	remove_obsolete_hosts ();
	save_hosts_db ();

	return TRUE;
}

static void
host_visited (EphyNode *host, GTime now)
{
	GValue value = { 0, };

	g_value_init (&value, G_TYPE_INT);
	g_value_set_int (&value, now);
	ephy_node_set_property (host, EPHY_NODE_HOST_PROP_LAST_VISIT, &value);
	g_value_unset (&value);
}

static EphyNode *
get_host_node (const char *url)
{
	GnomeVFSURI *vfs_uri = NULL;
	EphyNode *host = NULL;
	const char *host_name = NULL;
	const char *scheme = NULL;
	char *host_url = NULL;
	GTime now;

	/* bail out if it's a javascript: url */
	if (strncmp (url, "javascript:", 11) == 0) return NULL;

	now = time (NULL);

	vfs_uri = gnome_vfs_uri_new (url);

	if (vfs_uri)
	{
		scheme = gnome_vfs_uri_get_scheme (vfs_uri);
		host_name = gnome_vfs_uri_get_host_name (vfs_uri);
	}

	/* Build an host name */
	if (scheme == NULL || host_name == NULL)
	{
		host_url = g_strdup ("about:blank");
	}
	else if (strcmp (scheme, "file") == 0)
	{
		host_url = g_strdup ("file:///");
	}
	else
	{
		const char *tmp = host_name;

		if (g_str_has_prefix (host_name, "www."))
		{
			tmp = g_utf8_offset_to_pointer (host_name, 4);
		}

		host_url = g_strconcat (scheme, "://", tmp, "/", NULL);
	}

	g_return_val_if_fail (host_url != NULL, NULL);

	LOG ("host name is '%s'", host_url)

	g_static_rw_lock_reader_lock (priv->hosts_hash_lock);
	host = g_hash_table_lookup (priv->hosts_hash, host_url);
	g_static_rw_lock_reader_unlock (priv->hosts_hash_lock);

	if (!host)
	{
		GValue value = { 0, };

		LOG ("Host not found, adding as new")

		host = ephy_node_new (priv->db);

		g_value_init (&value, G_TYPE_STRING);
		g_value_set_string (&value, host_url);
		ephy_node_set_property (host, EPHY_NODE_HOST_PROP_HOST, &value);
		g_value_unset (&value);

		ephy_node_add_child (priv->hosts, host);
	}

	host_visited (host, now);

	if (vfs_uri)
	{
		gnome_vfs_uri_unref (vfs_uri);
	}

	return host;
}

static void
zoom_cb (EphyTab *tab, GParamSpec *pspec, EphyEmbed *embed)
{
	EphyNode *host;
	const char *address;
	GValue value = { 0, };

	address = ephy_tab_get_location (tab);

	LOG ("zoom_cb address '%s' zoom %f", address, ephy_tab_get_zoom (tab))

	host = get_host_node (address);
	if (host == NULL) return;

	g_value_init (&value, G_TYPE_FLOAT);
	g_value_set_float (&value, ephy_tab_get_zoom (tab));
	ephy_node_set_property (host, EPHY_NODE_HOST_PROP_ZOOM, &value);
	g_value_unset (&value);
}

static void
address_cb (EphyEmbed *embed, const char *address, EphyTab *tab)
{
	EphyNode *host;
	float zoom, current_zoom;
	gresult rv;

	g_return_if_fail (address != NULL);

	rv = ephy_embed_zoom_get (embed, &current_zoom);
	if (rv != G_OK) return;

	host = get_host_node (address);
	if (host == NULL) return;

	zoom = ephy_node_get_property_float (host, EPHY_NODE_HOST_PROP_ZOOM);
	if (zoom < 0.0)
	{
		zoom = 1.0;
	}

	LOG ("address_cb address '%s' current %f new %f", address, current_zoom, zoom)

	if (zoom != current_zoom)
	{
		ephy_embed_zoom_set (embed, zoom, FALSE);
	}
}

static void
tab_added_cb (GtkWidget *notebook, GtkWidget *embed)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (embed), "EphyTab"));
	g_return_if_fail (IS_EPHY_TAB (tab));

	g_signal_connect (G_OBJECT (tab), "notify::zoom",
			  G_CALLBACK (zoom_cb), embed);
	g_signal_connect (G_OBJECT (embed), "ge_location",
			  G_CALLBACK (address_cb), tab);
}

static void
tab_removed_cb (GtkWidget *notebook, GtkWidget *embed)
{
	EphyTab *tab;

	tab = EPHY_TAB (g_object_get_data (G_OBJECT (embed), "EphyTab"));
	g_return_if_fail (IS_EPHY_TAB (tab));

	g_signal_handlers_disconnect_by_func (G_OBJECT (tab),
					      G_CALLBACK (zoom_cb),
					      embed);
	g_signal_handlers_disconnect_by_func (G_OBJECT (embed),
					      G_CALLBACK (address_cb),
					      tab);
}

static void
new_window_cb (Session *session, EphyWindow *window)
{
	GtkWidget *notebook;

	notebook = ephy_window_get_notebook (window);

	g_signal_connect (notebook, "tab_added",
			  G_CALLBACK (tab_added_cb), NULL);
	g_signal_connect (notebook, "tab_removed",
			  G_CALLBACK (tab_removed_cb), NULL);
}

static void
host_added_cb (EphyNode *node, EphyNode *child, gpointer data)
{
	const char *host;

	host = ephy_node_get_property_string (child, EPHY_NODE_HOST_PROP_HOST);

	LOG ("host_added_cb host '%s'", host)

        g_static_rw_lock_writer_lock (priv->hosts_hash_lock);
        g_hash_table_insert (priv->hosts_hash, g_strdup (host), child);
        g_static_rw_lock_writer_unlock (priv->hosts_hash_lock);
}

static void
host_removed_cb (EphyNode *node, EphyNode *child, guint old_index, gpointer data)
{
	const char *host;

	host = ephy_node_get_property_string (child, EPHY_NODE_HOST_PROP_HOST);

	LOG ("host_removed_cb host '%s'", host)

        g_static_rw_lock_writer_lock (priv->hosts_hash_lock);
        g_hash_table_remove (priv->hosts_hash, host);
        g_static_rw_lock_writer_unlock (priv->hosts_hash_lock);
}

G_MODULE_EXPORT void
plugin_init (GTypeModule *module)
{
	Session *session;

	LOG ("Zoom persistence plugin initialising")

	priv = g_new0 (EphyZoomPersistencePrivate, 1);

	priv->db = ephy_node_db_new (EPHY_NODE_DB_ZOOM_PERSISTENCE_HOSTS);

	priv->xml_file = g_build_filename (ephy_dot_dir (),
					   EPHY_ZOOM_PERSISTENCE_XML_FILE,
					   NULL);

	priv->hosts_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
						  g_free, NULL);
	priv->hosts_hash_lock = g_new0 (GStaticRWLock, 1);
	g_static_rw_lock_init (priv->hosts_hash_lock);

	priv->hosts = ephy_node_new_with_id (priv->db, HOSTS_NODE_ID);
	ephy_node_ref (priv->hosts);

        ephy_node_signal_connect_object (priv->hosts,
                                         EPHY_NODE_CHILD_ADDED,
                                         (EphyNodeCallback) host_added_cb,
                                         NULL);
        ephy_node_signal_connect_object (priv->hosts,
                                         EPHY_NODE_CHILD_REMOVED,
                                         (EphyNodeCallback) host_removed_cb,
                                         NULL);

	load_hosts_db ();

	priv->autosave_timeout = g_timeout_add (HISTORY_SAVE_INTERVAL,
						(GSourceFunc) periodic_save_cb,
						NULL);

	session = SESSION (ephy_shell_get_session (ephy_shell));
	g_signal_connect (session, "new_window",
			  G_CALLBACK (new_window_cb), NULL);
}

G_MODULE_EXPORT void
plugin_exit (void)
{
	LOG ("Zoom persistence plugin exiting")

	g_source_remove (priv->autosave_timeout);
	save_hosts_db ();

	ephy_node_unref (priv->hosts);

	g_object_unref (priv->db);

	g_hash_table_destroy (priv->hosts_hash);
	g_static_rw_lock_free (priv->hosts_hash_lock);

	g_free (priv->xml_file);

	g_free (priv);
	priv = NULL;
}
