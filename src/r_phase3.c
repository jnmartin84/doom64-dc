//Renderer phase 3 - World Rendering Routines
//#include <gl/GL.h>
//#include <gl/GLU.h>
#include "doomdef.h"
#include "r_local.h"

#include <dc/matrix.h>
#include <dc/pvr.h>
#include <math.h>

#define SUBMIT_VERT_ARRAY 0
extern int brightness;
extern short SwapShort(short dat);
extern int VideoFilter;

extern pvr_poly_cxt_t **tcxt;
extern void **tsptrs;

extern pvr_poly_hdr_t **headers_for_sprites;
extern pvr_poly_hdr_t **headers2_for_sprites;

extern float *all_u;
extern float *all_v;
extern float *all_u2;
extern float *all_v2;


pvr_vertex_t __attribute__ ((aligned(32))) quad2[4];

pvr_poly_hdr_t hdr;
pvr_poly_cxt_t cxt;

pvr_poly_hdr_t thdr;

//d64Vertex_t dVTX[4];
d64Vertex_t *dVTX[4];
d64Triangle_t dT1, dT2;

const float inv64 = 1.0f / 64.0f;
const float inv255 = 1.0f / 255.0f;
const float inv256 = 1.0f / 256.0f;
const float inv1024 = 1.0f / 1024.0f;
const float inv65535 = 1.0f / 65536.0f;

static inline int vismask(d64Triangle_t *triangle)
{
	d64Vertex_t *v0 = &triangle->dVerts[0];
	d64Vertex_t *v1 = &triangle->dVerts[1];
	d64Vertex_t *v2 = &triangle->dVerts[2];

	return (v0->v.z > -v0->w) << 0 | (v1->v.z > -v1->w) << 1 | (v2->v.z > -v2->w) << 2;
}

static inline int vismask2(d64Triangle_t *triangle, d64Triangle_t *triangle2)
{
	d64Vertex_t *v0 = &triangle->dVerts[0];
	d64Vertex_t *v1 = &triangle->dVerts[1];
	d64Vertex_t *v2 = &triangle->dVerts[2];
	d64Vertex_t *v3 = &triangle2->dVerts[2];

	return (v0->v.z > -v0->w) << 0 | (v1->v.z > -v1->w) << 1 | (v2->v.z > -v2->w) << 2 | (v3->v.z > -v3->w) << 3;
}

/* 
credit to Kazade / glDC code for my clipping implementation
https://github.com/Kazade/GLdc/blob/572fa01b03b070e8911db43ca1fb55e3a4f8bdd5/GL/platforms/software.c#L140
*/

void blend_color(float t, uint32_t v1c, uint32_t v2c, uint32_t *dc)
{
	//a,r,g,b
	float v1[4];
	float v2[4];
	float d[4];
	const float invt = 1.0f - t;

	v1[0] = (float) ((v1c>>24)&0xff);
	v1[1] = (float) ((v1c>>16)&0xff);
	v1[2] = (float) ((v1c>>8)&0xff);
	v1[3] = (float) ((v1c)&0xff);

	v2[0] = (float) ((v2c>>24)&0xff);
	v2[1] = (float) ((v2c>>16)&0xff);
	v2[2] = (float) ((v2c>>8)&0xff);
	v2[3] = (float) ((v2c)&0xff);
	
	d[0] = invt*v1[0] + t*v2[0];
	d[1] = invt*v1[1] + t*v2[1];
	d[2] = invt*v1[2] + t*v2[2];
	d[3] = invt*v1[3] + t*v2[3];

	*dc = D64_PVR_PACK_COLOR((uint8_t)d[0],(uint8_t)d[1],(uint8_t)d[2],(uint8_t)d[3]);
}

void clip_edge(d64Vertex_t *v1, d64Vertex_t *v2, d64Vertex_t *out)
{
	const float d0 = v1->w + v1->v.z;
	const float d1 = v2->w + v2->v.z;
	const float t = (fabs(d0) * frsqrt((d1-d0)*(d1-d0))) + 0.000001f;
	const float invt = 1.0f - t;

	// I wish there was a field in pvr_vertex_t I could overload for this
	out->w   = invt * v1->w   + t * v2->w;

	out->v.x = invt * v1->v.x + t * v2->v.x;
	out->v.y = invt * v1->v.y + t * v2->v.y;
	out->v.z = invt * v1->v.z + t * v2->v.z;
	out->v.u = invt * v1->v.u + t * v2->v.u;
	out->v.v = invt * v1->v.v + t * v2->v.v;

	blend_color(t, v1->v.argb, v2->v.argb, &(out->v.argb));
	blend_color(t, v1->v.oargb, v2->v.oargb, &(out->v.oargb));
}

uint32_t lighted_color(uint32_t c, float lc)
{
	float r = (float)UNPACK_R(c) * inv255;
	float g = (float)UNPACK_G(c) * inv255;
	float b = (float)UNPACK_B(c) * inv255;
	float a = (float)UNPACK_A(c) * inv255;

	return PVR_PACK_COLOR(a, (0.5 + (r*0.5))*lc, (0.5 + (g*0.5))*lc, (0.5 + (b*0.5))*lc);
}

pvr_vertex_t __attribute__((aligned(32))) cq_stverts[8];

void clip_quad(d64Triangle_t *triangle, d64Triangle_t *triangle2, pvr_poly_hdr_t *hdr, int lightlevel, pvr_list_t list, int specd) {
	int vm = vismask2(triangle, triangle2);

	float lightc = (float)lightlevel * inv255;

	if (!vm) {
		return;
	}
		
	if (vm == 15) {
		if (!specd) {
			triangle->dVerts[0].v.oargb = lighted_color(triangle->dVerts[0].v.argb, lightc);
			triangle->dVerts[1].v.oargb = lighted_color(triangle->dVerts[1].v.argb, lightc);
			triangle->dVerts[2].v.oargb = lighted_color(triangle->dVerts[2].v.argb, lightc);
			triangle2->dVerts[2].v.oargb = lighted_color(triangle2->dVerts[2].v.argb, lightc);
		}

		triangle->dVerts[0].v.flags = PVR_CMD_VERTEX;
		triangle->dVerts[1].v.flags = PVR_CMD_VERTEX;
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;
		triangle2->dVerts[2].v.flags = PVR_CMD_VERTEX_EOL;

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle2->dVerts[2]);

#if SUBMIT_VERT_ARRAY
		memcpy(&cq_stverts[2], &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		memcpy(&cq_stverts[0], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&cq_stverts[1], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		memcpy(&cq_stverts[3], &triangle2->dVerts[2].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, cq_stverts, sizeof(pvr_vertex_t) * 4);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle2->dVerts[2].v, sizeof(pvr_vertex_t));
#endif
	} else {
		clip_triangle(triangle, hdr, lightlevel, list, specd);
		clip_triangle(triangle2, hdr, lightlevel, list, specd);
	}
}

void clip_triangle(d64Triangle_t *triangle, pvr_poly_hdr_t *hdr, int lightlevel, pvr_list_t list, int specd) {
#if SUBMIT_VERT_ARRAY
	pvr_vertex_t __attribute__((aligned(32))) stverts[6];
#endif
	int vm = vismask(triangle);
	float lightc = (float)lightlevel * inv255;

	if (!vm) {
		return;
	}

	triangle->dVerts[0].v.flags = PVR_CMD_VERTEX;
	triangle->dVerts[1].v.flags = PVR_CMD_VERTEX;
	triangle->dVerts[2].v.flags = PVR_CMD_VERTEX_EOL;

	if (!specd) {
		triangle->dVerts[0].v.oargb = lighted_color(triangle->dVerts[0].v.argb, lightc);
		triangle->dVerts[1].v.oargb = lighted_color(triangle->dVerts[1].v.argb, lightc);
		triangle->dVerts[2].v.oargb = lighted_color(triangle->dVerts[2].v.argb, lightc);
	}

	switch (vm) {
	/* all verts visible */
	case 7: {
		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);

#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 3);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[0] visible */
	case 1: {
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[1]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);
		
#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->spare[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->spare[1].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 3);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[1] visible */
	case 2: {
		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->spare[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->spare[1].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 3);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[2] visible */
	case 4: {
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;
#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->spare[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->spare[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 3);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[0] and dVerts[1] visible */
	case 3: {
		/* out 1 */
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[0]);

		/* out 2 */
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);
		
		triangle->spare[0].v.flags = PVR_CMD_VERTEX_EOL;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;
#if SUBMIT_VERT_ARRAY		
		memcpy(&stverts[0], &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->spare[0].v, sizeof(pvr_vertex_t));

		memcpy(&stverts[3], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[4], &triangle->spare[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[5], &triangle->spare[0].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 6);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[0] and dVerts[2] visible */
	case 5: {
		/* out 1 */
		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);

		/* out 2 */
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);//, vm);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;

#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->spare[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));

		triangle->spare[0].v.flags = PVR_CMD_VERTEX_EOL;
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;

		memcpy(&stverts[3], &triangle->spare[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[4], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[5], &triangle->spare[0].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 6);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;
		triangle->spare[0].v.flags = PVR_CMD_VERTEX_EOL;
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	/* dVerts[1] and dVerts[2] visible */
	case 6: {
		/* out 1 */
		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[1]);

		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

#if SUBMIT_VERT_ARRAY
		memcpy(&stverts[0], &triangle->spare[0].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[1], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[2], &triangle->spare[1].v, sizeof(pvr_vertex_t));

		memcpy(&stverts[3], &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[4], &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		memcpy(&stverts[5], &triangle->spare[1].v, sizeof(pvr_vertex_t));

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, stverts, sizeof(pvr_vertex_t) * 6);
#else
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
#endif
	}
	break;
	}
}

//-----------------------------------//
void R_RenderWorld(subsector_t *sub);

void R_WallPrep(seg_t *seg);
void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight, int topOffset, int bottomOffset, int topColor, int bottomColor);
void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color);

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color, int ceiling, int lightlevel, int alpha);

