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

#ifndef PACKET_SOCKET_H
#define PACKET_SOCKET_H

#include <netpacket/packet.h>
#include <linux/ip.h>
#include <net/ethernet.h>

typedef struct pkt_ctx_s pkt_ctx_t;
struct pkt_ctx_s
{
	int sock;
	char * iface;
	int mtu_size;
	int protocol;
	struct sockaddr_ll output;
	/* layer 2*/
	struct ether_header ethhdr;
	/* layer 3*/
	struct iphdr iphdr;
	off_t iphdr_offset;
	/* layer 4 & + */
	off_t data_offset;
};

pkt_ctx_t * pkt_socket( char * iface, int protocol );

int pkt_send( pkt_ctx_t * sock, void * buffer, int length );

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length );

void pkt_close( pkt_ctx_t * sock );

#endif /* PACKET_SOCKET_H */
