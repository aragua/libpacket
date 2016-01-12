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
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
#include "analyze.h"

int main ( int argc, char **argv )
{
	pkt_ctx_t * sock = 0;

	if ( argc <= 1 )
	{
		printf("Usage:\n\t%s <iface> [ip]\n", argv[0]);
		return EXIT_FAILURE;
	}


	/* test packet.c */
	{
		printf("Testing raw packet ... ");
		sock = pkt_socket( argv[1], ETH_P_IP );
		if ( !sock )
		{
			perror("pkt_socket");
			return EXIT_FAILURE;
		}

		if ( pkt_send( sock, "It works!!!", 12 ) < 0 )
		{
			perror("pkt_send");
			return EXIT_FAILURE;
		}

		pkt_close( sock );
		printf("OK\n");
	}


	/* test ethernet.c */
	{
		struct ether_addr ehdr;
		int idx;

		printf("Testing ethernet packet ... ");
		sock = eth_socket( argv[1], ETH_P_ALL );
		if ( !sock )
		{
			perror("eth_socket");
			return EXIT_FAILURE;
		}

		for ( idx = 0 ; idx < ETH_ALEN ; idx++ )
			ehdr.ether_addr_octet[idx] = 0xff;

		if ( eth_sendto( sock, "It works!!!", 12, ehdr ) < 0 )
		{
			perror("eth_send");
			return EXIT_FAILURE;
		}

		eth_close( sock );
		printf("OK\n");
	}

	/* test arp.c */
	if ( argc > 2 )
	{
		struct ether_addr eaddr;

		printf("Testing arp ... ");

		if ( arp( argv[1], inet_addr(argv[2]), &eaddr ) < 0 )
		{
			perror("arp");
			return EXIT_FAILURE;
		}

		printf("OK : %s is at %02x:%02x:%02x:%02x:%02x:%02x\n",
			   argv[2],
				eaddr.ether_addr_octet[0],
				eaddr.ether_addr_octet[1],
				eaddr.ether_addr_octet[2],
				eaddr.ether_addr_octet[3],
				eaddr.ether_addr_octet[4],
				eaddr.ether_addr_octet[5]);
	}

	/* test ip.c */
	if ( argc > 3 )
	{
		in_addr_t destaddr;

		printf("Testing ip packet ... ");
		destaddr = inet_addr(argv[3]);
		sock = ip_socket( argv[1], &destaddr, 0xfc /* SMP */ );
		if ( !sock )
		{
			perror("eth_socket");
			return EXIT_FAILURE;
		}
		if ( ip_sendto( sock, "It works!!!", 12, inet_addr(argv[2]) ) < 0 )
		{
			perror("ip_sendto");
			return EXIT_FAILURE;
		}

		ip_close( sock );
		printf("OK\n");
	}

	/* test tcp connection */
	{
		int ret = 0 ;
		uint8_t buffer[1500], data[] = { 0x08, 0x00, 0x27, 0x18, 0xbc, 0x13,
										 0x08, 0x00, 0x27, 0xf6, 0x52, 0xa4,
										 0x08, 0x00, 0x45, 0x00, 0x00, 0x3c,
										 0x97, 0x85, 0x40, 0x00, 0x40, 0x06,
										 0x8f, 0x34, 0x0a, 0x00, 0x00, 0x01,
										 0x0a, 0x00, 0x00, 0x02, 0x91, 0x40,
										 0x00, 0x16, 0xe9, 0x69, 0x08, 0xf8,
										 0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
										 0x72, 0x10, 0x9d, 0xab, 0x00, 0x00,
										 0x02, 0x04, 0x05, 0xb4, 0x04, 0x02,
										 0x08, 0x0a, 0x01, 0x0d, 0x9f, 0x7d,
										 0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
										 0x03, 0x06 };

		printf("Testing tcp packet ... ");

		sock = pkt_socket( argv[1], ETH_P_IP );
		if ( !sock )
		{
			perror("pkt_socket");
			return EXIT_FAILURE;
		}

		if ( pkt_send( sock, data, 74 ) < 0 )
		{
			perror("pkt_send");
			return EXIT_FAILURE;
		}

		ret = pkt_recv( sock, buffer, 1500 );
		if ( ret < 0 )
		{
			perror("pkt_recv");
			pkt_close( sock );
			return EXIT_FAILURE;
		}
		printf("############# Start ############\n");
		dump_packet(buffer,ret);
		analyze_packet( buffer, ret);
		printf("#############  End  #############\n");

		pkt_close( sock );

		printf("OK\n");
	}

	return EXIT_SUCCESS;
}
