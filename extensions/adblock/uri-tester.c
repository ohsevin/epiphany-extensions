/*
 *  Copyright © 2011 Igalia S.L.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  Some parts of this file based on the Midori's 'adblock' extension,
 *  licensed with the GNU Lesser General Public License 2.1, Copyright
 *  (C) 2009-2010 Christian Dywan <christian@twotoasts.de> and 2009
 *  Alexander Butenko <a.butenka@gmail.com>. Check Midori's web site
 *  at http://www.twotoasts.de
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "uri-tester.h"

#include "ephy-debug.h"
#include "ephy-file-helpers.h"

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>
#include <webkit/webkit.h>

#define DEFAULT_FILTER_URL "http://adblockplus.mozdev.org/easylist/easylist.txt"
#define FILTERS_LIST_FILENAME "filters.list"
#define SIGNATURE_SIZE 8
#define UPDATE_FREQUENCY 24 * 60 * 60 /* In seconds */

#define URI_TESTER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_URI_TESTER, UriTesterPrivate))

struct _UriTesterPrivate
{
  GSList *filters;

  GHashTable* pattern;
  GHashTable* keys;
  GHashTable* optslist;
  GHashTable* urlcache;
};

enum
{
  PROP_0,
  PROP_FILTERS,
};

G_DEFINE_DYNAMIC_TYPE (UriTester, uri_tester, G_TYPE_OBJECT);

/* Private functions. */

static void uri_tester_class_init (UriTesterClass *klass);
static void uri_tester_init (UriTester *dialog);

static char *
uri_tester_fixup_regexp (char* src);

static gboolean
uri_tester_parse_file_at_uri (UriTester* tester, const char* fileuri);

static char *
uri_tester_ensure_data_dir ()
{
  char* folder = NULL;

  /* Ensure adblock's dir is there. */
  folder = g_build_filename (ephy_dot_dir (), "extensions", "data", "adblock", NULL);
  g_mkdir_with_parents (folder, 0700);

  return folder;
}

static char*
uri_tester_get_fileuri_for_url (const char* url)
{
  char* filename = NULL;
  char* folder = NULL;
  char* path = NULL;
  char* uri = NULL;

  if (!strncmp (url, "file", 4))
    return g_strndup (url + 7, strlen (url) - 7);

  folder = uri_tester_ensure_data_dir ();
  filename = g_compute_checksum_for_string (G_CHECKSUM_MD5, url, -1);

  path = g_build_filename (folder, filename, NULL);
  uri = g_filename_to_uri (path, NULL, NULL);

  g_free (filename);
  g_free (path);
  g_free (folder);

  return uri;
}

static void
uri_tester_download_notify_status_cb (WebKitDownload *download,
                                      GParamSpec *pspec,
                                      UriTester *tester)
{
  const char *dest = NULL;

  if (webkit_download_get_status (download) != WEBKIT_DOWNLOAD_STATUS_FINISHED)
    return;

  LOG ("Download from %s to %s completed",
       webkit_download_get_source_uri (download),
       webkit_download_get_destination_uri (download));

  /* Parse the file from disk. */
  dest = webkit_download_get_destination_uri (download);
  uri_tester_parse_file_at_uri (tester, dest);
}

static void
uri_tester_retrieve_filter (UriTester *tester, const char *url, const char *fileuri)
{
  WebKitNetworkRequest* request = NULL;
  WebKitDownload* download = NULL;

  g_return_if_fail (IS_URI_TESTER (tester));
  g_return_if_fail (url != NULL);
  g_return_if_fail (fileuri != NULL);

  request = webkit_network_request_new (url);
  download = webkit_download_new (request);
  g_object_unref (request);

  webkit_download_set_destination_uri (download, fileuri);

  g_signal_connect (download, "notify::status",
                    G_CALLBACK (uri_tester_download_notify_status_cb), tester);

  webkit_download_start (download);
}

static gboolean
uri_tester_filter_is_valid (const char *fileuri)
{
  GFile *file = NULL;
  GFileInfo *file_info = NULL;
  gboolean result;

  /* Now check if the local file is too old. */
  file = g_file_new_for_uri (fileuri);
  file_info = g_file_query_info (file,
                                 G_FILE_ATTRIBUTE_TIME_MODIFIED,
                                 G_FILE_QUERY_INFO_NONE,
                                 NULL,
                                 NULL);
  result = FALSE;
  if (file_info)
    {
      GTimeVal current_time;
      GTimeVal mod_time;

      g_get_current_time (&current_time);
      g_file_info_get_modification_time (file_info, &mod_time);

      if (current_time.tv_sec > mod_time.tv_sec)
        {
          gint64 expire_time = mod_time.tv_sec + UPDATE_FREQUENCY;
          result = current_time.tv_sec < expire_time;
        }
      g_object_unref (file_info);
    }

  g_object_unref (file);

  return result;
}

