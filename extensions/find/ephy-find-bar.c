/*
 *  Copyright (C) 2004 Tommi Komulainen
 *  Copyright (C) 2004 Christian Persch
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "config.h"

#include "ephy-find-bar.h"
#include "eggfindbarprivate.h"

#include "mozilla-find.h"

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-command-manager.h>
#include "ephy-debug.h"

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtkaction.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkmain.h>
#include <string.h>

#define EPHY_FIND_BAR_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object),EPHY_TYPE_FIND_BAR, EphyFindBarPrivate))

struct _EphyFindBarPrivate
{
	EphyWindow *window;
	EphyEmbed *embed;

	GtkWidget *offscreen_window;
	gulong set_focus_handler;
	gboolean preedit_changed;
	gboolean ppv_mode;
	GtkAction *find_action;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

static GObjectClass *parent_class = NULL;
static GType type = 0;

static void
ensure_offscren_window (EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;
	GdkScreen *screen;

	LOG ("ensure_offscren_window")

	if (priv->offscreen_window == NULL)
	{
		LOG ("creating off-screen window")

		priv->offscreen_window = gtk_window_new (GTK_WINDOW_POPUP);
		gtk_window_set_modal (GTK_WINDOW (priv->offscreen_window), TRUE);

		gtk_widget_realize (priv->offscreen_window);

		screen = gtk_widget_get_screen (GTK_WIDGET (bar));
		gtk_window_set_screen (GTK_WINDOW (priv->offscreen_window), screen);
		gtk_window_move (GTK_WINDOW (priv->offscreen_window),
				 gdk_screen_get_width (screen) + 1,
				 gdk_screen_get_height (screen) + 1);
	}
}

static void
update_navigation_controls (EphyFindBar *bar,
			    gboolean can_find_next,
			    gboolean can_find_prev)
{
	EggFindBar *ebar = EGG_FIND_BAR (bar);
	EggFindBarPrivate *epriv = ebar->priv;
return;
	gtk_widget_set_sensitive (GTK_WIDGET (epriv->next_button), can_find_next);
	gtk_widget_set_sensitive (GTK_WIDGET (epriv->previous_button), can_find_prev);
}

#if 0
static void
check_text_case (const char *text, gboolean *upper, gboolean *lower)
{
	*upper = *lower = FALSE;

	for (; text != NULL && text[0] != '\0'; text = g_utf8_next_char (text))
	{
		gunichar c = g_utf8_get_char (text);

		if (g_unichar_isupper (c)) *upper = TRUE;
		else if (g_unichar_islower (c)) *lower = TRUE;
		else continue;

		if (*upper && *lower) break;
	}
//	*upper = FALSE; *lower = TRUE;
}
#endif

static void
set_status_text (EphyFindBar *bar,
		 guint32 count)
{
	EggFindBar *ebar = EGG_FIND_BAR (bar);
	char *status;

	status = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
				  	     "%d match found",
					     "%d matches found",
					     count),
				  count);

	egg_find_bar_set_status_text (ebar, status);

	g_free (status);
}

static void
update_find_properties (EphyFindBar *bar)
{
	EggFindBar *ebar = EGG_FIND_BAR (bar);
	EphyFindBarPrivate *priv = bar->priv;
	const char *text;

	text = egg_find_bar_get_search_string (ebar);

	LOG ("update_find_properties text:%s", text ? text : "")

	if (text == NULL || text[0] == '\0')
	{
		mozilla_find_set_properties (priv->embed, "", FALSE, TRUE);
		egg_find_bar_set_status_text (ebar, NULL);
		update_navigation_controls (bar, FALSE, FALSE);
	}
	else if (GTK_WIDGET_VISIBLE (bar))
	{
		/* match case automatically if the user types MiXeD cAsE or
		 * UPPER CASE search string
		 */
//		gboolean upper, lower;

//		check_text_case (text, &upper, &lower);

		gboolean upper;
		guint32 count;

		upper = egg_find_bar_get_case_sensitive (ebar);

		count = mozilla_find_set_properties (priv->embed, text, upper, TRUE);
		set_status_text (bar, count);

		update_navigation_controls (bar, TRUE, TRUE);
	}
}

static void
sync_search_string_cb (EphyFindBar *bar,
		       GParamSpec *pspec,
		       gpointer data)
{
	update_find_properties (bar);

	mozilla_find_next (bar->priv->embed, FALSE);
}

static void
sync_case_sensitive_cb (EphyFindBar *bar,
		        GParamSpec *pspec,
		        gpointer data)
{
	update_find_properties (bar);
}

