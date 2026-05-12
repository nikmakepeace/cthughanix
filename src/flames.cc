#include "cthugha.h"
#include "translate.h"
#include "display.h"
#include "interface.h"
#include "imath.h"
#include "cth_buffer.h"
#include "CthughaBuffer.h"
#include "AutoChanger.h"


CoreOptionEntryList generalFlameEntries;	


void flame_clear();	void flame_upslow();	void flame_upsubtle();	
void flame_upfast();	void flame_leftslow();	void flame_leftsubtle();
void flame_leftfast();	void flame_rightslow();	void flame_rightsubtle();
void flame_rightfast();	void flame_water();	void flame_watersubtle();
void flame_skyline();	void flame_weird();	void flame_zzz();
void flame_fade();	void flame_general_subtle();
void flame_general_slow();

void flame_general_subtle_no_trans();
void flame_general_subtle_trans();

void flame_general_slow_no_trans();
void flame_general_slow_trans();

void flame_down();


const char * OptionGeneralFlame::text() const {
    char str[512];

    if(lock)
	sprintf(str, "locked:%d", value);
    else
	sprintf(str, "%d", value);
    
    return str;
}


class FlameEntry : public  CoreOptionEntry {
public:
    void (*flame)();

    FlameEntry(void (*f)(), const char * name, const char * desc, int inUse = 1) :
	CoreOptionEntry(name, desc, inUse), flame(f) {}

    int operator()() {
	(*flame)();
	return 0;
    }
};

CoreOptionEntry * _flames[] = {
    new FlameEntry(flame_clear,		"Clear",	"Blank the buffer", 0),
    new FlameEntry(flame_upslow,	"u-Sl",		"Up Slow"),
    new FlameEntry(flame_upsubtle,	"u-Su",		"Up Subtle"),
    new FlameEntry(flame_upfast,	"u-Fa",		"Up Fast"),
    new FlameEntry(flame_leftslow,	"l-Sl",		"Left Slow"),
    new FlameEntry(flame_leftsubtle,	"l-Su",		"Left Subtle"),
    new FlameEntry(flame_leftfast,	"l-Fa",		"Left Fast"),
    new FlameEntry(flame_rightslow,	"r-Sl",		"Right Slow"),
    new FlameEntry(flame_rightsubtle,	"r-Su",		"Right Subtle"),
    new FlameEntry(flame_rightfast,	"r-Fa",		"Right Fast"),
    new FlameEntry(flame_water,		"Water", 	"Water"),
    new FlameEntry(flame_watersubtle,	"Wa-s",		"Water Subtle"),
    new FlameEntry(flame_skyline,	"Skyline",	"Skyline"),
    new FlameEntry(flame_weird,		"Weird", 	"Weird"),
    new FlameEntry(flame_zzz,		"Zzz",		"Zzz"),	
    new FlameEntry(flame_fade,		"Fade",		"Fade"),
    new FlameEntry(flame_general_subtle,"GenSubt",	"General Subtle"),
    new FlameEntry(flame_general_slow,	"GenSlow",	"General Slow"),
    new FlameEntry(flame_down,		"Down",		"Falling Down"),
};
int _nFlames = sizeof(_flames)/sizeof(CoreOptionEntry*);

static int flame_offset[4];			/* table for general flames */

/*****************************************************************************/

unsigned char divsub[4*256];
unsigned char divsub2[4*256];
unsigned int divsub4[256];

unsigned int divsub_s0[4*256];
unsigned int divsub_s1[4*256];
unsigned int divsub_s2[4*256];
unsigned int divsub_s3[4*256];

/*
 * Initialize the tables used in the flame-functions
 */