void R_RenderThings(subsector_t *sub);
void R_RenderLaser(mobj_t *thing);
void R_RenderPSprites(void);
//-----------------------------------//

extern pvr_ptr_t *tex_txr_ptr;

extern uint8_t *pt;
int renderstuffinit = 0;
extern int firsttex;

void R_RenderAll(void) // 80026590
{
	subsector_t *sub;

	while (endsubsector--, (endsubsector >= solidsubsectors)) {
		sub = *endsubsector;
		frontsector = sub->sector;
		R_RenderWorld(sub);

		sub->drawindex = 0x7fff;
	}
}

void R_RenderWorld(subsector_t *sub) // 80026638
{
	leaf_t *lf;
	seg_t *seg;

	fixed_t xoffset;
	fixed_t yoffset;
	int numverts;
	int i;

#if 0
    gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 255);
#endif
	numverts = sub->numverts;

	/* */
	/* Render Walls */
	/* */
	lf = &leafs[sub->leaf];
	for (i = 0; i < numverts; i++) {
		seg = lf->seg;

		if (seg && (seg->flags & 1)) {
			R_WallPrep(seg);
		}

		lf++;
	}

	/* */
	/* Render Ceilings */
	/* */
	if ((frontsector->ceilingpic != -1) && (viewz < frontsector->ceilingheight)) {
		if (frontsector->flags & MS_SCROLLCEILING) {
			xoffset = frontsector->xoffset;
			yoffset = frontsector->yoffset;
		} else {
			xoffset = 0;
			yoffset = 0;
		}

		lf = &leafs[sub->leaf];
		R_RenderPlane(lf, numverts, frontsector->ceilingheight >> FRACBITS,
						textures[frontsector->ceilingpic],
						xoffset, yoffset,
						lights[frontsector->colors[0]].rgba, 1, frontsector->lightlevel, 255);
	}

	/* */
	/* Render Floors */
	/* */
	if ((frontsector->floorpic != -1) && (frontsector->floorheight < viewz)) {
		if (!(frontsector->flags & MS_LIQUIDFLOOR)) {
			if (frontsector->flags & MS_SCROLLFLOOR) {
				xoffset = frontsector->xoffset;
				yoffset = frontsector->yoffset;
			} else {
				xoffset = 0;
				yoffset = 0;
			}

			lf = &leafs[sub->leaf];
			R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
							textures[frontsector->floorpic],
							xoffset, yoffset,
							lights[frontsector->colors[1]].rgba, 0, frontsector->lightlevel, 255);
		} else {
#if 0
			gDPPipeSync(GFX1++);
			gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2);
#endif
			if (frontsector->flags & MS_SCROLLFLOOR) {
				xoffset = frontsector->xoffset;
				yoffset = frontsector->yoffset;
			} else {
				xoffset = scrollfrac;
				yoffset = 0;
			}

			//--------------------------------------------------------------
			lf = &leafs[sub->leaf];
			R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
							textures[frontsector->floorpic + 1],
							xoffset, yoffset,
							lights[frontsector->colors[1]].rgba, 0, frontsector->lightlevel, 255);
			//--------------------------------------------------------------

			lf = &leafs[sub->leaf];
#if 0
			gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 160);
#endif
			R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
							textures[frontsector->floorpic],
							-yoffset, xoffset,
							lights[frontsector->colors[1]].rgba, 0, frontsector->lightlevel, 160);
#if 0
			gDPPipeSync(GFX1++);
			gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
#endif
		}
	}

	/* */
	/* Render Things */
	/* */
	R_RenderThings(sub);
}

