//Renderer phase 3 - World Rendering Routines
#include "doomdef.h"
#include "r_local.h"

#include <dc/matrix.h>
#include <dc/pvr.h>
#include <math.h>

extern int brightness;
extern short SwapShort(short dat);
extern int VideoFilter;

extern pvr_poly_cxt_t **tcxt;

extern pvr_poly_hdr_t pvr_sprite_hdr;
extern pvr_poly_hdr_t pvr_sprite_hdr_nofilter;

extern float *all_u;
extern float *all_v;
extern float *all_u2;
extern float *all_v2;

pvr_vertex_t __attribute__ ((aligned(32))) quad2[4];

pvr_poly_hdr_t hdr;
pvr_poly_cxt_t cxt;

pvr_poly_hdr_t thdr;

d64Vertex_t *dVTX[4];
d64Triangle_t dT1, dT2;

const float inv64 = 1.0f / 64.0f;
const float inv255 = 1.0f / 255.0f;
const float inv1024 = 1.0f / 1024.0f;
const float halfinv1024 = 0.5f / 1024.0f;
const float inv65536 = 1.0f / 65536.0f;

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

	out->w   = invt * v1->w   + t * v2->w;

	out->v.x = invt * v1->v.x + t * v2->v.x;
	out->v.y = invt * v1->v.y + t * v2->v.y;
	out->v.z = invt * v1->v.z + t * v2->v.z;

	out->v.u = invt * v1->v.u + t * v2->v.u;
	out->v.v = invt * v1->v.v + t * v2->v.v;

	blend_color(t, v1->v.argb, v2->v.argb, &(out->v.argb));
	blend_color(t, v1->v.oargb, v2->v.oargb, &(out->v.oargb));
}

uint32_t lighted_color(uint32_t c, int ll)
{
	uint8_t r = (uint8_t)((UNPACK_R(c)*ll)>>8);//(((int)(256 + ((int)UNPACK_R(c)))*(int)ll) >> 9);
	uint8_t g = (uint8_t)((UNPACK_G(c)*ll)>>8);//(((int)(256 + ((int)UNPACK_G(c)))*(int)ll) >> 9);
	uint8_t b = (uint8_t)((UNPACK_B(c)*ll)>>8);//(((int)(256 + ((int)UNPACK_B(c)))*(int)ll) >> 9);
	
	uint8_t a = UNPACK_A(c);
	return D64_PVR_PACK_COLOR(a,r,g,b);
}

void surfaceNormal(d64Triangle_t *surface, d64Vertex_t *normResult)
{
	float ux = surface->dVerts[1].v.x - surface->dVerts[0].v.x;	
	float uy = surface->dVerts[1].v.y - surface->dVerts[0].v.y;	
	float uz = surface->dVerts[1].v.z - surface->dVerts[0].v.z;	

	float vx = surface->dVerts[2].v.x - surface->dVerts[0].v.x;	
	float vy = surface->dVerts[2].v.y - surface->dVerts[0].v.y;	
	float vz = surface->dVerts[2].v.z - surface->dVerts[0].v.z;	

	normResult->v.x = (uy * vz) - (uz * vy);
	normResult->v.y = (uz * vx) - (ux * vz);
	normResult->v.z = (ux * vy) - (uy * vx);
	
	//vec3f_normalize(normResult->v.x, normResult->v.y, normResult->v.z);
}

extern int dont_color;
extern int lightidx;
extern projectile_light_t projectile_lights[NUM_DYNLIGHT];

d64Vertex_t lightverts[NUM_DYNLIGHT];

static void R_TransformProjectileLights(void)
{
	for (int i = 0; i < lightidx + 1; i++) {
		d64Vertex_t *lightvert = &lightverts[i];
		lightvert->v.x = projectile_lights[i].x;
		lightvert->v.y = projectile_lights[i].z;
		lightvert->v.z = -projectile_lights[i].y;
		transform_vert(lightvert);
	}
}

int recompute_norm = 1;
static d64Vertex_t norm;
static float crossprod;

