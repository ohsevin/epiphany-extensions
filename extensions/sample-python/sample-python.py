#!/usr/bin/env python
# -*- coding: utf-8 -*-

print "Sample Python extension"
print "Copyright © 2005 Adam Hooper"
print

def attach_window(window):
	print "attach_window"

def detach_window(window):
	print "detach_window"

def attach_tab(window, tab):
	print "attach_tab"

def detach_tab(window, tab):
	print "detach_tab"
