#ifndef __TAB_HEADER_H
#define __TAB_HEADER_H

/* 
 * This is the header of a translate table
 */

typedef struct {				/* header for tab-files */
    long id;					/* "HDKB" to identify file */
    char description[40];			/* asciiz */
    short size_x;			
    short size_y;
} tab_header;

#endif