int init_flames() {
    int i;

    for(i=0; i < 4*256; i++) {
	divsub[i]  = i >> 2 ? (i >> 2) - 1 : 0;
	divsub2[i] = i >> 1 ? (i >> 1) - 1 : 0;

#if (__BYTE_ORDER == __BIG_ENDIAN)
	divsub_s3[i] = (unsigned int)divsub[i];
	divsub_s2[i] = (unsigned int)divsub[i] << 8;
	divsub_s1[i] = (unsigned int)divsub[i] << 16;
	divsub_s0[i] = (unsigned int)divsub[i] << 24;
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
	divsub_s0[i] = (unsigned int)divsub[i];
	divsub_s1[i] = (unsigned int)divsub[i] << 8;
	divsub_s2[i] = (unsigned int)divsub[i] << 16;
	divsub_s3[i] = (unsigned int)divsub[i] << 24;
//#elif
//#error unknown endianess
#endif
    }
    for(i=0; i < 256; i++) 
	divsub4[i] = (i <= 2 ? 0 : i - 2);

    return 0;
}

    

void general_offset() {
    int i;
    /* offset to the neighbors */
    int position[9] = {
	-BUFF_WIDTH-1,	-BUFF_WIDTH, -BUFF_WIDTH+1,
	-1,		0,	     1,
	+BUFF_WIDTH-1,  +BUFF_WIDTH, +BUFF_WIDTH+1
    };

    int gen = CthughaBuffer::current->flameGeneral;
    int shift = gen % 9;
    gen = gen / 9;
    /* generate offset-table */
    for(i=0; i < 4; i++) {
	int p = gen % 9;
	gen = gen / 9;
	flame_offset[i] = position[p] +  position[shift];
    }
} 


/*
 * The trivial flame function. Used for debugging wave-fuctions
 */
void flame_clear() {
    memset( active_buffer, 0, BUFF_SIZE);
}

/*****************************************************************************
 *  FLAME-UP
 *****************************************************************************/

#define PTR     unsigned char * ptr;		\
		ptr = active_buffer;		\
		active_buffer = passive_buffer; \
		passive_buffer = ptr;
	

void flame_upslow() {		
    int i;
    unsigned int tmp;
    unsigned int tmp2;
    PTR;
    ptr = active_buffer + BUFF_WIDTH;

    ptr ++;    
    tmp = (unsigned int)(* (ptr-2-1)) + 
	(unsigned int)(* (ptr-1-1)) + 
	(unsigned int)(* (ptr-1));
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp = tmp - 
	    (unsigned int)(* (ptr-2-1)) + 
	    (unsigned int)(* (ptr+1-1));
	tmp2 = tmp + 
	    (unsigned int)(* (ptr+BUFF_WIDTH-1));
	* (ptr - BUFF_WIDTH-1) = divsub[tmp2]; 
	ptr ++;
    }
}

void flame_upsubtle() {
    flame_offset[0] = -1+BUFF_WIDTH;
    flame_offset[1] = 0+BUFF_WIDTH;
    flame_offset[2] = 1+BUFF_WIDTH;
    flame_offset[3] = BUFF_WIDTH+BUFF_WIDTH;

    if( (CthughaBuffer::current->translate.current() == NULL) || 
	(((TranslateEntry*)CthughaBuffer::current->translate.current())->trans == NULL) )
	flame_general_subtle_no_trans();
    else
	flame_general_subtle_trans();
}

/* about 24 fps */
void flame_upfast() {				
    int i;
    int tmp;
    PTR;
    ptr = active_buffer + BUFF_SIZE;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =  			
	    (int)(* ptr) +
	    (int)(* (ptr+BUFF_WIDTH-1)) + 
	    (int)(* (ptr+BUFF_WIDTH+1)) + 
	    (int)(* (ptr+BUFF_WIDTH));
	* ptr = divsub[tmp];
	ptr --;
    }
}

/*****************************************************************************
 *  FLAME-LEFT
 *****************************************************************************/

/* about 24 fps */
void flame_leftslow() {	
    int i;
    int tmp;
    PTR;
    ptr = active_buffer + BUFF_WIDTH;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =  				
	    (int)(* (ptr-BUFF_WIDTH+1)) +
	    (int)(* ptr) + 					
	    (int)(* (ptr+1)) + 				
	    (int)(* (ptr+BUFF_WIDTH));
	* (ptr-BUFF_WIDTH) = divsub[tmp];  
	ptr ++;
    }
}

