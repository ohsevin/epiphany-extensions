/*
 *  Copyright (C) 2004 Jean-François Rameau
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

#include "smart-bookmarks-prefs-ui.h"
#include "smart-bookmarks-prefs.h"
#include "ephy-debug.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkstock.h>
#include <epiphany/ephy-dialog.h>

enum
{
	PROP_WINDOW,
	PROP_USE_GNOME_DICT,
	PROP_OPEN_IN_TAB,
};

static const
EphyDialogProperty properties [] =
{
	{ "smart_bookmarks_ui",	NULL            , PT_NORMAL   , 0 },
	{ "use_gnome_dict",	CONF_USE_GDICT  , PT_AUTOAPPLY, 0 },
	{ "open_in_tab",	CONF_OPEN_IN_TAB, PT_AUTOAPPLY, 0 },

	{ NULL }
};

void smart_bookmarks_ui_response_cb (GtkWidget *widget,
				     gint response_id,
				     EphyDialog *dialog);

static EphyDialog *dialog = NULL;

void
smart_bookmarks_show_prefs_ui_cb (GtkAction *action,
				  EphyWindow *window)
{
	if (dialog == NULL)
	{
		EphyDialog **ptr;
		GtkWidget *window;

		dialog = ephy_dialog_new ();
		ptr = &dialog;
		g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer *) ptr);

		ephy_dialog_construct (dialog, properties,
				       SHARE_DIR "/glade/smart-bookmarks.glade",
				       properties[PROP_WINDOW].id,
				       GETTEXT_PACKAGE);

		window = ephy_dialog_get_control (dialog,
						  properties[PROP_WINDOW].id);
		gtk_window_set_icon_name (GTK_WINDOW (window),
					  GTK_STOCK_PREFERENCES);

	}

	ephy_dialog_set_parent (dialog, GTK_WIDGET (window));
	ephy_dialog_show (dialog);
}

void
smart_bookmarks_ui_response_cb (GtkWidget *widget,
				gint response_id,
				EphyDialog *dialog)
{
	g_object_unref (dialog);
}
