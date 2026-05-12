#include "cthugha.h"
#include "xcthugha.h"
#include "options.h"

#include <ctype.h>

#include <X11/Xresource.h>

static XrmDatabase database;


int open_ini_sys() {
    XrmInitialize();
    if( (database = XrmGetDatabase(xcth_display)) == NULL)
	return 1;
    return 0;
}



int get_ini_str_sys(const char * name, char * value) {
    char * str_type;
    XrmValue Entry;
    char class_[512];
	
    strcpy(class_, name);	class_[0] = 'C';
    
    if( XrmGetResource(database, name, class_, &str_type, &Entry) ) {
	while( isspace(*Entry.addr) )
	    Entry.addr ++;
	if ( *Entry.addr == '\0')
	    return 1;
	strcpy(value, Entry.addr);
	return 0;
    }
    return 1;
}



#define CTH_XWIN
#include "options.cc"
