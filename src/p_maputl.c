
/* P_maputl.c */

#include "doomdef.h"
#include "p_local.h"

/*
===================
=
= P_AproxDistance
=
= Gives an estimation of distance (not exact)
=
===================
*/

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy)
{
	dx = D_abs(dx);
	dy = D_abs(dy);
	if (dx < dy)
		return dx + dy - (dx >> 1);
	return dx + dy - (dy >> 1);
}

/*
==================
=
= P_LineOpening
=
= Sets opentop and openbottom to the window through a two sided line
= OPTIMIZE: keep this precalculated
==================
*/

fixed_t opentop, openbottom, openrange; // 800A5740, 800A5744, 800A5748
fixed_t lowfloor; // 800A574C

void P_LineOpening(line_t *linedef) // 80017F40
{
	sector_t *front, *back;

	//if (linedef->sidenum[1] == -1)
	if (linedef->backsector == 0) // D64 change this line
	{ /* single sided line */
		openrange = 0;
		return;
	}

	front = linedef->frontsector;
	back = linedef->backsector;

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;
	if (front->floorheight > back->floorheight) {
		openbottom = front->floorheight;
		lowfloor = back->floorheight;
	} else {
		openbottom = back->floorheight;
		lowfloor = front->floorheight;
	}

	openrange = opentop - openbottom;
}

/*
===============================================================================

						BLOCK MAP ITERATORS

For each line/thing in the given mapblock, call the passed function.
If the function returns false, exit with false without checking anything else.

===============================================================================
*/

/*
==================
=
= P_BlockLinesIterator
=
= The validcount flags are used to avoid checking lines
= that are marked in multiple mapblocks, so increment validcount before
= the first call to P_BlockLinesIterator, then make one or more calls to it
===================
*/

bool P_BlockLinesIterator(int x, int y,
			     bool (*func)(line_t *)) // 80017FE8
{
	int offset;
	short *list;
	line_t *ld;

	if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
		return true;

	offset = y * bmapwidth + x;

	offset = *(blockmap + offset);

	for (list = blockmaplump + offset; *list != -1; list++) {
		ld = &lines[*list];
		if (ld->validcount == validcount)
			continue; /* line has already been checked */
		ld->validcount = validcount;

		if (!func(ld))
			return false;
	}

	return true; /* everything was checked */
}

/*
==================
=
= P_BlockThingsIterator
=
==================
*/

bool P_BlockThingsIterator(int x, int y,
			      bool (*func)(mobj_t *)) // 8001811C
{
	mobj_t *mobj;

	if (x < 0 || y < 0 || x >= bmapwidth || y >= bmapheight)
		return true;

	for (mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext) {
		if (!func(mobj))
			return false;
	}

	return true;
}

/*
===============================================================================

INTERCEPT ROUTINES

===============================================================================
*/

intercept_t intercepts[MAXINTERCEPTS]; // 800A5750
intercept_t *intercept_p; // 800a5d50