static gboolean
find_entry_key_press_event_cb (GtkEntry *entry,
			       GdkEventKey *event,
			       EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;
	guint mask = gtk_accelerator_get_default_mod_mask ();
	gboolean handled = FALSE;

	LOG ("find_entry_key_press_event_cb")

	/* Hide the bar when ESC is pressed */
	if ((event->state & mask) == 0)
	{
		handled = TRUE;
		switch (event->keyval)
		{
			case GDK_Up:
				mozilla_embed_scroll_lines (priv->embed, -1);
				break;
			case GDK_Down:
				mozilla_embed_scroll_lines (priv->embed, 1);
				break;
			case GDK_Page_Up:
				mozilla_embed_scroll_pages (priv->embed, -1);
				break;
			case GDK_Page_Down:
				mozilla_embed_scroll_pages (priv->embed, 1);
				break;
			default:
				handled = FALSE;
				break;
		}
	}

	return handled;
}

static void
entry_preedit_changed_cb (GtkIMContext *context,
			  EphyFindBar *bar)
{
	LOG ("entry_preedit_changed_cb")

	bar->priv->preedit_changed = TRUE;
}

static void
embed_net_stop_cb (EphyEmbed *embed,
		   EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;

	/* the highlighting is lost if the page changes, so redo the
	 * highlighting once the new page has finished loading
	 */
	if (GTK_WIDGET_VISIBLE (bar))
	{
		guint32 count;

		LOG ("embed_net_stop_cb")

		mozilla_find_set_highlight (priv->embed, FALSE);
		count = mozilla_find_set_highlight (priv->embed, TRUE);
		set_status_text (bar, count);
	}
}

/* Cut and paste from gtkwindow.c */
static void
send_focus_change (GtkWidget *widget,
                   gboolean   in)
{
  GdkEvent *fevent = gdk_event_new (GDK_FOCUS_CHANGE);

  g_object_ref (widget);
   
 if (in)
    GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
  else
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

  fevent->focus_change.type = GDK_FOCUS_CHANGE;
  fevent->focus_change.window = g_object_ref (widget->window);
  fevent->focus_change.in = in;
  
  gtk_widget_event (widget, fevent);
  
  g_object_notify (G_OBJECT (widget), "has_focus");

  g_object_unref (widget);
  gdk_event_free (fevent);
}

/* Code adapted from gtktreeview.c:gtk_tree_view_key_press() and
 * gtk_tree_view_real_start_interactive_seach()
 */
static gboolean
embed_key_press_event_cb (EphyEmbed *embed,
			  GdkEventKey *event,
			  EphyFindBar *bar)
{
	EggFindBar *ebar = EGG_FIND_BAR (bar);
	EggFindBarPrivate *epriv = ebar->priv;
	EphyFindBarPrivate *priv = bar->priv;
	GtkWidget *widget = GTK_WIDGET (bar);
	GtkWidgetClass *entry_parent_class;
	GdkWindow *event_window;
	GdkEvent *new_event;
	GdkEventKey *new_event_key;
	char *old_text;
	const char *new_text;
	gboolean handled = FALSE, retval, text_modified;

	/* don't acccept the keypress if find is disabled (f.e. for image embeds) */
	if (!gtk_action_is_sensitive (priv->find_action) || priv->ppv_mode) return FALSE;

	LOG ("embed_key_press_event_cb, find bar is %svisible", GTK_WIDGET_VISIBLE (widget) ? "" : "in")

	/* if the bar is invisible, show the offscreen window and add the entry to it */
	if (!GTK_WIDGET_VISIBLE (widget))
	{
		LOG ("using off-screen window")
		ensure_offscren_window (bar);
		g_return_val_if_fail (priv->offscreen_window != NULL, FALSE);

		gtk_widget_reparent (ebar->priv->find_entry, priv->offscreen_window);
		gtk_widget_show (priv->offscreen_window);
	}

	g_return_val_if_fail (GTK_WIDGET_REALIZED (epriv->find_entry), FALSE);

	/* We pass the event to the search_entry.  If its text changes, then we start
	 * the typeahead find capabilities.
	 */

	priv->preedit_changed = FALSE;

	/* Make a copy of the current text */
	old_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (epriv->find_entry)));
	new_event = gdk_event_copy ((GdkEvent *) event);
	new_event_key = (GdkEventKey *) new_event;
	event_window = new_event_key->window;
	new_event_key->window = epriv->find_entry->window;

	/* Send the event to the window.  If the preedit_changed signal is emitted
	* during this event, we will set priv->imcontext_changed  */
	retval = gtk_widget_event (epriv->find_entry, new_event);

	new_event_key->window = event_window;

	if (!GTK_WIDGET_VISIBLE (widget))
	{
		g_return_val_if_fail (priv->offscreen_window != NULL, FALSE);

		LOG ("hiding off-screen window again")

		gtk_widget_hide (priv->offscreen_window);
		gtk_widget_reparent (ebar->priv->find_entry,
				     ebar->priv->find_entry_box);
	}

	/* We check to make sure that the entry tried to handle the text, and that
	* the text has changed.
	*/
	new_text = gtk_entry_get_text (GTK_ENTRY (ebar->priv->find_entry));
	text_modified = strcmp (old_text, new_text) != 0;

	if (priv->preedit_changed || (retval && text_modified))
	{
		gtk_widget_show (widget);

		/* Grab focus will select all the text.  We don't want that to happen, so we
		 * call the parent instance and bypass the selection change.  This is probably
		 * really non-kosher.
		 */
		entry_parent_class = g_type_class_peek_parent (GTK_ENTRY_GET_CLASS (epriv->find_entry));
		(entry_parent_class->grab_focus) (epriv->find_entry);

		/* send focus-in event */
		send_focus_change (epriv->find_entry, TRUE);

		handled = TRUE;
	}
	else
	{
		ephy_embed_activate (priv->embed);
	}

	g_free (old_text);
	gdk_event_free (new_event);

	return handled;
}

