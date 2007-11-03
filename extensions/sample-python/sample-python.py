#!/usr/bin/env python
# -*- coding: utf-8 -*-

print "Sample Python extension"
print "Copyright © 2005 Adam Hooper"
print

def attach_window(window):
	print "attach_window"

def detach_window(window):
	print "detach_window"

def attach_tab(window, embed):
	print "attach_embed"

def detach_tab(window, embed):
	print "detach_embed"