static void
uri_tester_load_patterns (UriTester *tester)
{
  GSList *filter = NULL;
  char *url = NULL;
  char *fileuri = NULL;

  /* Load patterns from the list of filters. */
  for (filter = tester->priv->filters; filter; filter = g_slist_next(filter))
    {
      url = (char*)filter->data;
      fileuri = uri_tester_get_fileuri_for_url (url);

      if (!uri_tester_filter_is_valid (fileuri))
        uri_tester_retrieve_filter (tester, url, fileuri);
      else
        uri_tester_parse_file_at_uri (tester, fileuri);

      g_free (fileuri);
    }
}

static void
uri_tester_load_filters (UriTester *tester)
{
  GSList *list = NULL;
  char *data_dir = NULL;
  char *filepath = NULL;

  data_dir =  uri_tester_ensure_data_dir ();
  filepath = g_build_filename (data_dir, FILTERS_LIST_FILENAME, NULL);

  if (g_file_test (filepath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
    {
      GFile* file = NULL;
      char *contents = NULL;
      gsize length = 0;
      GError *error = NULL;

      file = g_file_new_for_path (filepath);
      if (g_file_load_contents (file, NULL, &contents, &length, NULL, &error))
        {
          char **urls_array = NULL;
          char *url = NULL;
          int i = 0;

          urls_array = g_strsplit (contents, ";", -1);
          for (i = 0; urls_array [i]; i++)
            {
              url = g_strstrip (g_strdup (urls_array[i]));
              if (!g_str_equal (url, ""))
                list = g_slist_prepend (list, url);
            }
          g_strfreev (urls_array);

          g_free (contents);
        }

      if (error)
        {
          LOG ("Error loading filters from %s: %s", filepath, error->message);
          g_error_free (error);
        }

      g_object_unref (file);
    }
  else
    {
      /* No file exists yet, so use the default filter and save it. */
      list = g_slist_prepend (list, g_strdup (DEFAULT_FILTER_URL));
    }

  g_free (filepath);

  uri_tester_set_filters (tester, g_slist_reverse(list));
}

static void
uri_tester_save_filters (UriTester *tester)
{
  FILE* file = NULL;
  char *data_dir = NULL;
  char *filepath = NULL;

  data_dir =  uri_tester_ensure_data_dir ();
  filepath = g_build_filename (data_dir, FILTERS_LIST_FILENAME, NULL);

  if ((file = g_fopen (filepath, "w")))
    {
      GSList *item = NULL;
      char *filter = NULL;

      for (item = tester->priv->filters; item; item = g_slist_next (item))
        {
          filter = g_strdup_printf ("%s;", (char*)item->data);
          fputs (filter, file);
          g_free (filter);
        }
      fclose (file);
    }
  g_free (filepath);
}

static inline gboolean
uri_tester_check_filter_options (GRegex*       regex,
                                 const char*  opts,
                                 const char*  req_uri,
                                 const char*  page_uri)
{
  if (g_regex_match_simple (",third-party", opts,
                            G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY))
    {
      if (page_uri && g_regex_match_full (regex, page_uri, -1, 0, 0, NULL, NULL))
        return TRUE;
    }

  return FALSE;
}

static inline gboolean
uri_tester_is_matched_by_pattern (UriTester *tester,
                                  const char*  req_uri,
                                  const char*  page_uri)
{
  GHashTableIter iter;
  gpointer patt, regex;
  char* opts;

  g_hash_table_iter_init (&iter, tester->priv->pattern);
  while (g_hash_table_iter_next (&iter, &patt, &regex))
    {
      if (g_regex_match_full (regex, req_uri, -1, 0, 0, NULL, NULL))
        {
          opts = g_hash_table_lookup (tester->priv->optslist, patt);
          if (opts && uri_tester_check_filter_options (regex, opts, req_uri, page_uri) == TRUE)
            return FALSE;
          else
            {
              LOG ("blocked by pattern regexp=%s -- %s", g_regex_get_pattern (regex), req_uri);
              return TRUE;
            }
        }
    }
  return FALSE;
}

static inline gboolean
uri_tester_is_matched_by_key (UriTester *tester,
                              const char*  opts,
                              const char*  req_uri,
                              const char*  page_uri)
{
  UriTesterPrivate* priv = NULL;
  char* uri;
  int len;
  int pos = 0;
  GList* regex_bl = NULL;

  priv = tester->priv;

  uri = uri_tester_fixup_regexp ((char*)req_uri);
  len = strlen (uri);
  for (pos = len - SIGNATURE_SIZE; pos >= 0; pos--)
    {
      char* sig = g_strndup (uri + pos, SIGNATURE_SIZE);
      GRegex* regex = g_hash_table_lookup (priv->keys, sig);
      char* opts = NULL;

      if (regex && !g_list_find (regex_bl, regex))
        {
          if (g_regex_match_full (regex, req_uri, -1, 0, 0, NULL, NULL))
            {
              opts = g_hash_table_lookup (tester->priv->optslist, sig);
              g_free (sig);
              if (opts && uri_tester_check_filter_options (regex, opts, req_uri, page_uri))
                {
                  g_free (uri);
                  g_list_free (regex_bl);
                  return FALSE;
                }
              else
                {
                  LOG ("blocked by regexp=%s -- %s", g_regex_get_pattern (regex), uri);
                  g_free (uri);
                  g_list_free (regex_bl);
                  return TRUE;
                }
            }
          regex_bl = g_list_prepend (regex_bl, regex);
        }
      g_free (sig);
    }
  g_free (uri);
  g_list_free (regex_bl);
  return FALSE;
}

static gboolean
uri_tester_is_matched (UriTester *tester,
                       const char*  opts,
                       const char*  req_uri,
                       const char*  page_uri)
{
  UriTesterPrivate* priv = NULL;
  gboolean foundbykey;
  gboolean foundbypattern;
  char* value;

  priv = tester->priv;

  /* Check cached URLs first. */
  if ((value = g_hash_table_lookup (priv->urlcache, req_uri)))
    return (value[0] != '0') ? TRUE : FALSE;

  /* Look for a match either by key or by pattern. */
  foundbykey = uri_tester_is_matched_by_key (tester, opts, req_uri, page_uri);
  foundbypattern = uri_tester_is_matched_by_pattern (tester, req_uri, page_uri);

  if (foundbykey || foundbypattern)
    {
      g_hash_table_insert (priv->urlcache, g_strdup (req_uri), g_strdup("1"));
      return TRUE;
    }
  g_hash_table_insert (priv->urlcache, g_strdup (req_uri), g_strdup("0"));
  return FALSE;
}

static char *
uri_tester_fixup_regexp (char* src)
{
  char* dst;
  GString* str;
  int len;

  if (!src)
    return NULL;

  str = g_string_new ("");

  /* Lets strip first .* */
  if (src[0] == '*')
    {
      (void)*src++;
    }

  do
    {
      switch (*src)
        {
        case '*':
          g_string_append (str, ".*");
          break;
        /* case '.': */
        /*   g_string_append (str, "\\."); */
        /*   break; */
        case '?':
          g_string_append (str, "\\?");
          break;
        case '|':
          g_string_append (str, "");
          break;
          /* FIXME: We actually need to match :[0-9]+ or '/'. Sign
           * means "here could be port number or nothing". So bla.com^
           * will match bla.com/ or bla.com:8080/ but not bla.com.au/.
           */
        case '^':
          g_string_append (str, "");
          break;
        case '+':
          break;
        default:
          g_string_append_printf (str,"%c", *src);
          break;
        }
      src++;
    }
  while (*src);

  dst = g_strdup (str->str);
  g_string_free (str, TRUE);

  /* We dont need .* in the end of url. Thats stupid. */
  len = strlen (dst);
  if (dst && dst[len-1] == '*' && dst[len-2] == '.')
    {
      dst[len-2] = '\0';
    }
  return dst;
}

static void
uri_tester_compile_regexp (UriTester* tester,
                           char*      patt,
                           char*      opts)
{
  GRegex* regex;
  GError* error = NULL;
  int pos = 0;
  char *sig;

  regex = g_regex_new (patt, G_REGEX_OPTIMIZE,
                       G_REGEX_MATCH_NOTEMPTY, &error);
  if (error)
    {
      g_warning ("%s: %s", G_STRFUNC, error->message);
      g_error_free (error);
      return;
    }

  if (!g_regex_match_simple ("^/.*[\\^\\$\\*].*/$", patt, G_REGEX_UNGREEDY, G_REGEX_MATCH_NOTEMPTY))
    {
      int len = strlen (patt);
      int signature_count = 0;
      for (pos = len - SIGNATURE_SIZE; pos >= 0; pos--) {
        sig = g_strndup (patt + pos, SIGNATURE_SIZE);
        if (!g_regex_match_simple ("[\\*]", sig, G_REGEX_UNGREEDY, G_REGEX_MATCH_NOTEMPTY) &&
            !g_hash_table_lookup (tester->priv->keys, sig))
          {
            LOG ("sig: %s %s", sig, patt);
            g_hash_table_insert (tester->priv->keys, g_strdup (sig), g_regex_ref (regex));
            g_hash_table_insert (tester->priv->optslist, g_strdup (sig), g_strdup (opts));
            signature_count++;
          }
        else
          {
            if (g_regex_match_simple ("^\\*", sig, G_REGEX_UNGREEDY, G_REGEX_MATCH_NOTEMPTY) &&
                !g_hash_table_lookup (tester->priv->pattern, patt))
              {
                LOG ("patt2: %s %s", sig, patt);
                g_hash_table_insert (tester->priv->pattern, g_strdup (patt), g_regex_ref (regex));
                g_hash_table_insert (tester->priv->optslist, g_strdup (patt), g_strdup (opts));
              }
          }
        g_free (sig);
      }
      if (signature_count > 1 && g_hash_table_lookup (tester->priv->pattern, opts))
        g_hash_table_steal (tester->priv->pattern, patt);
    }
  else
    {
      LOG ("patt: %s%s", patt, "");
      /* Pattern is a regexp chars. */
      g_hash_table_insert (tester->priv->pattern, g_strdup (patt), g_regex_ref (regex));
      g_hash_table_insert (tester->priv->optslist, g_strdup (patt), g_strdup (opts));
    }

  g_regex_unref (regex);
}

static char*
uri_tester_add_url_pattern (UriTester* tester,
                            char* format,
                            char* type,
                            char* line)
{
  char** data;
  char* patt;
  char* fixed_patt;
  char* format_patt;
  char* opts;

  data = g_strsplit (line, "$", -1);
  if (data && data[0] && data[1] && data[2])
    {
      patt = g_strdup_printf ("%s%s", data[0], data[1]);
      opts = g_strdup_printf ("type=%s,regexp=%s,%s", type, patt, data[2]);
    }
  else if (data && data[0] && data[1])
    {
      patt = g_strdup (data[0]);
      opts = g_strdup_printf ("type=%s,regexp=%s,%s", type, patt, data[1]);
    }
  else
    {
      patt = g_strdup (data[0]);
      opts = g_strdup_printf ("type=%s,regexp=%s", type, patt);
    }

  fixed_patt = uri_tester_fixup_regexp (patt);
  format_patt =  g_strdup_printf (format, fixed_patt);

  LOG ("got: %s opts %s", format_patt, opts);
  uri_tester_compile_regexp (tester, format_patt, opts);

  g_strfreev (data);
  g_free (patt);
  g_free (fixed_patt);
  return format_patt;
}

static char*
uri_tester_parse_line (UriTester* tester, char* line)
{
  if (!line)
    return NULL;
  g_strchomp (line);

  /* Ignore comments and new lines. */
  if (line[0] == '!')
    return NULL;

  /* FIXME: No support for whitelisting. */
  if (line[0] == '@' && line[1] == '@')
    return NULL;

  /* FIXME: No support for [include] and [exclude] tags. */
  if (line[0] == '[')
    return NULL;

  /* Got CSS block hider. */
  if (line[0] == '#' && line[1] == '#' )
    {
      return NULL;
    }
  /* Got CSS block hider. Workaround. */
  if (line[0] == '#')
    return NULL;

  /* Got per domain CSS hider rule. */
  if (strstr (line, "##"))
    {
      return NULL;
    }

  /* Got per domain CSS hider rule. Workaround. */
  if (strchr (line, '#'))
    {
      return NULL;
    }
  /* Got URL blocker rule. */
  if (line[0] == '|' && line[1] == '|' )
    {
      (void)*line++;
      (void)*line++;
      return uri_tester_add_url_pattern (tester, "%s", "fulluri", line);
    }
  if (line[0] == '|')
    {
      (void)*line++;
      return uri_tester_add_url_pattern (tester, "^%s", "fulluri", line);
    }
  return uri_tester_add_url_pattern (tester, "%s", "uri", line);
}

static gboolean
uri_tester_parse_file_at_uri (UriTester* tester, const char* fileuri)
{
  FILE* file;
  char line[2000];
  char *path = NULL;
  gboolean result = FALSE;

  path = g_filename_from_uri (fileuri, NULL, NULL);
  if ((file = g_fopen (path, "r")))
    {
      while (fgets (line, 2000, file))
        g_free (uri_tester_parse_line (tester, line));
      fclose (file);

      result = TRUE;
    }
  g_free (path);

  return result;
}

static void
uri_tester_init (UriTester *tester)
{
  UriTesterPrivate* priv = NULL;

  LOG ("UriTester initializing %p", tester);

  priv = URI_TESTER_GET_PRIVATE (tester);
  tester->priv = priv;

  priv->filters = NULL;
  priv->pattern = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         (GDestroyNotify)g_free,
                                         (GDestroyNotify)g_regex_unref);
  priv->keys = g_hash_table_new_full (g_str_hash, g_str_equal,
                                      (GDestroyNotify)g_free,
                                      (GDestroyNotify)g_regex_unref);
  priv->optslist = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          NULL,
                                          (GDestroyNotify)g_free);
  priv->urlcache = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          (GDestroyNotify)g_free,
                                          (GDestroyNotify)g_free);

  uri_tester_load_filters (tester);
  uri_tester_load_patterns (tester);
}

