/*
 *  Copyright (C) 2004 Adam Hooper
 *  Copyright (C) 2004 Crispin Flowerday
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Sidebar.h"
#include "ephy-debug.h"

#include <nsIGenericFactory.h>
#include <nsIComponentManager.h>
#include <nsIComponentRegistrar.h>
#include <nsICategoryManager.h>
#include <nsIServiceManager.h>
#include <nsCOMPtr.h>
#include <nsXPCOM.h>

#include <glib.h>

NS_GENERIC_FACTORY_CONSTRUCTOR(SidebarProxy)

static const nsModuleComponentInfo sAppComp =
{
	EPHY_SIDEBAR_CLASSNAME,
	EPHY_SIDEBAR_CID,
	NS_SIDEBAR_CONTRACTID,
	SidebarProxyConstructor
};

static nsCOMPtr<nsIGenericFactory> sFactory = 0;

extern "C" void
mozilla_register_component (GObject * object)
{
	nsresult rv;

	g_return_if_fail (sFactory == NULL);

	SidebarProxy::SetSignalObject (object);

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	nsCOMPtr<nsIGenericFactory> factory;
	rv = NS_NewGenericFactory (getter_AddRefs (factory),
				   &sAppComp);
	if (NS_FAILED (rv) || !factory)
	{
		g_warning ("Failed to make a factory for %s\n",
			   sAppComp.mDescription);
		g_return_if_reached();
	}

	rv = cr->RegisterFactory (sAppComp.mCID, sAppComp.mDescription,
				  sAppComp.mContractID, factory);
	if (NS_FAILED (rv))
	{
		g_warning ("Failed to register %s\n", sAppComp.mDescription);
		g_return_if_reached();
		
	}

	nsCOMPtr<nsICategoryManager> cm;
	cm = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
	g_return_if_fail (NS_SUCCEEDED (rv));

	cm->AddCategoryEntry("JavaScript global property",
			     "sidebar", NS_SIDEBAR_CONTRACTID,
			     false, true, nsnull);

	sFactory = factory;
}

extern "C" void
mozilla_unregister_component ()
{
	nsresult rv;

	g_return_if_fail (sFactory);

	SidebarProxy::SetSignalObject (NULL);

	nsCOMPtr<nsIComponentRegistrar> cr;
	rv = NS_GetComponentRegistrar (getter_AddRefs (cr));
	g_return_if_fail (NS_SUCCEEDED (rv));

	rv = cr->UnregisterFactory (sAppComp.mCID, sFactory);
	g_return_if_fail (NS_SUCCEEDED (rv));
	
	sFactory = NULL;
}