void flame_leftsubtle() {				
    flame_offset[0] = +1;
    flame_offset[1] = +BUFF_WIDTH;
    flame_offset[2] = 1+BUFF_WIDTH;
    flame_offset[3] = BUFF_WIDTH+BUFF_WIDTH;

    if( (CthughaBuffer::current->translate.current() == NULL) || 
	(((TranslateEntry*)CthughaBuffer::current->translate.current())->trans == NULL) )
	flame_general_subtle_no_trans();
    else
	flame_general_subtle_trans();
}

/* about 26 fps */
void flame_leftfast() {	
    int i;
    int tmp;
    PTR;
    ptr = active_buffer + BUFF_SIZE;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =  
	    (int)(* ptr) +
	    (int)(* (ptr+BUFF_WIDTH+1)) + 
	    (int)(* (ptr+BUFF_WIDTH+1)) + 
	    (int)(* (ptr+BUFF_WIDTH));
	* ptr = divsub[tmp];
	ptr --;
    }
}

/*****************************************************************************
 *  FLAME-RIGHT
 *****************************************************************************/

/* about 23 fps */
void flame_rightslow() {				
    int i;
    int tmp;
    unsigned char * src = passive_buffer + BUFF_WIDTH+1;
    unsigned char * dst = active_buffer+1;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp = 
	    (int)(* (src-BUFF_WIDTH-1)) + 
	    (int)(* src) + 			
	    (int)(* (src-1)) + 			
	    (int)(* (src+BUFF_WIDTH));
	* dst = divsub[tmp];
	dst ++;
	src ++;
    }
}

void flame_rightsubtle() {	
    flame_offset[0] = -1;
    flame_offset[1] = BUFF_WIDTH-1;
    flame_offset[2] = BUFF_WIDTH;
    flame_offset[3] = BUFF_WIDTH+BUFF_WIDTH;

    if( (CthughaBuffer::current->translate.current() == NULL) || 
	(((TranslateEntry*)CthughaBuffer::current->translate.current())->trans == NULL) )
	flame_general_subtle_no_trans();
    else
	flame_general_subtle_trans();
}

/* about 29 fps */
void flame_rightfast() {				
    int i;
    int tmp;
    PTR;
    ptr = active_buffer + BUFF_SIZE;

    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =  			
	    (int)(* ptr) +
	    (int)(* (ptr+BUFF_WIDTH-1)) + 		
	    (int)(* (ptr+BUFF_WIDTH-1)) + 	
	    (int)(* (ptr+BUFF_WIDTH));
	* ptr = divsub[tmp];
	ptr --;
    }
}

/****************************************************************************
 *  FLAME-WATER
 *****************************************************************************/

/* about 23 fps */
void flame_water() {
    int i;
    int tmp;
    unsigned char * src = passive_buffer + BUFF_WIDTH;
    unsigned char * dst = active_buffer;

    for(i=BUFF_SIZE/2+BUFF_WIDTH; i != 0; i--) {
	tmp = 
	    (int)(* (src-1)) + 		
	    (int)(* src) + 	
	    (int)(* (src+1)) + 	
	    (int)(* (src+BUFF_WIDTH));
	* dst = divsub[tmp];
	dst ++;
	src ++;
    }

    src = passive_buffer + BUFF_WIDTH * (BUFF_HEIGHT-1);
    dst = active_buffer + BUFF_WIDTH * (BUFF_HEIGHT-0);
    for(i=BUFF_SIZE/2; i != 0; i--) {
	tmp = 
	    (int)(* (src-BUFF_WIDTH+1)) + 
	    (int)(* src) + 
	    (int)(* (src+1)) + 				
	    (int)(* (src-BUFF_WIDTH));
	* dst = divsub[tmp];
	dst --;
	src --;
    }
}

/* about 29 fps */
void flame_watersubtle() {
    int i;
    unsigned char tmp;
    char * src = (char*)(passive_buffer + BUFF_WIDTH);
    char * dst = (char*)active_buffer;

    for(i=BUFF_SIZE/2+BUFF_WIDTH; i != 0; i--) {
	tmp = 
	    (int)(* (src-1)) + 	
	    (int)(* src) + 	
	    (int)(* (src+1)) + 				
	    (int)(* (src+BUFF_WIDTH));
	* dst = divsub[tmp];
	dst ++;
	src ++;
    }

    src = (char*)passive_buffer + BUFF_WIDTH * (BUFF_HEIGHT-1);
    dst = (char*)active_buffer + BUFF_WIDTH * (BUFF_HEIGHT-0);
    for(i=BUFF_SIZE/2; i != 0; i--) {
	tmp = 
	    (int)(* (src-BUFF_WIDTH+1)) + 
	    (int)(* src) + 	
	    (int)(* (src+1)) + 
	    (int)(* (src-BUFF_WIDTH));
	* dst = divsub[tmp];
	dst --;
	src --;
    }
}

