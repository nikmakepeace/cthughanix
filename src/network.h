#ifndef __NETWORK_H
#define __NETWORK_H


int make_socket(int type, short port);

extern int SRV_PORT;
extern int CLT_PORT;
#define CLT_PORT2	(CLT_PORT+1)
extern int REQ_PORT;




#endif
