// Sound subsystem entry points shared by the main program and sound backends.
// AudioRuntime owns backend selection; this header exposes lifecycle hooks plus
// the older mixer bridge still used by OSS capture paths.

#ifndef __SOUND_H__
#define __SOUND_H__

#include "Option.h"

#include "SoundDevice.h"

int init_sound();
int exit_sound();

// Parent/child control channel used by SoundDeviceFork.
int sound_communicate(int to_child = 1);

int mixer_initial_volume(char* name, int volume);
int init_mixer();

extern char dev_mixer[];
extern char pulse_server[];
extern int pulse_latency_msec;
extern char audio_output_dump[];
const char* pulse_server_name();
const char* pulse_server_display_name();

#endif
