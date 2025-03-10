#include "doomdef.h"
#include "p_local.h"

fixed_t sightzstart; //800A5F20 // eye z of looker
fixed_t topslope,
	bottomslope; //800A5F24 ,800A5F28 // slopes to top and bottom of target

divline_t strace; //800A5F30 // from t1 to t2
fixed_t t2x, t2y; //800A5F40, 800A5F44

int t1xs, t1ys, t2xs, t2ys; //800A5F48,800A5F4C,800A5F50,800A5F54

/*
===============
=
= P_CheckSights
=
= Check sights of all mobj thinkers that are going to change state this
= tic and have MF_COUNTKILL set
===============
*/
bool PS_CrossBSPNode(int bspnum);
void P_CheckSights(void) // 8001EB00
{
	mobj_t *mobj;

	for (mobj = mobjhead.next; mobj != &mobjhead; mobj = mobj->next) {
		if (!mobj) {
			dbgio_printf("P_CheckSights: NULL mobj\n");
			return;
		}

		// must be killable
		if (!(mobj->flags & MF_COUNTKILL))
			continue;

		// must be about to change states
		if (mobj->tics != 1)
			continue;

		mobj->flags &= ~MF_SEETARGET;

		// must have a target
		if (!mobj->target)
			continue;

		if (P_CheckSight(mobj, mobj->target))
			mobj->flags |= MF_SEETARGET;
	}
}

/**********************************

Returns true if a straight line between t1 and t2 is unobstructed

**********************************/
extern int reject_length;
bool /* __attribute__((noinline)) */ P_CheckSight(mobj_t *t1, mobj_t *t2) // 8001EBCC
{
	int s1, s2;
	int pnum, bytenum, bitnum;

	//
	// check for trivial rejection
	//
	s1 = (t1->subsector->sector - sectors);
	s2 = (t2->subsector->sector - sectors);
	pnum = s1 * numsectors + s2;
	bytenum = pnum >> 3;
	bitnum = 1 << (pnum & 7);

#if RANGECHECK
	if (reject_length - 1 < bytenum) {
		I_Error("invalid bytenum %08x", bytenum);
	}
#endif	

	if (rejectmatrix[bytenum] & bitnum) {
		return false; // can't possibly be connected
	}

	// look from eyes of t1 to any part of t2

	++validcount;

	// make sure it never lies exactly on a vertex coordinate

	strace.x = (t1->x & ~0x1ffff) | 0x10000;
	strace.y = (t1->y & ~0x1ffff) | 0x10000;
	t2x = (t2->x & ~0x1ffff) | 0x10000;
	t2y = (t2->y & ~0x1ffff) | 0x10000;
	strace.dx = t2x - strace.x;
	strace.dy = t2y - strace.y;

	t1xs = strace.x >> FRACBITS;
	t1ys = strace.y >> FRACBITS;
	t2xs = t2x >> FRACBITS;
	t2ys = t2y >> FRACBITS;

	sightzstart = (t1->z + t1->height) - (t1->height >> 2);
	topslope = (t2->z + t2->height) - sightzstart;
	bottomslope = (t2->z) - sightzstart;

	return PS_CrossBSPNode(numnodes - 1);
}

