/*
 *  Copyright © 2003 Nat Friedman <nat@nat.org>
 *  Copyright © 2004 Christian Persch
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

#include "dashboard-frontend-xmlwriter.h"

#include <libxml/xmlIO.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
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

	memset ((char *) &sock, 0, sizeof (sock));
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

	ret = xmlTextWriterStartElement (writer, (const xmlChar*) "CluePacket");
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, (const xmlChar*) "Frontend", frontend);
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, (const xmlChar*) "Context", context);
	if (ret < 0) goto error;

	ret = xmlTextWriterWriteElement (writer, (const xmlChar*) "Focused",
					 focused ? (const xmlChar*) "true" : (const xmlChar*) "false");
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

	ret = xmlTextWriterStartElement (writer, (const xmlChar*) "Clue");
	if (ret < 0) return ret;

	ret = xmlTextWriterWriteAttribute (writer, (const xmlChar*) "Type", type);
	if (ret < 0) return ret;

	ret = xmlTextWriterWriteFormatAttribute (writer, (const xmlChar*) "Relevance", "%d", relevance);
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