void R_WallPrep(seg_t *seg)
{
	sector_t *backsector;
	line_t *li;
	side_t *side;
	fixed_t f_ceilingheight;
	fixed_t f_floorheight;
	fixed_t b_ceilingheight;
	fixed_t b_floorheight;
	fixed_t m_top;
	fixed_t m_bottom;
	fixed_t rowoffs;
	unsigned int height2;
	int height;
	int frontheight;
	int sideheight1;
	short pic;

	float r1, g1, b1;
	float r2, g2, b2;
	unsigned int thingcolor;
	unsigned int upcolor;
	unsigned int lowcolor;
	unsigned int topcolor;
	unsigned int bottomcolor;
	unsigned int tmp_upcolor;
	unsigned int tmp_lowcolor;
	int curRowoffset;
	float invfrontheight;

	r1=g1=b1=0;
	r2=g2=b2=0;

	topcolor=tmp_upcolor=bottomcolor=tmp_lowcolor=0;

	li = seg->linedef;
	side = seg->sidedef;
	
	// [GEC] Prevents errors in textures in T coordinates, but is not applied to switches
    curRowoffset = side->rowoffset & (127 << FRACBITS);

	thingcolor = lights[frontsector->colors[2]].rgba;
	upcolor = lights[frontsector->colors[3]].rgba;
	lowcolor = lights[frontsector->colors[4]].rgba;

	// get front side top and bottom
	f_ceilingheight = frontsector->ceilingheight >> 16;
	f_floorheight = frontsector->floorheight >> 16;
	frontheight = f_ceilingheight - f_floorheight;
	invfrontheight = 1.0f / (float)frontheight;

	if (li->flags & ML_BLENDING) {
		r1 = (float)((upcolor  >> 24) & 0xff);
		g1 = (float)((upcolor  >> 16) & 0xff);
		b1 = (float)((upcolor  >> 8) & 0xff);
		r2 = (float)((lowcolor >> 24) & 0xff);
		g2 = (float)((lowcolor >> 16) & 0xff);
		b2 = (float)((lowcolor >> 8) & 0xff);
		tmp_upcolor = upcolor;
		tmp_lowcolor = lowcolor;
	} else {
		topcolor = thingcolor;
		bottomcolor = thingcolor;
	} 

	m_bottom = f_floorheight; // set middle bottom
	m_top = f_ceilingheight;  // set middle top

	backsector = seg->backsector;
	if (backsector) {
		b_floorheight = backsector->floorheight >> 16;
		b_ceilingheight = backsector->ceilingheight >> 16;

		if ((b_ceilingheight < f_ceilingheight) && (backsector->ceilingpic != -1)) {
			height = f_ceilingheight - b_ceilingheight;

			if (li->flags & ML_DONTPEGTOP) {
				rowoffs = (curRowoffset >> 16) + height;
			} else {
				rowoffs = ((height + 127) & ~127) + (curRowoffset >> 16);
			}

			if (li->flags & ML_BLENDING) {
				if (frontheight && !(li->flags & ML_BLENDFULLTOP)) {
#if 1
					sideheight1 = b_ceilingheight - f_floorheight;

					float scale1 = (float)sideheight1 * invfrontheight; // / (float)frontheight;
					float scale2 = (float)height * invfrontheight; // / (float)frontheight;
			
					float nr1 = r1*scale1;
					float ng1 = g1*scale1;
					float nb1 = b1*scale1;

					float nr2 = r2*scale2;
					float ng2 = g2*scale2;
					float nb2 = b2*scale2;

					float rf = nr1 + nr2;
					float gf = ng1 + ng2;
					float bf = nb1 + nb2;

					if (!((rf < 256) && (gf < 256) && (bf < 256))) {
						float scale = 255.0f;

						if (rf >= gf && rf >= bf) {
							scale /= rf;
						} else if (gf >= rf && gf >= bf) {
							scale /= gf;
						} else {
							scale /= bf;
						}

						rf *= scale;
						gf *= scale;
						bf *= scale;
					}

					tmp_lowcolor = ((int)rf << 24) | ((int)gf << 16) | ((int)bf << 8) | 0xff;

					if (gamemap == 3 && brightness > 100) {
						int x1 = li->v1->x >> 16;
						int y1 = li->v1->y >> 16;
						int x2 = li->v2->x >> 16;
						int y2 = li->v2->y >> 16;

						if ( ( (x1 == 1040 && y1 == -176) && (x2 == 1008 && y2 == -176) ) ||
							( (x1 == 1008 && y1 == -464) && (x2 == 1040 && y2 == -464) ) ) {
							float scale = 1.0f - ((float)(brightness-100)*0.0025f);

							tmp_upcolor = ((int)(r1*scale)<<24) |
											((int)(g1*scale)<<16) |
											((int)(b1*scale)<<8) |
											0xff;

							tmp_lowcolor = ((int)(rf*scale) << 24) |
											((int)(gf*scale) << 16) |
											((int)(bf*scale) << 8) |
											0xff;
						}
					}
#endif
#if 0
                    if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << 16) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }
					unsigned int rf = (((unsigned int)r2 * height2) >> 16) + (((unsigned int)r1*(65536 - height2)) >> 16);
					unsigned int gf = (((unsigned int)g2 * height2) >> 16) + (((unsigned int)g1*(65536 - height2)) >> 16);
					unsigned int bf = (((unsigned int)b2 * height2) >> 16) + (((unsigned int)b1*(65536 - height2)) >> 16);

#if 0
					if (!((rf < 256) && (gf < 256) && (bf < 256))) {
						unsigned int max;
						if (rf >= gf && rf >= bf) {
							max = rf;
						} else if (gf >= rf && gf >= bf) {
							max = gf;
						} else {
							max = bf;
						}

						rf = (((rf<<16) / max) * 255) >> 16;
						gf = (((gf<<16) / max) * 255) >> 16;
						bf = (((bf<<16) / max) * 255) >> 16;
					}
#endif
/*                    tmp_lowcolor = (((((r2 - r1) * height2) >> 16) + r1) << 24) |
                                   (((((g2 - g1) * height2) >> 16) + g1) << 16) |
                                   (((((b2 - b1) * height2) >> 16) + b1) << 8)  | 0xff;*/
					tmp_lowcolor = (rf << 24) | (gf << 16) | (bf << 8) | 0xff;
#endif
				} 

				if (li->flags & ML_INVERSEBLEND) {
					bottomcolor = tmp_upcolor;
					topcolor = tmp_lowcolor;
				} else {
					topcolor = tmp_upcolor;
					bottomcolor = tmp_lowcolor;
				}

				// clip middle color upper
				upcolor = tmp_lowcolor;
			}

			R_RenderWall(seg, li->flags, textures[side->toptexture],
						 f_ceilingheight, b_ceilingheight,
						 rowoffs - height, rowoffs,
						 topcolor, bottomcolor);

			m_top = b_ceilingheight; // clip middle top height

			if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_SWITCHX08) {
				if (SWITCHMASK(li->flags) == ML_SWITCHX04) {
					pic = side->bottomtexture;
					rowoffs = side->rowoffset >> 16;
				} else {
					pic = side->midtexture;
					rowoffs = side->rowoffset >> 16;
				}
				R_RenderSwitch(seg, pic, b_ceilingheight + rowoffs + 48, thingcolor);
			}
		}

		if (f_floorheight < b_floorheight) {
			height = f_ceilingheight - b_floorheight;

			if ((li->flags & ML_DONTPEGBOTTOM) == 0) {
				rowoffs = curRowoffset >> 16;
			} else {
				rowoffs = height + (curRowoffset >> 16);
			}

			if (li->flags & ML_BLENDING) {
				if (frontheight && !(li->flags & ML_BLENDFULLBOTTOM)) {
#if 1
					int sideheight1 = b_floorheight - f_floorheight;

					float scale1 = (float)sideheight1 * invfrontheight; // / (float)frontheight;
					float scale2 = (float)height * invfrontheight; // / (float)frontheight;
			
					float nr1 = r1*scale1;
					float ng1 = g1*scale1;
					float nb1 = b1*scale1;

					float nr2 = r2*scale2;
					float ng2 = g2*scale2;
					float nb2 = b2*scale2;

					float rf = nr1 + nr2;
					float gf = ng1 + ng2;
					float bf = nb1 + nb2;

					if (!((rf < 256) && (gf < 256) && (bf < 256))) {
						float scale = 255.0f;

						if (rf >= gf && rf >= bf) {
							scale /= rf;
						} else if (gf >= rf && gf >= bf) {
							scale /= gf;
						} else {
							scale /= bf;
						}

						rf *= scale;
						gf *= scale;
						bf *= scale;
					}

					tmp_upcolor = ((int)rf << 24) | ((int)gf << 16) | ((int)bf << 8) | 0xff;

					if (gamemap == 3 && brightness > 100) {
						int x1 = li->v1->x >> 16;
						int y1 = li->v1->y >> 16;
						int x2 = li->v2->x >> 16;
						int y2 = li->v2->y >> 16;

						if ( ( (x1 == 1040 && y1 == -176) && (x2 == 1008 && y2 == -176) ) ||
							( (x1 == 1008 && y1 == -464) && (x2 == 1040 && y2 == -464) ) ) {
							float scale = 1.0f - ((float)(brightness-100)*0.0025f);

							tmp_lowcolor = ((int)(r2*scale)<<24) |
											((int)(g2*scale)<<16) |
											((int)(b2*scale)<<8) |
											0xff;

							tmp_upcolor = ((int)(rf*scale) << 24) |
											((int)(gf*scale) << 16) |
											((int)(bf*scale) << 8) |
											0xff;
						}
					}
#endif
#if 0
					if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << 16) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }

					unsigned int rf = (((unsigned int)r2 * height2) >> 16) + (((unsigned int)r1*(65536 - height2)) >> 16);
					unsigned int gf = (((unsigned int)g2 * height2) >> 16) + (((unsigned int)g1*(65536 - height2)) >> 16);
					unsigned int bf = (((unsigned int)b2 * height2) >> 16) + (((unsigned int)b1*(65536 - height2)) >> 16);

#if 0
					if (!((rf < 256) && (gf < 256) && (bf < 256))) {
						unsigned int max;
						if (rf >= gf && rf >= bf) {
							max = rf;
						} else if (gf >= rf && gf >= bf) {
							max = gf;
						} else {
							max = bf;
						}

						rf = (((rf<<16) / max) * 255) >> 16;
						gf = (((gf<<16) / max) * 255) >> 16;
						bf = (((bf<<16) / max) * 255) >> 16;
					}
#endif 

/*                    tmp_upcolor = (((((r2 - r1) * height2) >> 16) + r1) << 24) |
                                   (((((g2 - g1) * height2) >> 16) + g1) << 16) |
                                   (((((b2 - b1) * height2) >> 16) + b1) << 8)  | 0xff;*/
					tmp_upcolor = (rf << 24) | (gf << 16) | (bf << 8) | 0xff;
#endif
				}

				topcolor = tmp_upcolor;
				bottomcolor = lowcolor;

				// clip middle color lower
				lowcolor = tmp_upcolor;
			}

			R_RenderWall(seg, li->flags, textures[side->bottomtexture],
						 b_floorheight, f_floorheight,
						 rowoffs, rowoffs + (b_floorheight - f_floorheight),
						 topcolor, bottomcolor);

			m_bottom = b_floorheight; // clip middle bottom height
			if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_CHECKFLOORHEIGHT) {
				if (SWITCHMASK(li->flags) == ML_SWITCHX02) {
					pic = side->toptexture;
					rowoffs = side->rowoffset >> 16;
				} else {
					pic = side->midtexture;
					rowoffs = side->rowoffset >> 16;
				}
				R_RenderSwitch(seg, pic, b_floorheight + rowoffs - 16, thingcolor);
			}
		}

		if (!(li->flags & ML_DRAWMASKED)) {
			return;
		}
	}

	height = m_top - m_bottom;

	if (li->flags & ML_DONTPEGBOTTOM) {
		rowoffs = ((height + 127) & ~127) + (curRowoffset >> 16);
	} else if (li->flags & ML_DONTPEGTOP) {
		rowoffs = (curRowoffset >> 16) - m_bottom;
	} else {
		rowoffs = (curRowoffset >> 16) + height;
	}

	if (li->flags & ML_BLENDING) {
		topcolor = upcolor;
		bottomcolor = lowcolor;
	}

	R_RenderWall(seg, li->flags, textures[side->midtexture],
				 m_top, m_bottom,
				 rowoffs - height, rowoffs,
				 topcolor, bottomcolor);

	if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) {
		if (SWITCHMASK(li->flags) == ML_SWITCHX02) {
			pic = side->toptexture;
			rowoffs = side->rowoffset >> 16;
		} else {
			pic = side->bottomtexture;
			rowoffs = side->rowoffset >> 16;
		}
		R_RenderSwitch(seg, pic, m_bottom + rowoffs + 48, thingcolor);
	}
}

int last_width = 64;
int last_height = 64;

void P_CachePvrTexture(int i, int tag);

