/* Copyright (C) 2004 Red Hat, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the Gnome Library; see the file COPYING.LIB.  If not,
write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef __EGG_FIND_BAR_PRIVATE_H__
#define __EGG_FIND_BAR_PRIVATE_H__

struct _EggFindBarPrivate
{
  gchar *search_string;
  GtkWidget *hbox;
#ifdef SUCK
  GtkWidget *close_button;
#endif
  GtkWidget *find_entry_box;
  GtkWidget *find_entry;
  GtkWidget *next_button;
  GtkWidget *previous_button;
  GtkWidget *case_button;
  GtkWidget *status_separator;
  GtkWidget *status_label;
  guint case_sensitive : 1;
  guint update_id;
};

#define EGG_FIND_BAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EGG_TYPE_FIND_BAR, EggFindBarPrivate))

#endif /* __EGG_FIND_BAR_PRIVATE_H__ */
