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

#ifndef GREASEMONKEY_SCRIPT_H
#define GREASEMONKEY_SCRIPT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_GREASEMONKEY_SCRIPT	(greasemonkey_script_get_type ())
#define GREASEMONKEY_SCRIPT(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_GREASEMONKEY_SCRIPT, GreasemonkeyScript))
#define GREASEMONKEY_SCRIPT_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_GREASEMONKEY_SCRIPT, GreasemonkeyScriptClass))
#define IS_GREASEMONKEY_SCRIPT(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_GREASEMONKEY_SCRIPT))
#define IS_GREASEMONKEY_SCRIPT_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_GREASEMONKEY_SCRIPT))
#define GREASEMONKEY_SCRIPT_GET_CLASS(o)(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_GREASEMONKEY_SCRIPT, GreasemonkeyScriptClass))

typedef struct _GreasemonkeyScript		GreasemonkeyScript;
typedef struct _GreasemonkeyScriptClass		GreasemonkeyScriptClass;
typedef struct _GreasemonkeyScriptPrivate	GreasemonkeyScriptPrivate;

struct _GreasemonkeyScriptClass
{
	GObjectClass parent_class;
};

struct _GreasemonkeyScript
{
	GObject parent_instance;

	/*< private >*/
	GreasemonkeyScriptPrivate *priv;
};

GType			greasemonkey_script_get_type		(void);

GType			greasemonkey_script_register_type	(GTypeModule *module);

GreasemonkeyScript	*greasemonkey_script_new		(const char *filename);

gboolean		greasemonkey_script_applies_to_url	(GreasemonkeyScript *gs,
								 const char *url);

G_END_DECLS

#endif
