# -*- coding: utf-8 -*-

# Copyright (C) 2005 by Magnus Therning

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from sets import Set
import pickle
import os, os.path
import time

# GConf keys
GCONF_DIR = '/apps/epiphany/extensions/epilicious'
GCONF_UN = '/apps/epiphany/extensions/epilicious/username'
GCONF_PWD = '/apps/epiphany/extensions/epilicious/password'
GCONF_BACK = '/apps/epiphany/extensions/epilicious/backend'
GCONF_KW = '/apps/epiphany/extensions/epilicious/keyword'
GCONF_EXCL = '/apps/epiphany/extensions/epilicious/exclude'

# Nice to have for debugging
logger = None
def get_logger():
    '''Create and return a file logger.

    The file used for logging is ~/.epilicious.log.

    @return: a file logger
    '''
    import logging
    global logger

    if logger:
        return logger

    logger = logging.getLogger('epilicious')
    hdlr = logging.FileHandler(os.path.join(os.environ['HOME'], '.epilicious.log'))
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    hdlr.setFormatter(formatter)
    logger.addHandler(hdlr)
    logger.setLevel(logging.ERROR)
    return logger

_file_name = os.path.join(os.environ['HOME'], '.gnome2', \
        'epiphany', 'epilicious_old')

def get_old():
    '''Read the base snapshot.

    The base snapshot is read from ~/.gnome2/epiphany/epilicious_old.

    @return: the base snapshot (an empty dictionary if none exists)
    '''
    res = {}
    try:
        f = open(_file_name, 'r')
        res = pickle.load(f)
        f.close()
    except IOError, e:
        res = {}
    return res

def save_snapshot(snap):
    '''Save a snapshot for future synchronisations.

    The base snapshot is written to ~/.gnome2/epiphany/epilicious_old.

    @param snap: The snapshot
    '''
    f = open(_file_name, 'w+')
    pickle.dump(snap, f)
    f.close()

def remove_urls(old, remote, local, rem_store, loc_store):
    '''Remove URLs from local and remote storage.

    @param old: The base snapshot
    @param remote: The remote snapshot
    @param local: The local snapshot
    @param rem_store: The remote storage (L{BaseStore})
    @param loc_store: The local storage (L{EpiphanyStore})
    '''
    o = Set(old.keys())
    r = Set(remote.keys())
    l = Set(local.keys())
    for url in o - l:
        rem_store.url_delete(url)
    for url in o - r:
        loc_store.url_delete(url)

def find_changed_urls(old, remote, local):
    '''Find the interesting set of URLs.

    A URL is interesting if it has been added or had its tags changed.

    >>> o = {0:'',     2:'',     4:'',     6:'',8:''}
    >>> r = {     1:'',2:'',     4:'',5:'',6:''     }
    >>> l = {0:'',     2:'',3:'',     5:'',6:''     }
    >>> res = calculate_pertinent_urls(o, r, l); res.sort(); res
    [0, 1, 3, 4]

    @param old: The base snapshot
    @param remote: The remote snapshot
    @param local: The local snapshot
    @return: a list of URLs
    '''

    def create_unique_id_for_url(url, desc, tags):
        tags.sort()
        return ','.join([url, desc, ','.join(tags)])

    o_ids = dict([[create_unique_id_for_url(u, old[u][0], old[u][1]), u] \
            for u in old.keys()])
    r_ids = dict([[create_unique_id_for_url(u, remote[u][0], remote[u][1]), u] \
            for u in remote.keys()])
    l_ids = dict([[create_unique_id_for_url(u, local[u][0], local[u][1]), u] \
            for u in local.keys()])

    o = Set(o_ids.keys())
    r = Set(r_ids.keys())
    l = Set(l_ids.keys())

    l_changed = l - o # added locally
    r_changed = r - o # added remotely
    b_changed = l & r # existing on both sides, with identical tags

    result = []
    for id in l_changed - b_changed:
        if l_ids[id] not in result: result.append(l_ids[id])
    for id in r_changed - b_changed:
        if r_ids[id] not in result: result.append(r_ids[id])
    get_logger().info('Interesting URLs: %s' % str(result))
    return result

def _get_tags(url, old, rem, loc):
    '''Get the tags and the description for a URL in all three locations.

    @param url: The URL to get tags for
    @param old: The base snapshot
    @param rem: The remote (Del.icio.us) snapshot
    @param loc: The local (Epiphany) snapshot
    @return: a tuple of description and lists of tags found in C{old}, C{rem},
        and C{loc}.
    '''
    try:
        otags = Set(old[url][1])
        desc = old[url][0]
    except:
        otags = Set()
    try:
        rtags = Set(rem[url][1])
        desc = rem[url][0]
    except:
        rtags = Set()
    try:
        ltags = Set(loc[url][1])
        desc = loc[url][0]
    except:
        ltags = Set()
    return desc, otags, rtags, ltags

def sync_tags_on_urls(curls, old, remote, \
        local, rem_store, loc_store):
    '''Synchronise tags on URLs, and add URLs that don't exist.

    @param curls: The list of URLs to synchronise
    @param old: The base snapshot
    @param remote: The remote snapshot
    @param local: The local snapshot
    @param rem_store: The remote storage (L{BaseStore})
    @param loc_store: The local storage (L{EpiphanyStore})
    '''
    get_logger().info('Number of urls to sync %i.' % len(curls))
    for url in curls:
        desc, otags, rtags, ltags = _get_tags(url, old, remote, local)
        if otags and (not rtags or not ltags):
            get_logger().info('Skipping %s' % url)
            continue
        get_logger().info(url)
        rem_store.url_sync(url, desc, otags - ltags, ltags - otags)
        loc_store.url_sync(url, desc, otags - rtags, rtags - otags)
