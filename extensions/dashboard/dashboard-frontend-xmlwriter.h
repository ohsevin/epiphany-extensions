/*
 *  Copyright (C) 2003 Nat Friedman <nat@nat.org>
 *  Copyright (C) 2004 Christian Persch
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