/****************************************************************************
 *  FLAME-others
 *****************************************************************************/

/* about 23 fps */
void flame_skyline() {
    int i;
    int tmp;
    unsigned char * src = passive_buffer + BUFF_WIDTH+1;
    unsigned char * dst = active_buffer;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp = 
	    (int)(* (src-1)) + 
	    (int)(* src) + 			
	    (int)(* (src+1)) + 	
	    (int)(* (src));
	* dst = divsub[tmp];
	dst ++;
	src ++;
    }
}

/* about 28 fps */
void flame_weird() {				
    int i;
    unsigned char tmp;
    char * src = (char*)passive_buffer + BUFF_WIDTH+1;
    char * dst = (char*)active_buffer+1;
	
    for(i=BUFF_SIZE; i != 0; i--) {
	tmp = 		
	    (* (src-1)) |
	    (* src) |		
	    (* (src+1)) |
	    (* (src+BUFF_WIDTH));
	* dst = divsub[tmp];
	dst ++;
	src ++;
    }
}

/*****************************************************************************
 *  new from cthugha 5.3
 *****************************************************************************/

void flame_zzz() {
    int i;
    unsigned char tmp;
    PTR;
    ptr = active_buffer + BUFF_WIDTH;

    for(i=BUFF_SIZE; i != 0; i--) {
	tmp = 
	    (* (ptr-1)) + 
	    (* (ptr+BUFF_WIDTH));
	*(ptr-BUFF_WIDTH) = divsub2[tmp];
	ptr ++;
    }
}

void flame_fade() {
    int i;
    unsigned int tmp;
    PTR;
    ptr = active_buffer;

    for(i=BUFF_SIZE/4; i != 0; i--) {
	tmp = (* (unsigned int*) ptr);
	*(unsigned int*)ptr = 
	    divsub4[(tmp) & 0xff] +
	    (divsub4[(tmp >> 8) & 0xff] << 8) +
	    (divsub4[(tmp >> 16) & 0xff] << 16) +
	    (divsub4[(tmp >> 24) & 0xff] << 24);
	ptr += 4;
    }
}

/*****************************************************************************
 * general flame functions
 *
 * this functions can replace most of the functions above.
 *****************************************************************************/

/*****************************************************************************
 * general subtle flame
 *****************************************************************************/
void flame_general_subtle() {				
    general_offset();

    if( (CthughaBuffer::current->translate.current() == NULL) || 
	(((TranslateEntry*)CthughaBuffer::current->translate.current())->trans == NULL) )
	flame_general_subtle_no_trans();
    else
	flame_general_subtle_trans();
}

