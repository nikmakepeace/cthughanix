#include "cthugha.h"
#include "network.h"

#if WITH_NETWORK == 1

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

/* 
 * create an PF_INET socket 
 */
int make_socket(int type, short port) {
    int sock;
    struct sockaddr_in my_s_addr;

    /* create socket for request */
    if( (sock = socket(PF_INET, type, 0)) < 0) {
	printfee("Can not create socket.");
	return -1;
    }

    /* bind socket */
    my_s_addr.sin_family = AF_INET;
    my_s_addr.sin_port = htons(port);
    my_s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if( bind(sock, (struct sockaddr*)&my_s_addr, sizeof(my_s_addr)) < 0) {
	printfee("Can not bind socket");
	close(sock);
	return -1;
    }

    return sock;
}


#endif