void applyProjectileLighting(d64Triangle_t *triangle, d64Vertex_t *vertex) {
	pvr_vertex_t *coord = &(vertex->v);

	float lightingr = 0.0f;
	float lightingg = 0.0f;
	float lightingb = 0.0f;

	if (dont_color) return;

	// only compute norm if this global is set on entry
	if (recompute_norm) {
		surfaceNormal(triangle, &norm);
		// and don't compute again unless it gets explicitly set again
		// this saves MANY redundant computations of the surface normal
		// for wall quads
		vec3f_length(norm.v.x, norm.v.y, norm.v.z, crossprod);
		vec3f_normalize(norm.v.x, norm.v.y, norm.v.z);
		recompute_norm = 0;
	}

	for (int i = 0; i < lightidx + 1; i++) {
		d64Vertex_t *lightvert = &lightverts[i];

		float dx = (lightvert->v.x - coord->x);
		float dy = (lightvert->v.y - coord->y);
		float dz = (lightvert->v.z - coord->z);
		float lr = projectile_lights[i].radius;

		float lightdist;
		vec3f_length(dx, dy, dz, lightdist);

		if (lightdist < lr) {
			float dotprod;

			vec3f_dot(dx, dy, dz, norm.v.x, norm.v.y, norm.v.z, dotprod);

			if (dotprod >= 0) {
				float linear_scalar = ((lr - lightdist) / lr);
				float lum = ((dotprod + 0.1)/lightdist);
				float ls2 = linear_scalar*linear_scalar;

				// distance attenuation on its own is not good enough here
				// also need the dot product luminance scaling
				float light_scale = lum * ls2;
				
				lightingr += (projectile_lights[i].r * light_scale);
				lightingg += (projectile_lights[i].g * light_scale);
				lightingb += (projectile_lights[i].b * light_scale);
			}
		}
	}

	if (coord->oargb != 0) {
		float coord_r = (float)((coord->oargb >> 16) & 0xff) / 255.0f;
		float coord_g = (float)((coord->oargb >> 8) & 0xff) / 255.0f;
		float coord_b = (float)(coord->oargb & 0xff) / 255.0f;
		lightingr += coord_r;
		lightingg += coord_g;
		lightingb += coord_b;
	}

	if ((lightingr > 1.0f) || (lightingg > 1.0f) || (lightingb > 1.0f)) {
		float maxrgb = 0.0f;
		if (lightingr > maxrgb) maxrgb = lightingr;
		if (lightingg > maxrgb) maxrgb = lightingg;
		if (lightingb > maxrgb) maxrgb = lightingb;

		lightingr /= maxrgb;
		lightingg /= maxrgb;
		lightingb /= maxrgb;
	}

	if ((lightingr + lightingg + lightingb) > 0.0f) {
		const int component_intensity = 192;
		
		// this is a hack to handle dark areas
		// because I am using a specular highlight color for dynamic lighting,
		// really dark vertices with textures applied are still basically solid black
		// with a colorful full-face specular highlight
		// set the vertex color to a dark but not too dark gray
		// now the dynamic lighting has a useful effect
		int coord_r = (coord->argb >> 16) & 0xff;
		int coord_g = (coord->argb >> 8) & 0xff;
		int coord_b = coord->argb & 0xff;

		if (coord_r < 0x10 && coord_g < 0x10 && coord_b < 0x10) {
			coord_r += 0x37;
			coord_g += 0x37;
			coord_b += 0x37;

			coord->argb = D64_PVR_PACK_COLOR(0xff, coord_r, coord_g, coord_b);
		}

		coord->oargb = 0xff000000 | 
		(((int)(lightingr*component_intensity) & 0xff) << 16) | 
		(((int)(lightingg*component_intensity) & 0xff) << 8) | 
		(((int)(lightingb*component_intensity) & 0xff));
	}
}

// this is special-cased for all of the quads that do not require near-z clipping and will happily submit with 4 unmodified verts
void clip_quad(d64Triangle_t *triangle, d64Triangle_t *triangle2, pvr_poly_hdr_t *hdr, int lightlevel, pvr_list_t list, int specd) {
	int vm = vismask2(triangle, triangle2);

	if (!vm) {
		return;
	}

	// this state falls through to clip_triangle when needed
	recompute_norm = 1;
		
	if (vm == 15) {
		if (!specd) {
			triangle->dVerts[0].v.oargb = lighted_color(triangle->dVerts[0].v.argb, lightlevel);
			triangle->dVerts[1].v.oargb = lighted_color(triangle->dVerts[1].v.argb, lightlevel);
			triangle->dVerts[2].v.oargb = lighted_color(triangle->dVerts[2].v.argb, lightlevel);
			triangle2->dVerts[2].v.oargb = lighted_color(triangle2->dVerts[2].v.argb, lightlevel);
		}

		triangle->dVerts[0].v.flags = PVR_CMD_VERTEX;
		triangle->dVerts[1].v.flags = PVR_CMD_VERTEX;
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;
		triangle2->dVerts[2].v.flags = PVR_CMD_VERTEX_EOL;

		// surface normal gets computed here for entire quad
		applyProjectileLighting(triangle,&triangle->dVerts[0]);
		// the next 3 calls reuse norm
		applyProjectileLighting(triangle,&triangle->dVerts[1]);
		applyProjectileLighting(triangle,&triangle->dVerts[2]);
		applyProjectileLighting(triangle2,&triangle2->dVerts[2]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle2->dVerts[2]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));

		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle2->dVerts[2].v, sizeof(pvr_vertex_t));
	} else {
		// this is for every quad with at least one vert that needs clipping
		// this first call with recompute the surface normal for the quad based on the triangle
		clip_triangle(triangle, hdr, lightlevel, list, specd);
		// this second call will reuse the surface normal computed in previous call
		clip_triangle(triangle2, hdr, lightlevel, list, specd);
	}
}