static void
set_focus_cb (EphyWindow *window,
	      GtkWidget *widget,
	      EphyFindBar *bar)
{
	GtkWidget *wbar = GTK_WIDGET (bar);

	LOG ("set_focus")

	while (widget != NULL && widget != wbar)
	{
		widget = widget->parent;
	}

	/* if widget == bar, the new focus widget is in the bar, so we
	 * don't deactivate.
	 */
	if (widget != wbar)
	{
		gtk_widget_hide (wbar);
	}
}

static void
unset_embed (EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;

	LOG ("unset_embed")

	if (priv->embed != NULL)
	{
		EphyEmbed **embedptr;

		g_signal_handlers_disconnect_matched (priv->embed, G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL, bar);

		embedptr = &priv->embed;
		g_object_remove_weak_pointer (G_OBJECT (priv->embed), (gpointer *) embedptr);

		priv->embed = NULL;	
	}

}

static void
update_find_bar (EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;

	LOG ("embed realized!")

	update_find_properties (bar);

	g_signal_handlers_disconnect_by_func
		(priv->embed, G_CALLBACK (update_find_bar), bar);
}

static void
sync_active_tab (EphyWindow *window,
		 GParamSpec *pspec,
		 EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;
//	EphyEmbed *embed;
	EphyEmbed **embedptr;

	LOG ("sync_active_tab")

	unset_embed (bar);

	//embed = ephy_window_get_active_embed (window);
	//if (embed == NULL) return; /* happens at startup */
	//priv->embed = embed;

	EphyTab *tab;
	tab = ephy_window_get_active_tab (window);
	if (tab == NULL) return; /* happens at startup */

	priv->embed = ephy_tab_get_embed (tab);
	embedptr = &priv->embed;
	g_object_add_weak_pointer (G_OBJECT (priv->embed), (gpointer *) embedptr);

	g_signal_connect_after (priv->embed, "net-stop",
				G_CALLBACK (embed_net_stop_cb), bar);
	g_signal_connect_after (priv->embed, "key_press_event",
				G_CALLBACK (embed_key_press_event_cb), bar);

	if (!GTK_WIDGET_REALIZED (priv->embed))
	{
		g_signal_connect_data (priv->embed, "realize",
				       G_CALLBACK (update_find_bar), bar,
				       NULL, G_CONNECT_SWAPPED);
		return;
	}

	update_find_properties (bar);
}

static void
sync_print_preview_mode (EphyWindow *window,
			 GParamSpec *pspec,
			 EphyFindBar *bar)
{
	EphyFindBarPrivate *priv = bar->priv;

	g_object_get (G_OBJECT (window), "print-preview-mode", &priv->ppv_mode, NULL);

	if (priv->ppv_mode && GTK_WIDGET_VISIBLE (GTK_WIDGET (bar)))
	{
		gtk_widget_hide (GTK_WIDGET (bar));
	}
}

static void
find_cb (GtkAction *action,
	 GtkWidget *bar)
{
	gtk_widget_show (bar);
	gtk_widget_grab_focus (bar);
}

