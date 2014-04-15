#ifndef _IP_H_
#define _IP_H_

#include <netinet/in.h>

#include "packet.h"

pkt_ctx_t * ip_socket( char * iface, in_addr_t * ipaddr, int protocol );
int ip_sendto( pkt_ctx_t * sock, void * buffer, int length, in_addr_t ipaddr );
int _ip_sendto( pkt_ctx_t * sock, void * buffer, int length, in_addr_t ipaddr );
int ip_recv( pkt_ctx_t * sock, void * buffer, int length );
int ip_recvfrom( pkt_ctx_t * sock, void * buffer, int length, in_addr_t * addr );
void ip_close( pkt_ctx_t * sock );

#endif /* _IP_H_ */
