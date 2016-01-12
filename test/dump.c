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
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
#include <sys/time.h>

#include "packet.h"
#include "analyze.h"

int main ( int argc, char **argv )
{
	pkt_ctx_t * sock = 0;

	if ( argc <= 1 )
	{
		printf("Usage:\n\t%s <iface>\n", argv[0]);
		return EXIT_FAILURE;
	}

	sock = pkt_socket( argv[1], ETH_P_ALL );
	if ( !sock )
	{
		perror("pkt_socket");
		return EXIT_FAILURE;
	}

	while (1)
	{
		int ret = 0;
		uint8_t buffer[1500];

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

	}

	pkt_close( sock );

	return EXIT_SUCCESS;
}