void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight,
				  int topOffset, int bottomOffset, int topColor, int bottomColor) // 80027138
{
	byte *data;
	vertex_t *v1;
	vertex_t *v2;
	int cms, cmt;
	int wshift, hshift;
	int texnum = (texture >> 4) - firsttex;

	// [GEC] Prevents errors in textures in S coordinates
    int curTextureoffset = (seg->sidedef->textureoffset + seg->offset) & (127 << FRACBITS);	
	
	if (texture != 16) {
		if (flags & ML_HMIRROR) {
			cms = 1; /* G_TX_MIRROR */
		} else {
			cms = 0; /* G_TX_NOMIRROR */
		}

		if (flags & ML_VMIRROR) {
			cmt = 1; /* G_TX_MIRROR */
		} else {
			cmt = 0; /* G_TX_NOMIRROR */
		}

		if ((texture != globallump) || (globalcm != (cms | cmt))) {
			data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64);
			
			P_CachePvrTexture(texnum, PU_CACHE);

			wshift = SwapShort(((textureN64_t*)data)->wshift);
			hshift = SwapShort(((textureN64_t*)data)->hshift);

			last_width = 1 << wshift;
			last_height = 1 << hshift;

			// cms is S/H mirror
			// cmt is T/V mirror
			if (cms && !cmt) {
				tcxt[texnum][texture&15].txr.uv_flip = PVR_UVFLIP_U;
			} else if (!cms && cmt) {
				tcxt[texnum][texture&15].txr.uv_flip = PVR_UVFLIP_V;
			} else if (cms && cmt) {
				tcxt[texnum][texture&15].txr.uv_flip = PVR_UVFLIP_UV;
			} else {
				tcxt[texnum][texture&15].txr.uv_flip = PVR_UVFLIP_NONE;
			}

			if (!VideoFilter) {
				tcxt[texnum][texture&15].txr.filter = PVR_FILTER_BILINEAR;
			} else {
				tcxt[texnum][texture&15].txr.filter = PVR_FILTER_NONE;
			}

			pvr_poly_compile(&thdr, &tcxt[texnum][texture&15]);
			globallump = texture;
			globalcm = (cms | cmt);
		}

		v1 = seg->v1;
		v2 = seg->v2;

		signed short sx1 = (signed short)(v1->x >> 16);
		signed short sx2 = (signed short)(v2->x >> 16);
		signed short sy1 = -((signed short)(v1->y >> 16));
		signed short sy2 = -((signed short)(v2->y >> 16));

		float x1 = (float)sx1;
		float x2 = (float)sx2;
		float z1 = (float)sy1;
		float z2 = (float)sy2;
		float y1 = (float)topHeight;
		float y2 = (float)bottomHeight;
		short stu1 = curTextureoffset >> 16;//(seg->sidedef->textureoffset + seg->offset) >> 16;
		short stu2 = stu1 + (seg->length >> 4);
		short stv1 = topOffset;
		short stv2 = bottomOffset;
		float tu1 = (float)stu1 / (float)last_width;
		float tu2 = (float)stu2 / (float)last_width;
		float tv1 = (float)stv1 / (float)last_height;
		float tv2 = (float)stv2 / (float)last_height;

		dVTX[0] = &(dT1.dVerts[0]);
		dVTX[1] = &(dT1.dVerts[1]);
		dVTX[2] = &(dT1.dVerts[2]);
		dVTX[3] = &(dT2.dVerts[2]);
		dVTX[0]->v.x = dVTX[3]->v.x = x1;
		dVTX[1]->v.x = dVTX[2]->v.x = x2;
		dVTX[0]->v.y = dVTX[1]->v.y = y1;
		dVTX[3]->v.y = dVTX[2]->v.y = y2;
		dVTX[0]->v.z = dVTX[3]->v.z = z1;
		dVTX[1]->v.z = dVTX[2]->v.z = z2;
		dVTX[0]->v.u = dVTX[3]->v.u = tu1;
		dVTX[1]->v.u = dVTX[2]->v.u = tu2;
		dVTX[0]->v.v = dVTX[1]->v.v = tv1;
		dVTX[3]->v.v = dVTX[2]->v.v = tv2;

		transform_vert(dVTX[0]);
		transform_vert(dVTX[1]);
		transform_vert(dVTX[2]);
		transform_vert(dVTX[3]);

		color_vert(dVTX[0], topColor);
		color_vert(dVTX[1], topColor);
		color_vert(dVTX[2], bottomColor);
		color_vert(dVTX[3], bottomColor);

		memcpy(&(dT2.dVerts[0]), dVTX[0], sizeof(d64Vertex_t));
		memcpy(&(dT2.dVerts[1]), dVTX[2], sizeof(d64Vertex_t));

		clip_quad(&dT1, &dT2, &thdr, frontsector->lightlevel, PVR_LIST_TR_POLY, 0);
	}
}

int last_sw;
int last_sh;

void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color) // 80027654
{
	byte *data;
	vertex_t *v1;
	vertex_t *v2;
	fixed_t x, y;
	fixed_t sin, cos;
	int wshift, hshift;

	if (texture != globallump) {
		data = W_CacheLumpNum(firsttex + texture, PU_CACHE, dec_d64);

		P_CachePvrTexture(texture, PU_CACHE);

		wshift = SwapShort(((textureN64_t*)data)->wshift);
		hshift = SwapShort(((textureN64_t*)data)->hshift);

		last_sw = 1 << wshift;
		last_sh = 1 << hshift;

		if (!VideoFilter) {
			tcxt[texture][0].txr.filter = PVR_FILTER_BILINEAR;
		} else {
			tcxt[texture][0].txr.filter = PVR_FILTER_NONE;
		}
		pvr_poly_compile(&thdr, &tcxt[texture][0]);
		globallump = texture;
	}

#if 0
	gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, 0, 1);
	gSPVertex(GFX1++, VTX1, 4, 0);
	gSP1Quadrangle(GFX1++, 0, 1, 2, 3, 1);
#endif

	v1 = seg->linedef->v1;
	v2 = seg->linedef->v2;

	x = v1->x + v2->x;
	if (x < 0) {
		x = x + 1;
	}

	y = v1->y + v2->y;
	if (y < 0) {
		y = y + 1;
	}

	x >>= 1;
	y >>= 1;

	cos = finecosine[seg->angle >> ANGLETOFINESHIFT] << 1;
	sin = finesine[seg->angle >> ANGLETOFINESHIFT] << 1;

	float x1 = (float)(((x) - (cos << 3) + sin) >> 16);
	float x2 = (float)(((x) + (cos << 3) + sin) >> 16);
	float y1 = (float)topOffset;
	float y2 = y1 - 32.0f;
	float z1 = (float)(((-y) + (sin << 3) + cos) >> 16);
	float z2 = (float)(((-y) - (sin << 3) + cos) >> 16);

	float tu1 = 0.0f;
	float tu2 = 32.0f / (float)last_sw;
	float tv1 = 0.0f;
	float tv2 = 32.0f / (float)last_sh;

	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);
	dVTX[3] = &(dT2.dVerts[2]);

	dVTX[0]->v.x = dVTX[3]->v.x = x1;
	dVTX[1]->v.x = dVTX[2]->v.x = x2;

	dVTX[0]->v.y = dVTX[1]->v.y = y1;
	dVTX[3]->v.y = dVTX[2]->v.y = y2;

	dVTX[0]->v.z = dVTX[3]->v.z = z1;
	dVTX[1]->v.z = dVTX[2]->v.z = z2;

	dVTX[0]->v.u = dVTX[3]->v.u = tu1;
	dVTX[1]->v.u = dVTX[2]->v.u = tu2;
	dVTX[0]->v.v = dVTX[1]->v.v = tv1;
	dVTX[3]->v.v = dVTX[2]->v.v = tv2;

	transform_vert(dVTX[0]);
	transform_vert(dVTX[1]);
	transform_vert(dVTX[2]);
	transform_vert(dVTX[3]);

	color_vert(dVTX[0], color);
	color_vert(dVTX[1], color);
	color_vert(dVTX[2], color);
	color_vert(dVTX[3], color);

	memcpy(&(dT2.dVerts[0]), dVTX[0], sizeof(d64Vertex_t));
	memcpy(&(dT2.dVerts[1]), dVTX[2], sizeof(d64Vertex_t));

