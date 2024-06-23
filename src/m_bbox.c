/* m_bbox.c */

#include "doomdef.h"

void M_ClearBox(fixed_t *box) // 80000450
{
	box[BOXTOP] = box[BOXRIGHT] = MININT;
	box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y) // 80000470
{
	if (x<box[BOXLEFT])
		box[BOXLEFT] = x;
	if (x>box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y<box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	if (y>box[BOXTOP])
		box[BOXTOP] = y;
}

// From Nova, used to clip the automap so it runs efficiently.

boolean M_BoxIntersect(fixed_t a[static 4], fixed_t b[static 4])
{
    return a[BOXLEFT] < b[BOXRIGHT] && a[BOXRIGHT] > b[BOXLEFT]
        && a[BOXBOTTOM] < b[BOXTOP] && a[BOXTOP] > b[BOXBOTTOM];
}