void clip_triangle(d64Triangle_t *triangle, pvr_poly_hdr_t *hdr, int lightlevel, pvr_list_t list, int specd) {
	int vm = vismask(triangle);

	if (!vm) {
		return;
	}

	triangle->dVerts[0].v.flags = PVR_CMD_VERTEX;
	triangle->dVerts[1].v.flags = PVR_CMD_VERTEX;
	triangle->dVerts[2].v.flags = PVR_CMD_VERTEX_EOL;

	if (!specd) {
		triangle->dVerts[0].v.oargb = lighted_color(triangle->dVerts[0].v.argb, lightlevel);
		triangle->dVerts[1].v.oargb = lighted_color(triangle->dVerts[1].v.argb, lightlevel);
		triangle->dVerts[2].v.oargb = lighted_color(triangle->dVerts[2].v.argb, lightlevel);
	}

	switch (vm) {
	/* all verts visible */
	case 7: {
		applyProjectileLighting(triangle,&triangle->dVerts[0]);
		applyProjectileLighting(triangle,&triangle->dVerts[1]);
		applyProjectileLighting(triangle,&triangle->dVerts[2]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[0] visible */
	case 1: {
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[0]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);
		
		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[1] visible */
	case 2: {
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[1]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[2] visible */
	case 4: {
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;

		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[2]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[0] and dVerts[1] visible */
	case 3: {
		triangle->spare[0].v.flags = PVR_CMD_VERTEX_EOL;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;

		/* out 1 */
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[0]);

		/* out 2 */
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[0]);
		applyProjectileLighting(triangle,&triangle->dVerts[1]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[0] and dVerts[2] visible */
	case 5: {
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX;
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX_EOL;

		/* out 1 */
		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);

		/* out 2 */
		clip_edge(&triangle->dVerts[1], &triangle->dVerts[2], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[0]);
		applyProjectileLighting(triangle,&triangle->dVerts[2]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[0]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->dVerts[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
	}
	break;
	/* dVerts[1] and dVerts[2] visible */
	case 6: {
		triangle->dVerts[2].v.flags = PVR_CMD_VERTEX_EOL;
		triangle->spare[0].v.flags = PVR_CMD_VERTEX;
		triangle->spare[1].v.flags = PVR_CMD_VERTEX;

		/* out 1 */
		clip_edge(&triangle->dVerts[0], &triangle->dVerts[1], &triangle->spare[0]);
		clip_edge(&triangle->dVerts[2], &triangle->dVerts[0], &triangle->spare[1]);

		applyProjectileLighting(triangle,&triangle->dVerts[1]);
		applyProjectileLighting(triangle,&triangle->dVerts[2]);
		applyProjectileLighting(triangle,&triangle->spare[0]);
		applyProjectileLighting(triangle,&triangle->spare[1]);

		perspdiv(&triangle->dVerts[1]);
		perspdiv(&triangle->dVerts[2]);
		perspdiv(&triangle->spare[0]);
		perspdiv(&triangle->spare[1]);

		pvr_list_prim(list, hdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(list, &triangle->spare[0].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->spare[1].v, sizeof(pvr_vertex_t));
		pvr_list_prim(list, &triangle->dVerts[2].v, sizeof(pvr_vertex_t));
	}
	break;
	}
}

void R_RenderWorld(subsector_t *sub);

void R_WallPrep(seg_t *seg);
void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight, int topOffset, int bottomOffset, int topColor, int bottomColor);
void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color);

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color, int ceiling, int lightlevel, int alpha);

void R_RenderThings(subsector_t *sub);
void R_RenderLaser(mobj_t *thing);
void R_RenderPSprites(void);

void R_RenderAll(void)
{
	subsector_t *sub;

	R_TransformProjectileLights();

	while (endsubsector--, (endsubsector >= solidsubsectors)) {
		sub = *endsubsector;
		frontsector = sub->sector;
		R_RenderWorld(sub);

		sub->drawindex = 0x7fff;
	}
}

void R_RenderWorld(subsector_t *sub)
{
	leaf_t *lf;
	seg_t *seg;

	fixed_t xoffset;
	fixed_t yoffset;
	int numverts;
	int i;

	numverts = sub->numverts;

	// render walls
	lf = &leafs[sub->leaf];
	for (i = 0; i < numverts; i++) {
		seg = lf->seg;

		if (seg && (seg->flags & 1)) {
			R_WallPrep(seg);
		}

		lf++;
	}

	// render ceilings
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

	// Render Floors
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
		} else { // liquid floors
			if (frontsector->flags & MS_SCROLLFLOOR) {
				xoffset = frontsector->xoffset;
				yoffset = frontsector->yoffset;
			} else {
				xoffset = scrollfrac;
				yoffset = 0;
			}

			lf = &leafs[sub->leaf];
			dont_color = 1;
			R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
							textures[frontsector->floorpic + 1],
							xoffset, yoffset,
							lights[frontsector->colors[1]].rgba, 0, frontsector->lightlevel, 255);
			dont_color = 0;

			lf = &leafs[sub->leaf];
			R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
							textures[frontsector->floorpic],
							-yoffset, xoffset,
							lights[frontsector->colors[1]].rgba, 0, frontsector->lightlevel, 160);
		}
	}

	// render things
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
	fixed_t height;
	int frontheight;
	int sideheight;
	short pic;

	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	float rn, gn, bn;
	float scale;
	unsigned int thingcolor;
	unsigned int upcolor;
	unsigned int lowcolor;
	unsigned int topcolor;
	unsigned int bottomcolor;
	unsigned int tmp_upcolor;
	unsigned int tmp_lowcolor;
	int curRowoffset;

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

	if (li->flags & ML_BLENDING) {
		r1 = ((upcolor  >> 24) & 0xff);
		g1 = ((upcolor  >> 16) & 0xff);
		b1 = ((upcolor  >> 8) & 0xff);
		r2 = ((lowcolor >> 24) & 0xff);
		g2 = ((lowcolor >> 16) & 0xff);
		b2 = ((lowcolor >> 8) & 0xff);
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
					sideheight = b_ceilingheight - f_ceilingheight;

					scale = (float)sideheight / (float)frontheight;

					rn = ((float)r1-(float)r2)*scale + (float)r1;
					gn = ((float)g1-(float)g2)*scale + (float)g1;
					bn = ((float)b1-(float)b2)*scale + (float)b1;

					if (!((rn < 256) && (gn < 256) && (bn < 256))) { // Rescale if out of color bounds
						scale = 255.0f;

						if (rn >= gn && rn >= bn) {
							scale /= rn;
						} else if (gn >= rn && gn >= bn) {
							scale /= gn;
						} else {
							scale /= bn;
						}

						rn *= scale;
						gn *= scale;
						bn *= scale;
					}

					tmp_lowcolor = ((int)rn << 24) | ((int)gn << 16) | ((int)bn << 8) | 0xff;

					if (gamemap == 3 && (brightness > 57) && (brightness < 90)) {
						int x1 = li->v1->x >> 16;
						int y1 = li->v1->y >> 16;
						int x2 = li->v2->x >> 16;
						int y2 = li->v2->y >> 16;

						if ( ( (x1 == 1040 && y1 == -176) && (x2 == 1008 && y2 == -176) ) ||
							( (x1 == 1008 && y1 == -464) && (x2 == 1040 && y2 == -464) ) ) {
							float scale = 1.0f - ((float)((/*brightness*/75-60)*3.0f)*0.0025f);

							tmp_upcolor = ((int)(r1*scale)<<24) |
											((int)(g1*scale)<<16) |
											((int)(b1*scale)<<8) |
											0xff;

							tmp_lowcolor = ((int)(rn*scale) << 24) |
											((int)(gn*scale) << 16) |
											((int)(bn*scale) << 8) |
											0xff;
						}
					}
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
					sideheight = b_floorheight - f_ceilingheight;

					scale = (float)sideheight / (float)frontheight;

					rn = ((float)r1-(float)r2)*scale + (float)r1;
					gn = ((float)g1-(float)g2)*scale + (float)g1;
					bn = ((float)b1-(float)b2)*scale + (float)b1;

					if (!((rn < 256) && (gn < 256) && (bn < 256))) { // Rescale if out of color bounds
						scale = 255.0f;

						if (rn >= gn && rn >= bn) {
							scale /= rn;
						} else if (gn >= rn && gn >= bn) {
							scale /= gn;
						} else {
							scale /= bn;
						}

						rn *= scale;
						gn *= scale;
						bn *= scale;
					}

					tmp_upcolor = ((int)rn << 24) | ((int)gn << 16) | ((int)bn << 8) | 0xff;
					if (gamemap == 3 && (brightness > 57) && (brightness < 90)) {
						int x1 = li->v1->x >> 16;
						int y1 = li->v1->y >> 16;
						int x2 = li->v2->x >> 16;
						int y2 = li->v2->y >> 16;

						if ( ( (x1 == 1040 && y1 == -176) && (x2 == 1008 && y2 == -176) ) ||
							( (x1 == 1008 && y1 == -464) && (x2 == 1040 && y2 == -464) ) ) {
							float scale = 1.0f - ((float)((/*brightness*/75-60)*3.0f)*0.0025f);

							tmp_lowcolor = ((int)(r2*scale)<<24) |
											((int)(g2*scale)<<16) |
											((int)(b2*scale)<<8) |
											0xff;

							tmp_upcolor = ((int)(rn*scale) << 24) |
											((int)(gn*scale) << 16) |
											((int)(bn*scale) << 8) |
											0xff;
						}
					}
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

#define INTEGER_VERT 0

float last_width_inv = 1.0f / 64.0f;
float last_height_inv = 1.0f / 64.0f;

void P_CachePvrTexture(int i, int tag);

void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight,
				  int topOffset, int bottomOffset, int topColor, int bottomColor)
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
			cms = 2;
		} else {
			cms = 0;
		}

		if (flags & ML_VMIRROR) {
			cmt = 1;
		} else {
			cmt = 0;
		}

		if ((texture != globallump) || (globalcm != (cms | cmt))) {
			data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64);
			wshift = SwapShort(((textureN64_t*)data)->wshift);
			hshift = SwapShort(((textureN64_t*)data)->hshift);
			last_width_inv = 1.0f / (float)(1 << wshift);
			last_height_inv = 1.0f / (float)(1 << hshift);

			P_CachePvrTexture(texnum, PU_CACHE);

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

