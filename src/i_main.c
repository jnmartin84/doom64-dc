#include "i_main.h"
#include "doomdef.h"
#include "st_main.h"
#include <kos.h>
#include <kos/thread.h>
#include <dc/asic.h>
#include <sys/time.h>
#include <dc/vblank.h>
#include <dc/video.h>
#include <arch/irq.h>
#include <unistd.h>

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

pvr_init_params_t pvr_params = {
{ PVR_BINSIZE_16, 0, PVR_BINSIZE_16, 0, 0 }, 512*1024 /*VERTBUF_SIZE*/, 1, 0, 0, 3
};

uint8_t __attribute__((aligned(32))) op_buf[OP_VERTBUF_SIZE];
uint8_t __attribute__((aligned(32))) tr_buf[TR_VERTBUF_SIZE];

int side = 0;

extern int globallump;
extern int globalcm;

//----------
kthread_t *main_thread;
kthread_attr_t main_attr;

kthread_t *sys_ticker_thread;
kthread_attr_t sys_ticker_attr;

boolean disabledrawing = false;

mutex_t vbi2mtx;
mutex_t rdpmtx;
condvar_t vbi2cv;

volatile int vbi2msg = 0;
volatile int rdpmsg = 0;
volatile s32 vsync = 0;
volatile s32 drawsync2 = 0;
volatile s32 drawsync1 = 0;

u32 NextFrameIdx = 0;

s32 ControllerPakStatus = 1;
s32 gamepad_system_busy = 0;
s32 FilesUsed = -1;
u32 SystemTickerStatus = 0;

int vsinternal = 0;

void vblfunc(uint32_t c, void *d)
{
	vsync++;
}

int __attribute__((noreturn)) main(int argc, char **argv)
{
	dbgio_dev_select("serial");
	vid_set_enabled(0);
#if REAL_SCREEN_WD == 640
	vid_set_mode(DM_640x480, PM_RGB565);
#else
	vid_set_mode(DM_320x240, PM_RGB565);
#endif

	pvr_init(&pvr_params);

#if NO_DITHER
	PVR_SET(PVR_FB_CFG_2, 0x00000001);
#endif

	vblank_handler_add(&vblfunc, NULL);

	vid_set_enabled(1);

	mutex_init(&vbi2mtx, MUTEX_TYPE_NORMAL);
	mutex_init(&rdpmtx, MUTEX_TYPE_NORMAL);
	cond_init(&vbi2cv);

    main_attr.create_detached = 0;
    main_attr.stack_size = 65536;
    main_attr.stack_ptr = NULL;
    main_attr.prio = 10;
    main_attr.label = "I_Main";

    main_thread = thd_create_ex(&main_attr, I_Main, NULL);
	dbgio_printf("started main thread\n");

	thd_join(main_thread, NULL);

	while(1) {
		; // don't care anymore
	}
}

void *I_Main(void *arg);
void *I_SystemTicker(void *arg);

void *I_Main(void *arg)
{
    D_DoomMain();
    return 0;
}

uint64_t running = 0;

void *I_SystemTicker(void *arg)
{
	while(!running) {
		thd_pass();
	}

	while(true) {
		int isrdp = 0;

		mutex_lock(&rdpmtx);
		isrdp = rdpmsg;
		mutex_unlock(&rdpmtx);

		if(isrdp) {
			mutex_lock(&rdpmtx);
			rdpmsg = 0;
			mutex_unlock(&rdpmtx);

			SystemTickerStatus |= 16;
			thd_pass();
			continue;
		}

		if (SystemTickerStatus & 16) {
			if ((u32)(vsync - drawsync2) < 2) {
				thd_pass();
				continue;
			}

			SystemTickerStatus &= ~16;

			if (demoplayback) {
				vsync = drawsync2 + 2;
			}

			drawsync1 = vsync - drawsync2;
			drawsync2 = vsync;

			mutex_lock(&vbi2mtx);
			vbi2msg = 1;
			cond_signal(&vbi2cv);
			mutex_unlock(&vbi2mtx);
		}

		thd_pass();
	}

	return 0;
}

extern void S_Init(void);

void I_Init(void)
{
    sys_ticker_attr.create_detached = 0;
    sys_ticker_attr.stack_size = 32768;
    sys_ticker_attr.stack_ptr = NULL;
    sys_ticker_attr.prio = 9;
    sys_ticker_attr.label = "I_SystemTicker";

    sys_ticker_thread = thd_create_ex(&sys_ticker_attr, I_SystemTicker, NULL);

	/* osJamMesg(&sys_msgque_vbi2, (OSMesg)VID_MSG_KICKSTART, OS_MESG_NOBLOCK); */
	// initial value must be 1 or everything deadlocks
	vbi2msg = 1;
	rdpmsg = 0;

	dbgio_printf("I_Init: started system ticker thread\n");
}

