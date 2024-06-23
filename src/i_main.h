#include <stdint.h>

#define f32 float
#define f64 double

#define	SCREEN_HT	240
#define	SCREEN_WD	320

/* this stack size is in bytes, and is a lot larger
 * than this program needs.
 */
#define	STACKSIZE	0x2000

/* for mem_heap and z_zone
 */
//#define MEM_HEAP_SIZE 0x400000 
//(0x500000 - 0x50 - 0x60 - 0x20 - 0x80 - 0xA0 - 0x1000) 
