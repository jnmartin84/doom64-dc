#include "doomdef.h"
#include "p_local.h"

#define CLIPRADIUS 23
#define SIDE_ON 0
#define SIDE_FRONT 1
#define SIDE_BACK -1

fixed_t bestslidefrac; // 800A5728
line_t *bestslideline; // 800A572C
mobj_t *slidemo; // 800A5730

/*
====================
=
= PTR_SlideTraverse
=
====================
*/

bool PTR_SlideTraverse(intercept_t *in) // 800170AC
{
	line_t *li;

	li = in->d.line;

	if (!(li->flags & ML_TWOSIDED)) {
		if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
			return true; /* don't hit the back side */

		goto isblocking;
	}

	/* set openrange, opentop, openbottom */
	P_LineOpening(li);

	if (openrange < slidemo->height) {
		goto isblocking; /* doesn't fit */
	}

	if (opentop - slidemo->z < slidemo->height) {
		goto isblocking; /* mobj is too high */
	}

	if (openbottom - slidemo->z > 24 * FRACUNIT) {
		goto isblocking; /* too big a step up */
	}

	/* this line doesn't block movement */
	return true;

	/* the line does block movement, */
	/* see if it is closer than best so far */

isblocking:

	if (in->frac < bestslidefrac) {
		bestslidefrac = in->frac;
		bestslideline = li;
	}

	return false; /* stop */
}

/*
===============================================
= P_SlideMove
= The momx / momy move is bad, so try to slide
= along a wall.
= Find the first line hit, move flush to it,
= and slide along it
=
= This is a kludgy mess.
===============================================
*/

void P_SlideMove(mobj_t *mo) // 800171B0
{
	fixed_t tmxmove;
	fixed_t tmymove;
	fixed_t leadx;
	fixed_t leady;
	fixed_t trailx;
	fixed_t traily;
	fixed_t newx;
	fixed_t newy;
	int hitcount;
	line_t *ld;
	int an1;
	int an2;

	slidemo = mo;
	hitcount = 0;

retry:
	hitcount++;

	if (hitcount == 3) {
		goto stairstep; // don't loop forever
	}

	// trace along the three leading corners
	if (mo->momx > 0) {
		leadx = mo->x + mo->radius;
		trailx = mo->x - mo->radius;
	} else {
		leadx = mo->x - mo->radius;
		trailx = mo->x + mo->radius;
	}

	if (mo->momy > 0) {
		leady = mo->y + mo->radius;
		traily = mo->y - mo->radius;
	} else {
		leady = mo->y - mo->radius;
		traily = mo->y + mo->radius;
	}

	bestslidefrac = FRACUNIT + 1;

	P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
		       PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
		       PT_ADDLINES, PTR_SlideTraverse);
	P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
		       PT_ADDLINES, PTR_SlideTraverse);

	// move up to the wall
	if (bestslidefrac == FRACUNIT + 1) {
		// the move most have hit the middle, so stairstep
stairstep:
		if (!P_TryMove(mo, mo->x, mo->y + mo->momy)) {
			if (!P_TryMove(mo, mo->x + mo->momx, mo->y)) {
				// [d64] set momx and momy to 0
				mo->momx = 0;
				mo->momy = 0;
			}
		}
		return;
	}

	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0) {
		newx = FixedMul(mo->momx, bestslidefrac);
		newy = FixedMul(mo->momy, bestslidefrac);

		if (!P_TryMove(mo, mo->x + newx, mo->y + newy)) {
			bestslidefrac = FRACUNIT;

			// [d64] jump to hitslideline instead of stairstep
			goto hitslideline;
		}
	}

	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = 0xf800 - bestslidefrac;
	//bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

	if (bestslidefrac > FRACUNIT) {
		bestslidefrac = FRACUNIT;
	}

	if (bestslidefrac <= 0) {
		return;
	}

	//
	// [d64] code below is loosely based on P_HitSlideLine
	//
hitslideline:

	ld = bestslideline;

	if (ld->slopetype == ST_HORIZONTAL) {
		tmymove = 0;
	} else {
		tmymove = FixedMul(mo->momy, bestslidefrac);
	}

	if (ld->slopetype == ST_VERTICAL) {
		tmxmove = 0;
	} else {
		tmxmove = FixedMul(mo->momx, bestslidefrac);
	}

	//
	// [d64] this new algorithm seems to reduce the chances
	// of boosting the player's speed when wall running
	//

	an1 = finecosine[ld->fineangle];
	an2 = finesine[ld->fineangle];

	if (P_PointOnLineSide(mo->x, mo->y, bestslideline)) {
		//
		// [d64] same as deltaangle += ANG180 ?
		//
		an1 = -an1;
		an2 = -an2;
	}

	newx = FixedMul(tmxmove, an1);
	newy = FixedMul(tmymove, an2);

	mo->momx = FixedMul(newx + newy, an1);
	mo->momy = FixedMul(newx + newy, an2);

	if (!P_TryMove(mo, mo->x + mo->momx, mo->y + mo->momy)) {
		goto retry;
	}
}

//----------------------