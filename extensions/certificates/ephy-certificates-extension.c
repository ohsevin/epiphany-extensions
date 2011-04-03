/*
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2003 Christian Persch
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

#include "ephy-certificates-extension.h"

#include "ephy-debug.h"

#include <epiphany/epiphany.h>

#include <gtk/gtk.h>
#include <libpeas/peas.h>
#include <glib/gi18n-lib.h>

#define EPHY_CERTIFICATES_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_CERTIFICATES_EXTENSION, EphyCertificatesExtensionPrivate))

struct EphyCertificatesExtensionPrivate
{
	GtkWidget *cert_manager;
	GtkWidget *device_manager;
};

typedef struct
{
	GtkActionGroup *action_group;
	guint ui_id;
} WindowData;

#define CERT_MANAGER_URL	"chrome://pippki/content/certManager.xul"
#define DEVICE_MANAGER_URL	"chrome://pippki/content/device_manager.xul"
#define WINDOW_DATA_KEY "EphyCertificatesExtensionWindowData"

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
	const GTypeInfo our_info =
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

	const GInterfaceInfo extension_info =
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

	LOG ("EphyCertificatesExtension initialising");
}

static void
ephy_certificates_extension_finalize (GObject *object)
{
	EphyCertificatesExtension *extension = EPHY_CERTIFICATES_EXTENSION (object);
	EphyCertificatesExtensionPrivate *priv = extension->priv;

	LOG ("EphyCertificatesExtension finalising");

	if (priv->cert_manager)
	{
		GtkWidget **cert_manager = &priv->cert_manager;

		g_object_remove_weak_pointer (G_OBJECT (priv->cert_manager),
					      (gpointer *) cert_manager);
		gtk_widget_destroy (priv->cert_manager);
		priv->cert_manager = NULL;
	}

	if (priv->device_manager)
	{
		GtkWidget **device_manager = &priv->device_manager;

		g_object_remove_weak_pointer (G_OBJECT (priv->device_manager),
					      (gpointer *) device_manager);
		gtk_widget_destroy (priv->device_manager);
		priv->device_manager = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
manage_certificates_cb (GtkAction *action,
			EphyCertificatesExtension *extension)
{
	EphyCertificatesExtensionPrivate *priv = extension->priv;
	EphyEmbedSingle *single;
	GtkWidget *manager, *window, **widget;

	if (priv->cert_manager != NULL)
	{
		gtk_window_present (GTK_WINDOW (priv->cert_manager));
		return;
	}  

	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (embed_shell));
	manager = ephy_embed_single_open_window (single, NULL, CERT_MANAGER_URL,
						 "", "all,chrome,dialog");
	g_return_if_fail (manager != NULL);

	window = gtk_widget_get_toplevel (manager);
	g_return_if_fail (gtk_widget_is_toplevel (window));

	gtk_window_set_role (GTK_WINDOW (window), "epiphany-certificate-manager");
	gtk_window_set_title (GTK_WINDOW (window), _("Certificates"));

	priv->cert_manager = window;
	widget = &priv->cert_manager;
	g_object_add_weak_pointer (G_OBJECT (priv->cert_manager),
				   (gpointer *) widget);
}

static void
manage_devices_cb (GtkAction *action,
		   EphyCertificatesExtension *extension)
{
	EphyCertificatesExtensionPrivate *priv = extension->priv;
	EphyEmbedSingle *single;
	GtkWidget *manager, *window, **widget;

	if (priv->device_manager != NULL)
	{
		gtk_window_present (GTK_WINDOW (priv->device_manager));
		return;
	}  

	single = EPHY_EMBED_SINGLE (ephy_embed_shell_get_embed_single (embed_shell));
	manager = ephy_embed_single_open_window (single, NULL, DEVICE_MANAGER_URL,
						 "", "all,chrome,dialog");
	g_return_if_fail (manager != NULL);

	window = gtk_widget_get_toplevel (manager);
	g_return_if_fail (gtk_widget_is_toplevel (window));

	gtk_window_set_role (GTK_WINDOW (window), "epiphany-security-device-manager");
	gtk_window_set_title (GTK_WINDOW (window), _("Security Devices"));

	priv->device_manager = window;
	widget = &priv->device_manager;
	g_object_add_weak_pointer (G_OBJECT (priv->device_manager),
				   (gpointer *) widget);
}

static const GtkActionEntry action_entries_1 [] =
{
	{ "ToolsCertificateManager",
	  NULL /* stock icon */,
	  N_("Manage _Certificates"),
	  NULL, /* shortcut key */
	  N_("Manage your certificates"),
	  G_CALLBACK (manage_certificates_cb) },
	{ "ToolsSecurityDevicesManager",
	  NULL /* stock icon */,
	  N_("Manage Security _Devices"),
	  NULL, /* shortcut key */
	  N_("Manage your security devices"),
	  G_CALLBACK (manage_devices_cb) }
};

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	GtkActionGroup *action_group;
	guint ui_id;
	WindowData *win_data;

	LOG ("EphyCertificatesExtension attach_window");

	/* add UI */
	win_data = g_new (WindowData, 1);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	win_data->action_group = action_group = gtk_action_group_new
		("CertificatesExtensionActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group, action_entries_1,
				      G_N_ELEMENTS (action_entries_1), ext);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);
	g_object_unref (win_data->action_group);

	win_data->ui_id = ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ToolsSCSep1", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ToolsCertificateManagerItem",
			       "ToolsCertificateManager",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ToolsSecurityDevicesManagerItem",
			       "ToolsSecurityDevicesManager",
			       GTK_UI_MANAGER_MENUITEM, FALSE);
	gtk_ui_manager_add_ui (manager, ui_id, "/menubar/ToolsMenu",
			       "ToolsSCSep2", NULL,
			       GTK_UI_MANAGER_SEPARATOR, FALSE);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY, win_data,
				(GDestroyNotify) g_free);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkUIManager *manager;
	WindowData *win_data;

	LOG ("EphyCertificatesExtension detach_window");

	/* remove UI */
	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	win_data = (WindowData *) g_object_get_data (G_OBJECT (window),
						     WINDOW_DATA_KEY);
	g_return_if_fail (win_data != NULL);

	gtk_ui_manager_remove_ui (manager, win_data->ui_id);
	gtk_ui_manager_remove_action_group (manager, win_data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
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

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	ephy_certificates_extension_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    EPHY_TYPE_EXTENSION,
						    EPHY_TYPE_CERTIFICATES_EXTENSION);
}