#include "stdarg.h"

void I_Error(char *error, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, error);
    vsprintf (buffer, error, args);
    va_end (args);

	pvr_scene_finish();
	pvr_wait_ready();

    dbgio_printf("I_Error [%s]\n", buffer);

    while (true) {
		vid_waitvbl();
		pvr_scene_begin();
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, OP_VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, TR_VERTBUF_SIZE);
		I_ClearFrame();
		ST_Message(err_text_x, err_text_y, buffer, 0xffffffff);
		I_DrawFrame();
		pvr_scene_finish();
		pvr_wait_ready();
    }
}

typedef struct
{
	int pad_data;
} pad_t;

int last_joyx;
int last_joyy;
int last_Ltrig;
int last_Rtrig;

int I_GetControllerData(void)
{
	maple_device_t *controller;
	cont_state_t *cont;
	int ret = 0;

	controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

	if (controller) {
		cont = maple_dev_status(controller);

		// used for analog stick movement
		// see am_main.c, p_user.c
		last_joyx = ((cont->joyx * 3) / 4);
		last_joyy = -((cont->joyy * 3) / 4);
		// used for analog strafing, see p_user.c
		last_Ltrig = cont->ltrig;
		last_Rtrig = cont->rtrig;
		
		ret |= (last_joyy & 0xff);
		ret |= ((last_joyx & 0xff) << 8);

		// ATTACK
		ret |= (cont->buttons & CONT_A) ? PAD_Z_TRIG : 0;
		// USE
		ret |= (cont->buttons & CONT_B) ? PAD_RIGHT_C : 0;

		// AUTOMAP is x+y together
		if ((cont->buttons & CONT_X) && (cont->buttons & CONT_Y)) {
			ret |= PAD_UP_C;
		} else {
			// WEAPON BACKWARD
			ret |= (cont->buttons & CONT_X) ? PAD_A : 0;
			// WEAPON FORWARD
			ret |= (cont->buttons & CONT_Y) ? PAD_B : 0;
		}

		// MOVE
		ret |= (cont->buttons & CONT_DPAD_RIGHT) ? PAD_RIGHT : 0;
		ret |= (cont->buttons & CONT_DPAD_LEFT) ? PAD_LEFT : 0;
		ret |= (cont->buttons & CONT_DPAD_DOWN) ? PAD_DOWN : 0;
		ret |= (cont->buttons & CONT_DPAD_UP) ? PAD_UP : 0;

		// START
		ret |= (cont->buttons & CONT_START) ? PAD_START : 0;

		if(cont->ltrig && !cont->rtrig) {
			ret |= PAD_L_TRIG;
		}
		else if(cont->rtrig && !cont->ltrig) {
			ret |= PAD_R_TRIG;
		}
	}

	return ret;
}



void I_CheckGFX(void)
{
	// what this used to do is what that pvr base assert will fail on 
}

void I_ClearFrame(void) // 8000637C
{
	NextFrameIdx += 1;

	globallump = -1;
	globalcm = 0;
}

void I_DrawFrame(void)  // 80006570
{
	running++;

	mutex_lock(&vbi2mtx);
	while (!vbi2msg) {
		cond_wait(&vbi2cv, &vbi2mtx);
	}
	vbi2msg = 0;
	mutex_unlock(&vbi2mtx);
}

void I_GetScreenGrab(void)
{
}

long LongSwap(long dat)
{
	return dat;
}

short LittleShort(short dat)
{
	return (dat << 16) >> 16;
}

short SwapShort(short dat)
{
	return ((((dat << 8) | (dat >> 8 & 0xff)) << 16) >> 16);
}

short BigShort(short dat)
{
	return dat & 0xffff;
}

void I_MoveDisplay(int x,int y) // 80006790
{
}