#if 0
	clip_triangle(&dT1, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
	clip_triangle(&dT2, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
#endif
	clip_quad(&dT1, &dT2, &thdr, frontsector->lightlevel, PVR_LIST_TR_POLY, 0);
}

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color, int ceiling, int lightlevel, int alpha) // 80027B68
{
	vertex_t *vrt;
	fixed_t x;
	fixed_t y;
	int idx;
	int v00, v01, v02;
	short stu,stv;
	float tu,tv;
	d64Vertex_t dv0;

	uint32_t newcolor = (color & 0xffffff00) | alpha;
	int texnum = (texture >> 4) - firsttex;

	leaf_t *lf = leaf;

	if (texture != globallump) {
		P_CachePvrTexture(texnum, PU_CACHE);

		if (alpha != 255) {
			tcxt[texnum][texture&15].blend.src = PVR_BLEND_ONE;
			tcxt[texnum][texture&15].blend.dst = PVR_BLEND_DESTALPHA;
		} else {
			tcxt[texnum][texture&15].blend.src = PVR_BLEND_SRCALPHA;
			tcxt[texnum][texture&15].blend.dst = PVR_BLEND_INVSRCALPHA;
		}

		if (!VideoFilter) {
			tcxt[texnum][texture&15].txr.filter = PVR_FILTER_BILINEAR;
		} else {
			tcxt[texnum][texture&15].txr.filter = PVR_FILTER_NONE;
		}

		pvr_poly_compile(&thdr, &tcxt[texnum][texture&15]);
		globallump = texture;
	}

	if (numverts >= 32)
		numverts = 32;

	/* this is the origin vertex for a bunch of the triangles, this is a special case */
	dVTX[0] = &(dv0);
		
	dVTX[1] = &(dT1.dVerts[0]);
	dVTX[2] = &(dT1.dVerts[1]);
	dVTX[3] = &(dT1.dVerts[2]);

	vrt = lf[0].vertex;

	dVTX[0]->v.x = ((float)(vrt->x >> 16));
	dVTX[0]->v.y = (float)(zpos);
	dVTX[0]->v.z = -((float)(vrt->y >> 16));
	x = ((vrt->x + xpos) >> 16) & -64;
	y = ((vrt->y + ypos) >> 16) & -64;

	stu = (((vrt->x + xpos) & 0x3f0000U) >> 16);
	stv = -(((vrt->y + ypos) & 0x3f0000U) >> 16);
	tu = (float)stu * inv64;
	tv = (float)stv * inv64;

	dVTX[0]->v.u = tu;
	dVTX[0]->v.v = tv;

	transform_vert(dVTX[0]);
	color_vert(dVTX[0], newcolor);

	if (numverts & 1)
	{
		vertex_t *vrt1;
		vertex_t *vrt2;

		idx = 2;

		vrt1 = lf[1].vertex;

		dVTX[1]->v.x = ((float)(vrt1->x >> 16));
		dVTX[1]->v.y = (float)(zpos);
		dVTX[1]->v.z = -((float)(vrt1->y >> 16));

		stu = (((vrt1->x + xpos) >> FRACBITS) - x);
		stv = -(((vrt1->y + ypos) >> FRACBITS) - y);
		tu = (float)stu * inv64;
		tv = (float)stv * inv64;

		dVTX[1]->v.u = tu;
		dVTX[1]->v.v = tv;

		vrt2 = lf[2].vertex;

		dVTX[2]->v.x = ((float)(vrt2->x >> 16));
		dVTX[2]->v.y = (float)(zpos);
		dVTX[2]->v.z = -((float)(vrt2->y >> 16));

		stu = (((vrt2->x + xpos) >> FRACBITS) - x);
		stv = -(((vrt2->y + ypos) >> FRACBITS) - y);
		tu = (float)stu * inv64;
		tv = (float)stv * inv64;

		dVTX[2]->v.u = tu;
		dVTX[2]->v.v = tv;

		transform_vert(dVTX[1]);
		transform_vert(dVTX[2]);

		color_vert(dVTX[1], newcolor);
		color_vert(dVTX[2], newcolor);
		
		if (ceiling) {
		memcpy(&(dT2.dVerts[0]), dVTX[2], sizeof(d64Vertex_t));
		memcpy(&(dT2.dVerts[1]), dVTX[1], sizeof(d64Vertex_t));
		memcpy(&(dT2.dVerts[2]), &dv0, sizeof(d64Vertex_t));
		}
		else {
		memcpy(&(dT2.dVerts[0]), &dv0, sizeof(d64Vertex_t));
		memcpy(&(dT2.dVerts[1]), dVTX[1], sizeof(d64Vertex_t));
		memcpy(&(dT2.dVerts[2]), dVTX[2], sizeof(d64Vertex_t));
		}
		clip_triangle(&dT2, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
	}
	else
	{
		idx = 1;
	}

	numverts--;

	if (ceiling) {
		dVTX[1] = &(dT1.dVerts[2]);
		dVTX[2] = &(dT1.dVerts[1]);
		dVTX[3] = &(dT1.dVerts[0]);
	} else {
		dVTX[1] = &(dT1.dVerts[0]);
		dVTX[2] = &(dT1.dVerts[1]);
		dVTX[3] = &(dT1.dVerts[2]);
	}

	if (idx < numverts)
	{
		v00 = idx + 0;
		v01 = idx + 1;
		v02 = idx + 2;
		do
		{
			vertex_t *vrt1;
			vertex_t *vrt2;
			vertex_t *vrt3;

			vrt1 = lf[v00].vertex;
			vrt2 = lf[v01].vertex;
			vrt3 = lf[v02].vertex;

			stu = (((vrt1->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt1->y + ypos) >> FRACBITS) - y);
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
			dVTX[1]->v.x = ((float)(vrt1->x >> 16));
			dVTX[1]->v.y = (float)(zpos);
			dVTX[1]->v.z = -((float)(vrt1->y >> 16));
			dVTX[1]->v.u = tu;
			dVTX[1]->v.v = tv;

			stu = (((vrt2->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt2->y + ypos) >> FRACBITS) - y);
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
			dVTX[2]->v.x = ((float)(vrt2->x >> 16));
			dVTX[2]->v.y = (float)(zpos);
			dVTX[2]->v.z = -((float)(vrt2->y >> 16));
			dVTX[2]->v.u = tu;
			dVTX[2]->v.v = tv;

			stu = (((vrt3->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt3->y + ypos) >> FRACBITS) - y);
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
			dVTX[3]->v.x = ((float)(vrt3->x >> 16));
			dVTX[3]->v.y = (float)(zpos);
			dVTX[3]->v.z = -((float)(vrt3->y >> 16));
			dVTX[3]->v.u = tu;
			dVTX[3]->v.v = tv;

			transform_vert(dVTX[1]);
			transform_vert(dVTX[2]);
			transform_vert(dVTX[3]);

			color_vert(dVTX[1], newcolor);
			color_vert(dVTX[2], newcolor);
			color_vert(dVTX[3], newcolor);

			if(ceiling) {
				memcpy(&(dT2.dVerts[0]), &dv0, sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[1]), dVTX[3], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[2]), dVTX[1], sizeof(d64Vertex_t));
			} else {
				memcpy(&(dT2.dVerts[0]), dVTX[1], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[1]), dVTX[3], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[2]), &dv0, sizeof(d64Vertex_t));
			}

			clip_triangle(&dT1, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
			clip_triangle(&dT2, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);

			v00 += 2;
			v01 += 2;
			v02 += 2;
		} while (v02 < (numverts + 2));
	}

#if 0
	vrt = lf[0].vertex;
	stu = (((vrt->x + xpos) & 0x3f0000U) >> 16);
	stv = -(((vrt->y + ypos) & 0x3f0000U) >> 16);
	dVTX[0]->v.x = (float)(vrt->x >> 16);
	dVTX[0]->v.y = (float)zpos;
	dVTX[0]->v.z = (float)-(vrt->y >> 16);
	dVTX[0]->v.u = (float)stu * inv64;
	dVTX[0]->v.v = (float)stv * inv64;

	transform_vert(dVTX[0]);

	color_vert(dVTX[0], newcolor);

	x = ((vrt->x + xpos) >> 16) & -64;
	y = ((vrt->y + ypos) >> 16) & -64;

	if (numverts & 1) {
		vertex_t *vrt1;
		vertex_t *vrt2;

		idx = 2;

		vrt1 = lf[1].vertex;
		stu = (((vrt1->x + xpos) >> FRACBITS) - x);
		stv = (((vrt1->y + ypos) >> FRACBITS) - y);
		dVTX[1]->v.x = (float)(vrt1->x >> 16);
		dVTX[1]->v.y = (float)(zpos);
		dVTX[1]->v.z = (float)-(vrt1->y >> 16);
		dVTX[1]->v.u = (float)stu * inv64;
		dVTX[1]->v.v = (float)stv * inv64;

		vrt2 = lf[2].vertex;
		stu = (((vrt2->x + xpos) >> FRACBITS) - x);
		stv = -(((vrt2->y + ypos) >> FRACBITS) - y);
		dVTX[2]->v.x = (float)(vrt2->x >> 16);
		dVTX[2]->v.y = (float)(zpos);
		dVTX[2]->v.z = (float)-(vrt2->y >> 16);
		dVTX[2]->v.u = (float)stu * inv64;
		dVTX[2]->v.v = (float)stv * inv64;

		transform_vert(dVTX[1]);
		transform_vert(dVTX[2]);

		color_vert(dVTX[1], newcolor);
		color_vert(dVTX[2], newcolor);
		
		if (ceiling) {
			memcpy(&(dT2.dVerts[0]), dVTX[2], sizeof(d64Vertex_t));
			memcpy(&(dT2.dVerts[1]), dVTX[1], sizeof(d64Vertex_t));
			memcpy(&(dT2.dVerts[2]), &dv0, sizeof(d64Vertex_t));
		} else {
			memcpy(&(dT2.dVerts[0]), &dv0, sizeof(d64Vertex_t));
			memcpy(&(dT2.dVerts[1]), dVTX[1], sizeof(d64Vertex_t));
			memcpy(&(dT2.dVerts[2]), dVTX[2], sizeof(d64Vertex_t));
		}

		clip_triangle(&dT2, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
	} else {
		idx = 1;
	}

	numverts--;

	if (ceiling) {
		dVTX[1] = &(dT1.dVerts[2]);
		dVTX[2] = &(dT1.dVerts[1]);
		dVTX[3] = &(dT1.dVerts[0]);
	} else {
		dVTX[1] = &(dT1.dVerts[0]);
		dVTX[2] = &(dT1.dVerts[1]);
		dVTX[3] = &(dT1.dVerts[2]);
	}

	if (idx < numverts) {
		v00 = idx + 0;
		v01 = idx + 1;
		v02 = idx + 2;
		do {
			vertex_t *vrt1;
			vertex_t *vrt2;
			vertex_t *vrt3;

			vrt1 = lf[v00].vertex;
			vrt2 = lf[v01].vertex;
			vrt3 = lf[v02].vertex;

			stu = (((vrt1->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt1->y + ypos) >> FRACBITS) - y);
			dVTX[1]->v.x = (float)(vrt1->x >> 16);
			dVTX[1]->v.y = (float)zpos;
			dVTX[1]->v.z = (float)-(vrt1->y >> 16);
			dVTX[1]->v.u = (float)stu * inv64;
			dVTX[1]->v.v = (float)stv * inv64;

			stu = (((vrt2->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt2->y + ypos) >> FRACBITS) - y);
			dVTX[2]->v.x = (float)(vrt2->x >> 16);
			dVTX[2]->v.y = (float)zpos;
			dVTX[2]->v.z = (float)-(vrt2->y >> 16);
			dVTX[2]->v.u = (float)stu * inv64;
			dVTX[2]->v.v = (float)stv * inv64;

			stu = (((vrt3->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt3->y + ypos) >> FRACBITS) - y);
			dVTX[3]->v.x = (float)(vrt3->x >> 16);
			dVTX[3]->v.y = (float)zpos;
			dVTX[3]->v.z = (float)-(vrt3->y >> 16);
			dVTX[3]->v.u = (float)stu * inv64;
			dVTX[3]->v.v = (float)stv * inv64;

			transform_vert(dVTX[1]);
			transform_vert(dVTX[2]);
			transform_vert(dVTX[3]);

			color_vert(dVTX[1], newcolor);
			color_vert(dVTX[2], newcolor);
			color_vert(dVTX[3], newcolor);
			
			if(ceiling) {
				memcpy(&(dT2.dVerts[0]), &dv0, sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[1]), dVTX[3], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[2]), dVTX[1], sizeof(d64Vertex_t));
			} else {
				memcpy(&(dT2.dVerts[0]), dVTX[1], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[1]), dVTX[3], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[2]), &dv0, sizeof(d64Vertex_t));
			}

			clip_triangle(&dT1, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);
			clip_triangle(&dT2, &thdr, lightlevel, PVR_LIST_TR_POLY, 0);

			v00 += 2;
			v01 += 2;
			v02 += 2;
		} while (v02 < (numverts + 2));
	}