#if INTEGER_VERT
		signed short sx1 = (signed short)(v1->x >> 16);
		signed short sx2 = (signed short)(v2->x >> 16);
		signed short sy1 = -((signed short)(v1->y >> 16));
		signed short sy2 = -((signed short)(v2->y >> 16));

		float x1 = (float)sx1;
		float x2 = (float)sx2;
		float z1 = (float)sy1;
		float z2 = (float)sy2;
#else
		float x1 = (float)v1->x * inv65536;
		float z1 = -((float)v1->y * inv65536);
		float x2 = (float)v2->x * inv65536;
		float z2 = -((float)v2->y * inv65536);
#endif
		float y1 = (float)topHeight;
		float y2 = (float)bottomHeight;
		short stu1 = curTextureoffset >> 16;
		short stu2 = stu1 + (seg->length >> 4);
		short stv1 = topOffset;
		short stv2 = bottomOffset;
		float tu1 = (float)stu1 * last_width_inv;
		float tu2 = (float)stu2 * last_width_inv;
		float tv1 = (float)stv1 * last_height_inv;
		float tv2 = (float)stv2 * last_height_inv;

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

float last_sw_inv_x32;
float last_sh_inv_x32;

void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color)
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

		last_sw_inv_x32 = 32.0f / (float)(1 << wshift);
		last_sh_inv_x32 = 32.0f / (float)(1 << hshift);

		if (!VideoFilter) {
			tcxt[texture][0].txr.filter = PVR_FILTER_BILINEAR;
		} else {
			tcxt[texture][0].txr.filter = PVR_FILTER_NONE;
		}
		pvr_poly_compile(&thdr, &tcxt[texture][0]);
		globallump = texture;
	}

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

	float y1 = (float)topOffset;
	float y2 = y1 - 32.0f;