divline_t trace; // 800A5D58

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
bool P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
		       int flags, bool (*trav)(intercept_t *))
{
	fixed_t xt1, yt1, xt2, yt2;
	fixed_t xstep, ystep;
	fixed_t partial;
	fixed_t xintercept, yintercept;
	int mapx, mapy;
	int mapxstep, mapystep;
	int count;

	validcount++;
	intercept_p = intercepts;

	if (((x1 - bmaporgx) & (MAPBLOCKSIZE - 1)) == 0) {
		x1 += FRACUNIT; // don't side exactly on a line
	}

	if (((y1 - bmaporgy) & (MAPBLOCKSIZE - 1)) == 0) {
		y1 += FRACUNIT; // don't side exactly on a line
	}

	trace.x = x1;
	trace.y = y1;
	trace.dx = x2 - x1;
	trace.dy = y2 - y1;

	x1 -= bmaporgx;
	y1 -= bmaporgy;
	xt1 = x1 >> MAPBLOCKSHIFT;
	yt1 = y1 >> MAPBLOCKSHIFT;

	x2 -= bmaporgx;
	y2 -= bmaporgy;
	xt2 = x2 >> MAPBLOCKSHIFT;
	yt2 = y2 >> MAPBLOCKSHIFT;

	if (xt2 > xt1) {
		mapxstep = 1;
		partial = FRACUNIT - ((x1 >> MAPBTOFRAC) & (FRACUNIT - 1));
		ystep = FixedDivFloat(y2 - y1, D_abs(x2 - x1));
	} else if (xt2 < xt1) {
		mapxstep = -1;
		partial = (x1 >> MAPBTOFRAC) & (FRACUNIT - 1);
		ystep = FixedDivFloat(y2 - y1, D_abs(x2 - x1));
	} else {
		mapxstep = 0;
		partial = FRACUNIT;
		ystep = 256 * FRACUNIT;
	}

	yintercept = (y1 >> MAPBTOFRAC) + FixedMul(partial, ystep);

	if (yt2 > yt1) {
		mapystep = 1;
		partial = FRACUNIT - ((y1 >> MAPBTOFRAC) & (FRACUNIT - 1));
		xstep = FixedDivFloat(x2 - x1, D_abs(y2 - y1));
	} else if (yt2 < yt1) {
		mapystep = -1;
		partial = (y1 >> MAPBTOFRAC) & (FRACUNIT - 1);
		xstep = FixedDivFloat(x2 - x1, D_abs(y2 - y1));
	} else {
		mapystep = 0;
		partial = FRACUNIT;
		xstep = 256 * FRACUNIT;
	}

	xintercept = (x1 >> MAPBTOFRAC) + FixedMul(partial, xstep);

	// Step through map blocks.
	// Count is present to prevent a round off error
	// from skipping the break.
	mapx = xt1;
	mapy = yt1;

	for (count = 0; count < 64; count++) {
		// [d64]: switched order of function calls
		if (flags & PT_ADDTHINGS) {
			if (!P_BlockThingsIterator(mapx, mapy,
						   PIT_AddThingIntercepts)) {
				return false; // early out
			}
		}

		if (flags & PT_ADDLINES) {
			if (!P_BlockLinesIterator(mapx, mapy,
						  PIT_AddLineIntercepts)) {
				return false; // early out
			}
		}

		if (mapx == xt2 && mapy == yt2) {
			break;
		}

		if ((yintercept >> FRACBITS) == mapy) {
			yintercept += ystep;
			mapx += mapxstep;
		} else if ((xintercept >> FRACBITS) == mapx) {
			xintercept += xstep;
			mapy += mapystep;
		}
	}
	// go through the sorted list
	return P_TraverseIntercepts(trav, FRACUNIT);
}

//
// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
//
bool PIT_AddLineIntercepts(line_t *ld) // 80018574
{
	fixed_t frac;

	// hit the line
	frac = P_InterceptLine(ld, &trace);

	if (frac < 0) {
		return true; // behind source
	}

	// [d64] exit out if max intercepts has been hit
	if ((intercept_p - intercepts) >= MAXINTERCEPTS) {
		return true;
	}

	intercept_p->frac = frac;
	intercept_p->isaline = true;
	intercept_p->d.line = ld;
	intercept_p++;

	return true; // continue
}