#endif	
}

pvr_ptr_t pvr_troo[MAX_CACHED_SPRITES];
pvr_poly_hdr_t hdr_troo[MAX_CACHED_SPRITES];
pvr_poly_cxt_t cxt_troo[MAX_CACHED_SPRITES];
uint8_t __attribute__((aligned(32))) tmptroo[256*256];
//int donebefore = 0;
int lump_frame[575 + 310] = {-1};
int used_lumps[575 + 310] = {-1};
int used_lump_idx = 0;
int del_idx = 0;
int total_cached_vram = 0;

int last_flush_frame = 0;

static inline uint32_t np2(uint32_t v) {
v--;
v |= v >> 1;
v |= v >> 2;
v |= v >> 4;
v |= v >> 8;
v |= v >> 16;
v++;
return v;
}

char *W_GetNameForNum(int num);
extern char fnbuf[256];
extern int force_filter_flush;
int vram_low = 0;
// 2 - 348 decoration and item sprites (non-enemy)
// 349 - 923 enemy sprites
// 924 - 965 weapon sprites (non-enemy)
void R_RenderThings(subsector_t *sub)
{
	byte *data;
	vissprite_t *vissprite_p;

	mobj_t *thing;
	boolean flip;
	int lump;

	int compressed;
	int height;
	int width;
	int color;
byte *src;
	fixed_t xx, yy;
	int xpos1, xpos2;
	int ypos;
	int zpos1, zpos2;
	int spos;
int nosprite = 0;

	int external_pal;
#if 0
if(!donebefore) {
	dbgio_printf("initing the lump caching arrays and indices\n");
	memset(used_lumps, -1, sizeof(int)*(575+310));
	memset(lump_frame, -1, sizeof(int)*(575+310));
used_lump_idx = 0;
del_idx = 0;
total_cached_vram = 0;
donebefore = 1;	
}
#endif
#if 0
if(used_lump_idx) {
for(int i=0;i<used_lump_idx;i++) {
pvr_mem_free(pvr_troo[i]);	
}
}
memset(used_lumps, 0, sizeof(int)*used_lump_idx);
used_lump_idx = 0;
#endif
#if 0

if(!pvr_troo) {
	pvr_troo = pvr_mem_malloc(128*128);
	pvr_poly_cxt_txr(&cxt_troo, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_TWIDDLED, 128, 128, pvr_troo, PVR_FILTER_BILINEAR);

	cxt_troo.gen.specular = PVR_SPECULAR_ENABLE;
	cxt_troo.gen.fog_type = PVR_FOG_TABLE;
	cxt_troo.gen.fog_type2 = PVR_FOG_TABLE;
	pvr_poly_compile(&hdr_troo, &cxt_troo);
}
#endif
	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);
	dVTX[3] = &(dT2.dVerts[2]);

	vissprite_p = sub->vissprite;
	if (vissprite_p) {
		if (vissprite_p->thing->flags & MF_RENDERLASER) {
#if 0
			gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_RA_OPA_SURF2);
			gDPSetCombineMode(GFX1++, G_CC_D64COMB15, G_CC_D64COMB16);
#endif
			do {
				R_RenderLaser(vissprite_p->thing);
				vissprite_p = vissprite_p->next;
				if (vissprite_p == NULL) {
					break;
				}
			} while(vissprite_p->thing->flags & MF_RENDERLASER);

#if 0
			gDPPipeSync(GFX1++);
			gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
#endif
			if (vissprite_p == NULL) {
#if 0
				gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
#endif
				return;
			}
		}

#if 0
		gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
