/*
 *  Copyright © 2005 Mathias Hasselmann
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


#ifdef XPCOM_GLUE
#include <nsXPCOMGlue.h>
#include <gtkmozembed_glue.cpp>
#endif

#include <nsCOMPtr.h>
#include <nsIJVMManager.h>
#include <nsServiceManagerUtils.h>

#include "java-console.h"

gboolean
java_console_is_available (void)
{
	nsCOMPtr<nsIJVMManager> jvmmgr = do_GetService ("@mozilla.org/oji/jvm-mgr;1");
	NS_ENSURE_TRUE (jvmmgr, false);

	PRBool javaEnabled = PR_FALSE;
	nsresult rv = jvmmgr->GetJavaEnabled (&javaEnabled);
	NS_ENSURE_SUCCESS (rv, FALSE);

	return javaEnabled;
}

void	
java_console_show (void)
{
	nsCOMPtr<nsIJVMManager> jvmmgr = do_GetService ("@mozilla.org/oji/jvm-mgr;1");
	NS_ENSURE_TRUE (jvmmgr, );

	jvmmgr->ShowJavaConsole ();
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