#if INTEGER_VERT
	float x1 = (float)(((x) - (cos << 3) + sin) >> 16);
	float x2 = (float)(((x) + (cos << 3) + sin) >> 16);
	float z1 = (float)(((-y) + (sin << 3) + cos) >> 16);
	float z2 = (float)(((-y) - (sin << 3) + cos) >> 16);
#else
	float x1 = (float)((x) - (cos << 3) + sin) * inv65536;
	float x2 = (float)((x) + (cos << 3) + sin) * inv65536;
	float z1 = (float)((-y) + (sin << 3) + cos) * inv65536;
	float z2 = (float)((-y) - (sin << 3) + cos) * inv65536;
#endif
	float tu1 = 0.0f;
	float tu2 = last_sw_inv_x32;
	float tv1 = 0.0f;
	float tv2 = last_sh_inv_x32;

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

	// this is the origin vertex for a bunch of the triangles, this is a special case
	dVTX[0] = &(dv0);
		
	dVTX[1] = &(dT1.dVerts[0]);
	dVTX[2] = &(dT1.dVerts[1]);
	dVTX[3] = &(dT1.dVerts[2]);

	vrt = lf[0].vertex;
#if INTEGER_VERT
	dVTX[0]->v.x = ((float)(vrt->x >> 16));
	dVTX[0]->v.y = (float)(zpos);
	dVTX[0]->v.z = -((float)(vrt->y >> 16));
#else
	dVTX[0]->v.x = ((float)vrt->x * inv65536);
	dVTX[0]->v.y = (float)(zpos);
	dVTX[0]->v.z = -((float)vrt->y * inv65536);
