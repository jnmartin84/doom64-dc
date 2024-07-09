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
{ PVR_BINSIZE_16, 0, PVR_BINSIZE_16, 0, 0 }, VERTBUF_SIZE, 1, 0, 0, 3
};

uint8_t __attribute__((aligned(32))) op_buf[VERTBUF_SIZE];
uint8_t __attribute__((aligned(32))) tr_buf[VERTBUF_SIZE];

int side = 0;

extern int globallump;
extern int globalcm;

//----------

#define SYS_THREAD_ID_TICKER 3

kthread_t *sys_ticker_thread;
kthread_attr_t sys_ticker_attr;

//u32 vid_side;
u8 gamepad_bit_pattern; // 800A5260 // one bit for each controller

s32 File_Num;   // 800A54D8
s32 Pak_Size;   // 800A54DC
u8 *Pak_Data;   // 800A54E0
s32 Pak_Memory; // 800A54E4

boolean disabledrawing = false; // 8005A720

s32 vsync = 0;      // 8005A724
s32 drawsync2 = 0;  // 8005A728
s32 drawsync1 = 0;  // 8005A72C
u32 NextFrameIdx = 0;       // 8005A730

s32 ControllerPakStatus = 1; // 8005A738
s32 gamepad_system_busy = 0; // 8005A73C
s32 FilesUsed = -1; // 8005A740
u32 SystemTickerStatus = 0;  // 8005a744

int vsinternal = 0;

void vblfunc(uint32_t c, void *d)
{
	vsync++;
}

int __attribute__((noreturn)) main(int argc, char **argv)
{
	dbgio_dev_select("serial");
//	dbgio_dev_select("fb");
#if REAL_SCREEN_WD == 640
	vid_set_mode(DM_640x480, PM_RGB565);
#else
	vid_set_mode(DM_320x240, PM_RGB565);
#endif
	pvr_init(&pvr_params);

	vblank_handler_add(&vblfunc, NULL);
	I_Main(NULL);
	while(1) {}
}

void *I_Main(void *arg);
void *I_SystemTicker(void *arg);

void *I_Main(void *arg)
{
    D_DoomMain();
    return 0;
}

uint64_t running = 0;

extern int dc_fb;
extern int dc_next_fb;

