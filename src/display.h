/*
    CTHUGHA-L							display.h
	
    Funktions to access the Screen.
	- general stuff
	- screen-functions
	- palette-functions
	- text-functions
*/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__


#include "CoreOption.h"

/* 
 *  initialize and shut down ncurses
 */
int init_ncurses();						
void exit_ncurses();

extern int ncurses_use;




extern unsigned long bitmap_colors0[256];	/* "compiled" palette */
extern unsigned long bitmap_colors1[256];	
extern unsigned long bitmap_colors2[256];	
extern unsigned long bitmap_colors3[256];	
extern int rev_byte_order;

/* 
 *  Stuff about palettes
 */
typedef unsigned char Palette[256][3];		/* one Palette: 256 entries, each 3 bytes */

extern CoreOptionEntryList paletteEntries;

class PaletteEntry : public CoreOptionEntry {
    void random();				// randomize this palette
public:
    Palette pal;

    PaletteEntry(const char * name, const char * desc) : CoreOptionEntry(name, desc) {}
    PaletteEntry(FILE * file, const char * name);

    static char randomName[PATH_MAX];
    static int lastRandom;
    static int lastRandomPos;

    static void Random();			// re-randomize the last random palette
    static void addRandom();			// add a new random palette
 
    friend CoreOptionEntry * read_palette(FILE *, const char *, const char *);
};

int load_palettes();				/* initializiation */
int init_palettes();
int exit_palettes();
int update_palette();
extern int display_internal_pal;		/* disable internal pal. */
extern int display_external_pal;		/*  disable external pal. */
void cth_setpalette(Palette pal, int immed);


/* 
 *  Stuff for PCX
 */
int init_pcx();					/* initalize pcx */
int show_pcx();
extern int display_use_pcx;			/* allow pcx-usage */
extern int * pcx_palettes;			/* index to corresp. palette */

extern char display_prt_file[];			/* filename used by PrtScrn */
char * prtFileName(const char * ext);

int save_pcx(unsigned char * v, int w, int h, Palette pal);




#endif


