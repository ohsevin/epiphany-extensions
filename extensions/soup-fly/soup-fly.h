/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 *  Copyright © 2009 Igalia S.L.
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
 */

#ifndef SOUP_FLY_H
#define SOUP_FLY_H

#include <glib-object.h>
#include <gmodule.h>

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define TYPE_SOUP_FLY         (soup_fly_get_type ())
#define SOUP_FLY(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_SOUP_FLY, SoupFly))
#define SOUP_FLY_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_SOUP_FLY, SoupFlyClass))
#define IS_SOUP_FLY(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_SOUP_FLY))
#define IS_SOUP_FLY_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_SOUP_FLY))
#define SOUP_FLY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_SOUP_FLY, SoupFlyClass))

typedef struct _SoupFly   SoupFly;
typedef struct _SoupFlyClass    SoupFlyClass;
typedef struct _SoupFlyPrivate  SoupFlyPrivate;

struct _SoupFly {
  GtkWindow parent;

  /*< private >*/
  SoupFlyPrivate *priv;
};

struct _SoupFlyClass {
  GtkWindowClass parent_class;
};

GType       soup_fly_get_type      (void);
GType       soup_fly_register_type (GTypeModule *module);
SoupFly    *soup_fly_new           (void);
void        soup_fly_start         (SoupFly     *logger);
void        soup_fly_stop          (SoupFly     *logger);

G_END_DECLS

#endif
