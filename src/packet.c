#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#include "packet.h"

static int get_iface_info( pkt_ctx_t * sock, const char * iface )
{
	if ( !sock || !iface )
		return EXIT_FAILURE;

	if ( strncmp( iface, "netmap:", strlen("netmap:")) == 0 )
	{
#if HAVE_NETMAP
		sock->mode = NETMAP_MODE;
		sock->iface = strdup(iface+7);
		switch (sock->iface[strlen(sock->iface)-1])
		{
			case '-':
				sock->netmap_flags = NR_REG_ONE_NIC;
				break;
			case '*':
				sock->netmap_flags = NR_REG_NIC_SW;
				break;
			case '^':
				sock->netmap_flags = NR_REG_SW;
				break;
			default:
				sock->netmap_flags = NR_REG_ALL_NIC;
				break;
		}
		if ( sock->netmap_flags != NR_REG_ALL_NIC )
			sock->iface[strlen(sock->iface)-1] = 0;
#else
		fprintf( stderr, "Netmap mode not supported\n" );
		return EXIT_FAILURE;
#endif
	}
	else
	{
		sock->mode = AFPACKET_MODE;
		sock->iface = strdup(iface);
	}

	return EXIT_SUCCESS;
}

pkt_ctx_t * pkt_socket( char * iface, int protocol )
{
	pkt_ctx_t * ret_sock;
	struct ifreq ifr;

	if ( !iface )
		return NULL;

	ret_sock = malloc(sizeof(pkt_ctx_t));
	if ( !ret_sock )
		return NULL;

	memset( ret_sock, 0, sizeof(pkt_ctx_t) );
	if ( get_iface_info( ret_sock, iface) == EXIT_FAILURE )
	{
		pkt_close(ret_sock);
		return NULL;
	}
	ret_sock->protocol = protocol;

	switch ( ret_sock->mode )
	{
		case AFPACKET_MODE:
		{
			/* Get interface index */
			ret_sock->sock = socket(AF_PACKET, SOCK_RAW, htons(protocol) );
			if ( ret_sock->sock < 0)
			{
				perror("socket");
				pkt_close(ret_sock);
				return NULL;
			}
			memset(&ifr, 0, sizeof(ifr));
			strncpy (ifr.ifr_name, ret_sock->iface, sizeof(ifr.ifr_name) - 1);
			ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

			if ( ioctl( ret_sock->sock, SIOCGIFINDEX, &ifr) == -1 )
			{
				printf("No such interface: %s\n", iface );
				pkt_close(ret_sock);
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
				pkt_close(ret_sock);
				return NULL;
			}

			/* Get mtu size in order to alloc buffers */
			memset(&ifr, 0, sizeof(ifr));
			strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
			ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
			if ( ioctl( ret_sock->sock, SIOCGIFMTU, &ifr) == -1 )
			{
				printf("No such interface: %s\n", iface );
				pkt_close(ret_sock);
				return NULL;
			}
			ret_sock->mtu_size = ifr.ifr_mtu;
			break;
		}
		case NETMAP_MODE:
		{
#if HAVE_NETMAP
			ret_sock->desc = nm_open( iface, NULL, 0, NULL);
			if ( !ret_sock->desc )
			{
				fprintf( stderr, "nm_open failed\n");
				pkt_close(ret_sock);
				return NULL;
			}
			ret_sock->mtu_size = 1500;
			break;
#else
			fprintf( stderr, "Netmap mode not supported\n" );
			pkt_close(ret_sock);
			return NULL;
#endif
		}
		default:
			fprintf( stderr, "Unknown mode\n" );
			pkt_close(ret_sock);
			return NULL;
	}

	return ret_sock;
}

/* send a packet */
int pkt_send( pkt_ctx_t * sock, void * buffer, int length  )
{
	if ( sock->mode == AFPACKET_MODE )
	{
		return sendto( sock->sock,
					   buffer,
					   length,
					   0,
					   (struct sockaddr *)&sock->output,
					   sizeof(sock->output));
	}
	else
	{
#if HAVE_NETMAP
		return nm_inject( sock->desc, buffer, length );
#else
		fprintf( stderr, "Netmap mode not supported\n" );
		return -1;
#endif
	}
}

/*receive a packet */

#if HAVE_NETMAP
static int
nm_recv( struct nm_desc *d, void *buf, size_t size)
{
	struct pollfd pollfd;
	int ret;

	pollfd.fd = d->fd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;
	ret = poll(&pollfd, 1, -1);
	if ( ret > 0 )
	{
		if (pollfd.revents & POLLIN)
		{
			//move(pa, burst);
			int si = d->first_rx_ring;
			while ( si <= d->last_rx_ring )
			{
				struct netmap_ring * rxring = NETMAP_RXRING(d->nifp, si);
				struct netmap_slot * slot;

				if ( nm_ring_empty(rxring))
				{
					si++;
				}
				else
				{
					int space = 0;
					int idx;
					char * rxbuf;

					idx = rxring->cur;

					space = nm_ring_space(rxring);
					if ( space < 1 )
					{
						fprintf( stderr, "Warning : no slot in ring but poll say there is data\n");
						return 0;
					}
					slot = &rxring->slot[idx];
					if ( slot->buf_idx < 2)
					{
						fprintf( stderr, "wrong index rx[%d] = %d\n", idx, slot->buf_idx);
						sleep(2);
					}

					if ( slot->len > 2048 || slot->len > size )
					{
						fprintf( stderr, "wrong len %d rx[%d]\n", slot->len, idx );
						slot->len = 0;
						return 0;
					}

					rxbuf = NETMAP_BUF( rxring, slot->buf_idx );
					memcpy( buf, rxbuf, slot->len );

					idx = nm_ring_next( rxring, idx );

					rxring->head = rxring->cur = idx;
					ret = slot->len;
				}
			}
		}
		else
			return 0;
	}
	else if ( ret == 0 )
	{
		fprintf( stderr, "nm_recv timeout\n");
		return -1;
	}
	else
		perror("poll");

	return ret;
}
#endif

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length )
{
	if ( sock->mode == AFPACKET_MODE )
	{
		return recv( sock->sock, buffer, length, 0 );
	}
	else
	{
#if HAVE_NETMAP
		return nm_recv( sock->desc, buffer, length );
#else
		fprintf( stderr, "Netmap mode not supported\n" );
		return -1;
#endif
	}
}

/* clean and close */
void pkt_close( pkt_ctx_t * sock )
{
	if ( sock )
	{
		close(sock->sock);
		if ( sock->iface )
			free(sock->iface);
		free(sock);
	}
}
