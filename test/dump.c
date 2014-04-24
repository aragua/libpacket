#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"

static void analyze_packet( char * buffer, int len )
{
    if ( !buffer )
        printf("No buffer to treat\n");

    printf("Mac dest: %02x:%02x:%02x:%02x:%02x:%02x\n",
           buffer[0]&0xf,buffer[1]&0xf,buffer[2]&0xf,buffer[3]&0xf,buffer[4]&0xf,buffer[5]&0xf);
    printf("Mac dest: %02x:%02x:%02x:%02x:%02x:%02x\n",
           buffer[6]&0xf,buffer[7]&0xf,buffer[8]&0xf,buffer[9]&0xf,buffer[10]&0xf,buffer[11]&0xf);
    printf("Mac type: %02x%02x\n", buffer[12],buffer[13]);
}

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
        char buffer[1500];

        ret = pkt_recv( sock, buffer, 1500 );
        if ( ret < 0 )
        {
            perror("pkt_recv");
            pkt_close( sock );
            return EXIT_FAILURE;
        }
        analyze_packet( buffer, ret);
    }

    pkt_close( sock );

    return EXIT_SUCCESS;
}
