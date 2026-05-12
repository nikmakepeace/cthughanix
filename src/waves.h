/* 
 *  sound-display (wave-functions)
 */
#ifndef __WAVES_H
#define __WAVES_H

int init_tables();
int init_wave();

typedef unsigned char pal_table[256];		/* Table for display_wave */
extern pal_table tables[];			/* Palette-Tables */
extern int nr_tables;				/* number of tables */

typedef int WObject[2][3];	// a 3D object is a list of lines, each line
				// is two 3-space points.
				// the list is terminated by the final line having
				// coordinates that are all -1

extern OptionOnOff use_objects;			/* use 3-D objects */


#endif