//
// PIT_AddThingIntercepts
//
bool PIT_AddThingIntercepts(mobj_t *thing) // 8001860C
{
	fixed_t x1, y1, x2, y2;
	bool tracepositive;
	fixed_t frac;
	line_t templine;
	vertex_t tempvertex1, tempvertex2;

	// [d64]: added an early out check
	if ((thing->flags & MF_SHOOTABLE) == 0) {
		return true; // keep going
	}

	memset(&templine, 0, sizeof(line_t));

	tracepositive = (trace.dx ^ trace.dy) > 0;

	// check a corner to corner crossection for hit
	if (tracepositive) {
		x1 = thing->x - thing->radius;
		y1 = thing->y + thing->radius;

		x2 = thing->x + thing->radius;
		y2 = thing->y - thing->radius;
	} else {
		x1 = thing->x - thing->radius;
		y1 = thing->y - thing->radius;

		x2 = thing->x + thing->radius;
		y2 = thing->y + thing->radius;
	}

	tempvertex1.x = x1;
	tempvertex1.y = y1;
	tempvertex2.x = x2;
	tempvertex2.y = y2;

	templine.v1 = &tempvertex1;
	templine.v2 = &tempvertex2;

	templine.dx = x2 - x1;
	templine.dy = y2 - y1;

	frac = P_InterceptLine(&templine, &trace);

	if (frac < 0) {
		return true; // behind source
	}

	// [d64] exit out if max intercepts has been hit
	if ((intercept_p - intercepts) >= MAXINTERCEPTS) {
		return true;
	}

	intercept_p->frac = frac;
	intercept_p->isaline = false;
	intercept_p->d.thing = thing;
	intercept_p++;

	return true; // keep going
}

//
// P_InterceptLine
// [d64]: new function

fixed_t P_InterceptLine(line_t *line, divline_t *trace) // 8001872C
{
	fixed_t normal_x, normal_y;
	fixed_t v1x, v1y, v2x, v2y;
	fixed_t tx, ty, tdx, tdy;
	fixed_t dir_x, dir_y;
	fixed_t leftSide, rightSide;
	fixed_t ldx, ldy;
	fixed_t end_x, end_y;
	fixed_t num, den, frac;

	v1x = line->v1->x;
	v1y = line->v1->y;
	v2y = line->v2->y;
	v2x = line->v2->x;

	tx = trace->x;
	ty = trace->y;

	tdx = trace->dx;
	tdy = trace->dy;

	normal_y = (-tdx) >> 8;
	normal_x = tdy >> 8;

	dir_x = (v1x - tx) >> 8;
	dir_y = (v1y - ty) >> 8;

	leftSide = FixedMul(dir_x, normal_x) + FixedMul(dir_y, normal_y);
	rightSide = FixedMul((v2x - tx) >> 8, normal_x) +
		    FixedMul((v2y - ty) >> 8, normal_y);

	if ((leftSide < 0) == (rightSide < 0)) {
		// didn't cross
		return -1;
	}

	ldy = (v1y - v2y) >> 8;
	ldx = (v2x - v1x) >> 8;

	end_x = (tx + tdx);
	end_y = (ty + tdy);

	num = FixedMul(dir_x, ldy) + FixedMul(dir_y, ldx);
	den = FixedMul((end_x - v1x) >> 8, ldy) +
	      FixedMul((end_y - v1y) >> 8, ldx);

	frac = FixedDivFloat(num, num + den);

	return frac;
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
bool P_TraverseIntercepts(traverser_t func, fixed_t maxfrac) // 800188F0
{
	int count;
	fixed_t dist;
	intercept_t *scan;
	intercept_t *in;

	count = (int)(intercept_p - intercepts);

	in = 0; // shut up compiler warning

	while (count--) {
		dist = MAXINT;
		for (scan = intercepts; scan < intercept_p; scan++) {
			if (scan->frac < dist) {
				dist = scan->frac;
				in = scan;
			}
		}

		if (dist > maxfrac) {
			return true; // checked everything in range
		}

#if RANGECHECK
		if (!arch_valid_text_address((uintptr_t)func)) {
			I_Error("invalid traverser_t %08x", func);
		}
#endif

		if (!func(in)) {
			return false; // don't bother going farther
		}

/*
src/p_maputl.c: In function ‘P_TraverseIntercepts’:
src/p_maputl.c:519:26: error: dereference of NULL ‘in’ [CWE-476] [-Werror=analyzer-null-dereference]
  519 |                 in->frac = MAXINT;
*/
		if (in) {
			in->frac = MAXINT;
		} else {
			I_Error("null intercept");
		}
	}

	return true; // everything was traversed
}
