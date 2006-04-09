#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Listens to location changes using Python

import epiphany;

def location_cb(embed, address):
	print 'New location: %s' % address

def attach_tab(window, tab):
	embed = tab.get_embed()
	sig = embed.connect('ge-location', location_cb)
	embed._python_sample_location_sig = sig

def detach_tab(window, tab):
	embed = tab.get_embed()
	sig = embed._python_sample_location_sig
	del embed._python_sample_location_sig
	embed.disconnect(sig)
