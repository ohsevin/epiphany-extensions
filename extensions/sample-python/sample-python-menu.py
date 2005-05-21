#!/usr/bin/env python
#
# Sample Python Menu extension: creates menu entries using Python
#
# Copyright (C) 2005 Adam Hooper
#
# Places two menu entries in the Tools menu: one which is always enabled, and
# one which will only be enabled when the currently-viewed page has finished
# loading.

import gtk

_ui_str = """
<ui>
 <menubar name="menubar">
  <menu name="ToolsMenu" action="Tools">
   <separator/>
   <menuitem name="PythonSampleMenuAlwaysEnabled"
             action="PythonSampleMenuAlwaysEnabled"/>
   <menuitem name="PythonSampleMenuSometimesEnabled"
	     action="PythonSampleMenuSometimesEnabled"/>
   <separator/>
  </menu>
 </menubar>
</ui>
"""

# These are the callbacks: what our menu entries will do
def _dialog(window, msg):
	dialog = gtk.MessageDialog(window, gtk.DIALOG_MODAL, gtk.MESSAGE_INFO,
				   gtk.BUTTONS_OK, msg)
	dialog.run()
	dialog.destroy()

def _always_cb(action, window):
	_dialog(window, 'You clicked a menu entry')

def _sometimes_cb(action, window):
	_dialog(window, 'If you see this, the page must be loaded')

# This is to pass to gtk.ActionGroup.add_actions()
_actions = [('PythonSampleMenuAlwaysEnabled', None,
	     'Python Sample (always enabled)', None, None, _always_cb),
	    ('PythonSampleMenuSometimesEnabled', None,
	     'Python Sample (sometimes enabled)', None, None, _sometimes_cb),
	   ]

# The next three functions handle the SometimesEnabled action. Note that we
# use window.get_active_tab(): we want the menu entries to reflect the active
# tab, not necessarily the one which fired a signal.
def _update_action(window):
	sometimes = window.get_ui_manager().get_action('/menubar/ToolsMenu/PythonSampleMenuSometimesEnabled')

	tab = window.get_active_tab()

	# Tab is None when a window is first opened
	sensitive = (tab != None and tab.get_load_status() != True)
	sometimes.set_sensitive(sensitive)

def _switch_page_cb(notebook, page, page_num, window):
	_update_action(window)

def _load_status_cb(tab, pspec, window):
	_update_action(window)

# These implement the EphyExtension interface.
#
# The tab_attached, tab_detached and notebook parts are only necessary for the
# SometimesEnabled action. In other words, no signals are needed for the
# AlwaysEnabled menu entry.
def attach_window(window):
	ui_manager = window.get_ui_manager()
	group = gtk.ActionGroup('PythonSampleMenu')
	group.add_actions(_actions, window)
	ui_manager.insert_action_group(group, 0)
	ui_id = ui_manager.add_ui_from_string(_ui_str)

	window._python_sample_menu_window_data = (group, ui_id)

	notebook = window.get_notebook()
	sig = notebook.connect('switch_page', _switch_page_cb, window)
	notebook._python_sample_menu_sig = sig

def detach_window(window):
	notebook = window.get_notebook()
	notebook.disconnect(notebook._python_sample_menu_sig)
	del notebook._python_sample_menu_sig

	group, ui_id = window._python_sample_menu_window_data
	del window._python_sample_menu_window_data

	ui_manager = window.get_ui_manager()
	ui_manager.remove_ui(ui_id)
	ui_manager.remove_action_group(group)
	ui_manager.ensure_update()

def attach_tab(window, tab):
	sig = tab.connect('notify::load-status', _load_status_cb, window)
	tab._python_sample_menu_sig = sig

def detach_tab(window, tab):
	tab.disconnect(tab._python_sample_menu_sig)
	del tab._python_sample_menu_sig
