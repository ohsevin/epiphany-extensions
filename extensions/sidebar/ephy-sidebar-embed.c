/*
 *  Copyright (C) 2004 Crispin Flowerday
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
 * $Id$
 */

#include "config.h"

#include "ephy-sidebar-embed.h"
#include "sidebar-commands.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-shell.h>
#include <epiphany/ephy-tab.h>
#include <epiphany/ephy-embed-event.h>
#include <epiphany/ephy-embed-persist.h>
#include <epiphany/ephy-embed-factory.h>
#include <epiphany/ephy-command-manager.h>

#include "ephy-prefs.h"
#include "ephy-debug.h"

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkstock.h>

/* NOTE: we include gi18n.h instead of gi18n-lib.h since all string here
 * are the same as in epiphany. If that ever changes, fix this include!
 * #include <glib/gi18n-lib.h>
 */
#include <glib/gi18n.h>

#include <string.h>

#define EPHY_SIDEBAR_EMBED_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), EPHY_TYPE_SIDEBAR_EMBED, EphySidebarEmbedPrivate))

struct _EphySidebarEmbedPrivate
{
	EphyWindow *window;
	GtkActionGroup *action_group;
	guint ui_id;
	char * url;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static void ephy_sidebar_embed_class_init   (EphySidebarEmbedClass *klass);
static void ephy_sidebar_embed_init         (EphySidebarEmbed *sbembed);
static void ephy_sidebar_embed_create_embed (EphySidebarEmbed *sbembed);

static GObjectClass *parent_class = NULL;

static GType type = 0;

GType 
ephy_sidebar_embed_get_type (void)
{
	return type;
}

GType
ephy_sidebar_embed_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphySidebarEmbedClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_sidebar_embed_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphySidebarEmbed),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_sidebar_embed_init
	};

	type = g_type_module_register_type (module,
					    GTK_TYPE_BIN,
					    "EphySidebarEmbed",
					    &our_info, 0);

	return type;
}

EphyWindow *
ephy_sidebar_embed_get_window (EphySidebarEmbed *sbembed)
{
	return sbembed->priv->window;
}

EphyEmbed*
ephy_sidebar_embed_get_embed (EphySidebarEmbed *sbembed)
{
	GtkWidget *widget;

	widget = GTK_BIN (sbembed)->child;

	return widget ? EPHY_EMBED (widget) : NULL;
}

void
ephy_sidebar_embed_set_url (EphySidebarEmbed *sbembed,
			    const char * url)
{
	g_free (sbembed->priv->url);
	sbembed->priv->url = g_strdup (url);

	if (GTK_BIN (sbembed)->child != NULL)
	{
		gtk_widget_destroy (GTK_BIN (sbembed)->child);

		ephy_sidebar_embed_create_embed (sbembed);
	}
}

static void
popup_menu_at_coords (GtkMenu *menu,
		      gint *x,
		      gint *y,
		      gboolean *push_in,
		      gpointer user_data)
{
	EphyEmbedEvent *event = (EphyEmbedEvent *) user_data;

	ephy_embed_event_get_coords (event, x, y);

	*push_in = TRUE;
}

static void
hide_embed_popup_cb (GtkMenu *menu,
		     GtkUIManager *manager)
{
	GtkAction *action;

	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditCopyIP");
	g_object_set (action, "sensitive", TRUE, "visible", TRUE, NULL);
	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditCutIP");
	g_object_set (action, "sensitive", TRUE, "visible", TRUE, NULL);
	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditPasteIP");
	g_object_set (action, "sensitive", TRUE, "visible", TRUE, NULL);
}

