/*
 *  Copyright © 2009 Benjamin Otte
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

#ifndef EPHY_CELL_RENDERER_TAB_H

#include <epiphany/epiphany.h>


G_BEGIN_DECLS


#define EPHY_TYPE_CELL_RENDERER_TAB		      (ephy_cell_renderer_tab_get_type ())
#define EPHY_CELL_RENDERER_TAB(obj)		      (G_TYPE_CHECK_INSTANCE_CAST ((obj), EPHY_TYPE_CELL_RENDERER_TAB, EphyCellRendererTab))
#define EPHY_CELL_RENDERER_TAB_CLASS(klass)	      (G_TYPE_CHECK_CLASS_CAST ((klass), EPHY_TYPE_CELL_RENDERER_TAB, EphyCellRendererTabClass))
#define EPHY_IS_CELL_RENDERER_TAB(obj)		      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EPHY_TYPE_CELL_RENDERER_TAB))
#define EPHY_IS_CELL_RENDERER_TAB_CLASS(klass)	      (G_TYPE_CHECK_CLASS_TYPE ((klass), EPHY_TYPE_CELL_RENDERER_TAB))
#define EPHY_CELL_RENDERER_TAB_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), EPHY_TYPE_CELL_RENDERER_TAB, EphyCellRendererTabClass))

typedef struct _EphyCellRendererTab EphyCellRendererTab;
typedef struct _EphyCellRendererTabClass EphyCellRendererTabClass;
typedef struct _EphyCellRendererTabPrivate EphyCellRendererTabPrivate;

struct _EphyCellRendererTab
{
  GtkCellRenderer parent;

  EphyCellRendererTabPrivate *priv;
};

struct _EphyCellRendererTabClass
{
  GtkCellRendererClass parent_class;
};

GType            ephy_cell_renderer_tab_get_type (void) G_GNUC_CONST;
void             ephy_cell_renderer_tab_register (GTypeModule *module);

GtkCellRenderer *ephy_cell_renderer_tab_new      (void);


G_END_DECLS


#endif /* EPHY_CELL_RENDERER_TAB_H */
