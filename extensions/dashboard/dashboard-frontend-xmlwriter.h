/*
 *  Copyright (C) 2003 by whoever wrote the original dashboard-frontend.c
 *  Copyright (C) 2004 Christian Persch
 *
 *  FIXME: what was the license of dashboard-frontend.c ?
 *
 *  $Id$
 */

#ifndef __DASHBOARD_FRONTEND_XMLWRITER_H__
#define __DASHBOARD_FRONTEND_XMLWRITER_H__

#include <libxml/xmlwriter.h>
#include <glib.h>

G_BEGIN_DECLS

xmlTextWriterPtr	NewTextWriterDashboard	(const xmlChar *frontend,
						 gboolean focused,
						 const xmlChar *context);

int			FreeTextWriterDashboard	(xmlTextWriterPtr writer);

int			DashboardSendClue	(xmlTextWriterPtr writer,
						 const xmlChar *text,
						 const xmlChar *type,
						 int relevance);

int			DashboardSendCluePacket	(const xmlChar *frontend,
						 gboolean focused,
						 const xmlChar *context,
						 ...);

G_END_DECLS

#endif /* __DASHBOARD_FRONTEND_XMLWRITER_H__ */
