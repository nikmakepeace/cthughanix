#include "cthugha.h"
#include "interface.h"
#include "CthughaBuffer.h"
#include "flames.h"

/* 
 * some functions called and variables used
 */

int init_flames(void) { return 0; }
int init_translate(void) { return 0; }
int init_wave(void) { return 0; }

int BUFF_WIDTH = 320;
int BUFF_HEIGHT = 200;
xy screenSizes[] = {
};
int nScreenSizes = 0;
xy bufferSizes[] = {
};
int nBufferSizes = 0;

// stubs from AutoChanger.h
OptionOnOff trans_stretch("",0);
OptionOnOff transLoadOnDemand("",0);
OptionOnOff transLoadLate("", 0);

int show_pcx() { return 0; }

const char * OptionGeneralFlame::text() const { return ""; }

CoreOptionEntryList l;
CoreOptionEntryList pcxEntries;
CoreOptionEntryList generalFlameEntries;

CoreOption screen(-1, "", l);

TranslateOption::TranslateOption(int buffer, const char * name) : CoreOption(buffer, name,l) {}
int TranslateOption::operator()() { return 0; }
const char * TranslateOption::status() { return ""; }

CthughaBuffer::CthughaBuffer() :
    palChanged(1),
    
    flame       (99, "", l),
    palette     (99, "", l),
    pcx         (99, ""),
    translate   (99, ""),
    wave        (99, "", l),
    object      (99, "", l),
    flameGeneral(99),
    waveScale   (99, "", l),
    table       (99, "", l),
    border      (99, "", l),
    soundProcess(99, "", l),
    flashlight  (99, "", l),
    lastPalette(-1)
{}
int CthughaBuffer::nBuffers = 0;
CthughaBuffer * CthughaBuffer::current = NULL;

CthughaBuffer CthughaBuffer::buffers[1];
OptionInt CthughaBuffer::nCurrent("", 0, 1);

void PaletteEntry::Random() {}
void PaletteEntry::addRandom() {}
int PaletteEntry::lastRandomPos = -1;		// index of the last random palette


double fps = 0;

int display_use_pcx = 0;
OptionOnOff display_sync("",0);


Interface interfaceHelp("help", 
			"Cthugha Help",
			"F1,?    : Help                         \n"
			"F4      : CD Player                    \n"
			"F5      : Sound control panel          \n"
			"F6      : Mixer                        \n"
			"F7      : Sound client list            \n"
			"Ctrl+c  : Exit                         \n");

