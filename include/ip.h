/*

The MIT License (MIT)

Copyright (c) 2015 Fabien Lahoudere

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

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

void show_ip( uint8_t * buffer, int len );

#endif /* _IP_H_ */
