/*
 *  Copyright © 2005 Jean-François Rameau
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

#ifndef LIVEHTTPHEADERS_H
#define LIVEHTTPHEADERS_H

#include <epiphany/ephy-dialog.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_LIVEHTTPHEADERS_UI		(livehttpheaders_ui_get_type ())
#define LIVEHTTPHEADERS_UI(o)		(G_TYPE_CHECK_INSTANCE_CAST((o), TYPE_LIVEHTTPHEADERS_UI, LiveHTTPHeadersUI))
#define LIVEHTTPHEADERS_UI_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_LIVEHTTPHEADERS_UI, LiveHTTPHeadersUIClass))
#define IS_LIVEHTTPHEADERS_UI(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), TYPE_LIVEHTTPHEADERS_UI))
#define IS_LIVEHTTPHEADERS_UI_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), TYPE_LIVEHTTPHEADERS_UI))
#define LIVEHTTPHEADERS_UI_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),	TYPE_LIVEHTTPHEADERS_UI, LiveHTTPHeadersUIClass))

typedef struct _LiveHTTPHeadersUI	 LiveHTTPHeadersUI;
typedef struct _LiveHTTPHeadersUIClass	 LiveHTTPHeadersUIClass;
typedef struct _LiveHTTPHeadersUIPrivate LiveHTTPHeadersUIPrivate;

struct _LiveHTTPHeadersUI
{
	EphyDialog parent;

	/*< private >*/
	LiveHTTPHeadersUIPrivate *priv;
};

struct _LiveHTTPHeadersUIClass
{
	EphyDialogClass parent_class;
};

GType	 livehttpheaders_ui_get_type		(void);

GType	 livehttpheaders_ui_register_type	(GTypeModule *module);

LiveHTTPHeadersUI *livehttpheaders_ui_new	(void);
			     
G_END_DECLS

#endif
