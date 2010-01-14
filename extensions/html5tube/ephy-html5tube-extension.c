/*
 *  Copyright © 2010 Gustavo Noronha Silva
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
 */

#include "config.h"

#include "ephy-html5tube-extension.h"
#include "ephy-debug.h"

#include <epiphany/epiphany.h>
#include <JavaScriptCore/JavaScript.h>

#include <gmodule.h>

static GObjectClass *parent_class = NULL;

static GType type = 0;

static void
ephy_html5tube_extension_init (EphyHTML5TubeExtension *extension)
{
  LOG ("EphyHTML5TubeExtension initialising");
}

static void
ephy_html5tube_extension_finalize (GObject *object)
{
  LOG ("EphyHTML5TubeExtension finalising");

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
impl_attach_window (EphyExtension *ext,
		    EphyWindow *window)
{
  LOG ("EphyHTML5TubeExtension attach_window");
}

static void
impl_detach_window (EphyExtension *ext,
		    EphyWindow *window)
{
  LOG ("EphyHTML5TubeExtension detach_window");
}

/* These utility functions were copied from ephy-web-view.c */
static JSValueRef
js_object_get_property (JSContextRef js_context,
                        JSObjectRef js_object,
                        const char *name)
{
  JSStringRef js_string = JSStringCreateWithUTF8CString (name);
  JSValueRef js_value = JSObjectGetProperty (js_context, js_object, js_string, NULL);

  JSStringRelease (js_string);

  return js_value;
}

static JSObjectRef
js_object_get_property_as_object (JSContextRef js_context,
                                  JSObjectRef object,
                                  const char *attr)
{
  JSValueRef value = js_object_get_property (js_context, object, attr);

  if (!JSValueIsObject (js_context, value))
    return NULL;

  return JSValueToObject (js_context,
                          value,
                          NULL);
}

static char*
js_value_to_string (JSContextRef js_context,
                    JSValueRef js_value)
{
  gssize length;
  char* buffer;
  JSStringRef str;

  g_return_val_if_fail (JSValueIsString (js_context, js_value), NULL);

  str = JSValueToStringCopy (js_context, js_value, NULL);
  length = JSStringGetLength (str) + 1;

  buffer = g_malloc0 (length);
  JSStringGetUTF8CString (str, buffer, length);
  JSStringRelease (str);

  return buffer;
}

static void
replace_flash_object (WebKitWebView *web_view)
{
  WebKitWebFrame *main_frame;
  JSContextRef js_context;
  JSStringRef js_string;
  JSObjectRef js_global;
  JSObjectRef js_document;
  JSObjectRef js_object;
  JSObjectRef js_parent_element;
  JSValueRef js_flash_element;
  JSObjectRef js_flash_object;
  JSObjectRef js_flash_attributes;
  JSValueRef js_flash_vars;
  JSObjectRef js_flash_vars_object;
  JSValueRef js_flash_vars_value;
  JSValueRef js_video_element;
  JSObjectRef js_video_object;
  JSValueRef js_arguments[2];
  JSStringRef prop_name;
  JSValueRef prop_value;
  char *flashvars_string;
  GHashTable *flashvars_parameters;
  char *video_id;
  char *video_hash;
  char *video_uri;

  main_frame = webkit_web_view_get_main_frame (web_view);

  js_context =  (JSContextRef) webkit_web_frame_get_global_context (main_frame);

  js_global = JSContextGetGlobalObject (js_context);

  js_document = js_object_get_property_as_object (js_context,
                                                  js_global,
                                                  "document");

  if (!js_document)
    return;

  js_object = js_object_get_property_as_object (js_context,
                                                js_document,
                                                "getElementById");

  if (!js_object)
    return;

  /* First check whether youtube believes flash is supported. */
  js_string = JSStringCreateWithUTF8CString ("watch-noplayer-div");
  js_arguments[0] = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  js_flash_element = JSObjectCallAsFunction (js_context,
                                             js_object,
                                             js_document,
                                             1, js_arguments,
                                             NULL);

  /* If a watch-noplayer-div element exists, youtube thinks flash is not available,
   * let's fool it
   */
  if (js_flash_element && JSValueIsObject (js_context, js_flash_element))
      webkit_web_view_execute_script (web_view, "yt.www.watch.player.write('watch-player-div', true, true, null, '0', '0');");

  /* FLASH ELEMENT */
  js_string = JSStringCreateWithUTF8CString ("movie_player");
  js_arguments[0] = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  js_flash_element = JSObjectCallAsFunction (js_context,
                                             js_object,
                                             js_document,
                                             1, js_arguments,
                                             NULL);

  if (!js_flash_element || !JSValueIsObject (js_context, js_flash_element))
    return;

  js_flash_object = JSValueToObject (js_context, js_flash_element, NULL);

  js_flash_attributes = js_object_get_property_as_object (js_context, js_flash_object, "attributes");
  if (!js_flash_attributes)
    return;

  js_object = js_object_get_property_as_object (js_context, js_flash_attributes, "getNamedItem");
  if (!js_object)
    return;

  js_string = JSStringCreateWithUTF8CString ("flashvars");
  js_arguments[0] = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  js_flash_vars = JSObjectCallAsFunction (js_context,
                                          js_object,
                                          js_flash_attributes,
                                          1, js_arguments,
                                          NULL);

  if (!js_flash_vars || !JSValueIsObject (js_context, js_flash_vars))
    return;

  js_flash_vars_object = JSValueToObject (js_context, js_flash_vars, NULL);

  js_flash_vars_value = js_object_get_property (js_context, js_flash_vars_object, "value");

  if (!js_flash_vars_value || !JSValueIsString (js_context, js_flash_vars_value))
    return;

  flashvars_string = js_value_to_string (js_context, js_flash_vars_value);
  if (!flashvars_string)
    return;

  flashvars_parameters = soup_form_decode (flashvars_string);
  if (!flashvars_parameters) {
    g_free (flashvars_string);
    return;
  }

  video_id = g_hash_table_lookup (flashvars_parameters, "video_id");
  if (!video_id) {
    g_hash_table_destroy (flashvars_parameters);
    return;
  }

  video_hash = g_hash_table_lookup (flashvars_parameters, "t");
  if (!video_hash) {
    g_hash_table_destroy (flashvars_parameters);
    return;
  }

  video_uri = g_strdup_printf ("http://www.youtube.com/get_video?fmt=18&video_id=%s&t=%s", video_id, video_hash);

  g_hash_table_destroy (flashvars_parameters);

  /* VIDEO TAG */
  js_object = js_object_get_property_as_object (js_context,
                                                js_document,
                                                "createElement");

  js_string = JSStringCreateWithUTF8CString ("video");
  js_arguments[0] = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  js_video_element = JSObjectCallAsFunction (js_context,
                                             js_object,
                                             js_document,
                                             1, js_arguments,
                                             NULL);

  if (!js_video_element || !JSValueIsObject (js_context, js_video_element))
    return;

  js_video_object = JSValueToObject (js_context, js_video_element, NULL);
  if (!js_video_object)
    return;

  prop_name = JSStringCreateWithUTF8CString ("src");
  js_string = JSStringCreateWithUTF8CString (video_uri);
  g_free (video_uri);

  prop_value = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  JSObjectSetProperty (js_context, js_video_object, prop_name, prop_value, 0, NULL);
  JSStringRelease (prop_name);

  /* autoplay */
  prop_name = JSStringCreateWithUTF8CString ("autoplay");
  js_string = JSStringCreateWithUTF8CString ("autoplay");
  prop_value = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  JSObjectSetProperty (js_context, js_video_object, prop_name, prop_value, 0, NULL);
  JSStringRelease (prop_name);

  /* controls */
  prop_name = JSStringCreateWithUTF8CString ("controls");
  js_string = JSStringCreateWithUTF8CString ("controls");
  prop_value = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  JSObjectSetProperty (js_context, js_video_object, prop_name, prop_value, 0, NULL);
  JSStringRelease (prop_name);

  /* width - this should be 100%, but JSC is not letting me add that -
   * it always understands 0 */
  prop_name = JSStringCreateWithUTF8CString ("width");
  js_string = JSStringCreateWithUTF8CString ("640");
  prop_value = JSValueMakeString (js_context, js_string);
  JSStringRelease (js_string);

  JSObjectSetProperty (js_context, js_video_object, prop_name, prop_value, 0, NULL);
  JSStringRelease (prop_name);

  /* DOM REPLACEMENT */
  js_parent_element = js_object_get_property_as_object (js_context, js_flash_object, "parentNode");

  js_arguments[0] = js_video_element;
  js_arguments[1] = js_flash_element;

  js_object = js_object_get_property_as_object (js_context,
                                                js_document,
                                                "replaceChild");

  JSObjectCallAsFunction (js_context,
                          js_object,
                          JSValueToObject (js_context, js_parent_element, NULL),
                          2, js_arguments,
                          NULL);
}

static void
destroy_last_progress_cb (gpointer data)
{
    double *last_progress = (double*) data;

    g_free (last_progress);
}

static void
progress_changed_cb (WebKitWebView *web_view,
                     GParamSpec *pspec,
                     EphyExtension *ext)
{
  const char *uri = webkit_web_view_get_uri (web_view);
  double progress;
  double *last_progress;

  if (!uri || !g_str_has_prefix (uri, "http://www.youtube.com/"))
      return;

  progress = webkit_web_view_get_progress (web_view);
  last_progress = (double*) g_object_get_data (G_OBJECT (web_view),
                                               "html5tube-last-progress");

  if (!last_progress) {
      last_progress = g_malloc (sizeof (double));
      *last_progress = progress;
      g_object_set_data_full (G_OBJECT (web_view), "html5tube-last-progress",
                              last_progress, destroy_last_progress_cb);
  }

  if (*last_progress == 1.0) {
      *last_progress = progress;
  }

  if ((progress - *last_progress) > 0.1 || progress == 1.0)
      replace_flash_object (web_view);

  *last_progress = progress;
}

static void
impl_attach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
  WebKitWebView *web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);

  LOG ("attach_tab");

  g_signal_connect (web_view, "notify::progress",
                    G_CALLBACK (progress_changed_cb), ext);
}