static void
uri_tester_set_property (GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  UriTester *tester = URI_TESTER (object);

  switch (prop_id)
    {
    case PROP_FILTERS:
      uri_tester_set_filters (tester, (GSList*) g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
uri_tester_finalize (GObject *object)
{
  UriTesterPrivate *priv = URI_TESTER_GET_PRIVATE (URI_TESTER (object));

  LOG ("UriTester finalizing %p", object);

  g_slist_foreach (priv->filters, (GFunc) g_free, NULL);
  g_slist_free (priv->filters);

  g_hash_table_destroy (priv->pattern);
  g_hash_table_destroy (priv->keys);
  g_hash_table_destroy (priv->optslist);
  g_hash_table_destroy (priv->urlcache);

  G_OBJECT_CLASS (uri_tester_parent_class)->finalize (object);
}

static void
uri_tester_class_init (UriTesterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = uri_tester_set_property;
  object_class->finalize = uri_tester_finalize;

  g_object_class_install_property
    (object_class,
     PROP_FILTERS,
     g_param_spec_pointer ("filters",
                           "filters",
                           "filters",
                           G_PARAM_WRITABLE));

  g_type_class_add_private (object_class, sizeof (UriTesterPrivate));
}

static void
uri_tester_class_finalize (UriTesterClass *klass)
{
}

/* Public functions. */

void uri_tester_register (GTypeModule *module)
{
  uri_tester_register_type (module);
}

UriTester *
uri_tester_new (void)
{
  return g_object_new (TYPE_URI_TESTER, NULL);
}

gboolean
uri_tester_test_uri (UriTester *tester,
                     const char *req_uri,
                     const char *page_uri,
                     AdUriCheckType type)
{
  /* Don't block top level documents. */
  if (type == AD_URI_CHECK_TYPE_DOCUMENT)
    return FALSE;

  return uri_tester_is_matched (tester, NULL, req_uri, page_uri);
}

void
uri_tester_set_filters (UriTester *tester, GSList *filters)
{
  UriTesterPrivate *priv = tester->priv;

  if (priv->filters)
    {
      g_slist_foreach (priv->filters, (GFunc) g_free, NULL);
      g_slist_free (priv->filters);
    }

  /* Update private variable and save to disk. */
  priv->filters = filters;
  uri_tester_save_filters (tester);
}

GSList *
uri_tester_get_filters (UriTester *tester)
{
  return tester->priv->filters;
}

void
uri_tester_reload (UriTester *tester)
{
  GDir *g_data_dir = NULL;
  const char *data_dir = NULL;

  /* Remove data files in the data dir first. */
  data_dir = uri_tester_ensure_data_dir ();

  g_data_dir = g_dir_open (data_dir, 0, NULL);
  if (g_data_dir)
    {
      const char *filename = NULL;
      char *filepath = NULL;

      while ((filename = g_dir_read_name (g_data_dir)))
        {
          /* Omit the list of filters. */
          if (!g_strcmp0 (filename, FILTERS_LIST_FILENAME))
            continue;

          filepath = g_build_filename (data_dir, filename, NULL);
          g_unlink (filepath);

          g_free (filepath);
        }

      g_dir_close (g_data_dir);
    }

  /* Load patterns from current filters. */
  uri_tester_load_patterns (tester);
}
