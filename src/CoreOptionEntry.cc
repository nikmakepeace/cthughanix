#include "cthugha.h"
#include "CoreOption.h"

#include <ctype.h>

CoreOptionEntry::CoreOptionEntry(const char * n, const char * d, int inUse) :
    use("", inUse)
{
    if(n) {
	name = new char[strlen(n)+1];
	strcpy(name, n);
    } else
	name = "none";
    if(d) {
	desc = new char[strlen(d)+1];
	strcpy(desc, d);
    } else
	desc = "";
}

// compare up to first SPACE in other
int CoreOptionEntry::sameName(const char * other) {

    const char * n = name;
    while( (*other != '\0') && (*other != ' ') && (*n != '\0') ) {
	if( toupper(*other) != toupper(*n) )
	    return 0;
	other ++;
	n ++;
    }
    if( (*n == '\0') && ( (*other == '\0') || (*other == ' ')) )
	return 1;
    return 0;
} 


int OnEntry::sameName(const char * other) {
    
    if ( ! strncasecmp("yes", other, 3) )
	return 1;
    else if ( ! strncasecmp("on", other, 2) )
	return 1;
    else if ( ! strncasecmp("1", other, 1) )
	return 1;
    else
	return 0;
}

int OffEntry::sameName(const char * other) {

    if ( ! strncasecmp("no", other, 2) )
	return 1;
    else if ( ! strncasecmp("off", other, 3) )
	return 1;
    else if ( ! strncasecmp("0", other, 1) )
	return 1;
    else
	return 0;
}
