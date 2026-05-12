#include "cthugha.h"
#include "sound.h"


SoundDeviceRandom::SoundDeviceRandom() : SoundDevice() {
    soundFormat.setValue(0);
    soundChannels.setValue(2);

    tmpSize = 512 ;		// this always creates 512 bytes per request
}

int SoundDeviceRandom::read() {
    int x;
    static int v1=0, v2=0, maxdv=2; 

    char2 * sound_data = (char2*)tmpData;

    /* generate random sound */
    sound_data[0][0]=144; 
    sound_data[0][1]=112; 
    for (x=1; x < 256; x++) { 

	if (rand()%256 > sound_data[x-1][0]) 
	    v1+=rand()%maxdv; 
	else 
	    v1-=rand()%maxdv; 

	if (rand()%256 > sound_data[x-1][1]) 
	    v2+=rand()%maxdv; 
	else 
	    v2-=rand()%maxdv; 

	sound_data[x][0]=sound_data[x-1][0]+v1; 
	sound_data[x][1]=sound_data[x-1][1]+v2; 
    } 

    return 256;
}

void SoundDeviceRandom::update() {
    
    // alwazs use 8bit stereo
    soundFormat.setValue(SF_u8);
    soundChannels.setValue(2);
}

