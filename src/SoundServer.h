// -*- c++ -*-

#ifndef __SOUND_SERVER_H
#define __SOUND_SERVER_H

#include "Option.h"

#if WITH_NETWORK == 1

#include <sys/types.h>
#include <sys/socket.h>

#define MAX_CLIENTS	255

extern OptionTime srv_wait_time;		// waiting time
extern OptionOnOff server;			// enable sound server

class SoundServer {
    int nClients;
    struct sockaddr clientAddrs[MAX_CLIENTS];
    int clientSizes[MAX_CLIENTS];
    int bcast_socket;
    int request_socket;
    int add_client(struct sockaddr my_s_addr, int size);
    int remove_client(struct sockaddr my_s_addr, int size);
public:
    SoundServer();
    ~SoundServer();

    void operator()();

    friend class InterfaceServer;
    friend int serv_sound_read();
};

#else

class SoundServer {
    int nClients;
public:
    SoundServer() : nClients(0) {}
    void operator()() {}

    friend class InterfaceServer;
    friend int serv_sound_read();
};

#endif

extern SoundServer * soundServer;

#endif