/*
=================
=
= PS_SightCrossLine
=
= First checks the endpoints of the line to make sure that they cross the
= sight trace treated as an infinite line.
=
= If so, it calculates the fractional distance along the sight trace that
= the intersection occurs at.  If 0 < intercept < 1.0, the line will block
= the sight.
=================
*/
extern fixed_t FixedDivFloat(register fixed_t a, register fixed_t b);
#if 0
fixed_t /* __attribute__((noinline)) */ PS_SightCrossLine(line_t *line) // 8001EDD8
{
	int s1, s2;

	// p1, p2 are line endpoints
	int p1x = line->v1->x >> FRACBITS;
	int p1y = line->v1->y >> FRACBITS;
	int p2x = line->v2->x >> FRACBITS;
	int p2y = line->v2->y >> FRACBITS;

	// p3, p4 are sight endpoints
	int p3x = t1xs;
	int p3y = t1ys;
	int p4x = t2xs;
	int p4y = t2ys;

	int dx = p2x - p3x;
	int dy = p2y - p3y;

	int ndx = p4x - p3x; // this can be precomputed if worthwhile
	int ndy = p4y - p3y;

	s1 = (ndy * dx) < (dy * ndx);

	dx = p1x - p3x;
	dy = p1y - p3y;

	s2 = (ndy * dx) < (dy * ndx);

	if (s1 == s2)
		return -1; // line isn't crossed

	ndx = p1y - p2y; // vector normal to world line
	ndy = p2x - p1x;

	s1 = (ndx * dx) + (ndy * dy); // distance projected onto normal

	dx = p4x - p1x;
	dy = p4y - p1y;

	s2 = (ndx * dx) + (ndy * dy); // distance projected onto normal

	s2 = FixedDivFloat(s1, (s1+s2)); //FixedDiv(s1, (s1 + s2));

	return s2;
}
#endif
fixed_t PS_SightCrossLine(line_t *line) {
    int p1x = line->v1->x >> FRACBITS;
    int p1y = line->v1->y >> FRACBITS;
    int p2x = line->v2->x >> FRACBITS;
    int p2y = line->v2->y >> FRACBITS;

    int dx = p2x - t1xs;
    int dy = p2y - t1ys;
    int ndx = t2xs - t1xs;  // Precomputed
    int ndy = t2ys - t1ys;

    int s1 = (ndy * dx) < (dy * ndx);

    dx = p1x - t1xs;
    dy = p1y - t1ys;

    int s2 = (ndy * dx) < (dy * ndx);

    if (s1 == s2)
        return -1;  // Line isn't crossed

    // Compute normal to the world line
    ndx = p1y - p2y;
    ndy = p2x - p1x;

    // Project distances onto normal
    int proj1 = (ndx * dx) + (ndy * dy);

    dx = t2xs - p1x;  // Use precomputed t2xs instead of recomputing
    dy = t2ys - p1y;

    int proj2 = (ndx * dx) + (ndy * dy);

    // **Avoid division by zero**  
    int denom = proj1 + proj2;
    if (denom == 0)
        return 0;  // Safe fallback

    return FixedDivFloat(proj1, denom);
}

/*
=================
=
= PS_CrossSubsector
=
= Returns true if strace crosses the given subsector successfuly
=================
*/

bool /* __attribute__((noinline)) */ PS_CrossSubsector(subsector_t *sub) // 8001EF10
{
	seg_t *seg;
	line_t *line;
	int count;
	sector_t *front, *back;
	fixed_t opentop, openbottom;
	fixed_t frac, slope;

	//
	// check lines
	//
	count = sub->numlines;
	seg = &segs[sub->firstline];

	for (; count; seg++, count--) {
		line = seg->linedef;

		if (line->validcount == validcount)
			continue; // allready checked other side

		line->validcount = validcount;

		frac = PS_SightCrossLine(line);

		if (frac < 4 || frac >= (FRACUNIT + 1))
			continue;

		//
		// crosses line
		//
		back = line->backsector;
		if (!back)
			return false; // one sided line
		front = line->frontsector;

		if (__builtin_expect(front->floorheight == back->floorheight && 
							front->ceilingheight == back->ceilingheight, 1)) {
			continue; // no wall to block sight with
		}

		if (front->ceilingheight < back->ceilingheight)
			opentop = front->ceilingheight;
		else
			opentop = back->ceilingheight;
		if (front->floorheight > back->floorheight)
			openbottom = front->floorheight;
		else
			openbottom = back->floorheight;

		// quick test for totally closed doors
		if (__builtin_expect(openbottom >= opentop, 0)) {
			return false; // stop
		}

		frac = frac >> 2;

		if (__builtin_expect((front->floorheight != back->floorheight),0)) {
			slope = (int)((float)(((openbottom - sightzstart) << 6)) / (float)frac) << 8;
			if (slope > bottomslope)
				bottomslope = slope;
		}

		if (__builtin_expect((front->ceilingheight != back->ceilingheight),0)) {
			slope = (int)((float)(((opentop - sightzstart) << 6)) / (float)frac) << 8;
			if (slope < topslope)
				topslope = slope;
		}

		if (topslope <= bottomslope)
			return false; // stop
	}

	return true; // passed the subsector ok
}