#define MELTALPHA2 0.00392f
#define FB_TEX_W 512
#define FB_TEX_H 256
#define FB_TEX_SIZE (FB_TEX_W * FB_TEX_H * sizeof(uint16_t))
#define US_ONE_FRAME (1000000 / 60)
extern float empty_table[129];
void I_WIPE_MeltScreen(void)
{
	pvr_vertex_t __attribute__((aligned(32))) verts[8];
	pvr_poly_cxt_t wipecxt;
	pvr_poly_hdr_t wipehdr;
	pvr_ptr_t pvrfb = 0;
	pvr_vertex_t *vert;

	float x0,y0,x1,y1;
	float y0a,y1a;
	float u0,v0,u1,v1;
	float v1a;

	uint32_t save;
	uint16_t *fb = (uint16_t *)Z_Malloc(FB_TEX_SIZE, PU_STATIC, NULL);

	if (!fb) {
		goto wipe_return;
	}

	pvrfb = pvr_mem_malloc(FB_TEX_SIZE);

	if(!pvrfb) {
		goto wipe_end;
	}

	memset(fb, 0, FB_TEX_SIZE);

	save = irq_disable();
#if REAL_SCREEN_WD == 640
	for (uint32_t y=0;y<480;y+=2) {
		for (uint32_t x=0;x<640;x+=2) {
			// (y/2) * 512 == y << 8
			// y*640 == (y<<9) + (y<<7)
			fb[(y << 8) + (x >> 1)] = vram_s[((y<<9) + (y<<7)) + x];
		}
	}
#else
	for (uint32_t y=0;y<240;y++) {
		for (uint32_t x=0;x<320;x++) {
			fb[(y << 9) + x] = vram_s[(y<<8) + (y<<6) + x];
		}
	}
#endif
	irq_restore(save);

	pvr_txr_load(fb, pvrfb, FB_TEX_SIZE);
	pvr_poly_cxt_txr(&wipecxt, PVR_LIST_TR_POLY,
					PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
					FB_TEX_W, FB_TEX_H, pvrfb, PVR_FILTER_NONE);
	wipecxt.blend.src = PVR_BLEND_ONE;
	wipecxt.blend.dst = PVR_BLEND_ONE;
	pvr_poly_compile(&wipehdr,  &wipecxt);

	// Fill borders with black
	pvr_set_bg_color(0,0,0);
	pvr_fog_table_color(0.0f,0.0f,0.0f,0.0f);
	pvr_fog_table_custom(empty_table);

	u0 = 0.0f;
	u1 = 0.625f; // 320.0f / 512.0f;
	v0 = 0.0f;
	v1 = 0.9375f; // 240.0f / 256.0f;
	x0 = 0.0f;
	y0 = 0.0f;
	x1 = REAL_SCREEN_WD;
	y1 = REAL_SCREEN_HT;
	y0a = y0;
	y1a = y1;

	for (int vn = 0; vn < 4; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].z = 5.0f;
		verts[vn].argb = 0xffff0000; // red, alpha 255/255
		verts[vn].oargb = 0;
	}
	verts[3].flags = PVR_CMD_VERTEX_EOL;

	for (int vn = 4; vn < 8; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].z = 5.01f;
		verts[vn].argb = 0x10080808; // almost black, alpha 16/255
		verts[vn].oargb = 0;
	}
	verts[7].flags = PVR_CMD_VERTEX_EOL;

	for (int i=0;i<160;i+=2) {
		vid_waitvbl();

		pvr_scene_begin();
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, OP_VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, TR_VERTBUF_SIZE);
		vert = verts;
		vert->x = x0;
		vert->y = y1;
		vert->u = u0;
		vert->v = v1;
		vert++;

		vert->x = x0;
		vert->y = y0;
		vert->u = u0;
		vert->v = v0;
		vert++;

		vert->x = x1;
		vert->y = y1;
		vert->u = u1;
		vert->v = v1;
		vert++;

		vert->x = x1;
		vert->y = y0;
		vert->u = u1;
		vert->v = v0;
		vert++;

#if 1
		// I'm not sure if I need this but leaving it for now
		if (y1a > y1 + 31) {
			double ydiff = y1a - REAL_SCREEN_HT;
			y1a = REAL_SCREEN_HT;
			v1a = (240.0f - ydiff) / 256.0f;
		} else {
			v1a = v1;
		}
#endif

		vert->x = x0;
		vert->y = y1a;
		vert->u = u0;
		vert->v = v1a;
		vert++;

		vert->x = x0;
		vert->y = y0a;
		vert->u = u0;
		vert->v = v0;
		vert++;

		vert->x = x1;
		vert->y = y1a;
		vert->u = u1;
		vert->v = v1a;
		vert++;

		vert->x = x1;
		vert->y = y0a;
		vert->u = u1;
		vert->v = v0;

		pvr_list_prim(PVR_LIST_TR_POLY, &wipehdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_TR_POLY, &verts[0], 8 * sizeof(pvr_vertex_t));

		pvr_scene_finish();
		pvr_wait_ready();

		if(i < 158) {
			save = irq_disable();
#if REAL_SCREEN_WD == 640
			for (uint32_t y=0;y<480;y+=2) {
				for (uint32_t x=0;x<640;x+=2) {
					// (y/2) * 512 == y << 8
					// y*640 == (y<<9) + (y<<7)
					fb[(y << 8) + (x >> 1)] = vram_s[((y<<9) + (y<<7)) + x];
				}
			}
#else
			for (uint32_t y=0;y<240;y++) {
				for (uint32_t x=0;x<320;x++) {
					fb[(y << 9) + x] = vram_s[(y<<8) + (y<<6) + x];
				}
			}
#endif
			irq_restore(save);

			pvr_txr_load(fb, pvrfb, FB_TEX_SIZE);
#if REAL_SCREEN_WD == 640
			y0a += 1.0f;
			y1a += 1.0f;
#else
			y0a += 0.5f;
			y1a += 0.5f;
#endif
			usleep(US_ONE_FRAME);
		}		
	}

	pvr_mem_free(pvrfb);