#endif
		while (vissprite_p) {
			thing = vissprite_p->thing;
			lump = vissprite_p->lump;
			flip = vissprite_p->flip;

			if (thing->frame & FF_FULLBRIGHT) {
				color = 0xffffffff; // PACKRGBA(255, 255, 255, 255);
			} else {
				color = lights[vissprite_p->sector->colors[2]].rgba;
			}
#if 0
			gDPSetPrimColorD64(GFX1++, 0, vissprite_p->sector->lightlevel, thing->alpha);
#endif
			color = (color & 0xffffff00) | thing->alpha;

			data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);
			src = data + sizeof(spriteN64_t);
			compressed = SwapShort( ((spriteN64_t*)data)->compressed );
			width = SwapShort( ((spriteN64_t*)data)->width );
			height = SwapShort( ((spriteN64_t*)data)->height );

			spos = width;

			external_pal = 0;

			if (compressed) {
				int cmpsize = SwapShort(((spriteN64_t*)data)->cmpsize);
				if (cmpsize & 1) {
					external_pal = 1;
				}
			}

			if (flip) {
				xx = thing->x + (SwapShort(((spriteN64_t*)data)->xoffs) * viewsin);
				xpos1 = (xx - (width * viewsin)) >> 16;
				xpos2 = (xx) >> 16;

				yy = thing->y - (SwapShort(((spriteN64_t*)data)->xoffs) * viewcos);
				zpos1 = -(yy + (width * viewcos)) >> 16;
				zpos2 = -(yy) >> 16;
			} else {
				xx = thing->x - (SwapShort(((spriteN64_t*)data)->xoffs) * viewsin);
				xpos2 = (xx + (width * viewsin)) >> 16;
				xpos1 = (xx) >> 16;

				yy = thing->y + (SwapShort(((spriteN64_t*)data)->xoffs) * viewcos);
				zpos2 = -(yy - (width * viewcos)) >> 16;
				zpos1 = -(yy) >> 16;
			}

			ypos = (thing->z >> 16) + SwapShort(((spriteN64_t*)data)->yoffs);
			pvr_poly_hdr_t *theheader;
			if ((lump <= 348) || ((lump >= 924) && (lump <= 965))) {
				nosprite = 0;
				// pull in each side of sprite by one pixel
				// fix for filtering 'crud' around the edge due to lack of padding
				if(!flip) {
#if 0
					if (external_pal && thing->info->palette) {
						if (thing->info->palette == 1) {
							dVTX[0]->v.u = dVTX[3]->v.u = all2_u[lump] + inv1024;
							dVTX[1]->v.u = dVTX[2]->v.u = all2_u[lump] + (((float)spos - 1.0f)*inv1024);
						} else {
							// player PALPLAY02 , use lump - 100 in headers2
							dVTX[0]->v.u = dVTX[3]->v.u = all2_u[lump-100] + inv1024;
							dVTX[1]->v.u = dVTX[2]->v.u = all2_u[lump-100] + (((float)spos - 1.0f)*inv1024);
						}
					} else {
#endif
						dVTX[0]->v.u = dVTX[3]->v.u = all_u[lump] + inv1024;
						dVTX[1]->v.u = dVTX[2]->v.u = all_u[lump] + (((float)spos - 1.0f)*inv1024);
#if 0
					}
#endif
				} else {
#if 0
					if (external_pal && thing->info->palette) {
						if (thing->info->palette == 1) {
							dVTX[1]->v.u = dVTX[2]->v.u = all2_u[lump] + inv1024;
							dVTX[0]->v.u = dVTX[3]->v.u = all2_u[lump] + (((float)spos - 1.0f)*inv1024);
						} else {
							dVTX[1]->v.u = dVTX[2]->v.u = all2_u[lump-100] + inv1024;
							dVTX[0]->v.u = dVTX[3]->v.u = all2_u[lump-100] + (((float)spos - 1.0f)*inv1024);
						}
					} else {
#endif						
						dVTX[1]->v.u = dVTX[2]->v.u = all_u[lump] + inv1024;
						dVTX[0]->v.u = dVTX[3]->v.u = all_u[lump] + (((float)spos - 1.0f)*inv1024);
#if 0
					}
#endif
				}

#if 0
				if (external_pal && thing->info->palette) {
					if (thing->info->palette == 1) {
						theheader = headers2_for_sprites[lump];
						dVTX[0]->v.v = dVTX[1]->v.v = all2_v[lump] + inv1024;
						dVTX[3]->v.v = dVTX[2]->v.v = all2_v[lump] + (((float)height - 1.0f) *inv1024);
					} else {
						theheader = headers2_for_sprites[lump-100];
						dVTX[0]->v.v = dVTX[1]->v.v = all2_v[lump-100] + inv1024;
						dVTX[3]->v.v = dVTX[2]->v.v = all2_v[lump-100] + (((float)height - 1.0f) *inv1024);
					}
				} else {
#endif
					theheader = headers_for_sprites[lump];
					dVTX[0]->v.v = dVTX[1]->v.v = all_v[lump] + inv1024;
					dVTX[3]->v.v = dVTX[2]->v.v = all_v[lump] + (((float)height-1.0f)*inv1024);
#if 0
				}
#endif
			} else {
				int lumpoff = lump - 349;
				int cached_index = -1;
				int troowid = (width + 7) & ~7;
				uint32_t wp2 = np2((uint32_t)troowid);
				uint32_t hp2 = np2((uint32_t)height);

				if (external_pal && thing->info->palette) {
					void *newlump;
					int newlumpnum;
					char *lumpname = W_GetNameForNum(lump);
					
					// troo; [450,
					if (lumpname[0] == 'T') {
						lumpname[0] = 'N';
						lumpname[1] = 'I';
						lumpname[2] = 'T';
						lumpname[3] = 'E';
					}
					// sarg; [349,394]
					else if(lumpname[0] == 'S') {
						lumpname[1] = 'P';
						lumpname[2] = 'E';
						lumpname[3] = 'C';
					}
					// boss
					else if(lumpname[0] == 'B') {
						lumpname[1] = 'A';
						lumpname[2] = 'R';
						lumpname[3] = 'O';
					}
					// poss; [
					// play; [398-447]
					else if (lumpname[0] == 'P') {
						if (lumpname[1] == 'O') {
							lumpname[0] = 'Z';
							lumpname[2] = 'M';
							lumpname[3] = 'B';
						} else {
							if (thing->info->palette == 1) {
								lumpname[2] = 'Y';
								lumpname[3] = '1';
							} else {
								lumpname[2] = 'Y';
								lumpname[3] = '2';
							}
						}
					}

					newlumpnum = W_S2_GetNumForName(lumpname);
					newlump = W_S2_CacheLumpNum(newlumpnum, PU_CACHE, dec_jag);
					src = newlump + sizeof(spriteN64_t);
					lumpoff = 574 + newlumpnum;
				}

				if (force_filter_flush || vram_low || (last_flush_frame && ((NextFrameIdx - last_flush_frame) > (2*30)) && 
					(used_lump_idx > (MAX_CACHED_SPRITES * 3 / 4)))) {
					if(force_filter_flush) force_filter_flush = 0;
					for (int i=0;i<(575+310);i++) {
						if (used_lumps[i] != -1) {
							pvr_mem_free(pvr_troo[used_lumps[i]]);
						}
					}
					memset(used_lumps, 0xff, sizeof(int)*(575+310));
					memset(lump_frame, 0xff, sizeof(int)*(575+310));
					used_lump_idx = 0;
					del_idx = 0;
					last_flush_frame = NextFrameIdx;
					vram_low = 0;
				}

				if (used_lumps[lumpoff] != -1) {
					// found an index
					cached_index = used_lumps[lumpoff];
					lump_frame[lumpoff] = NextFrameIdx;
//					dbgio_printf("lump %d already cached at index %d\n", lump, cached_index);
					goto skip_cached_setup;
				}

				if (last_flush_frame == 0) last_flush_frame = NextFrameIdx;

				if (used_lump_idx < MAX_CACHED_SPRITES) {
					used_lumps[lumpoff] = used_lump_idx;
					lump_frame[lumpoff] = NextFrameIdx;
					cached_index = used_lump_idx;
					if (lumpoff < 575) {
						dbgio_printf("caching lump %d at index %d\n", lump, cached_index);
					} else {
						dbgio_printf("caching alternate lump %d at index %d\n", lump, cached_index);
					}
					used_lump_idx += 1;
				} else {
					nosprite = 1;
#if 1				
					// here it gets complicated
					// find if any of the lumps have the del_idx as their index
					// if so, set their index to -1
					//dbgio_printf("all lump cache slots used, find one to evict\n");
					//"caching lump %d at index %d\n", lump, cached_index);

					// this gets incremented if all possible cache indices are used in a single frame
					// and nothing can be evicted
					int passes = 0;

					int start_del_idx = del_idx;
					int next_del_idx_lump = -1;

					// for every possible enemy sprite lump number
					for (int i=0;i<(575+310);i++) {
						if (passes) {
							nosprite = 1;
							dbgio_printf("\t\t nothing was evictable");
							goto bail_evict;
						}

						// try to help this along by noting if we found the next del idx along the way
						if (used_lumps[i] == (del_idx + 1)) {
							dbgio_printf("\t\thaven't found del_idx yet, found next_del_idx though\n");
							next_del_idx_lump = i;
						}

						// if this enemy sprite lump number is already cached
						// and the cache index is our "del idx"
						// we should attempt to evict this one first
						if(used_lumps[i] == del_idx) {
							if (lump_frame[i] == NextFrameIdx) {
								dbgio_printf("\tlump %d at del_idx is used in this frame -- check more\n", i+349);

								// this can help us skip more passes through the entire lump set
								if (next_del_idx_lump != -1) {
									if (lump_frame[next_del_idx_lump] != NextFrameIdx) {
										dbgio_printf("\t\tevicted %d by way of next_del_idx_lump\n", next_del_idx_lump);
										del_idx = used_lumps[next_del_idx_lump];
										pvr_mem_free(pvr_troo[del_idx]);
										used_lumps[i] = -1;
										lump_frame[i] = -1;
										goto done_evicting;
									}
								}

								i = 0;
								del_idx += 1;

								// wrap
								if (del_idx == MAX_CACHED_SPRITES) {
									del_idx = 0;
								}

								// if after increment and/or wrap we are at the starting index, nothing was evictable
								if (del_idx == start_del_idx) {
									dbgio_printf("\t\t del_idx wrapped back to start, about to fail things\n");
									passes = 1;
								}

								continue;
							} else {
								dbgio_printf("\t\tevicted %d from old frame\n", i);
								pvr_mem_free(pvr_troo[del_idx]);
								used_lumps[i] = -1;
								lump_frame[i] = -1;
								goto done_evicting;
							}
						}
					}

done_evicting:
					cached_index = del_idx;
					used_lumps[lumpoff] = cached_index;
					lump_frame[lumpoff] = NextFrameIdx;
//					dbgio_printf("caching lump %d at index %d\n", lump, cached_index);

					del_idx += 1;
					if (del_idx == MAX_CACHED_SPRITES) {
						del_idx = 0;
					}

#if 0
					// try to evict some lumps not recently used
					int found = 0;

					for (int i=0;i<(575+310);i++) {
						if (lump_frame[i] < NextFrameIdx) {
							int oldlumpidx = used_lumps[i];
							pvr_mem_free(pvr_troo[oldlumpidx]);
							used_lumps[i] = -1;
							lump_frame[i] = -1;
							found += 1;
							if (found == 8) {
								break;
							}
						}
					}           // for i [0,575)
#endif
#endif
				}
bail_evict:
				if (nosprite) {
					dbgio_printf("could not cache lump %d\n", lumpoff);
				} else {
//				if(!nosprite) {
					if ((wp2*hp2) > (pvr_mem_available()*4)) {
						nosprite = 1;
						lump_frame[lumpoff] = -1;
						used_lumps[lumpoff] = -1;
						vram_low = 1;
						goto bail_pvr_alloc;
					}
	
					pvr_troo[cached_index] = pvr_mem_malloc(wp2*hp2);

					pvr_poly_cxt_txr(&cxt_troo[cached_index], PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(0) | PVR_TXRFMT_TWIDDLED, wp2, hp2, pvr_troo[cached_index], PVR_FILTER_BILINEAR);

					cxt_troo[cached_index].gen.specular = PVR_SPECULAR_ENABLE;
					cxt_troo[cached_index].gen.fog_type = PVR_FOG_TABLE;
					cxt_troo[cached_index].gen.fog_type2 = PVR_FOG_TABLE;
					if (VideoFilter) {
						cxt_troo[cached_index].txr.filter = PVR_FILTER_NONE;
					}
					pvr_poly_compile(&hdr_troo[cached_index], &cxt_troo[cached_index]);

					pvr_txr_load(src, pvr_troo[cached_index], wp2*hp2);
skip_cached_setup:

					if (!flip) {
						dVTX[0]->v.u = dVTX[3]->v.u = (1.0f / (float)wp2);
						dVTX[1]->v.u = dVTX[2]->v.u = ((float)troowid / (float)wp2) - (1.0f / (float)wp2);
					} else {
						dVTX[1]->v.u = dVTX[2]->v.u = (1.0f / (float)wp2);
						dVTX[0]->v.u = dVTX[3]->v.u = ((float)troowid / (float)wp2) - (1.0f / (float)wp2);
					}
					dVTX[0]->v.v = dVTX[1]->v.v = (1.0f / (float)hp2);
					dVTX[2]->v.v = dVTX[3]->v.v = ((float)height / (float)hp2) - (1.0f / (float)hp2);
					theheader = &hdr_troo[cached_index];
				}
			}

bail_pvr_alloc:
			if (!nosprite) {
				dVTX[0]->v.x = dVTX[3]->v.x = xpos1;
				dVTX[1]->v.x = dVTX[2]->v.x = xpos2;
				dVTX[0]->v.y = dVTX[1]->v.y = ypos;
				dVTX[3]->v.y = dVTX[2]->v.y = ypos - height;
				dVTX[0]->v.z = dVTX[3]->v.z = zpos1;
				dVTX[1]->v.z = dVTX[2]->v.z = zpos2;

				transform_vert(dVTX[0]);
				transform_vert(dVTX[1]);
				transform_vert(dVTX[2]);
				transform_vert(dVTX[3]);

				color_vert(dVTX[0], color);
				color_vert(dVTX[1], color);
				color_vert(dVTX[2], color);
				color_vert(dVTX[3], color);

				memcpy(&(dT2.dVerts[0]), dVTX[0], sizeof(d64Vertex_t));
				memcpy(&(dT2.dVerts[1]), dVTX[2], sizeof(d64Vertex_t));

				clip_quad(&dT1, &dT2, theheader, vissprite_p->sector->lightlevel, PVR_LIST_TR_POLY, 0);
			}
			vissprite_p = vissprite_p->next;
		}

		globallump = -1;
	}
}