void *I_SystemTicker(void *arg)
{
	while(!running) {
		thd_pass();
	}

	while(true) {
		 
		{		
			if (side & 1) {
				if (gamepad_system_busy) {
					gamepad_system_busy = 0;
				}
			}

			side++;

			if (SystemTickerStatus & 16) {
				if ((u32)(vsync - drawsync2) < 2) {
					thd_pass();
					continue;
				}
				if(vsync & 1) {
					thd_pass();
					continue;
				}
				
				SystemTickerStatus &= ~16;

				if (demoplayback || demorecording) {
					vsync = drawsync2 + 2;
				}

				drawsync1 = vsync - drawsync2;
				drawsync2 = vsync;
			}
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
    sys_ticker_attr.prio = 10;
    sys_ticker_attr.label = "I_SystemTicker";

    sys_ticker_thread = thd_create_ex(&sys_ticker_attr, I_SystemTicker, NULL);
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
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, VERTBUF_SIZE);
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
		
		// ATTACK
		ret |= (cont->buttons & CONT_A) ? PAD_Z_TRIG : 0;
		// USE
		ret |= (cont->buttons & CONT_B) ? PAD_RIGHT_C : 0;
		// WEAPON BACKWARD
		ret |= (cont->buttons & CONT_X) ? PAD_A : 0;
		// WEAPON FORWARD
		ret |= (cont->buttons & CONT_Y) ? PAD_B : 0;

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
		else if (cont->ltrig && cont->rtrig) {
			ret |= PAD_UP_C;
			last_Ltrig = 0;
			last_Rtrig = 0;
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

	R_RenderFilter();

	globallump = -1;
	globalcm = 0;
}

void I_DrawFrame(void)  // 80006570
{
//	vid_side ^= 1;

	SystemTickerStatus |= 16;

	running++;
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
#if 0
  int ViMode;

  ViMode = osViGetCurrentMode();

  osViModeTable[ViMode].comRegs.hStart =
       (int)(((int)video_hStart >> 0x10 & 65535) + x) % 65535 << 0x10 |
       (int)((video_hStart & 65535) + x) % 65535;

  osViModeTable[ViMode].fldRegs[0].vStart =
       (int)(((int)video_vStart1 >> 0x10 & 65535) + y) % 65535 << 0x10 |
       (int)((video_vStart1 & 65535) + y) % 65535;

  osViModeTable[ViMode].fldRegs[1].vStart =
       (int)(((int)video_vStart2 >> 0x10 & 65535) + y) % 65535 << 0x10 |
       (int)((video_vStart2 & 65535) + y) % 65535;
#endif
}

#define MELTALPHA2 0.00392f
#define FB_TEX_W 512
#define FB_TEX_H 256
#define FB_TEX_SIZE (FB_TEX_W * FB_TEX_H * sizeof(uint16_t))
#define US_ONE_FRAME (1000000 / 60)
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
					FB_TEX_W, FB_TEX_H, pvrfb, PVR_FILTER_BILINEAR);
	wipecxt.blend.src = PVR_BLEND_ONE;
	wipecxt.blend.dst = PVR_BLEND_ONE;
	pvr_poly_compile(&wipehdr,  &wipecxt);

	pvr_set_bg_color(0,0,0);
	pvr_fog_table_color(0.0f,0.0f,0.0f,0.0f);

	u0 = 0.0f;
	u1 = 0.625f; // 320.0f / 512.0f;
	v0 = 0.0f;
	v1 = 0.9375f; // 240.0f / 256.0f;
	x0 = 0.0f;
	y0 = 0.0f;
	x1 = REAL_SCREEN_WD - 1;
	y1 = REAL_SCREEN_HT - 1;
	y0a = y0;
	y1a = y1;

	for (int vn = 0; vn < 4; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].z = 5.0f;
		verts[vn].argb = 0xffff0000; // PVR_PACK_COLOR(1.0, 1.0, 0.0, 0.0);
		verts[vn].oargb = 0;
	}
	verts[3].flags = PVR_CMD_VERTEX_EOL;

	for (int vn = 4; vn < 8; vn++) {
		verts[vn].flags = PVR_CMD_VERTEX;
		verts[vn].z = 6.0f;
		verts[vn].argb = 0x01000000; // PVR_PACK_COLOR(MELTALPHA2, 0.0, 0.0, 0.0);
		verts[vn].oargb = 0;
	}
	verts[7].flags = PVR_CMD_VERTEX_EOL;

	for (int i=0;i<160;i+=2) {
		vid_waitvbl();

		pvr_scene_begin();
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, VERTBUF_SIZE);
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

		vert->x = x0;
		vert->y = y1a;
		vert->u = u0;
		vert->v = v1;
		vert++;

		vert->x = x0;
		vert->y = y0a;
		vert->u = u0;
		vert->v = v0;
		vert++;

		vert->x = x1;
		vert->y = y1a;
		vert->u = u1;
		vert->v = v1;
		vert++;

		vert->x = x1;
		vert->y = y0a;
		vert->u = u1;
		vert->v = v0;

		pvr_list_prim(PVR_LIST_TR_POLY, &wipehdr, sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_TR_POLY, &verts, sizeof(verts));

		pvr_scene_finish();
		pvr_wait_ready();

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
		y0a += 4.0f;
		y1a += 4.0f;
#else
		y0a += 2.0f;
		y1a += 2.0f;
#endif
		usleep(US_ONE_FRAME);
	}

	pvr_mem_free(pvrfb);

wipe_end:
	Z_Free(fb);
wipe_return:
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
					FB_TEX_W, FB_TEX_H, pvrfb, PVR_FILTER_BILINEAR);
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
	x1 = REAL_SCREEN_WD - 1;//639.0f;
	y1 = REAL_SCREEN_HT - 1;//479.0f;
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
		pvr_set_vertbuf(PVR_LIST_OP_POLY, op_buf, VERTBUF_SIZE);
		pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_buf, VERTBUF_SIZE);
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
#if 0
    int ret, file;
    OSPfsState *fState;
    s32 MaxFiles [2];
    u8 validpaks;

    ControllerPakStatus = 0;

    if (gamepad_system_busy != 0)
    {
do {
    osYieldThread();
} while (gamepad_system_busy != 0);
    }

    FilesUsed = -1;
    ret = PFS_ERR_NOPACK;

    osPfsIsPlug(&sys_msgque_joy, &validpaks);

    /* does the current controller have a memory pak? */
    if (validpaks & 1)
    {
ret = osPfsInit(&sys_msgque_joy, &ControllerPak, NULL);

if ((ret != PFS_ERR_NOPACK) &&
    (ret != PFS_ERR_ID_FATAL) &&
    (ret != PFS_ERR_DEVICE) &&
    (ret != PFS_ERR_CONTRFAIL))
{
    ret = osPfsNumFiles(&ControllerPak, MaxFiles, &FilesUsed);

    if (ret == PFS_ERR_INCONSISTENT)
ret = osPfsChecker(&ControllerPak);

    if (ret == 0)
    {
Pak_Memory = 123;
fState = FileState;
file = 0;
do
{
    ret = osPfsFileState(&ControllerPak, file, fState);
    file += 1;

    if (ret != 0)
      fState->file_size = 0;

    Pak_Memory -= (fState->file_size >> 8);
    fState += 1;
} while (file != 16);
ret = 0;
    }
}
    }

    ControllerPakStatus = 1;

    return ret;