#endif
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

	// ceilings and floors don't need computation for unit normal
	// they're always flat planes
	// it is either unit down or unit up
	// easy
	recompute_norm = 0;
	crossprod = 1;
	if (ceiling) {
		norm.v.x = 0;
		norm.v.y = 0;
		norm.v.z = -1;
	} else {
		norm.v.x = 0;
		norm.v.y = 0;
		norm.v.z = 1;
	}

	if (numverts & 1) {
		vertex_t *vrt1;
		vertex_t *vrt2;

		idx = 2;

		vrt1 = lf[1].vertex;

#if INTEGER_VERT
		dVTX[1]->v.x = ((float)(vrt1->x >> 16));
		dVTX[1]->v.y = (float)(zpos);
		dVTX[1]->v.z = -((float)(vrt1->y >> 16));
#else
		dVTX[1]->v.x = ((float)vrt1->x * inv65536);
		dVTX[1]->v.y = (float)(zpos);
		dVTX[1]->v.z = -((float)vrt1->y * inv65536);
#endif
		stu = (((vrt1->x + xpos) >> FRACBITS) - x);
		stv = -(((vrt1->y + ypos) >> FRACBITS) - y);
		tu = (float)stu * inv64;
		tv = (float)stv * inv64;

		dVTX[1]->v.u = tu;
		dVTX[1]->v.v = tv;

		vrt2 = lf[2].vertex;

#if INTEGER_VERT
		dVTX[2]->v.x = ((float)(vrt2->x >> 16));
		dVTX[2]->v.y = (float)(zpos);
		dVTX[2]->v.z = -((float)(vrt2->y >> 16));
#else
		dVTX[2]->v.x = ((float)vrt2->x * inv65536);
		dVTX[2]->v.y = (float)(zpos);
		dVTX[2]->v.z = -((float)vrt2->y * inv65536);
#endif
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
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
#if INTEGER_VERT
			dVTX[1]->v.x = ((float)(vrt1->x >> 16));
			dVTX[1]->v.y = (float)(zpos);
			dVTX[1]->v.z = -((float)(vrt1->y >> 16));
#else
			dVTX[1]->v.x = ((float)vrt1->x * inv65536);
			dVTX[1]->v.y = (float)(zpos);
			dVTX[1]->v.z = -((float)vrt1->y * inv65536);
#endif
			dVTX[1]->v.u = tu;
			dVTX[1]->v.v = tv;

			stu = (((vrt2->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt2->y + ypos) >> FRACBITS) - y);
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
#if INTEGER_VERT
			dVTX[2]->v.x = ((float)(vrt2->x >> 16));
			dVTX[2]->v.y = (float)(zpos);
			dVTX[2]->v.z = -((float)(vrt2->y >> 16));
#else
			dVTX[2]->v.x = ((float)vrt2->x * inv65536);
			dVTX[2]->v.y = (float)(zpos);
			dVTX[2]->v.z = -((float)vrt2->y * inv65536);
#endif

			dVTX[2]->v.u = tu;
			dVTX[2]->v.v = tv;

			stu = (((vrt3->x + xpos) >> FRACBITS) - x);
			stv = -(((vrt3->y + ypos) >> FRACBITS) - y);
			tu = (float)stu * inv64;
			tv = (float)stv * inv64;
#if INTEGER_VERT
			dVTX[3]->v.x = ((float)(vrt3->x >> 16));
			dVTX[3]->v.y = (float)(zpos);
			dVTX[3]->v.z = -((float)(vrt3->y >> 16));
#else
			dVTX[3]->v.x = ((float)vrt3->x * inv65536);
			dVTX[3]->v.y = (float)(zpos);
			dVTX[3]->v.z = -((float)vrt3->y * inv65536);
#endif
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
}

pvr_ptr_t pvr_troo[MAX_CACHED_SPRITES];
pvr_poly_hdr_t hdr_troo[MAX_CACHED_SPRITES];
pvr_poly_cxt_t cxt_troo[MAX_CACHED_SPRITES];
uint8_t __attribute__((aligned(32))) tmptroo[256*256];
int lump_frame[575 + 310] = {-1};
int used_lumps[575 + 310] = {-1};
int used_lump_idx = 0;
int del_idx = 0;
int total_cached_vram = 0;

int last_flush_frame = 0;

