/*
 *  Copyright (C) 2002 Philip Langdale
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
 * $Id$
 */

#include "mozilla-config.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Sidebar.h"
#include "ephy-debug.h"

#include <nsCOMPtr.h>
#include <nsCRT.h>
#include <nsIDOMWindowInternal.h>
#include <nsIProgrammingLanguage.h>
#include <nsEmbedString.h>

#include <glib.h>

NS_IMPL_ISUPPORTS2(SidebarProxy, nsISidebar, nsIClassInfo)

GObject * SidebarProxy::mSignalObject = NULL;

SidebarProxy::SidebarProxy()
{
	LOG ("SidebarProxy ctor(%p)", this);
}

SidebarProxy::~SidebarProxy()
{
	LOG ("SidebarProxy dtor(%p)", this);
}

void
SidebarProxy::SetSignalObject (GObject *aObject)
{
	mSignalObject = aObject;
}

/* void addPanel (in wstring aTitle, in string aContentURL, in string aCustomizeURL); */
NS_IMETHODIMP 
SidebarProxy::AddPanel (const PRUnichar *aTitle,
			const char *aContentURL, 
			const char *aCustomizeURL)
{
	nsEmbedCString title;

	NS_UTF16ToCString (nsEmbedString(aTitle),
			   NS_CSTRING_ENCODING_UTF8, title);

	if (mSignalObject)
	{
		g_signal_emit_by_name (mSignalObject, "add-sidebar",
				       aContentURL, title.get());
	}

	return NS_OK;
}

/* void addPersistentPanel (in wstring aTitle, in string aContentURL, in string aCustomizeURL); */
NS_IMETHODIMP 
SidebarProxy::AddPersistentPanel (const PRUnichar *aTitle,
				  const char *aContentURL, 
				  const char *aCustomizeURL)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addSearchEngine (in string engineURL, in string iconURL, in wstring suggestedTitle, in wst
ring suggestedCategory); */
NS_IMETHODIMP
SidebarProxy::AddSearchEngine (const char *engineURL,
			       const char *iconURL, 
			       const PRUnichar *suggestedTitle, 
			       const PRUnichar *suggestedCategory)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}


/* -----------------------------------------------------------------------------
 * nsIClassInfo Impl.
 * -----------------------------------------------------------------------------
 */

/* void getInterfaces (out PRUint32 count, [array, size_is (count), retval] out nsIIDPtr array); */
NS_IMETHODIMP SidebarProxy::GetInterfaces (PRUint32 *aCount,
					   nsIID * **aArray)
{
	PRUint32 count = 2;

	*aCount = count;

	*aArray = NS_STATIC_CAST(nsIID **, nsMemory::Alloc(count * sizeof(nsIID *)));
	NS_ENSURE_TRUE(*aArray, NS_ERROR_OUT_OF_MEMORY);

	nsIID *iid = NS_STATIC_CAST(nsIID *,
				    nsMemory::Clone(&(NS_GET_IID(nsISidebar)),
				    sizeof(nsIID)));
	if (!iid)
	{
		NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(0, *aArray);

		return NS_ERROR_OUT_OF_MEMORY;
	}
	(*aArray)[0] = iid;

	iid = NS_STATIC_CAST(nsIID *,
			     nsMemory::Clone(&(NS_GET_IID(nsIClassInfo)),
			     sizeof(nsIID)));
	if (!iid)
	{
		NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(1, *aArray);

		return NS_ERROR_OUT_OF_MEMORY;
	}
	(*aArray)[1] = iid;

	return NS_OK;
}

/* nsISupports getHelperForLanguage (in PRUint32 language); */
NS_IMETHODIMP SidebarProxy::GetHelperForLanguage (PRUint32 language,
						  nsISupports **_retval)
{
	*_retval = nsnull;
	return NS_OK;
}

/* readonly attribute string contractID; */
NS_IMETHODIMP SidebarProxy::GetContractID (char **aContractID)
{
	*aContractID = nsCRT::strdup (NS_SIDEBAR_CONTRACTID);
	return NS_OK;
}

/* readonly attribute string classDescription; */
NS_IMETHODIMP SidebarProxy::GetClassDescription (char **aClassDescription)
{
	*aClassDescription = nsCRT::strdup("Sidebar");
	return NS_OK;
}

/* readonly attribute nsCIDPtr classID; */
NS_IMETHODIMP SidebarProxy::GetClassID (nsCID **aClassID)
{
	*aClassID = nsnull;
	return NS_OK;
}

/* readonly attribute PRUint32 implementationLanguage; */
NS_IMETHODIMP SidebarProxy::GetImplementationLanguage (PRUint32 *aImplementationLanguage)
{
	*aImplementationLanguage = nsIProgrammingLanguage::CPLUSPLUS;
	return NS_OK;
}

/* readonly attribute PRUint32 flags; */
NS_IMETHODIMP SidebarProxy::GetFlags (PRUint32 *aFlags)
{
	*aFlags = nsIClassInfo::DOM_OBJECT;
	return NS_OK;
}

/* [notxpcom] readonly attribute nsCID classIDNoAlloc; */
NS_IMETHODIMP SidebarProxy::GetClassIDNoAlloc (nsCID *aClassIDNoAlloc)
{
	return NS_ERROR_NOT_AVAILABLE;
}
