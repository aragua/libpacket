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
