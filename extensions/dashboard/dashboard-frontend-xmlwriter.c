/*
 *  Copyright (C) 2003 Nat Friedman <nat@nat.org>
 *  Copyright (C) 2004 Christian Persch
 *
 *  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, provided that the above copyright notice(s) and this
 *  permission notice appear in all copies of the Software and that both the
 *  above copyright notice(s) and this permission notice appear in
 *  supporting documentation.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE
 *  BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES,
 *  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  Except as contained in this notice, the name of a copyright holder shall
 *  not be used in advertising or otherwise to promote the sale, use or other
 *  dealings in this Software without prior written authorization of the
 *  copyright holder.
 *
 *  $Id$
 */

#include "dashboard-frontend-xmlwriter.h"

#include <libxml/xmlIO.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define DASHBOARD_PORT		5913
#define DASHBOARD_TIMEOUT	200000

/*
 * Open a connection to the dashboard.  We never block and at
 * the first sign of a problem we bail.
 */
static int
dashboard_connect_with_timeout (int  *fd,
				long  timeout_usecs)
{
	struct sockaddr_in  sock;
	struct timeval      timeout;
	fd_set write_fds;

	*fd = socket (PF_INET, SOCK_STREAM, 0);
	if (*fd < 0) {
		//perror ("Dashboard: socket");
		return 0;
	}

	/*
	 * Set the socket to be non-blocking so that connect ()
	 * doesn't block.
	 */
	if (fcntl (*fd, F_SETFL, O_NONBLOCK) < 0) {
		//perror ("Dashboard: setting O_NONBLOCK");

		//if (close(*fd) < 0)
			//perror ("Dashboard: closing socket (1)");
		
		return 0;
	}

	bzero ((char *) &sock, sizeof (sock));
	sock.sin_family      = AF_INET;
	sock.sin_port        = htons (DASHBOARD_PORT);
	sock.sin_addr.s_addr = inet_addr ("127.0.0.1");

	timeout.tv_sec = 0;
	timeout.tv_usec = timeout_usecs;

	while (1) {

		/*
		 * Try to connect.
		 */
		if (connect (*fd, (struct sockaddr *) &sock,
			     sizeof (struct sockaddr_in)) < 0) {

			if (errno != EAGAIN &&
			    errno != EINPROGRESS) {
				//perror ("Dashboard: connect");

				//if (close(*fd) < 0)
					//perror ("Dashboard: closing socket (2)");

				return 0;
			}
				
		} else
			return 1;

		/*
		 * We couldn't connect, so we select on the fd and
		 * wait for the timer to run out, or for the fd to be
		 * ready.
		 */
		FD_ZERO (&write_fds);
		FD_SET (*fd, &write_fds);

		while (select (getdtablesize (), NULL, &write_fds, NULL, &timeout) < 0) {
			if (errno != EINTR) {
				//perror ("Dashboard: select");

				//if (close(*fd) < 0)
					//perror ("Dashboard: closing socket (3)");
		
				return 0;
			}
		}

		if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
			//fprintf (stderr, "Dashboard: Connection timed out.\n");

			//if (close(*fd) < 0)
				//perror ("Dashboard: closing socket (4)");
		
			return 0;
		}
		
	}

	return 1;
}

xmlTextWriterPtr
NewTextWriterDashboard (const xmlChar *frontend,
			gboolean focused,
			const xmlChar *context)
{
	xmlTextWriterPtr writer;
	xmlOutputBufferPtr buffer;
	int fd, ret;

	g_return_val_if_fail (frontend != NULL, NULL);
	g_return_val_if_fail (context != NULL, NULL);

	/* Connect */
	if (!dashboard_connect_with_timeout (&fd, DASHBOARD_TIMEOUT))
		return NULL;

	buffer = xmlOutputBufferCreateFd (fd, NULL);
	if (!buffer)
		return NULL;

	writer = xmlNewTextWriter (buffer);
	if (!writer)
		return NULL;

	ret = xmlTextWriterStartDocument (writer, NULL, NULL, NULL);
	if (ret < 0) goto error;

	ret = xmlTextWriterStartElement (writer, "CluePacket");
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, "Frontend", frontend);
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, "Context", context);
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, "Focused",
					 focused ? "true" : "false");
	if (ret < 0) goto error;

error:
	if (ret < 0) {
		xmlFreeTextWriter (writer);
		writer = NULL;
	}

	return writer;
}

int
FreeTextWriterDashboard (xmlTextWriterPtr writer)
{
	int ret;

	if (!writer)
		return -1;

	ret = xmlTextWriterEndElement (writer); /* </CluePacket> */
	if (ret < 0) goto cleanup;

	ret = xmlTextWriterEndDocument (writer);
	if (ret < 0) goto cleanup;
cleanup:

	xmlFreeTextWriter (writer);

	return ret;
}

int
DashboardSendClue (xmlTextWriterPtr writer,
		   const xmlChar *text,
		   const xmlChar *type,
		   int relevance)
{
	int ret;

	if (!writer)
		return -1;

	ret = xmlTextWriterStartElement (writer, "Clue");
	if (ret < 0) return ret;

	ret = xmlTextWriterWriteAttribute (writer, "Type", type);
	if (ret < 0) return ret;

	ret = xmlTextWriterWriteFormatAttribute (writer, "Relevance", "%d", relevance);
	if (ret < 0) return ret;

	ret = xmlTextWriterWriteString (writer, text);
	if (ret < 0) return ret;

	ret = xmlTextWriterEndElement (writer); /* </Clue> */

	return ret;
}

int
DashboardSendCluePacket (const xmlChar *frontend,
			 gboolean focused,
			 const xmlChar *context,
			 ...)
{
	xmlTextWriterPtr writer;
	int ret = 0;
	va_list args;
	xmlChar *clue_text, *clue_type;
	int clue_relevance;

	writer = NewTextWriterDashboard (frontend, focused, context);
	if (!writer)
		return -1;

	va_start (args, context);

	clue_text = va_arg (args, xmlChar *);
	while (clue_text && ret >= 0) {

		clue_type = va_arg (args, xmlChar *);
		clue_relevance = va_arg (args, int);
		ret = DashboardSendClue (writer, clue_text, clue_type, clue_relevance);

		clue_text = va_arg (args, xmlChar *);
	}

	va_end (args);

	if (ret < 0) {
		xmlFreeTextWriter (writer);
		return -1;
	}

	ret = FreeTextWriterDashboard (writer);

	return ret;
}
