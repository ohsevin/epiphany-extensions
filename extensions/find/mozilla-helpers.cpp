/*
 *  Copyright (C) 2003 Marco Pesenti Gritti
 *  Copyright (C) 2003, 2004, 2005 Christian Persch
 *  Copyright (C) 2004 Tommi Komulainen
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

#include <epiphany/ephy-embed.h>
#include "ephy-debug.h"

#include <nsCOMPtr.h>
#include <nsIDOMWindow.h>
#include <nsIWebBrowser.h>
#include <nsIWebBrowserFocus.h>
#include <nsIServiceManager.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIPrefBranch.h>
#include <nsIPrefService.h>

#include <gtkmozembed.h>
#include <gtkmozembed_internal.h>

#define PREFNAME	"accessibility.typeaheadfind"
#define SEA_SUFFIX	"sea"

static PRBool gEnabled = PR_TRUE, gEnabledSea = PR_TRUE;
static PRUint32 gNumEnabled = 0;

extern "C" void
mozilla_embed_scroll_pages (EphyEmbed *embed,
			    gint32 pages)
{
  g_return_if_fail (EPHY_IS_EMBED (embed));

  nsCOMPtr<nsIWebBrowser> webBrowser;
  gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
				   getter_AddRefs (webBrowser));
  nsCOMPtr<nsIWebBrowserFocus> focus (do_GetInterface(webBrowser));
  NS_ENSURE_TRUE (webBrowser && focus, );

  nsresult rv;
  nsCOMPtr<nsIDOMWindow> domWin;
  rv = focus->GetFocusedWindow (getter_AddRefs (domWin));
  if (NS_FAILED (rv) || !domWin)
    {
      rv = webBrowser->GetContentDOMWindow (getter_AddRefs (domWin));
    }
  NS_ENSURE_TRUE (domWin, );

  domWin->ScrollByPages ((PRInt32) pages);
}

extern "C" void
mozilla_embed_scroll_lines (EphyEmbed *embed,
			    gint32 lines)
{
  g_return_if_fail (EPHY_IS_EMBED (embed));

  nsCOMPtr<nsIWebBrowser> webBrowser;
  gtk_moz_embed_get_nsIWebBrowser (GTK_MOZ_EMBED (embed),
				   getter_AddRefs (webBrowser));
  nsCOMPtr<nsIWebBrowserFocus> focus (do_GetInterface(webBrowser));
  NS_ENSURE_TRUE (webBrowser && focus, );

  nsresult rv;
  nsCOMPtr<nsIDOMWindow> domWin;
  rv = focus->GetFocusedWindow (getter_AddRefs (domWin));
  if (NS_FAILED (rv) || !domWin)
    {
      rv = webBrowser->GetContentDOMWindow (getter_AddRefs (domWin));
    }
  NS_ENSURE_TRUE (domWin, );

  domWin->ScrollByLines ((PRInt32) lines);
}

void
mozilla_push_prefs (void)
{
  if (gNumEnabled++ > 0) return;

  LOG ("Setting prefs");

  nsCOMPtr<nsIPrefService> prefService (do_GetService (NS_PREFSERVICE_CONTRACTID));
  g_return_if_fail (prefService);
  NS_ENSURE_TRUE (prefService, );

  nsresult rv;
  nsCOMPtr<nsIPrefBranch> prefBranch;
  rv = prefService->GetBranch ("", getter_AddRefs (prefBranch));
  g_return_if_fail (NS_SUCCEEDED (rv) && prefBranch);
  NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && prefBranch, );

  rv  = prefBranch->GetBoolPref (PREFNAME, &gEnabled);
  rv |= prefBranch->SetBoolPref (PREFNAME, PR_FALSE);
  rv |= prefBranch->GetBoolPref (PREFNAME SEA_SUFFIX, &gEnabledSea);
  rv |= prefBranch->SetBoolPref (PREFNAME SEA_SUFFIX, PR_FALSE);
  g_return_if_fail (NS_SUCCEEDED (rv));
  NS_ENSURE_SUCCESS (rv, );
}

void
mozilla_pop_prefs (void)
{
  if (--gNumEnabled > 0) return;

  LOG ("Unsetting prefs");

  nsCOMPtr<nsIPrefService> prefService (do_GetService (NS_PREFSERVICE_CONTRACTID));
  g_return_if_fail (prefService);
  NS_ENSURE_TRUE (prefService, );

  nsresult rv;
  nsCOMPtr<nsIPrefBranch> prefBranch;
  rv = prefService->GetBranch ("", getter_AddRefs (prefBranch));
  g_return_if_fail (NS_SUCCEEDED (rv) && prefBranch);
  NS_ENSURE_TRUE (NS_SUCCEEDED (rv) && prefBranch, );

  rv |= prefBranch->SetBoolPref (PREFNAME, gEnabled);
  rv |= prefBranch->SetBoolPref (PREFNAME "sea", gEnabledSea);
  g_return_if_fail (NS_SUCCEEDED (rv));
  NS_ENSURE_SUCCESS (rv, );
}
