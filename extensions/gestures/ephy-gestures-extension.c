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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ephy-gestures-extension.h"
#include "ephy-gesture.h"

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-extension.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>

#include <gmodule.h>
#include <libxml/tree.h>

#include <string.h>

#define EPHY_GESTURES_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_GESTURES_EXTENSION, EphyGesturesExtensionPrivate))

struct EphyGesturesExtensionPrivate
{
	GHashTable *gestures;
};

#define EPHY_GESTURES_XML_FILE		"ephy-gestures.xml"
#define EPHY_GESTURES_XML_ROOT		"epiphany_gestures"
#define EPHY_GESTURES_XML_VERSION	"0.4"

static GObjectClass *parent_class = NULL;

static void ephy_gestures_extension_class_init	(EphyGesturesExtensionClass *klass);
static void ephy_gestures_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_gestures_extension_init	(EphyGesturesExtension *extension);

static GType type = 0;

GType
ephy_gestures_extension_get_type (void)
{
	g_return_val_if_fail (type != 0, 0);

	return type;
}

GType
ephy_gestures_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyGesturesExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_gestures_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyGesturesExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_gestures_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_gestures_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyGesturesExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
load_one_gesture (EphyGesturesExtension *extension,
		  xmlNodePtr node)
{
	xmlNodePtr child;
	GSList *sequences = NULL, *l;
	xmlChar *s, *action = NULL;

	for (child = node->children; child != NULL; child = child->next)
	{
		if (xmlStrEqual (child->name, "sequence"))
		{
			s = xmlNodeGetContent (child);

			sequences = g_slist_prepend (sequences, s);
		}
		else if (xmlStrEqual (child->name, "action"))
		{
			if (action == NULL)
			{
				action = xmlNodeGetContent (child);
			}
			else
			{
				g_warning ("Only one action per gesture allowed!\n");
			}
		}
	}

	if (sequences == NULL || action == NULL)
	{
		g_warning ("Error parsing gestures definition file\n");
		return;
	}

	for (l = sequences; l != NULL ; l = l->next)
	{
		g_hash_table_insert (extension->priv->gestures,
				     g_strdup (l->data),
				     g_strdup (action));
		xmlFree (l->data);
	}

	g_slist_free (sequences);
	xmlFree (action);
}

static void
load_gestures (EphyGesturesExtension *extension,
	       const char *filename)
{
	xmlDocPtr doc;
	xmlNodePtr root, child;
	xmlChar *tmp = NULL;

	if (g_file_test (filename, G_FILE_TEST_EXISTS) == FALSE)
	{
		return;
	}

	doc = xmlParseFile (filename);
	if (doc == NULL)
	{
		g_warning ("Failed to load the gestures definitions from %s\n",
			   filename);
		return;
	}

	root = xmlDocGetRootElement (doc);
	if (root == NULL || strcmp (root->name, EPHY_GESTURES_XML_ROOT) != 0)
	{
		g_warning ("Gestures definitions file %s has wrong format '%s'"
		           "(expected " EPHY_GESTURES_XML_ROOT ")\n",
			   filename, root ? (char*) root->name : "(unknown)");
		goto out;
	}

	tmp = xmlGetProp (root, "version");
	if (tmp  == NULL || strcmp (tmp, EPHY_GESTURES_XML_VERSION) != 0)
	{
		g_warning ("Gestures definitions file %s has wrong format version %s"
			   "(expected " EPHY_GESTURES_XML_VERSION ")\n",
			   filename, tmp ? (char*) tmp : "(unknown)");
		goto out;
	}

	for (child = root->children; child != NULL; child = child->next)
	{
		if (xmlStrEqual (child->name, "gesture"))
		{
			load_one_gesture (extension, child);
		}
	}

out:
	xmlFreeDoc (doc);

	xmlFree (tmp);
}