#endif
    return 0;
}

int I_DeletePakFile(int filenumb) // 80007224
{
#if 0
    int ret;
    OSPfsState *fState;

    ControllerPakStatus = 0;

    if (gamepad_system_busy != 0)
    {
do {
    osYieldThread();
} while (gamepad_system_busy != 0);
    }

    fState = &FileState[filenumb];

    if (fState->file_size == 0) {
ret = 0;
    }
    else
    {
ret = osPfsDeleteFile(&ControllerPak,
    FileState[filenumb].company_code,
    FileState[filenumb].game_code,
    FileState[filenumb].game_name,
    FileState[filenumb].ext_name);

if (ret == PFS_ERR_INCONSISTENT)
    ret = osPfsChecker(&ControllerPak);

if (ret == 0)
{
    Pak_Memory += (fState->file_size >> 8);
    fState->file_size = 0;
}
    }

    ControllerPakStatus = 1;

    return ret;
#endif
    return 0;
}

int I_SavePakFile(int filenumb, int flag, byte *data, int size) // 80007308
{
#if 0
    int ret;

    ControllerPakStatus = 0;

    if (gamepad_system_busy != 0)
    {
do {
osYieldThread();
} while (gamepad_system_busy != 0);
    }

    ret = osPfsReadWriteFile(&ControllerPak, filenumb, (u8)flag, 0, size, (u8*)data);

    if (ret == PFS_ERR_INCONSISTENT)
ret = osPfsChecker(&ControllerPak);

    ControllerPakStatus = 1;

    return ret;
#endif
    return 0;
}

#define COMPANY_CODE 0x3544     // 5D
#define GAME_CODE 0x4e444d45    // NDME

int I_ReadPakFile(void) // 800073B8
{
#if 0
    int ret;
    u8 *ext_name;

    ControllerPakStatus = 0;

    if (gamepad_system_busy != 0)
    {
do {
osYieldThread();
} while (gamepad_system_busy != 0);
    }

    Pak_Data = NULL;
    Pak_Size = 0;
    ext_name = NULL;

    ret = osPfsFindFile(&ControllerPak, COMPANY_CODE, GAME_CODE, Game_Name, ext_name, &File_Num);

    if (ret == 0)
    {
Pak_Size = FileState[File_Num].file_size;
Pak_Data = (byte *)Z_Malloc(Pak_Size, PU_STATIC, NULL);
ret = osPfsReadWriteFile(&ControllerPak, File_Num, PFS_READ, 0, Pak_Size, Pak_Data);
    }

    ControllerPakStatus = 1;

    return ret;
#endif
    return 0;
}

int I_CreatePakFile(void) // 800074D4
{
#if 0
    int ret;
    u8 ExtName [8];

    ControllerPakStatus = 0;

    if (gamepad_system_busy != 0)
    {
do {
  osYieldThread();
} while (gamepad_system_busy != 0);
    }

    if (Pak_Memory < 2)
Pak_Size = 256;
    else
Pak_Size = 512;

    Pak_Data = (byte *)Z_Malloc(Pak_Size, PU_STATIC, NULL);
    D_memset(Pak_Data, 0, Pak_Size);

    *(int*)ExtName = 0;

    ret = osPfsAllocateFile(&ControllerPak, COMPANY_CODE, GAME_CODE, Game_Name, ExtName, Pak_Size, &File_Num);

    if (ret == PFS_ERR_INCONSISTENT)
ret = osPfsChecker(&ControllerPak);

    if (ret == 0)
ret = osPfsReadWriteFile(&ControllerPak, File_Num, PFS_WRITE, 0, Pak_Size, Pak_Data);

    ControllerPakStatus = 1;

    return ret;
#endif
    return 0;
}