static void
show_context_menu (EphySidebarEmbed *sbembed,
		   EphyEmbed *embed,
		   EphyEmbedEvent *event)
{
	GtkUIManager *manager;
	GtkAction *action;
	EphyEmbedEventContext context;
	const char *popup;
	const GValue *value;
	gboolean framed, has_background, can_open_in_new;
	GtkWidget *widget;
	EphyWindow *window = sbembed->priv->window;
	gboolean hide_edit_actions = TRUE;
	gboolean can_copy, can_cut, can_paste;
	guint button;

	ephy_embed_event_get_property (event, "framed_page", &value);
	framed = g_value_get_int (value);

	has_background = ephy_embed_event_has_property (event, "background_image");
	can_open_in_new = ephy_embed_event_has_property (event, "link-has-web-scheme");

	context = ephy_embed_event_get_context (event);

	LOG ("show_embed_popup context %x", context);

	if ((context & EPHY_EMBED_CONTEXT_EMAIL_LINK) &&
	    (context & EPHY_EMBED_CONTEXT_IMAGE))
	{
		popup = "/EphyImageEmailLinkPopup";
	}
	else if (context & EPHY_EMBED_CONTEXT_EMAIL_LINK)
	{
		popup = "/EphyEmailLinkPopup";
	}
	else if ((context & EPHY_EMBED_CONTEXT_LINK) &&
		 (context & EPHY_EMBED_CONTEXT_IMAGE))
	{
		popup = "/EphySidebarImageLinkPopup";
	}
	else if (context & EPHY_EMBED_CONTEXT_LINK)
	{
		popup = "/EphySidebarLinkPopup";
	}
	else if (context & EPHY_EMBED_CONTEXT_IMAGE)
	{
		popup = "/EphySidebarImagePopup";
	}
	else if (context & EPHY_EMBED_CONTEXT_INPUT)
	{
		popup = "/EphyInputPopup";
		hide_edit_actions = FALSE;
	}
	else
	{
		popup = framed ? "/EphySidebarFramedDocumentPopup" :
				 "/EphySidebarDocumentPopup";
	}

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	action = gtk_ui_manager_get_action (manager, "/EphySidebarDocumentPopup/SaveBackgroundAsDP");
	g_object_set (action, "sensitive", has_background,
			      "visible", has_background, NULL);
	action = gtk_ui_manager_get_action (manager, "/EphyLinkPopup/OpenLinkInNewWindowLP");
	g_object_set (action, "sensitive", can_open_in_new, FALSE);
	action = gtk_ui_manager_get_action (manager, "/EphyLinkPopup/OpenLinkInNewTabLP");
	g_object_set (action, "sensitive", can_open_in_new, FALSE);

	can_copy = ephy_command_manager_can_do_command (EPHY_COMMAND_MANAGER (embed), "cmd_copy");
	can_cut = ephy_command_manager_can_do_command (EPHY_COMMAND_MANAGER (embed), "cmd_cut");
	can_paste = ephy_command_manager_can_do_command (EPHY_COMMAND_MANAGER (embed), "cmd_paste");
	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditCopyIP");
	g_object_set (action, "sensitive", can_copy, "visible", !hide_edit_actions || can_copy, NULL);
	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditCutIP");
	g_object_set (action, "sensitive", can_cut, "visible", !hide_edit_actions || can_cut, NULL);
	action = gtk_ui_manager_get_action (manager, "/EphyInputPopup/EditPasteIP");
	g_object_set (action, "sensitive", can_paste, "visible", !hide_edit_actions || can_paste, NULL);

	g_object_set_data_full (G_OBJECT (window), "context_event",
				g_object_ref (event),
				(GDestroyNotify)g_object_unref);

	widget = gtk_ui_manager_get_widget (manager, popup);
	g_return_if_fail (widget != NULL);

	g_signal_connect (widget, "hide",
			  G_CALLBACK (hide_embed_popup_cb), manager);

	button = ephy_embed_event_get_button (event);
	if (button == 0)
	{
		gtk_menu_popup (GTK_MENU (widget), NULL, NULL,
				popup_menu_at_coords, event, 2,
				gtk_get_current_event_time ());
		gtk_menu_shell_select_first (GTK_MENU_SHELL (widget), FALSE);
	}
	else
	{
		gtk_menu_popup (GTK_MENU (widget), NULL, NULL,
				NULL, NULL, button,
				gtk_get_current_event_time ());
	}
}

static gboolean
embed_contextmenu_cb (EphyEmbed *embed,
		      EphyEmbedEvent *event,
		      EphySidebarEmbed *sbembed)
{
	show_context_menu (sbembed, embed, event);

	return TRUE;
}

static void
save_property_url (EphyEmbed *embed,
		   EphyEmbedEvent *event,
		   const char *property,
		   const char *key)
{
	const char *location;
	const GValue *value;
	EphyEmbedPersist *persist;

	ephy_embed_event_get_property (event, property, &value);
	location = g_value_get_string (value);

	persist = EPHY_EMBED_PERSIST
		(ephy_embed_factory_new_object (EPHY_TYPE_EMBED_PERSIST));

	ephy_embed_persist_set_embed (persist, embed);
	ephy_embed_persist_set_flags (persist, 0);
	ephy_embed_persist_set_persist_key (persist, key);
	ephy_embed_persist_set_source (persist, location);

	ephy_embed_persist_save (persist);

	g_object_unref (persist);
}

