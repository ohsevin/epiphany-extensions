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

#ifndef AD_URI_TESTER_H
#define AD_URI_TESTER_H

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <epiphany/epiphany.h>

G_BEGIN_DECLS

#define TYPE_AD_URI_TESTER		(ad_uri_tester_get_type ())
#define AD_URI_TESTER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AD_URI_TESTER, AdUriTester))
#define AD_URI_TESTER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), TYPE_AD_URI_TESTER, AdUriTesterClass))
#define IS_AD_URI_TESTER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AD_URI_TESTER))
#define IS_AD_URI_TESTER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AD_URI_TESTER))
#define AD_URI_TESTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AD_URI_TESTER, AdUriTesterClass))

typedef struct _AdUriTester		AdUriTester;
typedef struct _AdUriTesterClass	AdUriTesterClass;
typedef struct _AdUriTesterPrivate	AdUriTesterPrivate;

struct _AdUriTester
{
	GObject parent_instance;

	/*< private >*/
	AdUriTesterPrivate *priv;
};

struct _AdUriTesterClass
{
	GObjectClass parent_class;
};

GType		 ad_uri_tester_get_type	(void);

GType		 ad_uri_tester_register_type	(GTypeModule *module);

AdUriTester	*ad_uri_tester_new		(void);

gboolean	 ad_uri_tester_test_uri		(AdUriTester *tester,
						 const char *uri,
						 AdUriCheckType type);

void		 ad_uri_tester_reload		(AdUriTester *tester);

G_END_DECLS

#endif /* AD_URI_TESTER_H */
