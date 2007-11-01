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

#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-statusbar.h>
#include <epiphany/ephy-window.h>
#include <epiphany/ephy-adblock.h>
#include <epiphany/ephy-adblock-manager.h>
#include <epiphany/ephy-embed-shell.h>
#include <epiphany/ephy-extension.h>

#include "ephy-adblock-extension.h"
#include "ephy-debug.h"
#include "ephy-file-helpers.h"

#include "ad-blocker.h"
#include "ad-uri-tester.h"
#include "adblock-ui.h"

#include <gtk/gtkaction.h>
#include <gtk/gtkactiongroup.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkimage.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkstock.h>

#include <gmodule.h>

#include <glib/gi18n-lib.h>

#define EPHY_ADBLOCK_EXTENSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_ADBLOCK_EXTENSION, EphyAdblockExtensionPrivate))

#define WINDOW_DATA_KEY		"EphyAdblockExtensionWindowData"
#define STATUSBAR_EVBOX_KEY	"EphyAdblockExtensionStatusbarEvbox"
#define EXTENSION_KEY		"EphyAdblockExtension"
#define ICON_FILENAME		"adblock-statusbar-icon.svg"

typedef struct
{
	EphyAdblockExtension	*extension;
	EphyWindow		*window;

	GtkActionGroup		*action_group;
	guint			ui_id;
} WindowData;

static void ephy_adblock_extension_edit_cb (GtkAction *action,
					    EphyWindow *window);

struct EphyAdblockExtensionPrivate
{
	AdUriTester *tester;
	AdblockUI   *ui;
};

static void ephy_adblock_extension_class_init	(EphyAdblockExtensionClass *klass);
static void ephy_adblock_extension_iface_init	(EphyExtensionIface *iface);
static void ephy_adblock_adblock_iface_init	(EphyAdBlockIface *iface);
static void ephy_adblock_extension_init		(EphyAdblockExtension *extension);
static AdBlocker * ensure_adblocker    		(EphyAdblockExtension *extension, EphyEmbed *embed);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType
ephy_adblock_extension_get_type (void)
{
	return type;
}

GType
ephy_adblock_extension_register_type (GTypeModule *module)
{
	const GTypeInfo our_info =
	{
		sizeof (EphyAdblockExtensionClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_adblock_extension_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyAdblockExtension),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_adblock_extension_init
	};

	const GInterfaceInfo adblock_info =
	{
		(GInterfaceInitFunc) ephy_adblock_adblock_iface_init,
		NULL,
		NULL
	};

	const GInterfaceInfo extension_info =
	{
		(GInterfaceInitFunc) ephy_adblock_extension_iface_init,
		NULL,
		NULL
	};

	type = g_type_module_register_type (module,
					    G_TYPE_OBJECT,
					    "EphyAdblockExtension",
					    &our_info, 0);

        g_type_module_add_interface (module,
                                     type,
                                     EPHY_TYPE_ADBLOCK,
                                     &adblock_info);

	g_type_module_add_interface (module,
				     type,
				     EPHY_TYPE_EXTENSION,
				     &extension_info);

	return type;
}

static void
ephy_adblock_extension_init (EphyAdblockExtension *extension)
{
	EphyAdBlockManager *manager;
	char *dirname;

	LOG ("EphyAdblockExtension initialising");

	extension->priv = EPHY_ADBLOCK_EXTENSION_GET_PRIVATE (extension);

	extension->priv->tester = ad_uri_tester_new ();

	/* ensure adblock's dir is there */
	dirname =  g_build_filename (ephy_dot_dir (), "extensions", "data", "adblock", NULL);
	g_mkdir_with_parents (dirname, 0775);
	g_free (dirname);

	/* register our extention as a blocker */
	manager = EPHY_ADBLOCK_MANAGER (
			ephy_embed_shell_get_adblock_manager (embed_shell));
	ephy_adblock_manager_set_blocker (manager, EPHY_ADBLOCK (extension));
}