static GtkAction *
get_action (EphyWindow *window)
{
	GtkUIManager *manager;
	GtkAction *action;

	manager = GTK_UI_MANAGER (ephy_window_get_ui_manager (window));
	action = gtk_ui_manager_get_action (manager, "/menubar/EditMenu/EditFindMenu");
	g_return_val_if_fail (action != NULL, NULL);

	return action;
}

static void
ephy_find_bar_set_window (EphyFindBar *bar,
			  EphyWindow *window)
{
	EphyFindBarPrivate *priv = bar->priv;
	GtkAction *action;

	priv->window = window;

	sync_active_tab (window, NULL, bar);
	g_signal_connect (window, "notify::active-tab",
			  G_CALLBACK (sync_active_tab), bar);
	sync_print_preview_mode (window, NULL, bar);
	g_signal_connect (window, "notify::print-preview-mode",
			  G_CALLBACK (sync_print_preview_mode), bar);

	action = get_action (window);
	g_return_if_fail (action != NULL);
	priv->find_action = action;

	/* Block the default signal handler */
	g_signal_handlers_block_matched (action, G_SIGNAL_MATCH_DATA, 
					 0, 0, NULL, NULL, priv->window);
	
	/* Add our signal handler */
	g_signal_connect (action, "activate",
			  G_CALLBACK (find_cb), bar);
}

static void
ephy_find_bar_show (GtkWidget *widget)
{
	EphyFindBar *bar = EPHY_FIND_BAR (widget);
	EphyFindBarPrivate *priv = bar->priv;
	guint32 count;

	LOG ("ephy_find_bar_show")

	GTK_WIDGET_CLASS (parent_class)->show (widget);

	if (priv->set_focus_handler == 0)
	{
		priv->set_focus_handler =
			g_signal_connect (priv->window, "set-focus",
					  G_CALLBACK (set_focus_cb), bar);
	}

	if (priv->embed != NULL)
	{
		count = mozilla_find_set_highlight (priv->embed, TRUE);
		set_status_text (bar, count);
	}
}

static void
ephy_find_bar_hide (GtkWidget *widget)
{
	EggFindBar *ebar = EGG_FIND_BAR (widget);
	EphyFindBar *bar = EPHY_FIND_BAR (widget);
	EphyFindBarPrivate *priv = bar->priv;

	LOG ("ephy_find_bar_hide")

	if (priv->set_focus_handler != 0)
	{
		g_signal_handlers_disconnect_by_func
			(priv->window, G_CALLBACK (set_focus_cb), bar);
		priv->set_focus_handler = 0;
	}

	if (priv->embed != NULL)
	{
		mozilla_find_set_highlight (priv->embed, FALSE);
		egg_find_bar_set_status_text (ebar, NULL);
	}

	GTK_WIDGET_CLASS (parent_class)->hide (widget);
}

static void
ephy_find_bar_screen_changed (GtkWidget *widget,
			      GdkScreen *new_screen)
{
	EggFindBarPrivate *epriv = EGG_FIND_BAR (widget)->priv;
	EphyFindBarPrivate *priv = EPHY_FIND_BAR (widget)->priv;

	if (priv->offscreen_window != NULL)
	{
		g_return_if_fail (gtk_widget_get_parent (epriv->find_entry) != priv->offscreen_window);

		gtk_widget_destroy (priv->offscreen_window);
		priv->offscreen_window = NULL;
	}

	if (GTK_WIDGET_CLASS (parent_class)->screen_changed)
	{
		GTK_WIDGET_CLASS (parent_class)->screen_changed (widget, new_screen);
	}
}

static void
ephy_find_bar_next (EggFindBar *ebar)
{
	EphyFindBar *bar = EPHY_FIND_BAR (ebar);
	gboolean found;

	LOG ("ephy_find_bar_next")

	found = mozilla_find_next (bar->priv->embed, FALSE);
	update_navigation_controls (bar, found, TRUE);
}

static void
ephy_find_bar_previous (EggFindBar *ebar)
{
	EphyFindBar *bar = EPHY_FIND_BAR (ebar);
	gboolean found;

	LOG ("ephy_find_bar_previous")

	found = mozilla_find_next (bar->priv->embed, TRUE);
	update_navigation_controls (bar, TRUE, found);
}

static void
ephy_find_bar_close (EggFindBar *ebar)
{
	EphyFindBar *bar = EPHY_FIND_BAR (ebar);

	LOG ("ephy_find_bar_close")

	g_return_if_fail (bar->priv->embed != NULL);

	gtk_widget_hide (GTK_WIDGET (ebar));
	ephy_embed_activate (bar->priv->embed);
}

