#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "arp.h"
#include "ethernet.h"
#include "packet.h"

int arp( char * iface, in_addr_t ip, struct ether_addr * eaddr )
{
	struct ifreq ifr;
	pkt_ctx_t * sock;
	arp_t * payload;
	int idx;
	void * buffer;

	sock = eth_socket( iface, ETH_P_ARP );
	if ( !sock )
	{
		perror("eth_socket");
		return EXIT_FAILURE;
	}

	buffer = malloc( sizeof( struct ether_header ) + sizeof( arp_t));
	if ( !buffer )
	{
		perror("malloc");
		eth_close( sock );
		return EXIT_FAILURE;
	}

	payload = buffer + sizeof( struct ether_header );

	/* Fill arp packet payload */
	payload->hw_type = htons(0x1);
	payload->prot_type = htons(ETH_P_IP);
	payload->hw_addr_len = ETH_ALEN;
	payload->prot_addr_len = 4;
	payload->opcode = htons(0x0001);
	memcpy( payload->src_mac, sock->ethhdr.ether_shost, ETH_ALEN );
	memset(&ifr, 0x00, sizeof(ifr));
	strcpy( ifr.ifr_name, iface );
	if ( ioctl( sock->sock, SIOCGIFADDR, &ifr ) >= 0 )
		memcpy( payload->src_ip, &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr, 4 );
	memcpy( payload->dest_ip, &ip, 4 );

	/* Arp packet must be broadcasted */
	for ( idx = 0 ; idx < ETH_ALEN ; idx++ )
		eaddr->ether_addr_octet[idx] = 0xff;

	if ( _eth_sendto( sock, buffer, sizeof(arp_t), *eaddr ) < 0 )
	{
		perror("eth_send");
		free(buffer);
		eth_close( sock );
		return EXIT_FAILURE;
	}

	memset ( buffer, 0, sizeof( struct ether_addr ) + sizeof( arp_t) );

	if ( eth_recv( sock, buffer, 60 ) < 0 )
	{
		perror("eth_recv");
		free(buffer);
		eth_close( sock );
		return EXIT_FAILURE;
	}

	/* todo check that it is really an arp reply */
	memcpy( eaddr->ether_addr_octet, buffer + sizeof(struct ether_header) + 8 /*hwaddr offset*/, ETH_ALEN );

	free(buffer);
	eth_close( sock );

	return 0;
}

void show_arp( uint8_t * buffer, int len )
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
