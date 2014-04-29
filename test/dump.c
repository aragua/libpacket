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
#include "ethernet.h"
#include "arp.h"
#include "ip.h"

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

static void show_arp( char * buffer, int len )
{
	struct in_addr tmp;
	arp_t * hdr;

	if ( !buffer || len < 8 )
		return;

	hdr = (arp_t *)buffer;
	switch ( ntohs(hdr->opcode) )
	{
	case 0x1:
		memcpy( &tmp.s_addr, hdr->dest_ip, 4);
		printf("ARP request - MAC %s ask for %s\n", MAC_to_str(hdr->src_mac), inet_ntoa(tmp) );
		break;
	case 0x2:
		memcpy( &tmp.s_addr, hdr->src_ip, 4);
		printf("ARP reply - IP %s is at %s\n", inet_ntoa(tmp), MAC_to_str(hdr->src_mac) );
		break;
	default :
		printf("Bad ARP packet\n");
	}
}

static void show_ip( char * buffer, int len )
{
    struct in_addr tmp;
    struct iphdr * hdr;
    struct protoent * proto;

	if ( len < 20 )
        return;

    hdr = (struct iphdr *) buffer;

    tmp.s_addr = hdr->daddr;
    printf("IP dest: %s\n", inet_ntoa(tmp) );

    tmp.s_addr = hdr->saddr;
    printf("IP src: %s\n", inet_ntoa(tmp) );

    proto = getprotobynumber(hdr->protocol);
    printf("IP protocol: %s\n", proto->p_name );
}

static void analyze_packet( char * buffer, int len )
{
    struct ether_header * hdr;

    if ( !buffer )
        printf("No buffer to treat\n");

    hdr = (struct ether_header *)buffer;

    if ( len >= ETH_HLEN )
    {
        printf("Mac dest: %02x:%02x:%02x:%02x:%02x:%02x\n",
               buffer[0]&0xff,buffer[1]&0xff,buffer[2]&0xff,buffer[3]&0xff,buffer[4]&0xff,buffer[5]&0xff);
		printf("Mac src : %02x:%02x:%02x:%02x:%02x:%02x\n",
               buffer[6]&0xff,buffer[7]&0xff,buffer[8]&0xff,buffer[9]&0xff,buffer[10]&0xff,buffer[11]&0xff);
        switch( ntohs(hdr->ether_type) )
        {
        case ETH_P_IP:
            show_ip( buffer + ETH_HLEN, len - ETH_HLEN );
            break;
        case ETH_P_ARP:
            show_arp( buffer + ETH_HLEN, len - ETH_HLEN );
            break;
		case ETH_P_ALL:
		{
			static struct timeval last_tv = { 0, 0 };
			struct timeval cur_tv, diff_tv;
			gettimeofday( &cur_tv, NULL );
			if ( last_tv.tv_sec != 0 )
			{
				int carry = 0;
				if ( last_tv.tv_usec > cur_tv.tv_usec )
				{
					diff_tv.tv_usec = 1000000 + cur_tv.tv_usec - last_tv.tv_usec;
					carry = 1;
				}
				else
					diff_tv.tv_usec = cur_tv.tv_usec - last_tv.tv_usec;

				diff_tv.tv_sec = cur_tv.tv_sec - last_tv.tv_sec - carry;

				printf("%lu%06lu\n", diff_tv.tv_sec, diff_tv.tv_usec );
			}
			last_tv = cur_tv;
			break;
		}
		default:
			printf("Mac type: %04x\n", ntohs(hdr->ether_type) );
            break;
        }
        printf("\n");
    }
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