static void
ephy_adblock_extension_finalize (GObject *object)
{
	EphyAdBlockManager *manager;
	EphyAdblockExtension *extension = EPHY_ADBLOCK_EXTENSION (object);

	LOG ("EphyAdblockExtension finalising");

	/* unregister our extension as a blocker */
	manager = EPHY_ADBLOCK_MANAGER (ephy_embed_shell_get_adblock_manager (embed_shell));
	ephy_adblock_manager_set_blocker (manager, NULL);
	
	if (extension->priv->ui != NULL)
	{
		g_object_unref (extension->priv->ui);
	}
	g_object_unref (extension->priv->tester);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
ephy_adblock_impl_should_load (EphyAdBlock *blocker, 
			       EphyEmbed *embed,
			       const char *url, 
			       AdUriCheckType type)
{
	EphyAdblockExtension *self;
	AdBlocker *adblocker;

	LOG ("ephy_adblock_impl_should_load checking %s", url);

	self = EPHY_ADBLOCK_EXTENSION (blocker);
	g_return_val_if_fail (self != NULL, TRUE);

	adblocker = ensure_adblocker (self, embed);

	if (ad_blocker_should_block (adblocker))
		return !ad_uri_tester_test_uri (self->priv->tester, url, type);
	else
		return TRUE;
}

static void
ephy_adblock_impl_edit_rule (EphyAdBlock *blocker, 
			     const char *url, 
			     gboolean allowed)
{
	EphyAdblockExtension *self;
	EphyAdblockExtensionPrivate *priv;

	LOG ("ephy_adblock_impl_edit_rule %s with state %d", url, allowed);

	self = EPHY_ADBLOCK_EXTENSION (blocker);
	priv = self->priv;

	if (priv->ui == NULL)
	{
		AdblockUI **ui;
 
		priv->ui =  g_object_new (TYPE_ADBLOCK_UI, 
					  "tester"     , priv->tester, 
					  "url"        , url, 
					  "url_allowed", allowed,
					  NULL);
		ui = &priv->ui;

		g_object_add_weak_pointer ((gpointer)priv->ui,
				           (gpointer *) ui);
	
		ephy_dialog_set_parent (EPHY_DIALOG (priv->ui), NULL);
	}

	ephy_dialog_show (EPHY_DIALOG (priv->ui));
}


static const GtkActionEntry edit_entries[] = {
	{ "EphyAdblockExtensionEdit", NULL, N_("Adblock Editor"), NULL,
	  N_("Edit Adblock"),
	  G_CALLBACK (ephy_adblock_extension_edit_cb) }
};

static gboolean
ephy_adblock_statusbar_icon_clicked_cb (GtkWidget *widget,
				        GdkEventButton *event,
				        EphyWindow *window)
{
	if (event->button == 1)
	{
		EphyAdblockExtension *ext;
		AdBlocker *blocker;
		EphyEmbed *embed = ephy_window_get_active_embed (window);

		ext = EPHY_ADBLOCK_EXTENSION (g_object_get_data (G_OBJECT (window),
								 EXTENSION_KEY));

		blocker = ensure_adblocker (ext, embed);
		g_return_val_if_fail (blocker != NULL, FALSE);

		ad_blocker_set_noblock (blocker);

		ephy_embed_reload (embed, FALSE);

		return TRUE;
	}

	return FALSE;
}

static void
update_statusbar (EphyWindow *window)
{
	AdBlocker *blocker;
	EphyEmbed *embed;
	GObject *statusbar;
	GtkWidget *evbox;
	int num_blocked;
	EphyAdblockExtension *ext;

	embed = ephy_window_get_active_embed (window);
	g_return_if_fail (embed != NULL);

	ext = EPHY_ADBLOCK_EXTENSION (g_object_get_data (G_OBJECT (window),
							 EXTENSION_KEY));
	g_return_if_fail (ext != NULL);

	blocker = ensure_adblocker (ext, embed);
	g_return_if_fail (blocker != NULL);

	statusbar = G_OBJECT (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	evbox = g_object_get_data (statusbar, STATUSBAR_EVBOX_KEY);
	g_return_if_fail (evbox != NULL);

	g_object_get (G_OBJECT (blocker), "num-blocked", &num_blocked, NULL);

	if (num_blocked == 0)
	{
		gtk_widget_hide (evbox);
	}
	else
	{
		char *tooltip;

		tooltip = g_strdup_printf (ngettext ("%d hidden advertisement",
						     "%d hidden advertisements",
						     num_blocked),
					   num_blocked);

		gtk_widget_set_tooltip_text (evbox, tooltip);
		g_free (tooltip);

		gtk_widget_show (evbox);
	}
}

static void
create_statusbar_icon (EphyWindow *window)
{
	EphyStatusbar *statusbar;
	char *filename;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;
	GtkWidget *evbox;
	int w, h;

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);

	/* FIXME themeing! */
	filename = g_build_filename (SHARE_DIR, ICON_FILENAME, NULL);
	pixbuf = gdk_pixbuf_new_from_file_at_size (filename, w, h, NULL);
	g_free (filename);

	/* Try not to crash when librsvg isn't installed! */
	if (pixbuf)
	{
		icon = gtk_image_new_from_pixbuf (pixbuf);
		g_object_unref (pixbuf);
	}
	else
	{
		icon = gtk_image_new_from_stock (GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_MENU);
	}

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	evbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (evbox), FALSE);

	gtk_container_add (GTK_CONTAINER (evbox), icon);

	gtk_widget_show (icon);
	/* don't show the evbox */

	ephy_statusbar_add_widget (statusbar, evbox);

	g_object_set_data (G_OBJECT (statusbar), STATUSBAR_EVBOX_KEY, evbox);

	g_signal_connect_after (evbox, "button-press-event",
				G_CALLBACK (ephy_adblock_statusbar_icon_clicked_cb),
				window);
}