static inline uint32_t np2(uint32_t v)
{
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

// 1 - 348 decoration and item sprites (non-enemy)
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

	pvr_poly_hdr_t *theheader;

	dVTX[0] = &(dT1.dVerts[0]);
	dVTX[1] = &(dT1.dVerts[1]);
	dVTX[2] = &(dT1.dVerts[2]);
	dVTX[3] = &(dT2.dVerts[2]);

	vissprite_p = sub->vissprite;
	if (vissprite_p) {
		if (vissprite_p->thing->flags & MF_RENDERLASER) {
			do {
				R_RenderLaser(vissprite_p->thing);
				vissprite_p = vissprite_p->next;
				if (vissprite_p == NULL) {
					break;
				}
			} while(vissprite_p->thing->flags & MF_RENDERLASER);

			if (vissprite_p == NULL) {
				return;
			}
		}

		while (vissprite_p) {
			thing = vissprite_p->thing;
			lump = vissprite_p->lump;
			flip = vissprite_p->flip;

			if (thing->frame & FF_FULLBRIGHT) {
				color = 0xffffffff; // PACKRGBA(255, 255, 255, 255);
			} else {
				color = lights[vissprite_p->sector->colors[2]].rgba;
			}

			color = (color & 0xffffff00) | thing->alpha;

			data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);
			src = data + sizeof(spriteN64_t);
			compressed = SwapShort(((spriteN64_t*)data)->compressed);
			width = SwapShort(((spriteN64_t*)data)->width);
			height = SwapShort(((spriteN64_t*)data)->height);

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

			if ((lump <= 348) || ((lump >= 924) && (lump <= 965))) {
				nosprite = 0;

				// pull in each side of sprite by half pixel
				// fix for filtering 'crud' around the edge due to lack of padding
				if(!flip) {
					dVTX[0]->v.u = dVTX[3]->v.u = all_u[lump] + halfinv1024;
					dVTX[1]->v.u = dVTX[2]->v.u = all_u[lump] + (((float)spos - 0.5f)*inv1024);
				} else {
					dVTX[1]->v.u = dVTX[2]->v.u = all_u[lump] + halfinv1024;
					dVTX[0]->v.u = dVTX[3]->v.u = all_u[lump] + (((float)spos - 0.5f)*inv1024);
				}

				if (!VideoFilter) {
					theheader = &pvr_sprite_hdr;
				} else {
					theheader = &pvr_sprite_hdr_nofilter;
				}

				dVTX[0]->v.v = dVTX[1]->v.v = all_v[lump] + halfinv1024;
				dVTX[3]->v.v = dVTX[2]->v.v = all_v[lump] + (((float)height-0.5f)*inv1024);
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

				// cache flush conditions
				// 1) explicit flags
				// 2) wasn't enough VRAM for last caching attempt
				// 3) this code has run before, it has been more than 2 seconds since the last time the cache code was called
				//      and more than 3/4 of the cache slots are used
				// with these conditions, the caching code works well, handles the worst scenes (Absolution) without slowdown
				if (force_filter_flush || vram_low || 
					(last_flush_frame && ((NextFrameIdx - last_flush_frame) > (2*30)) && 
					(used_lump_idx > (MAX_CACHED_SPRITES * 3 / 4)))) {

					force_filter_flush = 0;
					vram_low = 0;

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
					used_lump_idx += 1;
//					if (lumpoff < 575) {
//						dbgio_printf("caching lump %d at index %d\n", lump, cached_index);
//					} else {
//						dbgio_printf("caching alternate lump %d at index %d\n", lump, cached_index);
//					}
				} else {
					nosprite = 1;

					// here it gets complicated
					// find if any of the lumps have the del_idx as their index
					// if so, set their index to -1

					// this gets incremented if all possible cache indices are used in a single frame
					// and nothing can be evicted
					int passes = 0;

					int start_del_idx = del_idx;
					int next_del_idx_lump = -1;

					//dbgio_printf("all lump cache slots used, find one to evict\n");

					// for every possible enemy sprite lump number
					for (int i=0;i<(575+310);i++) {
						if (passes) {
							nosprite = 1;
//							dbgio_printf("\t\t nothing was evictable");
							goto bail_evict;
						}

						// try to help this along by noting if we found the next del idx along the way
						if (used_lumps[i] == (del_idx + 1)) {
//							dbgio_printf("\t\thaven't found del_idx yet, found next_del_idx though\n");
							next_del_idx_lump = i;
						}

						// if this enemy sprite lump number is already cached
						// and the cache index is our "del idx"
						// we should attempt to evict this one first
						if(used_lumps[i] == del_idx) {
							if (lump_frame[i] == NextFrameIdx) {
//								dbgio_printf("\tlump %d at del_idx is used in this frame -- check more\n", i+349);

								// this can help us skip more passes through the entire lump set
								if (next_del_idx_lump != -1) {
									if (lump_frame[next_del_idx_lump] != NextFrameIdx) {
//										dbgio_printf("\t\tevicted %d by way of next_del_idx_lump\n", next_del_idx_lump);
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
//									dbgio_printf("\t\t del_idx wrapped back to start, about to fail things\n");
									passes = 1;
								}

								continue;
							} else {
//								dbgio_printf("\t\tevicted %d from old frame\n", i);
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
				}
bail_evict:
//				if (nosprite) {
//					dbgio_printf("could not cache lump %d\n", lumpoff);
//				} else {
				if (!nosprite) {
					// vram_low gets set if the sprite will use more than 1/4 available VRAM
					if (((wp2*hp2)*4) > pvr_mem_available()) {
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
						dVTX[0]->v.u = dVTX[3]->v.u = 0.0f;
						dVTX[1]->v.u = dVTX[2]->v.u = ((float)troowid / (float)wp2);
					} else {
						dVTX[1]->v.u = dVTX[2]->v.u = 0.0f;
						dVTX[0]->v.u = dVTX[3]->v.u = ((float)troowid / (float)wp2);
					}
					dVTX[0]->v.v = dVTX[1]->v.v = 0.0f;
					dVTX[2]->v.v = dVTX[3]->v.v = ((float)height / (float)hp2);
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

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
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

	clip_triangle(&dT1, &hdr, 255, PVR_LIST_TR_POLY, 0);
	clip_triangle(&dT2, &hdr, 255, PVR_LIST_TR_POLY, 0);

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

	psp = &viewplayer->psprites[0];

	flagtranslucent = (viewplayer->mo->flags & MF_SHADOW) != 0;

	for (i = 0; i < NUMPSPRITES; i++, psp++) {
		/* a null state means not active */
		if ((state = psp->state) != 0) {
			pvr_vertex_t *vert = quad2;
			float u1,v1,u2,v2;
			float x1,y1,x2,y2;
			uint8_t a1;
			uint32_t quad_color;
			uint32_t quad_light_color = 0;

			sprdef = &sprites[state->sprite];
			sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
			lump = sprframe->lump[0];

			data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);//dec_none);
			width = SwapShort(((spriteN64_t*)data)->width);
			width2 = (width + 7) & ~7;
			height = SwapShort(((spriteN64_t*)data)->height);

			u1 = all_u[lump];
			v1 = all_v[lump];
			u2 = all_u2[lump];
			v2 = all_v2[lump];

			if (flagtranslucent)
				a1 = 144;
			else
				a1 = psp->alpha;

			if (psp->state->frame & FF_FULLBRIGHT) {
				quad_color = D64_PVR_REPACK_COLOR_ALPHA(0xffffffff, a1);
			} else {
				uint32_t color = lights[frontsector->colors[2]].rgba;
				quad_color = D64_PVR_REPACK_COLOR_ALPHA(color, a1);
				quad_light_color = lighted_color(D64_PVR_REPACK_COLOR_ALPHA(color, a1), frontsector->lightlevel);
			}

			for(int i = 0; i < 4; i++) {
				quad2[i].argb = quad_color;
				quad2[i].oargb = quad_light_color;
			}

			{
				float lightingr = 0.0f;
				float lightingg = 0.0f;
				float lightingb = 0.0f;
				uint32_t projectile_light = 0;
				for (int i=0;i<lightidx+1;i++) {
					d64Vertex_t *lightvert = &lightverts[i];
					
					float dx = lightvert->v.x;
					float dy = lightvert->v.y;
					float dz = lightvert->v.z;
					float lr = projectile_lights[i].radius;

					float lightdist;
					vec3f_length(dx,dy,dz,lightdist);

					if (lightdist < lr) {
						float linear_scalar = ((lr - lightdist) / lr);
						float lum = 0.5f;
						float ls2 = linear_scalar*linear_scalar;

						float light_scale = lum * ls2;
							
						lightingr += (projectile_lights[i].r * light_scale);
						lightingg += (projectile_lights[i].g * light_scale);
						lightingb += (projectile_lights[i].b * light_scale);
					}		
				}

				if (quad_light_color != 0) {
					float coord_r = (float)((quad_light_color >> 16) & 0xff) / 255.0f;
					float coord_g = (float)((quad_light_color >> 8) & 0xff) / 255.0f;
					float coord_b = (float)(quad_light_color & 0xff) / 255.0f;
					lightingr += coord_r;
					lightingg += coord_g;
					lightingb += coord_b;
				}

				if ((lightingr > 1.0f) || (lightingg > 1.0f) || (lightingb > 1.0f)) {
					float maxrgb = 0.0f;
					if (lightingr > maxrgb) maxrgb = lightingr;
					if (lightingg > maxrgb) maxrgb = lightingg;
					if (lightingb > maxrgb) maxrgb = lightingb;

					lightingr /= maxrgb;
					lightingg /= maxrgb;
					lightingb /= maxrgb;
				}

				if ((lightingr + lightingg + lightingb) > 0.0f) {
					const int component_intensity = 96;
					projectile_light = 0xff000000 | 
					(((int)(lightingr*component_intensity) & 0xff) << 16) | 
					(((int)(lightingg*component_intensity) & 0xff) << 8) | 
					(((int)(lightingb*component_intensity) & 0xff));
				}

				for (int i = 0; i < 4; i++) {
					// this is a hack to handle dark areas
					// because I am using a specular highlight color for dynamic lighting,
					// really dark vertices with textures applied are still basically solid black
					// with a colorful full-face specular highlight
					// set the vertex color to a dark but not too dark gray
					// now the dynamic lighting has a useful effect
					int coord_r = (quad_color >> 16) & 0xff;
					int coord_g = (quad_color >> 8) & 0xff;
					int coord_b = quad_color & 0xff;
					if (coord_r < 0x10 && coord_g < 0x10 && coord_b < 0x10) {
						coord_r += 0x37;
						coord_g += 0x37;
						coord_b += 0x37;

						quad_color = D64_PVR_PACK_COLOR(0xff, coord_r, coord_g, coord_b);
						quad2[i].argb = quad_color;
					}
					quad2[i].oargb = projectile_light;
				}
			}

			x = (((psp->sx >> 16) - SwapShort(((spriteN64_t*)data)->xoffs)) + 160);
			y = (((psp->sy >> 16) - SwapShort(((spriteN64_t*)data)->yoffs)) + 239);

			if (viewplayer->onground) {
				x += (quakeviewx >> 22);
				y += (quakeviewy >> 16);
			}

			x1 = (float)x * RES_RATIO;
			y1 = (float)y * RES_RATIO;
			x2 = x1 + ((float)width2 * RES_RATIO);
			y2 = y1 + ((float)height * RES_RATIO);

			// pull in each side of sprite by half pixel
			// fix for filtering 'crud' around the edge due to lack of padding
			vert->x = x1;
			vert->y = y2;
			vert->u = u1 + halfinv1024;
			vert->v = v2 - halfinv1024;
			vert++;

			vert->x = x1;
			vert->y = y1;
			vert->u = u1 + halfinv1024;
			vert->v = v1 + halfinv1024;
			vert++;

			vert->x = x2;
			vert->y = y2;
			vert->u = u2 - halfinv1024;
			vert->v = v2 - halfinv1024;
			vert++;

			vert->x = x2;
			vert->y = y1;
			vert->u = u2 - halfinv1024;
			vert->v = v1 + halfinv1024;

			if (!VideoFilter) {
				pvr_list_prim(PVR_LIST_TR_POLY, &pvr_sprite_hdr, sizeof(pvr_poly_hdr_t));
			} else {
				pvr_list_prim(PVR_LIST_TR_POLY, &pvr_sprite_hdr_nofilter, sizeof(pvr_poly_hdr_t));
			}
			pvr_list_prim(PVR_LIST_TR_POLY, &quad2, sizeof(quad2));
		} // if ((state = psp->state) != 0)
	} // for i < numsprites
}
