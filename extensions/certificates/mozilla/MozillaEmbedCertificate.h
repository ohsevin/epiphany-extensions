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

#ifndef MOZILLA_EMBED_CERTIFICATE_CLASS_H
#define MOZILLA_EMBED_CERTIFICATE_CLASS_H

#include <nsCOMPtr.h>
#include <nsIRequest.h>
#include <nsIX509Cert.h>

class MozillaEmbedCertificate
{
public:
	MozillaEmbedCertificate();
	virtual ~MozillaEmbedCertificate();

	nsresult SetCertificateFromRequest (nsIRequest *request);
	nsresult ViewCertificate ();
	nsresult GetHasServerCertificate (PRBool *aHasServerCert);

private:
	nsCOMPtr<nsIX509Cert> mServerCert;
};

#endif