static void
impl_detach_tab (EphyExtension *ext,
		 EphyWindow *window,
		 EphyEmbed *embed)
{
  WebKitWebView *web_view = EPHY_GET_WEBKIT_WEB_VIEW_FROM_EMBED (embed);

  LOG ("detach_tab");

  g_signal_handlers_disconnect_by_func (web_view, progress_changed_cb, ext);
}

static void
ephy_html5tube_extension_iface_init (EphyExtensionIface *iface)
{
  iface->attach_window = impl_attach_window;
  iface->detach_window = impl_detach_window;
  iface->attach_tab = impl_attach_tab;
  iface->detach_tab = impl_detach_tab;
}

static void
ephy_html5tube_extension_class_init (EphyHTML5TubeExtensionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = ephy_html5tube_extension_finalize;
}

GType
ephy_html5tube_extension_get_type (void)
{
  return type;
}

GType
ephy_html5tube_extension_register_type (GTypeModule *module)
{
  const GTypeInfo our_info =
    {
      sizeof (EphyHTML5TubeExtensionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ephy_html5tube_extension_class_init,
      NULL,
      NULL, /* class_data */
      sizeof (EphyHTML5TubeExtension),
      0, /* n_preallocs */
      (GInstanceInitFunc) ephy_html5tube_extension_init
    };

  const GInterfaceInfo extension_info =
    {
      (GInterfaceInitFunc) ephy_html5tube_extension_iface_init,
      NULL,
      NULL
    };

  type = g_type_module_register_type (module,
                                      G_TYPE_OBJECT,
                                      "EphyHTML5TubeExtension",
                                      &our_info, 0);

  g_type_module_add_interface (module,
                               type,
                               EPHY_TYPE_EXTENSION,
                               &extension_info);

  return type;
}
