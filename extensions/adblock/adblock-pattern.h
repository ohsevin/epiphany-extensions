/*
 *  Copyright © 2004 Adam Hooper
 *  Copyright © 2005, 2006 Jean-François Rameau
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

#ifndef ADBLOCK_PATTERN_H
#define ADBLOCK_PATTERN_H

#include <glib.h>

G_BEGIN_DECLS

typedef enum
{
	PATTERN_BLACKLIST,
	PATTERN_WHITELIST,
	PATTERN_DEFAULT_BLACKLIST
} AdblockPatternType;

/**
 * Load patterns of type @type into a hash table:
 * key is textual pattern
 * value is compiled pattern
 */
void adblock_pattern_load (GHashTable *patterns, AdblockPatternType type);

/*
 * Save textual patterns from a list
 *
 */
void adblock_pattern_save (GSList *patterns, AdblockPatternType type);

/*
 * Get patterns from filteset.G site
 *
 */
void adblock_pattern_get_filtersetg_patterns (void);

G_END_DECLS

#endif /* ADBLOCK_PATTERN_H */
