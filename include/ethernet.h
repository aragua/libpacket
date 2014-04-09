#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include <netinet/ether.h>

int eth_socket( char * iface, int protocol );
int eth_sendto( int sock, void * buffer, int length, struct ether_addr eaddr );
int eth_recv( int sock, void * buffer, int length );
void eth_close( int sock );

#endif /* _ETHERNET_H_ */