static void
ephy_gestures_extension_init (EphyGesturesExtension *extension)
{
	char *filename;

	extension->priv = EPHY_GESTURES_EXTENSION_GET_PRIVATE (extension);

	extension->priv->gestures =
		g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	/* load the system gestures definitions */
	filename = g_build_filename (SHARE_DIR, EPHY_GESTURES_XML_FILE, NULL);
	load_gestures (extension, filename);
	g_free (filename);

	/* now load the user's gestures definitions */
	filename = g_build_filename (ephy_dot_dir (), EPHY_GESTURES_XML_FILE, NULL);
	load_gestures (extension, filename);
	g_free (filename);
}

static void
ephy_gestures_extension_finalize (GObject *object)
{
	EphyGesturesExtension *extension = EPHY_GESTURES_EXTENSION (object);

	g_hash_table_destroy (extension->priv->gestures);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gesture_performed_cb (EphyGesture *gesture,
		      const char *sequence,
		      EphyGesturesExtension *extension)
{
	EphyWindow *window;
	const char *path;

	LOG ("Gesture: sequence '%s'", sequence)

	path = g_hash_table_lookup (extension->priv->gestures, sequence);
	if (path == NULL)
	{
		return;
	}

	window = EPHY_WINDOW (ephy_gesture_get_window (gesture));
	g_return_if_fail (EPHY_IS_WINDOW (window));

	if (strcmp (path, "fallback") == 0)
	{
		/* Fall back to normal click */
		EphyEmbed *embed;
		EphyEmbedEvent *event;
		gint handled = FALSE;
		guint type;

		embed = ephy_window_get_active_embed (window);
		g_return_if_fail (EPHY_IS_EMBED (embed));

		event = ephy_gesture_get_event (gesture);
		g_return_if_fail (EPHY_IS_EMBED_EVENT (event));

		type = ephy_embed_event_get_event_type (event);

		g_signal_emit_by_name (embed, "ge_dom_mouse_click", event,
				       &handled);

		if (handled == FALSE && type == EPHY_EMBED_EVENT_MOUSE_BUTTON3)
		{
			g_signal_emit_by_name (embed, "ge_context_menu",
					       event, &handled);
		}
	}
	else
	{
		GtkUIManager *manager;
		GtkAction *action;
		
		manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
		
		action = gtk_ui_manager_get_action (manager, path);
		
		if (action == NULL)
		{
			g_warning ("Action for path '%s' not found!\n", path);
			return;
		}

		gtk_action_activate (action);
	}
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyGesturesExtension *extension)
{
        EphyEmbedEventContext context;
	EphyEmbedEventType type;
	gint handled = FALSE;

	type = ephy_embed_event_get_event_type (event);
        context = ephy_embed_event_get_context (event);

	if (type == EPHY_EMBED_EVENT_MOUSE_BUTTON2 &&
            !(context & EPHY_EMBED_CONTEXT_INPUT))
	{
		EphyGesture *gesture;
		GtkWidget *toplevel;
		EphyTab *tab;

		tab = ephy_tab_for_embed (embed);
		g_return_val_if_fail (EPHY_IS_TAB (tab), handled);

		toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
		g_return_val_if_fail (toplevel != NULL, handled);

		gesture = ephy_gesture_new (toplevel, event);

		g_signal_connect (gesture, "gesture-performed",
				  G_CALLBACK (gesture_performed_cb),
				  extension);

		ephy_gesture_start (gesture);

		handled = TRUE;
	}

	return handled;
}

static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("Attach tab")

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_connect (embed, "ge_dom_mouse_down",
			  G_CALLBACK (dom_mouse_down_cb), extension);
}

static void
impl_detach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("Detach tab")

	g_return_if_fail (EPHY_IS_TAB (tab));

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), extension);
}

static void
ephy_gestures_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_gestures_extension_class_init (EphyGesturesExtensionClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = ephy_gestures_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyGesturesExtensionPrivate));
}
