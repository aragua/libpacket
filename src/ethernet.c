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

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "ethernet.h"
#include "packet.h"

pkt_ctx_t * eth_socket( char * iface, int protocol )
{
	pkt_ctx_t * ret;

	ret = pkt_socket( iface, protocol );
	if ( ret )
	{
		struct ifreq ifr;
		memset(&ifr, 0x00, sizeof(ifr));
		strcpy( ifr.ifr_name, ret->iface );
		if ( ioctl( ret->sock, SIOCGIFHWADDR, &ifr ) >= 0 )
			memcpy( ret->ethhdr.ether_shost, ifr.ifr_hwaddr.sa_data, ETH_ALEN );
		ret->ethhdr.ether_type = htons(protocol);
		ret->iphdr_offset = sizeof(struct ether_header);
	}
	return ret;
}

int eth_sendto( pkt_ctx_t * sock,
				void * buffer,
				int length,
				struct ether_addr eaddr )
{
	int ret = 0;
	char * buf;

	if ( !buffer )
		return EXIT_FAILURE;

	buf = malloc( sizeof(struct ether_addr) + length );
	if ( !buf )
		return -1;

	memcpy( buf + sizeof(struct ether_addr), buffer, length );

	ret = _eth_sendto( sock, buf, length, eaddr );

	free(buf);

	return ret;
}

int _eth_sendto( pkt_ctx_t * sock,
				 void * buffer,
				 int length,
				 struct ether_addr eaddr )
{
	int ret = 0;

	if ( !buffer )
		return EXIT_FAILURE;

	memcpy( buffer, &sock->ethhdr, sizeof(sock->ethhdr) );
	memcpy( ((struct ether_header *)buffer)->ether_dhost,
			eaddr.ether_addr_octet,
			ETH_ALEN );
	ret = pkt_send( sock, buffer, sizeof(sock->ethhdr) + length );
	return ret;
}

int eth_recv( pkt_ctx_t * sock, void * buffer, int length )
{
	return pkt_recv( sock, buffer, length );
}

void eth_close( pkt_ctx_t * sock )
{
	if ( sock )
	{
		pkt_close(sock);
	}
}


char * MAC_to_str( const uint8_t * addr )
{
	static char return_string[32];

	if ( !addr )
		return NULL;

	snprintf( return_string,
			  32,
			  "%02x:%02x:%02x:%02x:%02x:%02x",
			  addr[0]&0xff, addr[1]&0xff, addr[2]&0xff,
			addr[3]&0xff, addr[4]&0xff, addr[5]&0xff
			);
	return return_string;
}
