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

#include "ephy-certificates-extension.h"

#include "mozilla-embed-certificate.h"
#include "ephy-debug.h"

#include <epiphany/ephy-window.h>
#include <epiphany/ephy-extension.h>
#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-tab.h>

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkrc.h>
#include <gtk/gtkeventbox.h>
#include <gmodule.h>
#include <glib/gi18n-lib.h>

#define EPHY_CERTIFICATES_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_CERTIFICATES_EXTENSION, EphyCertificatesExtensionPrivate))

struct EphyCertificatesExtensionPrivate
{
};

static void ephy_certificates_extension_class_init	 (EphyCertificatesExtensionClass *klass);
static void ephy_certificates_extension_iface_init	 (EphyExtensionIface *iface);
static void ephy_certificates_extension_init		 (EphyCertificatesExtension *extension);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_certificates_extension_get_type (void)
{
	return type;
}

GType
ephy_certificates_extension_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyCertificatesExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_certificates_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyCertificatesExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_certificates_extension_init
	};

	static const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_certificates_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyCertificatesExtension",
					    &our_info, 0);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);
	return type;
}

static void
ephy_certificates_extension_init (EphyCertificatesExtension *extension)
{
	extension->priv = EPHY_CERTIFICATES_EXTENSION_GET_PRIVATE (extension);

	LOG ("EphyCertificatesExtension initialising")
}

static void
ephy_certificates_extension_finalize (GObject *object)
{
	LOG ("EphyCertificatesExtension finalising")

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_certificates_extension_view_certificate_cb (GtkAction *action,
						 EphyWindow *window)
{
	EphyEmbed *embed;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (EPHY_IS_EMBED (embed));

	mozilla_embed_view_certificate (embed);
}

static void
sync_security_status (EphyTab *tab,
		      GParamSpec *pspec,
		      EphyWindow *window)
{
	GtkUIManager *manager;
	GtkAction *action;
	gboolean is_secure;

	if (ephy_window_get_active_tab (window) != tab) return;

	manager = GTK_UI_MANAGER (window->ui_merge);

	action = gtk_ui_manager_get_action (manager, "/menubar/ViewMenu/ViewServerCertificateItem");
	g_return_if_fail (action != NULL);

	is_secure = ephy_tab_get_security_level (tab) > STATE_IS_INSECURE;
	g_object_set (action, "sensitive", is_secure, NULL);
}

static void
tab_added_cb (GtkWidget *notebook,
	      EphyEmbed *embed,
	      EphyWindow *window)
{
	EphyTab *tab;

	g_return_if_fail (EPHY_IS_EMBED (embed));
	mozilla_embed_certificate_attach (embed);

	tab = ephy_tab_for_embed (embed);
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_connect_after (tab, "notify::security-level",
				G_CALLBACK (sync_security_status), window);
}

static void
tab_removed_cb (GtkWidget *notebook,
		EphyEmbed *embed,
		EphyWindow *window)
{
	EphyTab *tab;

	tab = ephy_tab_for_embed (embed);
	g_return_if_fail (EPHY_IS_TAB (tab));

	g_signal_handlers_disconnect_by_func
		(tab, G_CALLBACK (sync_security_status), window);
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyWindow *window)
{
	EphyTab *tab;

	tab = ephy_window_get_active_tab (window);
	sync_security_status (tab, NULL, window);
}


static void
padlock_button_press_cb (GtkWidget *ebox,
			 GdkEventButton *event,
			 EphyWindow *window)
{
	EphyEmbed *embed;

	if (event->type == GDK_BUTTON_PRESS && event->button == 1 /* left */)
	{
		embed = ephy_window_get_active_embed (window);
		g_return_if_fail (EPHY_IS_EMBED (embed));
	
		mozilla_embed_view_certificate (embed);
	}
}

static GtkActionEntry action_entries [] =
{
	{ "ViewServerCertificate",
	  NULL /* stock icon */,
	  N_("Server _Certificate"),
	  NULL, /* shortcut key */
	  N_("Display the web server's certificate"),
	  G_CALLBACK (ephy_certificates_extension_view_certificate_cb) }
};
static const guint n_action_entries = G_N_ELEMENTS (action_entries);

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	EphyCertificatesExtension *extension = EPHY_CERTIFICATES_EXTENSION (ext);
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
	GtkWidget *notebook;
	GtkWidget *statusbar, *frame, *ebox;
	GList *children, *l;

	LOG ("EphyCertificatesExtension attach_window")

	/* catch tab added/removed/switched */
	notebook = ephy_window_get_notebook (window);
	g_signal_connect_after (notebook, "tab_added",
				G_CALLBACK (tab_added_cb), window);
	g_signal_connect_after (notebook, "tab_removed",
				G_CALLBACK (tab_added_cb), window);
	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), window);

	/* make padlock icon clickable */
	statusbar = ephy_window_get_statusbar (window);

	/* FIXME: find a better method to get the security icon frame :) */
	children = gtk_container_get_children (GTK_CONTAINER (statusbar));
	for (l = children; l != NULL; l = l->next)
	{
		if (GTK_IS_FRAME (l->data) && GTK_IS_EVENT_BOX (GTK_BIN (l->data)->child)) break;
	}
	g_return_if_fail (l != NULL);

	ebox = GTK_BIN (l->data)->child;
	g_list_free (children);

	ebox = GTK_WIDGET (l->data);

	gtk_widget_add_events (ebox, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (ebox, "button-press-event",
			  G_CALLBACK (padlock_button_press_cb), window);

	/* add UI */
	manager = GTK_UI_MANAGER (window->ui_merge);

	action_group = gtk_action_group_new ("CertificatesExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group, action_entries,
				      n_action_entries, window);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);
	g_object_unref (action_group);

	ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ViewMenu",
			       "ViewSCSep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ViewMenu",
			       "ViewServerCertificateItem",
			       "ViewServerCertificate",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	LOG ("EphyCertificatesExtension detach_window")
}

static void
ephy_certificates_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
}

static void
ephy_certificates_extension_class_init (EphyCertificatesExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_certificates_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyCertificatesExtensionPrivate));
}
