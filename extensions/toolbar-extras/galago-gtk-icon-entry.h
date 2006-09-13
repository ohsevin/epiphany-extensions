/**
 * @file libgalago-gtk/galago-gtk-icon-entry.h Entry widget
 *
 * @Copyright © 2004 Christian Hammond.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#ifndef _GALAGO_GTK_ICON_ENTRY_H_
#define _GALAGO_GTK_ICON_ENTRY_H_

typedef struct _GalagoGtkIconEntry      GalagoGtkIconEntry;
typedef struct _GalagoGtkIconEntryClass GalagoGtkIconEntryClass;
typedef struct _GalagoGtkIconEntryPriv  GalagoGtkIconEntryPriv;

#include <gtk/gtkentry.h>
#include <gtk/gtkimage.h>

#define GALAGO_GTK_TYPE_ICON_ENTRY (galago_gtk_icon_entry_get_type())
#define GALAGO_GTK_ICON_ENTRY(obj) \
		(G_TYPE_CHECK_INSTANCE_CAST((obj), GALAGO_GTK_TYPE_ICON_ENTRY, GalagoGtkIconEntry))
#define GALAGO_GTK_ICON_ENTRY_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_CAST((klass), GALAGO_GTK_TYPE_ICON_ENTRY, GalagoGtkIconEntryClass))
#define GALAGO_GTK_IS_ICON_ENTRY(obj) \
		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GALAGO_GTK_TYPE_ICON_ENTRY))
#define GALAGO_GTK_IS_ICON_ENTRY_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_TYPE((klass), GALAGO_GTK_TYPE_ICON_ENTRY))
#define GALAGO_GTK_ICON_ENTRY_GET_CLASS(obj) \
		(G_TYPE_INSTANCE_GET_CLASS ((obj), GALAGO_GTK_TYPE_ICON_ENTRY, GalagoGtkIconEntryClass))

struct _GalagoGtkIconEntry
{
	GtkEntry parent_object;

	GalagoGtkIconEntryPriv *priv;

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

struct _GalagoGtkIconEntryClass
{
	GtkEntryClass parent_class;

	/* Signals */
	void (*icon_clicked)(GalagoGtkIconEntry *entry, int button);

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

G_BEGIN_DECLS

GType galago_gtk_icon_entry_get_type(void);

GtkWidget *galago_gtk_icon_entry_new(void);

void galago_gtk_icon_entry_set_icon(GalagoGtkIconEntry *entry, GtkWidget *icon);

void galago_gtk_icon_entry_set_icon_highlight(GalagoGtkIconEntry *entry,
											  gboolean highlight);

GtkWidget *galago_gtk_icon_entry_get_icon(const GalagoGtkIconEntry *entry);

gboolean galago_gtk_icon_entry_get_icon_highlight(const GalagoGtkIconEntry *entry);

#endif /* _GALAGO_GTK_ICON_ENTRY_H_ */
