/*
   libstroke - an X11 stroke interface library
   Copyright (c) 1996,1997,1998,1999  Mark F. Willey, ETLA Technical

   See the files COPYRIGHT and LICENSE for distribution information.

   This file pulled from libstroke-0.5.1 (http://www.etla.net/libstroke/)
   and trimmed down for Galeon.
*/

/* largest number of points allowed to be sampled */
#define STROKE_MAX_POINTS 10000

/* number of sample points required to have a valid stroke */
#define STROKE_MIN_POINTS 50

/* maximum number of numbers in stroke */
#define STROKE_MAX_SEQUENCE 20

/* threshold of size of smaller axis needed for it to define its own
   bin size */
#define STROKE_SCALE_RATIO 4

/* minimum percentage of points in bin needed to add to sequence */
#define STROKE_BIN_COUNT_PERCENT 0.07

/* translate stroke to sequence */
int stroke_trans (char *sequence);

/* record point in stroke */
void stroke_record (int x, int y);

/* initialize stroke functions */
void stroke_init (void);