/*
=================
=
= PS_CrossBSPNode
=
= Returns true if strace crosses the given node successfuly
=================
*/
#if 0
bool PS_CrossBSPNode(int bspnum) // 8001F15C
{
	node_t *bsp;
	int side1, side2;
	int bsp_num;
	fixed_t dx, dy;
	fixed_t left, right;

	if (bspnum & NF_SUBSECTOR) {
		bsp_num = (bspnum & ~NF_SUBSECTOR);
//#if RANGECHECK
		if (bsp_num >= numsubsectors) {
			I_Error("ss %i with numss = %i",
				bsp_num, numsubsectors);
		}
//#endif
		return PS_CrossSubsector(&subsectors[bsp_num]);
	}

	bsp = &nodes[bspnum];

	// decide which side the start point is on
	side1 = 1;

	dx = (strace.x - bsp->line.x);
	dy = (strace.y - bsp->line.y);

	left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
	right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

	if (right < left)
		side1 = 0; // front side

	// cross the starting side
	if (!PS_CrossBSPNode(bsp->children[side1]))
		return false;

	// the partition plane is crossed here
	side2 = 1;

	dx = (t2x - bsp->line.x);
	dy = (t2y - bsp->line.y);

	left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
	right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

	if (right < left)
		side2 = 0; // front side

	if (side1 == side2)
		return true; // the line doesn't touch the other side

	// cross the ending side
	return PS_CrossBSPNode(bsp->children[side1 ^ 1]);
}
#else

#define CROSSBSP_STACK_SIZE 192
static int crossstack[CROSSBSP_STACK_SIZE];

bool /* __attribute__((noinline)) */ PS_CrossBSPNode(int bspnum)
{
	size_t crossstack_top = 0;
 
	// Push the initial node onto the stack
	crossstack[crossstack_top++] = bspnum;

	while (crossstack_top > 0) {
		// Pop the top element
		int current_bspnum = crossstack[--crossstack_top];

		// Check if it's a subsector
		if (current_bspnum & NF_SUBSECTOR) {
			int bsp_num = (current_bspnum & ~NF_SUBSECTOR);
#if RANGECHECK
			if (bsp_num >= numsubsectors) {
				I_Error("ss %i with numss = %i", bsp_num, numsubsectors);
			}
#endif

			if (!PS_CrossSubsector(&subsectors[bsp_num])) {
				return false;
			}
			continue;
		}

		node_t *bsp = &nodes[current_bspnum];

		// Decide which side the start point is on
		int side1 = 1;
		fixed_t dx = (strace.x - bsp->line.x);
		fixed_t dy = (strace.y - bsp->line.y);

		fixed_t left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
		fixed_t right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

		if (right < left) {
			side1 = 0; // front side
		}

		// Push the starting side onto the stack
		if (crossstack_top >= CROSSBSP_STACK_SIZE) {
			// overflowed stack, give up
			return false;
		}

 		crossstack[crossstack_top++] = bsp->children[side1];

		// Determine which side the endpoint is on
		int side2 = 1;
		dx = (t2x - bsp->line.x);
		dy = (t2y - bsp->line.y);

		left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
		right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

		if (right < left) {
			side2 = 0; // front side
		}

		// If the line doesn't touch the other side, skip
		if (side1 != side2) {
			if (crossstack_top >= CROSSBSP_STACK_SIZE) {
				// overflowed stack, give up
				return false;
			}

			crossstack[crossstack_top++] = bsp->children[side1 ^ 1];
		}
	}

	return true;
}
#endif