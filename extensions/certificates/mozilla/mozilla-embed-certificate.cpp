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

#include "mozilla-embed-certificate.h"
#include "MozillaEmbedCertificate.h"

#include <nsCOMPtr.h>
#include <nsIRequest.h>

#define DATA_KEY "EphyCertificateExtension::MozillaEmbedCertificate"

static void embed_security_change_cb (GObject *embed_object, 
				      gpointer request_ptr,
				      guint state,
				      gpointer dummy)
{
	MozillaEmbedCertificate *cert;

	cert = (MozillaEmbedCertificate *) g_object_get_data (embed_object, DATA_KEY);
	if (!cert) return;

	nsCOMPtr<nsIRequest> request = static_cast<nsIRequest*>(request_ptr);
	NS_ENSURE_TRUE (request,);

	cert->SetCertificateFromRequest (request);
}

static void
delete_cert (gpointer cert_ptr)
{
	MozillaEmbedCertificate *cert = (MozillaEmbedCertificate *) cert_ptr;

	g_return_if_fail (cert != NULL);

	delete cert;
}

extern "C" void
mozilla_embed_certificate_attach (EphyEmbed *embed)
{
	GObject *embed_object = G_OBJECT (embed);
	MozillaEmbedCertificate *cert;

	cert = (MozillaEmbedCertificate *) g_object_get_data (embed_object, DATA_KEY);
	if (!cert)
	{
		cert = new MozillaEmbedCertificate ();

		g_object_set_data_full (embed_object, DATA_KEY, cert,
					(GDestroyNotify) delete_cert);
	}

	g_signal_connect_object (G_OBJECT (embed), "security_change",
				 G_CALLBACK (embed_security_change_cb),
				 NULL, (GConnectFlags) 0);
}

extern "C" void
mozilla_embed_view_certificate (EphyEmbed *embed)
{
	GObject *embed_object = G_OBJECT (embed);
	MozillaEmbedCertificate *cert;

	cert = (MozillaEmbedCertificate *) g_object_get_data (embed_object, DATA_KEY);
	if (!cert) return;

	cert->ViewCertificate ();
}

extern "C" gboolean
mozilla_embed_has_certificate (EphyEmbed *embed)
{
	GObject *embed_object = G_OBJECT (embed);
	MozillaEmbedCertificate *cert;

	cert = (MozillaEmbedCertificate *) g_object_get_data (embed_object, DATA_KEY);
	if (!cert) return FALSE;

	PRBool hasCert;
	cert->GetHasServerCertificate (&hasCert);

	return (gboolean) hasCert;
}