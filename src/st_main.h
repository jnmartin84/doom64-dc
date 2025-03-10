#define ST_BELOW_OVL 0
#define ST_ABOVE_OVL 1

#define FLASHDELAY 8 /* # of tics delay (1/30 sec) */
#define FLASHTIMES 6 /* # of times to flash new frag amount (EVEN!) */

typedef struct {
	int active;
	int doDraw;
	int delay;
	int times;
} sbflash_t;

extern sbflash_t flashCards[6];
extern bool tryopen[6];

extern uint8_t *sfontlump;
extern uint8_t *statuslump;
extern int sumbolslump;

extern int err_text_x;
extern int err_text_y;

#define FIRST_SYMBOL 0x80
#define LAST_SYMBOL 0x90 // 0x91 for Right arrow

typedef struct {
	int x;
	int y;
	int w;
	int h;
} symboldata_t;

extern symboldata_t symboldata[];

void ST_Init(void);
void ST_InitEveryLevel(void);
void ST_Ticker(void);
void ST_Drawer(void);
void ST_UpdateFlash(void);

int ST_GetCenterTextX(char *text);

void ST_DrawNumber(int x, int y, int val, int mode, uint32_t color, int prio);
void ST_DrawString(int x, int y, char *text, uint32_t color, int prio);
void ST_DrawSymbol(int xpos, int ypos, int index, uint32_t color, int prio);
void ST_Message(int x, int y, char *text, uint32_t color, int prio);