void flame_general_subtle_no_trans() {
    int i;
    unsigned char tmp;
    unsigned char * ptr = active_buffer;
    int offset1, offset2, offset3, offset4;
    unsigned int t2;
    
    /* initialize offsets
     *
     *  ptr          -> destination (active_buffer)
     *  ptr + offset -> source (passive_buffer)
     */
    offset1 = flame_offset[0] + (passive_buffer - active_buffer);
    offset2 = flame_offset[1] + (passive_buffer - active_buffer);
    offset3 = flame_offset[2] + (passive_buffer - active_buffer);
    offset4 = flame_offset[3] + (passive_buffer - active_buffer);
    
    for(i=BUFF_SIZE/4; i != 0; i--) {
	tmp =
	    (* (ptr + offset1)) +
	    (* (ptr + offset2)) +
	    (* (ptr + offset3)) +
	    (* (ptr + offset4));
	t2 = divsub_s0[tmp];
	tmp =
	    (* (ptr + offset1 + 1)) +
	    (* (ptr + offset2 + 1)) +
	    (* (ptr + offset3 + 1)) +
	    (* (ptr + offset4 + 1));
	t2 |= divsub_s1[tmp];

	tmp =
	    (* (ptr + offset1 + 2)) +
	    (* (ptr + offset2 + 2)) +
	    (* (ptr + offset3 + 2)) +
	    (* (ptr + offset4 + 2));
	t2 |= divsub_s2[tmp];

	tmp =
	    (* (ptr + offset1 + 3)) +
	    (* (ptr + offset2 + 3)) +
	    (* (ptr + offset3 + 3)) +
	    (* (ptr + offset4 + 3));
	t2 |= divsub_s3[tmp];

	*(unsigned int*)ptr = t2;
	ptr += 4;
    }
}
void flame_general_subtle_trans() {
    int i;
    unsigned char tmp;
    unsigned char * ptr = active_buffer;
    unsigned char * offset1, *offset2, *offset3, *offset4;
    int * trans = ((TranslateEntry*)CthughaBuffer::current->translate.current())->trans;
    CthughaBuffer::current->done_translate = 1;

    offset1 = passive_buffer + flame_offset[0];
    offset2 = passive_buffer + flame_offset[1];
    offset3 = passive_buffer + flame_offset[2];
    offset4 = passive_buffer + flame_offset[3];

    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =
	    (* (*trans + offset1)) +
	    (* (*trans + offset2)) +
	    (* (*trans + offset3)) +
	    (* (*trans + offset4));
	*ptr = divsub[tmp];

	trans ++;
	ptr ++;
    }
}

/*****************************************************************************
 * general slow flame
 *
 * is a bit slower than traditional flame functions
 *****************************************************************************/

void flame_general_slow() {
    general_offset();

    if( (CthughaBuffer::current->translate.current() == NULL) || 
	(((TranslateEntry*)CthughaBuffer::current->translate.current())->trans == NULL) )
	flame_general_slow_no_trans();
    else
	flame_general_slow_trans();
}

void flame_general_slow_no_trans() {
    int i;
    int tmp;
    unsigned char * ptr = active_buffer;
    int offset1, offset2, offset3, offset4;

    /* initialize offsets
     *
     *  ptr          -> destination (active_buffer)
     *  ptr + offset -> source (passive_buffer)
     */
    offset1 = flame_offset[0] + (passive_buffer - active_buffer);
    offset2 = flame_offset[1] + (passive_buffer - active_buffer);
    offset3 = flame_offset[2] + (passive_buffer - active_buffer);
    offset4 = flame_offset[3] + (passive_buffer - active_buffer);

    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =
	    (int)(* (ptr + offset1)) +
	    (int)(* (ptr + offset2)) +
	    (int)(* (ptr + offset3)) +
	    (int)(* (ptr + offset4));
	*ptr = divsub[tmp];
	ptr ++;
    }
}

void flame_general_slow_trans() {
    int i;
    int tmp;
    unsigned char * ptr = active_buffer;
    unsigned char * offset1, *offset2, *offset3, *offset4;
    int * trans = ((TranslateEntry*)CthughaBuffer::current->translate.current())->trans;
    CthughaBuffer::current->done_translate = 1;

    offset1 = passive_buffer + flame_offset[0];
    offset2 = passive_buffer + flame_offset[1];
    offset3 = passive_buffer + flame_offset[2];
    offset4 = passive_buffer + flame_offset[3];

    for(i=BUFF_SIZE; i != 0; i--) {
	tmp =
	    (int)(* (*trans + offset1)) +
	    (int)(* (*trans + offset2)) +
	    (int)(* (*trans + offset3)) +
	    (int)(* (*trans + offset4));
	* ptr = divsub[tmp];
	ptr ++; trans ++;
    }
}


/* by Deischi
   Let the buffer fall down
   */
void flame_down() {
    int i;
    unsigned char * src = passive_buffer - BUFF_WIDTH;
    unsigned char * dst = active_buffer;

    for(i=BUFF_HEIGHT; i != 0; i--) {
	memcpy(dst, src, BUFF_WIDTH);
	src += BUFF_WIDTH;
	dst += BUFF_WIDTH;
    }
}



