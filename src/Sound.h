// -*-c++-*-
#ifndef __SOUND_H__
#define __SOUND_H__

#include "Option.h"

#include "SoundDevice.h"

/* 
 *  General stuff
 */
int init_sound();
int exit_sound();

int sound_communicate(int to_child = 1);

extern int sine[320];
int sound_read();
int sound_fork_process();
int sound_kill_process();

/* 
 *  sound-processing
 */
int massage_audio();


/*
 * things for the mixer
 */
int mixer_initial_volume(char * name, int volume);
int init_mixer();

extern char dev_mixer[];

#endif