static d64Vertex_t __attribute__((aligned(32))) laserverts[6];

void R_RenderLaser(mobj_t *thing)
{
	laserdata_t *laserdata = (laserdata_t *)thing->extradata;

	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;

	pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	pvr_poly_compile(&hdr, &cxt);
	
	laserverts[0].v.x = (laserdata->x1 >> 16);
	laserverts[0].v.y = (laserdata->z1 >> 16);
	laserverts[0].v.z = -(laserdata->y1 >> 16);
	laserverts[0].v.argb = 0xffff0000; // PVR_PACK_COLOR(1,1,0,0);

	laserverts[1].v.x = ((laserdata->x1 - laserdata->slopey) >> 16);
	laserverts[1].v.y = (laserdata->z1 >> 16);
	laserverts[1].v.z = (-(laserdata->y1 + laserdata->slopex) >> 16);
	laserverts[1].v.argb = 0xff000000; // PVR_PACK_COLOR(1,0,0,0);

	laserverts[2].v.x = ((laserdata->x2 - laserdata->slopey) >> 16);
	laserverts[2].v.y = (laserdata->z2 >> 16);
	laserverts[2].v.z = (-(laserdata->y2 + laserdata->slopex) >> 16);
	laserverts[2].v.argb = 0xff000000; // PVR_PACK_COLOR(1,0,0,0);

	laserverts[3].v.x = (laserdata->x2 >> 16);
	laserverts[3].v.y = (laserdata->z2 >> 16);
	laserverts[3].v.z = -(laserdata->y2 >> 16);
	laserverts[3].v.argb = 0xffff0000; // PVR_PACK_COLOR(1,1,0,0);

	laserverts[4].v.x = ((laserdata->x2 + laserdata->slopey) >> 16);
	laserverts[4].v.y = (laserdata->z2 >> 16);
	laserverts[4].v.z = (-(laserdata->y2 - laserdata->slopex) >> 16);
	laserverts[4].v.argb = 0xff000000; // PVR_PACK_COLOR(1,0,0,0);

	laserverts[5].v.x = ((laserdata->x1 + laserdata->slopey) >> 16);
	laserverts[5].v.y = (laserdata->z1 >> 16);
	laserverts[5].v.z = (-(laserdata->y1 - laserdata->slopex) >> 16);
	laserverts[5].v.argb = 0xff000000; // PVR_PACK_COLOR(1,0,0,0);

	transform_vert(&laserverts[0]);
	transform_vert(&laserverts[1]);
	transform_vert(&laserverts[2]);
	transform_vert(&laserverts[3]);
	transform_vert(&laserverts[4]);
	transform_vert(&laserverts[5]);

	// 0 2 3
	// 0 1 2
	memcpy(&(dT1.dVerts[0]), &laserverts[0], sizeof(d64Vertex_t));
	memcpy(&(dT1.dVerts[1]), &laserverts[2], sizeof(d64Vertex_t));
	memcpy(&(dT1.dVerts[2]), &laserverts[3], sizeof(d64Vertex_t));

	memcpy(&(dT2.dVerts[0]), &laserverts[0], sizeof(d64Vertex_t));
	memcpy(&(dT2.dVerts[1]), &laserverts[1], sizeof(d64Vertex_t));
	memcpy(&(dT2.dVerts[2]), &laserverts[2], sizeof(d64Vertex_t));
	
	clip_triangle(&dT1, &hdr, 255, PVR_LIST_OP_POLY, 0);
	clip_triangle(&dT2, &hdr, 255, PVR_LIST_OP_POLY, 0);

	// 0 3 5
	// 3 4 5
	memcpy(&(dT1.dVerts[0]), &laserverts[0], sizeof(d64Vertex_t));
	memcpy(&(dT1.dVerts[1]), &laserverts[3], sizeof(d64Vertex_t));
	memcpy(&(dT1.dVerts[2]), &laserverts[5], sizeof(d64Vertex_t));

	memcpy(&(dT2.dVerts[0]), &laserverts[3], sizeof(d64Vertex_t));
	memcpy(&(dT2.dVerts[1]), &laserverts[4], sizeof(d64Vertex_t));
	memcpy(&(dT2.dVerts[2]), &laserverts[5], sizeof(d64Vertex_t));
	
	clip_triangle(&dT1, &hdr, 255, PVR_LIST_TR_POLY, 0);
	clip_triangle(&dT2, &hdr, 255, PVR_LIST_TR_POLY, 0);
}

void R_RenderPSprites(void)
{
	int				i;
	pspdef_t		*psp;
#if 0
	pspdef_t		*psptmp;
#endif
	state_t			*state;
	spritedef_t		*sprdef;
	spriteframe_t	*sprframe;
	int				lump;
	int				flagtranslucent;

	byte		    *data;

	int             width;
	int             height;
	int             width2;
	int             x, y;

	for (int i=0;i<4;i++) {
		// under envflash
		quad2[i].z = 4.99;
		quad2[i].flags = PVR_CMD_VERTEX;
	}
	quad2[3].flags = PVR_CMD_VERTEX_EOL;

#if 0
	gDPPipeSync(GFX1++);
	gDPSetTexturePersp(GFX1++, G_TP_NONE);
	gDPSetCombineMode(GFX1++, G_CC_D64COMB17, G_CC_D64COMB18);
#endif

	psp = &viewplayer->psprites[0];

	flagtranslucent = (viewplayer->mo->flags & MF_SHADOW) != 0;

#if 0
	psptmp = psp;
	for (i = 0; i < NUMPSPRITES; i++, psptmp++) {
		if(flagtranslucent || ((psptmp->state != 0) && (psptmp->alpha < 255))) {
			gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
			break;
		}
	}
#endif

	for (i = 0; i < NUMPSPRITES; i++, psp++) {
		/* a null state means not active */ 
		if ((state = psp->state) != 0) { 
			sprdef = &sprites[state->sprite];
			sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
			lump = sprframe->lump[0];

			data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);//dec_none);
			width = SwapShort(((spriteN64_t*)data)->width);
			width2 = (width + 7) & ~7;
			height = SwapShort(((spriteN64_t*)data)->height);

#if 0
			if (psp->state->frame & FF_FULLBRIGHT) {
				gDPSetPrimColorD64(GFX1, 0, 0, PACKRGBA(255,255,255,0));//0xffffff00
			} else {
				gDPSetPrimColorD64(GFX1, 0, frontsector->lightlevel,
				lights[frontsector->colors[2]].rgba & ~255); // remove alpha value
			}

			// apply alpha value
			if (flagtranslucent) {
				GFX1->words.w1 |= 144;
			} else {
				GFX1->words.w1 |= psp->alpha;
			}
#endif
			pvr_vertex_t *vert = quad2;
			float u1 = all_u[lump];
			float v1 = all_v[lump];
			float u2 = all_u2[lump];
			float v2 = all_v2[lump];

			if (psp->state->frame & FF_FULLBRIGHT) {
				uint8_t a1;

				if(flagtranslucent)
					a1 = 144;
				else
					a1 = psp->alpha;

				for(int i = 0; i < 4; i++) {
					quad2[i].argb = D64_PVR_REPACK_COLOR_ALPHA(0xffffffff, a1);
					quad2[i].oargb = 0;
				}
			} else {
				uint32_t color = lights[frontsector->colors[2]].rgba;
				float lightc = (float)frontsector->lightlevel * inv255;
				uint32_t pspr_color;
				uint8_t a1;
					
				if(flagtranslucent)
					a1 = 144;
				else
					a1 = psp->alpha;

				// hi Immorpher -- fixed the dynamic lighting of weapons
				pspr_color = lighted_color(D64_PVR_REPACK_COLOR_ALPHA(color, a1), lightc);

				for(int i = 0; i < 4; i++) {
					quad2[i].argb = D64_PVR_REPACK_COLOR_ALPHA(color, a1);
					quad2[i].oargb = pspr_color;
				}
			}

			x = (((psp->sx >> 16) - SwapShort(((spriteN64_t*)data)->xoffs)) + 160);
			y = (((psp->sy >> 16) - SwapShort(((spriteN64_t*)data)->yoffs)) + 239);

			if (viewplayer->onground) {
				x += (quakeviewx >> 22);
				y += (quakeviewy >> 16);
			}

			float x1 = (float)x * RES_RATIO;
			float y1 = (float)y * RES_RATIO;
			float x2 = x1 + ((float)width2 * RES_RATIO);
			float y2 = y1 + ((float)height * RES_RATIO);

			// pull in each side of sprite by one pixel
			// fix for filtering 'crud' around the edge due to lack of padding
			vert->x = x1;
			vert->y = y2;
			vert->u = u1 + inv1024;
			vert->v = v2 - inv1024;
			vert++;

			vert->x = x1;
			vert->y = y1;
			vert->u = u1 + inv1024;
			vert->v = v1 + inv1024;
			vert++;

			vert->x = x2;
			vert->y = y2;
			vert->u = u2 - inv1024;
			vert->v = v2 - inv1024;
			vert++;

			vert->x = x2;
			vert->y = y1;
			vert->u = u2 - inv1024;
			vert->v = v1 + inv1024;

			pvr_list_prim(PVR_LIST_TR_POLY, headers_for_sprites[lump], sizeof(pvr_poly_hdr_t));
			pvr_list_prim(PVR_LIST_TR_POLY, &quad2, sizeof(quad2));
		} // if ((state = psp->state) != 0)
	} // for i < numsprites
}