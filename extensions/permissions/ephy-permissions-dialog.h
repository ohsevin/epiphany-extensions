/*
 *  Copyright © 2002 Jorn Baayen
 *  Copyright © 2003 Marco Pesenti Gritti
 *  Copyright © 2005 Christian Persch
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

#ifndef EPHY_PERMISSIONS_DIALOG_H
#define EPHY_PERMISSIONS_DIALOG_H

#include <gmodule.h>
#include <gtk/gtkdialog.h>

G_BEGIN_DECLS

#define EPHY_TYPE_PERMISSIONS_DIALOG		(ephy_permissions_dialog_get_type ())
#define EPHY_PERMISSIONS_DIALOG(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EPHY_TYPE_PERMISSIONS_DIALOG, EphyPermissionsDialog))
#define EPHY_PERMISSIONS_DIALOG_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_PERMISSIONS_DIALOG, EphyPermissionsDialogClass))
#define EPHY_IS_PERMISSIONS_DIALOG(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_PERMISSIONS_DIALOG))
#define EPHY_IS_PERMISSIONS_DIALOG_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_PERMISSIONS_DIALOG))
#define EPHY_PERMISSIONS_DIALOG_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_PERMISSIONS_DIALOG, EphyPermissionsDialogClass))

typedef struct EphyPermissionsDialog		EphyPermissionsDialog;
typedef struct EphyPermissionsDialogPrivate	EphyPermissionsDialogPrivate;
typedef struct EphyPermissionsDialogClass	EphyPermissionsDialogClass;

struct EphyPermissionsDialog
{
	GtkDialog parent;

	/*< private >*/
	EphyPermissionsDialogPrivate *priv;
};

struct EphyPermissionsDialogClass
{
	GtkDialogClass parent_class;
};

GType		ephy_permissions_dialog_get_type	(void);

GType		ephy_permissions_dialog_register_type	(GTypeModule *module);

GtkWidget      *ephy_permissions_dialog_new		(void);

void		ephy_permissions_dialog_add_tab		(EphyPermissionsDialog *dialog,
							 const char *type,
							 const char *title);

G_END_DECLS

#endif
