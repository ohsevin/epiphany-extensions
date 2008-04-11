/*
 *  Copyright © 2004 Adam Hooper
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

#ifndef EXTENSIONS_MANAGER_UI_H
#define EXTENSIONS_MANAGER_UI_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define TYPE_EXTENSIONS_MANAGER_UI		(extensions_manager_ui_get_type ())
#define EXTENSIONS_MANAGER_UI(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_EXTENSIONS_MANAGER_UI, ExtensionsManagerUI))
#define EXTENSIONS_MANAGER_UI_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_EXTENSIONS_MANAGER_UI, ExtensionsManagerUIClass))
#define IS_EXTENSIONS_MANAGER_UI(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_EXTENSIONS_MANAGER_UI))
#define IS_EXTENSIONS_MANAGER_UI_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_EXTENSIONS_MANAGER_UI))
#define EXTENSIONS_MANAGER_UI_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_EXTENSIONS_MANAGER_UI, ExtensionsManagerUIClass))

typedef struct ExtensionsManagerUI		ExtensionsManagerUI;
typedef struct ExtensionsManagerUIClass		ExtensionsManagerUIClass;
typedef struct ExtensionsManagerUIPrivate	ExtensionsManagerUIPrivate;

struct ExtensionsManagerUI
{
	EphyDialog parent;

	/*< private >*/
	ExtensionsManagerUIPrivate *priv;
};

struct ExtensionsManagerUIClass
{
	EphyDialogClass parent_class;
};

GType			extensions_manager_ui_get_type		(void);

GType			extensions_manager_ui_register_type	(GTypeModule *module);

ExtensionsManagerUI    *extensions_manager_ui_new		(void);

G_END_DECLS

#endif
