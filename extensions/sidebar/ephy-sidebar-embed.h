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
 *  $Id$
 */

#ifndef EPHY_SIDEBAR_EMBED_H
#define EPHY_SIDEBAR_EMBED_H

#include <glib-object.h>
#include <gtk/gtkbin.h>

#include <epiphany/ephy-embed.h>
#include <epiphany/ephy-window.h>

G_BEGIN_DECLS

#define EPHY_TYPE_SIDEBAR_EMBED		(ephy_sidebar_embed_get_type ())
#define EPHY_SIDEBAR_EMBED(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_SIDEBAR_EMBED, EphySidebarEmbed))
#define EPHY_SIDEBAR_EMBED_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_SIDEBAR_EMBED, EphySidebarEmbedClass))
#define EPHY_IS_SIDEBAR_EMBED(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_SIDEBAR_EMBED))
#define EPHY_IS_SIDEBAR_EMBED_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_SIDEBAR_EMBED))
#define EPHY_SIDEBAR_EMBED_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_SIDEBAR_EMBED, EphySidebarEmbedClass))

typedef struct _EphySidebarEmbed	EphySidebarEmbed;
typedef struct _EphySidebarEmbedPrivate	EphySidebarEmbedPrivate;
typedef struct _EphySidebarEmbedClass	EphySidebarEmbedClass;

struct _EphySidebarEmbed 
{
	GtkBin parent;

	/*< private >*/
	EphySidebarEmbedPrivate *priv;
};

struct _EphySidebarEmbedClass
{
	GtkBinClass parent_class;
};

GType	    ephy_sidebar_embed_get_type		(void);

GType	    ephy_sidebar_embed_register_type	(GTypeModule *module);

GtkWidget  *ephy_sidebar_embed_new		(EphyWindow *window);

void	    ephy_sidebar_embed_set_url		(EphySidebarEmbed *embed,
						 const char *url);

EphyWindow *ephy_sidebar_embed_get_window 	(EphySidebarEmbed *embed);

EphyEmbed  *ephy_sidebar_embed_get_embed	(EphySidebarEmbed *embed);

G_END_DECLS

#endif
