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


void dump_packet( uint8_t * buffer, int len )
{
	int idx;
	if ( !buffer || len <= 0 )
		printf("No buffer to treat\n");

	printf("Buffer size %d:\n", len );
	for ( idx = 0; idx < len ; idx++ )
	{
		if ( idx % 16 == 0 )
			printf("%08x", idx );
		printf(" %02x", buffer[idx]&0xff );
		if ( idx % 16 == 15 )
			printf(" \n");
	}
	printf("\n");
}

void analyze_packet( uint8_t * buffer, int len )
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