static gboolean
embed_mouse_click_cb (EphyEmbed *embed,
		      EphyEmbedEvent *event,
		      EphySidebarEmbed *sbembed)
{
	EphyEmbedEventContext context;
	guint button, modifier;
	gboolean handled = TRUE;
	gboolean with_control, with_shift, is_left_click, is_middle_click;
	gboolean is_link, is_image, is_middle_clickable;
	gboolean is_input;
	const GValue *targetValue;

	g_return_val_if_fail (EPHY_IS_EMBED_EVENT(event), FALSE);

	button = ephy_embed_event_get_button (event);
	context = ephy_embed_event_get_context (event);
	modifier = ephy_embed_event_get_modifier (event);

	LOG ("ephy_sidebar_mouse_click_cb: type %d, context %x, modifier %x",
	     button, context, modifier);

	with_control = (modifier & GDK_CONTROL_MASK) != 0;
	with_shift = (modifier & GDK_SHIFT_MASK) != 0;
	is_left_click = (button == 1);
	is_middle_click = (button == 2);

	is_link = (context & EPHY_EMBED_CONTEXT_LINK) != 0;
	is_image = (context & EPHY_EMBED_CONTEXT_IMAGE) != 0;
	is_middle_clickable = !((context & EPHY_EMBED_CONTEXT_LINK)
				|| (context & EPHY_EMBED_CONTEXT_INPUT)
				|| (context & EPHY_EMBED_CONTEXT_EMAIL_LINK));
	is_input = (context & EPHY_EMBED_CONTEXT_INPUT) != 0;

	ephy_embed_event_get_property (event, "link_target", &targetValue);

	/* ctrl+click or middle click opens the link in new tab */
	if (is_link && ((is_left_click && with_control) || is_middle_click))
	{
		const GValue *value;
		const char *link_address;

		ephy_embed_event_get_property (event, "link", &value);
		link_address = g_value_get_string (value);

		ephy_shell_new_tab (ephy_shell, sbembed->priv->window, NULL,
				    link_address,
				    EPHY_NEW_TAB_OPEN_PAGE |
				    EPHY_NEW_TAB_IN_EXISTING_WINDOW);

	}
	/* shift+click saves the link target */
	else if (is_link && is_left_click && with_shift)
	{
		save_property_url (embed, event, "link", CONF_STATE_SAVE_DIR);
	}
	/* Left+click on a _content target opens in current tab */
	else if (is_left_click && is_link &&
		 !strcmp(g_value_get_string(targetValue), "_content"))

	{
		const GValue *value;
		const char *link_address;

		ephy_embed_event_get_property (event, "link", &value);
		link_address = g_value_get_string (value);
		ephy_window_load_url (sbembed->priv->window, link_address);
	}
	/* shift+click saves the non-link image
	 * Note: pressing enter to submit a form synthesizes a mouse click event
	 */
	else if (is_image && is_left_click && with_shift && !is_input)
{
		save_property_url (embed, event, "image", CONF_STATE_SAVE_IMAGE_DIR);
	}
	/* we didn't handle the event */
	else
	{
		handled = FALSE;
	}

	return handled;
}

static void 
embed_new_window_cb (EphyEmbed *embed,
		     EphyEmbed **new_embed,
		     EphyEmbedChrome chromemask,
		     gpointer data)
{
	EphyWindow *window;
	EphyTab *new_tab;

	window = ephy_window_new_with_chrome (chromemask);

	new_tab = ephy_tab_new ();
	gtk_widget_show (GTK_WIDGET (new_tab));

	ephy_window_add_tab (window, new_tab, -1, FALSE);

	*new_embed = ephy_tab_get_embed (new_tab);
}

static void
ephy_sidebar_embed_create_embed (EphySidebarEmbed *sbembed)
{
	EphyEmbed *embed;

	embed = EPHY_EMBED(ephy_embed_factory_new_object (EPHY_TYPE_EMBED));
	
	gtk_container_add (GTK_CONTAINER (sbembed), GTK_WIDGET (embed));
	gtk_widget_show (GTK_WIDGET (embed));

	if (sbembed->priv->url == NULL)
	{
		ephy_embed_load_url (embed, "about:blank");
	}
	else
	{
		ephy_embed_load_url (embed, sbembed->priv->url);

		g_signal_connect (G_OBJECT (embed),
				  "ge_new_window",
				  G_CALLBACK(embed_new_window_cb),
				  NULL);
		g_signal_connect (G_OBJECT (embed),
				  "ge_dom_mouse_click",
				  G_CALLBACK(embed_mouse_click_cb),
				  sbembed);
		g_signal_connect (G_OBJECT (embed),
				  "ge_context_menu",
				  G_CALLBACK(embed_contextmenu_cb),
				  sbembed);
	}
}

#define EPHY_STOCK_DOWNLOAD	"ephy-download"

