#include "cthugha.h"
#include "CoreOption.h"
#include "SoundAnalyze.h"
#include "display.h"
#include "CthughaBuffer.h"
#include "imath.h"

class FlashlightEntry : public OnEntry {
public:
    int operator()() {
	int i,j,l;
	static Palette Pal;
	
	PaletteEntry * palE = (PaletteEntry*)CthughaBuffer::current->palette.current();
	if(palE == NULL)
	    return 1;
	
	// make a copy of the current palette
	memcpy(Pal, palE->pal, sizeof(Palette));
	
	for(l=soundAnalyze.fire<<3,i=0; (i < 256) && (l > 0); i++, l -= 8)
	    for(j=0; j < 3; j++)
		Pal[i][j] = min( Pal[i][j] + l, 255);
	
	CthughaBuffer::current->setPalette(Pal);

	return 0;
    }
};

CoreOptionEntry * flashlight_entries[] = {
    new OffEntry(),
    new FlashlightEntry()
};



