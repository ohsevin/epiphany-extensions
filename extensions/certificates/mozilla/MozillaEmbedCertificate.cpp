/*
 *  Copyright (C) 2003 Christian Persch
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "MozillaEmbedCertificate.h"

#include "ephy-debug.h"

#include <nsIChannel.h>
#include <nsISSLStatus.h>
#include <nsISSLStatusProvider.h>
#include <nsICertificateDialogs.h>
#include <nsIServiceManager.h>

MozillaEmbedCertificate::MozillaEmbedCertificate ()
{
	LOG ("MozillaEmbedCertificate ctor")
}

MozillaEmbedCertificate::~MozillaEmbedCertificate ()
{
	LOG ("MozillaEmbedCertificate dtor")
}

nsresult
MozillaEmbedCertificate::SetCertificateFromRequest (nsIRequest *request)
{
	nsCOMPtr<nsIChannel> channel = do_QueryInterface (request);
	NS_ENSURE_TRUE (channel, NS_ERROR_FAILURE);

	nsCOMPtr<nsISupports> info;
	channel->GetSecurityInfo(getter_AddRefs (info));

	nsCOMPtr<nsISSLStatusProvider> sp = do_QueryInterface (info);
	if (!sp)
	{
		mServerCert = nsnull;

		return NS_OK;
	}

	nsCOMPtr<nsISSLStatus> status;
	sp->GetSSLStatus(getter_AddRefs (status));
	NS_ENSURE_TRUE (status, NS_ERROR_FAILURE);

	status->GetServerCert (getter_AddRefs (mServerCert));
	NS_ENSURE_TRUE (mServerCert, NS_ERROR_FAILURE);

	return NS_OK;
}

nsresult
MozillaEmbedCertificate::ViewCertificate ()
{
	if (!mServerCert) return NS_OK;

	nsCOMPtr<nsICertificateDialogs> certDialogs =
		do_GetService (NS_CERTIFICATEDIALOGS_CONTRACTID);
	NS_ENSURE_TRUE (certDialogs, NS_ERROR_FAILURE);
	
	return certDialogs->ViewCert (NULL, mServerCert);
}

nsresult
MozillaEmbedCertificate::GetHasServerCertificate (PRBool *aHasServerCert)
{
	*aHasServerCert = mServerCert != nsnull;

	return NS_OK;
}