static void
destroy_statusbar_icon (EphyWindow *window)
{
	EphyStatusbar *statusbar;
	GtkWidget *evbox;

	statusbar = EPHY_STATUSBAR (ephy_window_get_statusbar (window));
	g_return_if_fail (statusbar != NULL);

	evbox = g_object_steal_data (G_OBJECT (statusbar), STATUSBAR_EVBOX_KEY);
	g_return_if_fail (evbox != NULL);

	ephy_statusbar_remove_widget (statusbar, evbox);
}

static void
ephy_adblock_extension_edit_cb (GtkAction *action, EphyWindow *window)
{
	WindowData *data;
	EphyAdblockExtensionPrivate *priv;
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	priv = data->extension->priv;

	if (priv->ui == NULL)
	{
		AdblockUI **ui;

		priv->ui =  g_object_new (TYPE_ADBLOCK_UI, 
					  "tester", priv->tester, 
					  NULL);
		ui = &priv->ui;

		g_object_add_weak_pointer ((gpointer)priv->ui,
				           (gpointer *) ui);
	}

	ephy_dialog_set_parent (EPHY_DIALOG (priv->ui), GTK_WIDGET (window));
	ephy_dialog_show (EPHY_DIALOG (priv->ui));
}

static void
switch_page_cb (GtkNotebook *notebook,
		GtkNotebookPage *page,
		guint page_num,
		EphyWindow *window)
{
	/* FIXME: this shouldn't happen anymore with gtk 2.10 ! Test & remove */
	if (GTK_WIDGET_REALIZED (window) == FALSE) return; /* on startup */

	update_statusbar (window);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;
	WindowData *data;
	GtkUIManager *manager;

	/* Add adblock editor's menu entry */
	data = g_new (WindowData, 1);
	g_object_set_data_full (G_OBJECT (window), 
			 	WINDOW_DATA_KEY, 
				data,
				g_free);

	data->extension = EPHY_ADBLOCK_EXTENSION (ext);
	data->window = window;

	data->action_group = gtk_action_group_new ("EphyAdblockExtension");
	gtk_action_group_set_translation_domain (data->action_group,
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (data->action_group, edit_entries,
				      G_N_ELEMENTS(edit_entries), window);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);
	/* ui manager refs the new action group */
	g_object_unref (data->action_group);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager,
			       data->ui_id,
			       "/menubar/ToolsMenu",
			       "EphyAdblockExtensionEdit",
			       "EphyAdblockExtensionEdit",
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);
	
	/* Remember the xtension attached to that window */
	g_object_set_data (G_OBJECT (window), EXTENSION_KEY, ext);

	/* The ad blocked icon */
	create_statusbar_icon (window);

	notebook = ephy_window_get_notebook (window);

	g_signal_connect_after (notebook, "switch_page",
				G_CALLBACK (switch_page_cb), window);
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
	GtkWidget *notebook;
	WindowData *data;
	GtkUIManager *manager;

	/* Remove editor ui */
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_assert (data != NULL);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);

	/* Remove icon stuff */
	notebook = ephy_window_get_notebook (window);

	g_signal_handlers_disconnect_by_func
		(notebook, G_CALLBACK (switch_page_cb), window);

	destroy_statusbar_icon (window);
}

