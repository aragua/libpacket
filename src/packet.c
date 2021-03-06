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

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "packet.h"

pkt_ctx_t * pkt_socket( char * iface, int protocol )
{
	pkt_ctx_t * ret_sock;
	struct ifreq ifr;

	if ( !iface )
		return NULL;

	ret_sock = malloc(sizeof(pkt_ctx_t ));
	if ( !ret_sock )
		return NULL;

	memset( ret_sock, 0, sizeof(pkt_ctx_t ) );
	ret_sock->iface = strdup(iface);
	ret_sock->protocol = protocol;

	ret_sock->sock = socket(AF_PACKET, SOCK_RAW, htons(protocol) );
	if ( ret_sock->sock < 0)
	{
		perror("socket");
		return NULL;
	}

	/* Get interface index */
	memset(&ifr, 0, sizeof(ifr));
	strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	if ( ioctl( ret_sock->sock, SIOCGIFINDEX, &ifr) == -1 )
	{
		printf("No such interface: %s\n", iface );
		close(ret_sock->sock);
		return NULL;
	}

	memset( &ret_sock->output, 0x00, sizeof(struct sockaddr_ll));
	ret_sock->output.sll_family = AF_PACKET;
	ret_sock->output.sll_ifindex   = ifr.ifr_ifindex;
	ret_sock->output.sll_protocol  = htons(protocol);

	bind( ret_sock->sock, (struct sockaddr*)&ret_sock->output, sizeof(ret_sock->output));

	/* Is the interface up? */
	ioctl( ret_sock->sock, SIOCGIFFLAGS, &ifr);
	if ( (ifr.ifr_flags & IFF_UP) == 0)
	{
		printf("Interface %s is down\n", iface );
		close(ret_sock->sock);
		return NULL;
	}

	/* Get mtu size in order to alloc buffers */
	memset(&ifr, 0, sizeof(ifr));
	strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
	if ( ioctl( ret_sock->sock, SIOCGIFMTU, &ifr) == -1 )
	{
		printf("No such interface: %s\n", iface );
		close(ret_sock->sock);
		return NULL;
	}
	ret_sock->mtu_size = ifr.ifr_mtu;

	return ret_sock;
}

int pkt_send( pkt_ctx_t * sock, void * buffer, int length  )
{
	return sendto( sock->sock,
				   buffer,
				   length,
				   0,
				   (struct sockaddr *)&sock->output,
				   sizeof(sock->output));
}

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length )
{
	return recv( sock->sock, buffer, length, 0 );
}

void pkt_close( pkt_ctx_t * sock )
{
	if ( sock )
	{
		close(sock->sock);
		free(sock->iface);
		free(sock);
	}
}