static GtkActionEntry action_entries [] =
{
	{ "SidebarSaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), NULL,
	  N_("Save the current page"),
	  G_CALLBACK (sidebar_cmd_file_save_as) },
	{ "SidebarSaveBackgroundAs", NULL, N_("_Save Background As..."), NULL,
	  NULL, G_CALLBACK (sidebar_cmd_save_background_as) },
	{ "SidebarOpenFrame", NULL, N_("_Open Frame"), NULL,
	  NULL, G_CALLBACK (sidebar_cmd_open_frame) },
	{ "SidebarDownloadLink", EPHY_STOCK_DOWNLOAD, N_("_Download Link"), NULL,
	  NULL, G_CALLBACK (sidebar_cmd_download_link) },
	{ "SidebarDownloadLinkAs", GTK_STOCK_SAVE_AS, N_("_Save Link As..."), NULL,
	  NULL, G_CALLBACK (sidebar_cmd_download_link_as) },
	{ "SidebarSaveImageAs", GTK_STOCK_SAVE_AS, N_("_Save Image As..."), NULL,
	  NULL, G_CALLBACK (sidebar_cmd_save_image_as) },
};

static void
ephy_sidebar_embed_set_window (EphySidebarEmbed *sbembed,
			       EphyWindow *window)
{
	GtkActionGroup *action_group;
	GtkUIManager *manager;

	g_return_if_fail (EPHY_IS_SIDEBAR_EMBED (sbembed));
	g_return_if_fail (EPHY_IS_WINDOW (window));

	g_return_if_fail (sbembed->priv->window == NULL);

	sbembed->priv->window = window;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));

	action_group = sbembed->priv->action_group =
		gtk_action_group_new ("SidebarContextMenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, action_entries,
				      G_N_ELEMENTS (action_entries), sbembed);

	gtk_ui_manager_insert_action_group (manager, action_group, -1);

	sbembed->priv->ui_id = 
		gtk_ui_manager_add_ui_from_file (manager,
						 SHARE_DIR "/xml/epiphany-sidebar-ui.xml",
						 NULL);
}

static void
ephy_sidebar_embed_size_allocate (GtkWidget *widget,
				  GtkAllocation *allocation)
{
	GtkWidget *child;

	widget->allocation = *allocation;
	child = GTK_BIN (widget)->child;

	if (child && GTK_WIDGET_VISIBLE (child))
	{
		gtk_widget_size_allocate (child, allocation);
	}
}

static void
ephy_sidebar_embed_map (GtkWidget *widget)
{
	/* Delay creating the embed till the sidebar is mapped, as
	 * the GtkMozEmbed widget crashes if we never realize it, by
	 * ensureing we don't have an embed when hidden, no unncessary
	 * reloads happen either */

	if (GTK_BIN (widget)->child == NULL)
	{
		ephy_sidebar_embed_create_embed (EPHY_SIDEBAR_EMBED (widget));
	}

	GTK_WIDGET_CLASS (parent_class)->map (widget);
}

static void
ephy_sidebar_embed_unmap (GtkWidget *widget)
{
	if (GTK_BIN (widget)->child)
	{
		gtk_widget_destroy (GTK_BIN (widget)->child);
	}

	GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
ephy_sidebar_embed_init (EphySidebarEmbed *sbembed)
{
	sbembed->priv = EPHY_SIDEBAR_EMBED_GET_PRIVATE (sbembed);
}

static void
ephy_sidebar_embed_finalize (GObject *object)
{
	EphySidebarEmbed *sbembed = EPHY_SIDEBAR_EMBED (object);
	GtkUIManager *manager;

	g_free (sbembed->priv->url);

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (sbembed->priv->window));

	if (sbembed->priv->ui_id != 0)
	{
		gtk_ui_manager_remove_ui (manager, sbembed->priv->ui_id);
	}

	gtk_ui_manager_remove_action_group (manager, sbembed->priv->action_group);
	g_object_unref (sbembed->priv->action_group);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
ephy_sidebar_embed_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	EphySidebarEmbed *embed = EPHY_SIDEBAR_EMBED (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, embed->priv->window);
			break;
	}
}

static void
ephy_sidebar_embed_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	EphySidebarEmbed *embed = EPHY_SIDEBAR_EMBED (object);
	
	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_sidebar_embed_set_window (embed, g_value_get_object (value));
			break;
	}
}

static void
ephy_sidebar_embed_class_init (EphySidebarEmbedClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	
	parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

	object_class->finalize = ephy_sidebar_embed_finalize;
	object_class->get_property = ephy_sidebar_embed_get_property;
	object_class->set_property = ephy_sidebar_embed_set_property;

	widget_class->size_allocate = ephy_sidebar_embed_size_allocate;
	widget_class->map           = ephy_sidebar_embed_map;
	widget_class->unmap         = ephy_sidebar_embed_unmap;

	g_object_class_install_property
		(object_class,
		 PROP_WINDOW,
		 g_param_spec_object ("window",
				      "window",
				      "Ephy Window",
				      EPHY_TYPE_WINDOW,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (klass, sizeof (EphySidebarEmbedPrivate));
}

GtkWidget *
ephy_sidebar_embed_new (EphyWindow *window)
{
	return GTK_WIDGET (g_object_new (EPHY_TYPE_SIDEBAR_EMBED,
					 "window", window,
					 NULL));
}
