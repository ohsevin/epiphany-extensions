/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003 Christian Persch
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

#include "mozilla-helpers.h"
#include "MozAdBlocker.h"

#include <nsCOMPtr.h>
#include <nsICategoryManager.h>
#include <nsIComponentRegistrar.h>
#include <nsIGenericFactory.h>
#include <nsIServiceManagerUtils.h>
#include <nsXPCOM.h>

#include <glib.h>

NS_GENERIC_FACTORY_CONSTRUCTOR(MozAdBlocker)

/*
 * XXX evil globals :(
 *
 * I don't even know if they're necessary, I'm just copying from link-checker
 */
static gboolean is_registered = FALSE;
static nsCOMPtr<nsIGenericFactory> factory = 0;

static const nsModuleComponentInfo sAppComp =
{
	G_MOZADBLOCKER_CLASSNAME,
	G_MOZADBLOCKER_CID,
	G_MOZADBLOCKER_CONTRACTID,
	MozAdBlockerConstructor
};

extern "C" void
mozilla_register_ad_blocker (void)
{
	nsresult rv;

	g_return_if_fail (is_registered == FALSE);

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = NS_NewGenericFactory (getter_AddRefs (factory), &sAppComp);
	if (NS_FAILED (rv) || !factory)
	{
		g_warning ("Failed to make a factory for %s\n",
			   sAppComp.mDescription);
		return;
	}

	rv = cr->RegisterFactory (sAppComp.mCID, sAppComp.mDescription,
				  sAppComp.mContractID, factory);
	if (NS_FAILED (rv))
	{
		g_warning ("Failed to register %s\n", sAppComp.mDescription);
		return;
	}

	nsCOMPtr<nsICategoryManager> cm =
		do_GetService(NS_CATEGORYMANAGER_CONTRACTID);
	g_return_if_fail (cm != NULL);

	rv = cm->AddCategoryEntry("content-policy",
				  G_MOZADBLOCKER_CONTRACTID,
				  G_MOZADBLOCKER_CONTRACTID,
				  PR_FALSE, PR_FALSE, nsnull);
	if (NS_FAILED (rv))
	{
		g_warning ("Failed to register content policy\n");
		return;
	}

	is_registered = TRUE;
}

extern "C" void
mozilla_unregister_ad_blocker (void)
{
	nsresult rv;

	g_return_if_fail (is_registered == TRUE);

	nsCOMPtr<nsICategoryManager> cm =
		do_GetService(NS_CATEGORYMANAGER_CONTRACTID);
	g_return_if_fail (cm != NULL);

	rv = cm->DeleteCategoryEntry("content-policy",
				     G_MOZADBLOCKER_CONTRACTID,
				     PR_TRUE);
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = cr->UnregisterFactory (sAppComp.mCID, factory);
	g_return_if_fail (NS_SUCCEEDED (rv));

	is_registered = FALSE;
}

extern "C" void
mozilla_set_ad_blocker (AdBlocker* blocker)
{
   MozAdBlocker::SetAdBlocker (blocker);
}
