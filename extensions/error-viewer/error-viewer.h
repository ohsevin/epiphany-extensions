/*
 *  Copyright © 2002 Jorn Baayen
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

#ifndef ERROR_VIEWER_H
#define ERROR_VIEWER_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define TYPE_ERROR_VIEWER		(error_viewer_get_type ())
#define ERROR_VIEWER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_ERROR_VIEWER, ErrorViewer))
#define ERROR_VIEWER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_ERROR_VIEWER, ErrorViewerClass))
#define IS_ERROR_VIEWER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_ERROR_VIEWER))
#define IS_ERROR_VIEWER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_ERROR_VIEWER))
#define ERROR_VIEWER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_ERROR_VIEWER, ErrorViewerClass))

typedef struct ErrorViewer		ErrorViewer;
typedef struct ErrorViewerClass		ErrorViewerClass;
typedef struct ErrorViewerPrivate	ErrorViewerPrivate;

struct ErrorViewer
{
	EphyDialog parent;

	/*< private >*/
	ErrorViewerPrivate *priv;
};

struct ErrorViewerClass
{
	EphyDialogClass parent_class;
};

typedef enum
{
	ERROR_VIEWER_ERROR,
	ERROR_VIEWER_WARNING,
	ERROR_VIEWER_INFO
} ErrorViewerErrorType;

GType		error_viewer_get_type		(void);

GType		error_viewer_register_type	(GTypeModule *module);

ErrorViewer    *error_viewer_new		(void);

void		error_viewer_append		(ErrorViewer *dialog,
						 ErrorViewerErrorType type,
						 const char *text);

void		error_viewer_use		(ErrorViewer *dialog);

void		error_viewer_unuse		(ErrorViewer *dialog);

G_END_DECLS

#endif
