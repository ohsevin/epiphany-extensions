/*
 *  Copyright (C) 2003 Philip Langdale
 *
 *  Based on PDM Dialog:
 *  	Copyright (C) 2002 Jorn Baayen
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

#ifndef PAGE_INFO_DIALOG_H
#define PAGE_INFO_DIALOG_H

#include <epiphany/ephy-dialog.h>
#include <epiphany/ephy-embed.h>

#include <glib.h>

G_BEGIN_DECLS

typedef struct PageInfoDialog PageInfoDialog;
typedef struct PageInfoDialogClass PageInfoDialogClass;

#define TYPE_PAGE_INFO_DIALOG             (page_info_dialog_get_type ())
#define PAGE_INFO_DIALOG(obj)             (GTK_CHECK_CAST ((obj), TYPE_PAGE_INFO_DIALOG, PageInfoDialog))
#define PAGE_INFO_DIALOG_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), PAGE_INFO_DIALOG, PageInfoDialogClass))
#define IS_PAGE_INFO_DIALOG(obj)          (GTK_CHECK_TYPE ((obj), TYPE_PAGE_INFO_DIALOG))
#define IS_PAGE_INFO_DIALOG_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), PAGE_INFO_DIALOG))

typedef struct PageInfoDialogPrivate PageInfoDialogPrivate;

typedef enum {
	PAGE_INFO_GENERAL,
	/*
	PAGE_INFO_FORMS,
	PAGE_INFO_LINKS,
	PAGE_INFO_MEDIA,
	*/
	/* PAGE_INFO_STYLESHEETS, */
	/* PAGE_INFO_PRIVACY, */
	/*PAGE_INFO_SECURITY*/
} PageInfoDialogPage;

struct PageInfoDialog
{
	EphyDialog parent;
	PageInfoDialogPrivate *priv;
};

struct PageInfoDialogClass
{
	EphyDialogClass parent_class;
};

GType		 page_info_dialog_get_type	(void);

GType		 page_info_dialog_register_type	(GTypeModule *module);

PageInfoDialog	*page_info_dialog_new              (GtkWidget *window,
						    EphyEmbed *embed);

/*
void		 page_info_dialog_set_current_page (PageInfoDialog     *dialog,
						    PageInfoDialogPage  page);
*/

G_END_DECLS

#endif