static void
ephy_find_bar_init (EphyFindBar *bar)
{
	EggFindBar *ebar = EGG_FIND_BAR (bar);
	EggFindBarPrivate *epriv = ebar->priv;
	EphyFindBarPrivate *priv;

	priv = bar->priv = EPHY_FIND_BAR_GET_PRIVATE (bar);

	/* connect signals */
	g_signal_connect (bar, "notify::search-string",
			  G_CALLBACK (sync_search_string_cb), NULL);
	g_signal_connect (bar, "notify::case-sensitive",
			  G_CALLBACK (sync_case_sensitive_cb), NULL);
		  
	g_signal_connect (epriv->find_entry, "key-press-event",
			  G_CALLBACK (find_entry_key_press_event_cb), bar);
	g_signal_connect (GTK_ENTRY (epriv->find_entry)->im_context,
			  "preedit-changed",
			  G_CALLBACK (entry_preedit_changed_cb), bar);

	update_navigation_controls (bar, FALSE, FALSE);
}

static void
ephy_find_bar_finalize (GObject *object)
{
	EphyFindBar *bar = EPHY_FIND_BAR (object);
	EphyFindBarPrivate *priv = bar->priv;

	/* Remove our signal handler */
	g_signal_handlers_disconnect_by_func
		(priv->find_action, G_CALLBACK (find_cb), bar);

	/* And unblock the default one */
	g_signal_handlers_unblock_matched (priv->find_action, G_SIGNAL_MATCH_DATA, 
					   0, 0, NULL, NULL, priv->window);
	
	g_signal_handlers_disconnect_matched (priv->window,
					      G_SIGNAL_MATCH_DATA,
					      0, 0, NULL, NULL, object);

	unset_embed (bar);

	if (priv->offscreen_window != NULL)
	{
		gtk_widget_destroy (priv->offscreen_window);
	}

	parent_class->finalize (object);
}

static void
ephy_find_bar_set_property (GObject *object,
				guint prop_id,
				const GValue *value,
				GParamSpec *pspec)
{
	EphyFindBar *bar = EPHY_FIND_BAR (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			ephy_find_bar_set_window (bar, g_value_get_object (value));
			break;
	}
}

static void
ephy_find_bar_get_property (GObject *object,
				guint prop_id,
				GValue *value,
				GParamSpec *pspec)
{
	/* no readable properties */
	g_assert_not_reached ();
}

static void
ephy_find_bar_class_init (EphyFindBarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	EggFindBarClass *find_bar_class = EGG_FIND_BAR_CLASS (klass);
	GtkBindingSet *binding_set;

        parent_class = g_type_class_peek_parent (klass);

        object_class->finalize = ephy_find_bar_finalize;
	object_class->set_property = ephy_find_bar_set_property;
	object_class->get_property = ephy_find_bar_get_property;

	widget_class->show = ephy_find_bar_show;
	widget_class->hide = ephy_find_bar_hide;
	widget_class->screen_changed = ephy_find_bar_screen_changed;

	find_bar_class->next = ephy_find_bar_next;
	find_bar_class->previous = ephy_find_bar_previous;
	find_bar_class->close = ephy_find_bar_close;

	g_object_class_install_property (object_class,
                                         PROP_WINDOW,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "Parent window",
							      EPHY_TYPE_WINDOW,
							      G_PARAM_WRITABLE |
							      G_PARAM_CONSTRUCT_ONLY));

	binding_set = gtk_binding_set_by_class (klass);
	
	gtk_binding_entry_add_signal (binding_set, GDK_g, GDK_CONTROL_MASK,
				      "next", 0);
	gtk_binding_entry_add_signal (binding_set, GDK_g, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
				      "previous", 0);

	g_type_class_add_private (klass, sizeof (EphyFindBarPrivate));
}

GType
ephy_find_bar_get_type (void)
{
	return type;
}

GType
ephy_find_bar_register_type (GTypeModule *module)
{
	static const GTypeInfo our_info =
	{
		sizeof (EphyFindBarClass),
		NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) ephy_find_bar_class_init,
		NULL,
		NULL, /* class_data */
		sizeof (EphyFindBar),
		0, /* n_preallocs */
		(GInstanceInitFunc) ephy_find_bar_init
	};

	type = g_type_module_register_type (module,
					    EGG_TYPE_FIND_BAR,
					    "EphyFindBar",
					    &our_info, 0);

	return type;
}

GtkWidget *
ephy_find_bar_new (EphyWindow *window)
{
	return g_object_new (EPHY_TYPE_FIND_BAR,
			     "window", window,
			     NULL);
}
