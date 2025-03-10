#include "doomdef.h"
#include "p_local.h"

//
// P_LineAttack
//
mobj_t *linetarget; // 800A56F8 // who got hit (or NULL)
mobj_t *shootthing; // 800A5700
line_t *shotline; // 800A56FC
fixed_t aimfrac; // 800A5720

fixed_t shootdirx;
fixed_t shootdiry;
fixed_t shootdirz;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t shootz; // 800A571C

int la_damage; // 800A5724
fixed_t attackrange; // 800A5704

fixed_t aimslope; // 800A5710
fixed_t aimpitch;

// For P_PathTraverse
fixed_t tx2; // 800A5714
fixed_t ty2; // 800A5718

// slopes to top and bottom of target
extern fixed_t topslope; // 800A5708
extern fixed_t bottomslope; // 800A570C

/*
==============
=
= PTR_AimTraverse
= Sets linetaget and aimslope when a target is aimed at.
=
==============
*/

bool PTR_AimTraverse(intercept_t *in) // 80017508
{
	line_t *li;
	mobj_t *th;
	fixed_t linebottomslope;
	fixed_t linetopslope;
	fixed_t thingtopslope;
	fixed_t thingbottomslope;
	fixed_t dist;

	if (in->isaline) {
		li = in->d.line;

		if (!(li->flags & ML_TWOSIDED)) {
			aimfrac = in->frac;
			shotline = li;
			return false; // stop
		}

		// Crosses a two sided line.
		// A two sided line will restrict
		// the possible target ranges.
		P_LineOpening(li);

		if (openbottom >= opentop) {
			aimfrac = in->frac;
			shotline = li;
			return false; // stop
		}

		dist = FixedMul(attackrange, in->frac);

		if (li->frontsector->floorheight !=
		    li->backsector->floorheight) {
			linebottomslope = FixedDivFloat(openbottom - shootz, dist);
			if (linebottomslope > bottomslope) {
				bottomslope = linebottomslope;
			}
		}

		if (li->frontsector->ceilingheight !=
		    li->backsector->ceilingheight) {
			linetopslope = FixedDivFloat(opentop - shootz, dist);
			if (linetopslope < topslope) {
				topslope = linetopslope;
			}
		}

		if (topslope <= bottomslope) {
			shotline = li;
			aimfrac = in->frac;
			return false; // stop
		}

		return true; // shot continues
	}

	// shoot a thing
	th = in->d.thing;
	if (th == shootthing) {
		return true; // can't shoot self
	}

	if ((th->flags & MF_SHOOTABLE) == 0) {
		return true; // corpse or something
	}

	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, in->frac);
	thingtopslope = FixedDivFloat(th->z + th->height - shootz, dist);

	if (thingtopslope < bottomslope) {
		return true; // shot over the thing
	}

	thingbottomslope = FixedDivFloat(th->z - shootz, dist);

	if (thingbottomslope > topslope) {
		return true; // shot under the thing
	}

	// this thing can be hit!
	if (thingtopslope > topslope) {
		thingtopslope = topslope;
	}

	if (thingbottomslope < bottomslope) {
		thingbottomslope = bottomslope;
	}

	aimslope = (thingtopslope + thingbottomslope) >> 1;
	linetarget = th;
	aimfrac = in->frac;

	return false; // don't go any farther
}

/*
==============
=
= PTR_ShootTraverse
= [d64]: Some logic from PTR_AimTraverse has been merged
=
==============
*/

