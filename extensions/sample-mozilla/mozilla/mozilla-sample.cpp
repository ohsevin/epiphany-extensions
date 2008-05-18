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

#include "mozilla-config.h"
#include "config.h"

#include <glib.h>

#ifdef XPCOM_GLUE
#include <nsXPCOMGlue.h>
#include <gtkmozembed_glue.cpp>
#endif

#include <nsCOMPtr.h>
#include <nsIDOMEvent.h>
#include <nsIDOMMouseEvent.h>

#include "mozilla-sample.h"

void
mozilla_do_something (gpointer dom_event)
{
	nsCOMPtr<nsIDOMEvent> ev = static_cast<nsIDOMEvent*>(dom_event);
	NS_ENSURE_TRUE (ev,);
	nsCOMPtr<nsIDOMMouseEvent> mev = do_QueryInterface (ev);
	NS_ENSURE_TRUE (mev,);

	nsresult rv;
	PRUint16 button;
	rv = mev->GetButton (&button);
	NS_ENSURE_SUCCESS (rv,);

	g_print ("Button %u\n", button);
}

gboolean
mozilla_glue_startup (void)
{
#ifdef XPCOM_GLUE
	static const GREVersionRange greVersion = {
	  "1.9a", PR_TRUE,
	  "2", PR_TRUE
	};
	char xpcomLocation[4096];

	if (NS_FAILED (GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0, xpcomLocation, sizeof (xpcomLocation))) ||
	    NS_FAILED (XPCOMGlueStartup (xpcomLocation)) ||
	    NS_FAILED (GTKEmbedGlueStartup ()) ||
	    NS_FAILED (GTKEmbedGlueStartupInternal()))
                return FALSE;

        return TRUE;
#else
        return TRUE;
#endif /* XPCOM_GLUE */
}
