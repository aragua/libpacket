#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include <netinet/ether.h>

#include "packet.h"

pkt_ctx_t * eth_socket( char * iface, int protocol );
int eth_sendto( pkt_ctx_t * sock, void * buffer, int length, struct ether_addr eaddr );
int eth_recv( pkt_ctx_t * sock, void * buffer, int length );
void eth_close( pkt_ctx_t * sock );

#endif /* _ETHERNET_H_ */