static void
content_blocked_cb (EphyEmbed *embed,
		    const char *address,
		    AdBlocker *blocker)
{
	LOG ("EphyAdblockExtension content blocked %s", address);

	ad_blocker_blocked_uri (blocker);
}

static void
location_changed_cb (EphyEmbed *embed,
		     const char *address,
		     AdBlocker *blocker)
{
	ad_blocker_reset (blocker);
}

static void
num_blocked_cb (AdBlocker *blocker,
		GParamSpec *pspec,
		EphyEmbed *embed)
{
	EphyTab *tab;
	EphyWindow *window;

	tab = ephy_tab_for_embed (embed);
	g_return_if_fail (tab != NULL);

	window = EPHY_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab)));
	g_return_if_fail (window != NULL);

	if (embed == ephy_window_get_active_embed (window))
	{
		update_statusbar (window);
	}
}

static AdBlocker *
ensure_adblocker (EphyAdblockExtension *ext,
		  EphyEmbed *embed)
{
	AdBlocker *blocker;

	blocker = g_object_get_data (G_OBJECT (embed), AD_BLOCKER_KEY);
	
	if (blocker == NULL)
	{
		g_return_val_if_fail (ext != NULL, NULL);

		blocker = ad_blocker_new ();
		g_return_val_if_fail (blocker != NULL, NULL);

		g_object_set_data_full (G_OBJECT (embed), AD_BLOCKER_KEY,
					blocker, (GDestroyNotify) g_object_unref);

		g_signal_connect (embed, "ge-location",
				  G_CALLBACK (location_changed_cb), blocker);

		g_signal_connect (embed, "content-blocked",
				  G_CALLBACK (content_blocked_cb), blocker);
	}

	return blocker;
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	EphyEmbed *embed;
	AdBlocker *blocker;
	
	embed = ephy_tab_get_embed (tab);

	blocker = ensure_adblocker (EPHY_ADBLOCK_EXTENSION (ext), embed);
	g_return_if_fail (blocker != NULL);

	g_signal_connect (blocker, "notify::num-blocked",
			  G_CALLBACK (num_blocked_cb), embed);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyTab *tab)
{
	AdBlocker *blocker;
	EphyEmbed *embed;
	
	embed = ephy_tab_get_embed (tab);

	blocker = g_object_steal_data (G_OBJECT (embed), AD_BLOCKER_KEY);
	g_return_if_fail (blocker != NULL);

	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (content_blocked_cb), blocker);
	g_signal_handlers_disconnect_by_func
		(embed, G_CALLBACK (location_changed_cb), blocker);
	g_signal_handlers_disconnect_by_func
		(blocker, G_CALLBACK (num_blocked_cb), embed);

	g_object_unref (blocker);
}

static void
ephy_adblock_adblock_iface_init (EphyAdBlockIface *iface)
{
	iface->should_load = ephy_adblock_impl_should_load;
	iface->edit_rule   = ephy_adblock_impl_edit_rule;
}

static void
ephy_adblock_extension_iface_init (EphyExtensionIface *iface)
{
	iface->attach_window = impl_attach_window;
	iface->detach_window = impl_detach_window;
	iface->attach_tab = impl_attach_tab;
	iface->detach_tab = impl_detach_tab;
}

static void
ephy_adblock_extension_class_init (EphyAdblockExtensionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = ephy_adblock_extension_finalize;

	g_type_class_add_private (object_class, sizeof (EphyAdblockExtensionPrivate));
}
