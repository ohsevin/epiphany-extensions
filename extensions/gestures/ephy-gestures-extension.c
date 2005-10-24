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

#include "config.h"

#include "ephy-gestures-extension.h"
#include "ephy-gesture.h"

#include "ephy-file-helpers.h"
#include "ephy-debug.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-extension.h>

#include <gmodule.h>
#include <libxml/tree.h>

#include <string.h>

#define WINDOW_DATA_KEY	"EphyGesturesExtension::WindowData"

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
		if (xmlStrEqual (child->name, (const xmlChar*) "sequence"))
		{
			s = xmlNodeGetContent (child);

			sequences = g_slist_prepend (sequences, s);
		}
		else if (xmlStrEqual (child->name, (const xmlChar*) "action"))
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
				     g_strdup ((char*)action));
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
	if (root == NULL || strcmp ((char*)root->name, EPHY_GESTURES_XML_ROOT) != 0)
	{
		g_warning ("Gestures definitions file %s has wrong format '%s'"
		           "(expected " EPHY_GESTURES_XML_ROOT ")\n",
			   filename, root ? (char*) root->name : "(unknown)");
		goto out;
	}

	tmp = xmlGetProp (root, (const xmlChar*) "version");
	if (tmp  == NULL || strcmp ((char*) tmp, (char*) EPHY_GESTURES_XML_VERSION) != 0)
	{
		g_warning ("Gestures definitions file %s has wrong format version %s"
			   "(expected " EPHY_GESTURES_XML_VERSION ")\n",
			   filename, tmp ? (char*) tmp : "(unknown)");
		goto out;
	}

	for (child = root->children; child != NULL; child = child->next)
	{
		if (xmlStrEqual (child->name, (const xmlChar*) "gesture"))
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
	const char *path;

	LOG ("Gesture: sequence '%s'", sequence);

	path = g_hash_table_lookup (extension->priv->gestures, sequence);
	if (path)
	{
		ephy_gesture_activate(gesture, path);
	}
}

static EphyGesture *
ensure_gesture (EphyGesturesExtension *extension,
		EphyWindow *window)
	        
{
	EphyGesture *gesture;

	gesture = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	if (gesture == NULL)
	{
		gesture = ephy_gesture_new (GTK_WIDGET (window));
		g_signal_connect (gesture, "gesture-performed",
				  G_CALLBACK (gesture_performed_cb),
				  extension);

		g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY,
					gesture,
					(GDestroyNotify) g_object_unref);
	}

	return gesture;
}

static gboolean
dom_mouse_down_cb (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   EphyGesturesExtension *extension)
{
        EphyEmbedEventContext context;
	guint button;
	gint handled = FALSE;
	EphyTab *tab;
	EphyWindow *window;
	GtkWidget *toplevel;
	gboolean ppv_mode;

	tab = ephy_tab_for_embed (embed);
	g_return_val_if_fail (EPHY_IS_TAB (tab), handled);
		
	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
	g_return_val_if_fail (toplevel != NULL, handled);

	/* disable gestures while print preview mode */
	window = EPHY_WINDOW (toplevel);
	g_return_val_if_fail (EPHY_IS_WINDOW (window), handled);

	g_object_get (window, "print-preview-mode", &ppv_mode, NULL);
	if (ppv_mode) return handled;

	button = ephy_embed_event_get_button (event);
        context = ephy_embed_event_get_context (event);

	if (button == 2 &&
	    !((context & EPHY_EMBED_CONTEXT_INPUT) ||
	      (context & EPHY_EMBED_CONTEXT_LINK)))
	{
		EphyGesture *gesture;

		gesture = ensure_gesture (extension, window);
		g_return_val_if_fail (gesture != NULL, FALSE);

		ephy_gesture_set_event (gesture, event);

		ephy_gesture_start (gesture);

		handled = TRUE;
	}

	return handled;
}

static void
impl_detach_window (EphyExtension *ext,
                    EphyWindow *window)
{
        g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_attach_tab (EphyExtension *extension,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;

	LOG ("Attach tab");

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

	LOG ("Detach tab");

	embed = ephy_tab_get_embed (tab);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (dom_mouse_down_cb), extension);
}

static void
ephy_gestures_extension_iface_init (EphyExtensionIface *iface)
{
	iface->detach_window = impl_detach_window;
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
