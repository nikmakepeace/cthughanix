// -*- c++ -*-

#ifndef __CDPLAYER_H
#define __CDPLAYER_H

#if WITH_CDROM == 1


#include "Option.h"

extern OptionInt cd_first_track;			/* start with track nr */
extern OptionOnOff cd_stop_on_exit;				
extern OptionOnOff cd_randomplay;
extern OptionOnOff cd_loop;
extern OptionOnOff cd_eject_on_end;

extern char dev_cd[];

class CDPlayer {
    int handle;
    
    int openCD();
    int readTOC();

    int playStart;	// track range playing now.
    int playStop;

    int * nextList;		// last+1 == stop

public:
    CDPlayer();
    ~CDPlayer();

    void operator()();

    void getInfo();

    enum {
	NoStatus,
	Playing,
	Paused,
	Completed,
	Stopped,
	Ejected
    } status;		// current status of CD
    int track;		// current track
    int relMin;
    int relSec;
    int relFrame;
    int absMin;
    int absSec;
    int absFrame;

    int first;		// first track number
    int last;		// last track number

    int eject();
    int play(int track);			/* start playing at track */
    int pause();				/* pause the CD */
    int stop();				/* stops playing the CD */
    int next(int skip);			/* skip by skip  tracks */
    int fast(int skip);
};

#else

// just a dummy class
class CDPlayer {
public:
    void operator()() {}
};

#endif

extern CDPlayer * cdPlayer;

#endif