bool PTR_ShootTraverse(intercept_t *in) // 800177A8
{
	fixed_t x, y, z;
	fixed_t frac;
	line_t *li;
	mobj_t *th;
	fixed_t slope, dist;
	fixed_t thingtopslope, thingbottomslope;
	sector_t *front, *back;

	if (in->isaline) {
		li = in->d.line;

		if (li->special && (li->special & MLU_SHOOT))
#if RANGECHECK
			P_UseSpecialLine(li, shootthing, 0);
#else
			P_UseSpecialLine(li, shootthing);
#endif

		front = li->frontsector;
		back = li->backsector;

		if (back) {
			// crosses a two sided line
			P_LineOpening(li);

			dist = FixedMul(attackrange, in->frac);

			if (front->floorheight != back->floorheight) {
				slope = FixedDivFloat(openbottom - shootz, dist);
				if (slope > bottomslope) {
					bottomslope = slope;
				}
			}
			if (front->ceilingheight != back->ceilingheight) {
				slope = FixedDivFloat(opentop - shootz, dist);
				if (slope < topslope) {
					topslope = slope;
				}
			}

			if (bottomslope < topslope) {
				return true; // shot continues
			}
		}

		// hit line
		// position a bit closer
		frac = in->frac - FixedDivFloat(4 * FRACUNIT, attackrange);

		x = trace.x + FixedMul(trace.dx, frac);
		y = trace.y + FixedMul(trace.dy, frac);
		z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

		if (front->ceilingpic == -1) {
			// don't shoot the sky!
			if (z > front->ceilingheight)
				return false;

			// it's a sky hack wall
			if (back && (back->ceilingpic == -1))
				return false;

			// don't shoot blank mid texture
			if ((back == NULL) && (sides[li->sidenum[0]].midtexture == 1))
				return false;
		}

		// Spawn bullet puffs.
		P_SpawnPuff(x, y, z);

		shotline = li;

		// don't go any farther
		return false;
	}

	// shoot a thing
	th = in->d.thing;
	if (th == shootthing)
		return true; // can't shoot self

	if ((th->flags & MF_SHOOTABLE) == 0)
		return true; // corpse or something

	// check angles to see if the thing can be aimed at
	dist = FixedMul(attackrange, in->frac);
	thingtopslope = FixedDivFloat((th->z + th->height) - shootz, dist);

	if (thingtopslope < bottomslope)
		return true; // shot over the thing

	thingbottomslope = FixedDivFloat(th->z - shootz, dist);

	if (thingbottomslope > topslope)
		return true; // shot under the thing

	// this thing can be hit!
	if (thingtopslope > topslope)
		thingtopslope = topslope;

	if (thingbottomslope < bottomslope)
		thingbottomslope = bottomslope;

	// hit thing
	// position a bit closer
	frac = in->frac - FixedDivFloat(10 * FRACUNIT, attackrange);

	x = trace.x + FixedMul(trace.dx, frac);
	y = trace.y + FixedMul(trace.dy, frac);
	z = shootz + FixedMul((thingtopslope + thingbottomslope) >> 1, FixedMul(frac, attackrange));

	// Spawn bullet puffs or blod spots,
	// depending on target type.
	if ((in->d.thing->flags & MF_NOBLOOD) != 0)
		P_SpawnPuff(x, y, z);
	else
		P_SpawnBlood(x, y, z, la_damage);

	if (la_damage)
		P_DamageMobj(th, shootthing, shootthing, la_damage);

	linetarget = th;
	// don't go any farther
	return false;
}

/*
=================
=
= P_AimLineAttack
=
=================
*/

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t zheight,
			fixed_t distance) // 80017C30
{
	int flags;
	fixed_t dist;

	angle >>= ANGLETOFINESHIFT;
	dist = distance >> FRACBITS;

	shootthing = t1;

	tx2 = t1->x + dist * finecosine[angle];
	ty2 = t1->y + dist * finesine[angle];

	// can't shoot outside view angles
	// [d64] use 120 instead of 100
	topslope = 120 * FRACUNIT / 160;
	bottomslope = -120 * FRACUNIT / 160;

	attackrange = distance;
	linetarget = NULL;
	shotline = NULL;
	aimfrac = 0;
	flags = PT_ADDLINES | PT_ADDTHINGS | PT_EARLYOUT;

	// [d64] new argument for shoot height
	if (!zheight)
		shootz = t1->z + (t1->height >> 1) + 12 * FRACUNIT;
	else
		shootz = t1->z + zheight;

	P_PathTraverse(t1->x, t1->y, tx2, ty2, flags, PTR_AimTraverse);

	if (linetarget)
		return aimslope;

	return 0;
}

/*
=================
=
= P_LineAttack
=
= [d64]: A lot of code from P_AimLineAttack and PTR_AimTraverse has been merged
=
=================
*/

void P_LineAttack(mobj_t *t1, angle_t angle, fixed_t zheight, fixed_t distance,
		  fixed_t slope, int damage) // 80017D74
{
	int flags;
	fixed_t dist;

	angle >>= ANGLETOFINESHIFT;
	dist = distance >> FRACBITS;

	shootthing = t1;
	la_damage = damage;

	tx2 = t1->x + dist * finecosine[angle];
	ty2 = t1->y + dist * finesine[angle];
	linetarget = NULL;
	shotline = NULL;

	if (!zheight)
		shootz = t1->z + (t1->height >> 1) + 12 * FRACUNIT;
	else
		shootz = t1->z + zheight;

	if (slope == MAXINT) {
		topslope = 120 * FRACUNIT / 160;
		bottomslope = -120 * FRACUNIT / 160;
	} else {
		topslope = slope + 1;
		bottomslope = slope - 1;
	}

	aimslope = topslope + bottomslope; // addu    $t3, $t8, $t9
	if (aimslope < 0) { // addiu   $at, $t3, 1
		aimslope = (aimslope + 1) >> 1; // sra     $t5, $at, 1
		//          ^^ that's really weird....
	} else { // bgez    $t3, loc_80017EC0
		aimslope >>= 1; // sra     $t5, $t3, 1
	}

	attackrange = distance;
	flags = PT_ADDLINES | PT_ADDTHINGS | PT_EARLYOUT;

	P_PathTraverse(t1->x, t1->y, tx2, ty2, flags, PTR_ShootTraverse);
}