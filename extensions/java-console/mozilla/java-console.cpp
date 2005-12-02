/*
 *  Copyright (C) 2005 Mathias Hasselmann
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

#include "java-console.h"

#include <nsCOMPtr.h>
#include <nsIJVMManager.h>
#include <nsIServiceManager.h>

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