wipe_end:
	Z_Free(fb);
wipe_return:
	I_WIPE_FadeOutScreen();
	return;
}

void I_WIPE_FadeOutScreen(void) // 80006D34
{
	pvr_vertex_t __attribute__((aligned(32))) verts[4];
	pvr_poly_cxt_t wipecxt;
	pvr_poly_hdr_t wipehdr;
	pvr_ptr_t pvrfb = 0;
	pvr_vertex_t *vert;

	float x0,y0,x1,y1;
	float u0,v0,u1,v1;

	uint16_t *fb = (uint16_t *)Z_Malloc(FB_TEX_SIZE, PU_STATIC, NULL);
	uint32_t save;

	if (!fb) {
		goto fade_return;
	}

	pvrfb = pvr_mem_malloc(FB_TEX_SIZE);

	if(!pvrfb) {
		goto fade_end;
	}

	memset(fb, 0, FB_TEX_SIZE);

	save = irq_disable();
#if REAL_SCREEN_WD == 640
	for (uint32_t y=0;y<480;y+=2) {
		for (uint32_t x=0;x<640;x+=2) {
			// (y/2) * 512 == y << 8
			// y*640 == (y<<9) + (y<<7)
			fb[(y << 8) + (x >> 1)] = vram_s[((y<<9) + (y<<7)) + x];
		}
	}
#else
	for (uint32_t y=0;y<240;y++) {
		for (uint32_t x=0;x<320;x++) {
			fb[(y << 9) + x] = vram_s[(y<<8) + (y<<6) + x];
		}
	}
#endif
	irq_restore(save);

	pvr_txr_load(fb, pvrfb, FB_TEX_SIZE);
	pvr_poly_cxt_txr(&wipecxt, PVR_LIST_TR_POLY,
					PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
					FB_TEX_W, FB_TEX_H, pvrfb, PVR_FILTER_NONE);
	wipecxt.blend.src = PVR_BLEND_ONE;
	wipecxt.blend.dst = PVR_BLEND_ONE;
	pvr_poly_compile(&wipehdr, &wipecxt);

	pvr_set_bg_color(0,0,0);
	pvr_fog_table_color(0.0f,0.0f,0.0f,0.0f);

	u0 = 0.0f;
	u1 = 0.625f; // 320.0f / 512.0f;
	v0 = 0.0f;
	v1 = 0.9375f; // 240.0f / 256.0f;
	x0 = 0.0f;
	y0 = 0.0f;
	x1 = REAL_SCREEN_WD;
	y1 = REAL_SCREEN_HT;
	for (int vn = 0; vn < 4; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].z = 5.0f;
		verts[vn].oargb = 0;
	}
	verts[3].flags = PVR_CMD_VERTEX_EOL;

	for (int i=248;i>=0;i-=8) {
		uint8_t ui = (uint8_t)(i & 0xff);
		uint32_t fcol = 0xff000000 | (ui << 16) | (ui << 8) | ui;

		vid_waitvbl();
		pvr_scene_begin();
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, OP_VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, TR_VERTBUF_SIZE);
		vert = verts;
		vert->x = x0;
		vert->y = y1;
		vert->u = u0;
		vert->v = v1;
		vert->argb = fcol;
		vert++;

		vert->x = x0;
		vert->y = y0;
		vert->u = u0;
		vert->v = v0;
		vert->argb = fcol;
		vert++;

		vert->x = x1;
		vert->y = y1;
		vert->u = u1;
		vert->v = v1;
		vert->argb = fcol;
		vert++;

		vert->x = x1;
		vert->y = y0;
		vert->u = u1;
		vert->v = v0;
		vert->argb = fcol;
		pvr_list_prim(PVR_LIST_TR_POLY, &wipehdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_TR_POLY, &verts, sizeof(verts));

		pvr_scene_finish();
		pvr_wait_ready();
		usleep(US_ONE_FRAME);
	}

	pvr_mem_free(pvrfb);

fade_end:
	Z_Free(fb);
fade_return:
	return;
}


int I_CheckControllerPak(void) // 800070B0
{
    return 0;
}

int I_DeletePakFile(int filenumb) // 80007224
{
    return 0;
}

int I_SavePakFile(int filenumb, int flag, byte *data, int size) // 80007308
{
    return 0;
}

#define COMPANY_CODE 0x3544     // 5D
#define GAME_CODE 0x4e444d45    // NDME

int I_ReadPakFile(void) // 800073B8
{
    return 0;
}

int I_CreatePakFile(void) // 800074D4
{
    return 0;
}
