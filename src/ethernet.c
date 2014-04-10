#include <stdlib.h>
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
    }
    return ret;
}

int eth_sendto( pkt_ctx_t * sock, void * buffer, int length, struct ether_addr eaddr )
{
    char * buf;
    int ret = 0;

    /* Todo : manage a pool of buffer in order to avoid malloc for each packet */
    buf = malloc( sizeof(struct ether_header) + length );
    if ( !buf )
        return -1;
    memcpy( buf, &sock->ethhdr, sizeof(sock->ethhdr) );
    memcpy( ((struct ether_header *)buf)->ether_dhost, eaddr.ether_addr_octet, ETH_ALEN );
    memcpy( buf + sizeof(sock->ethhdr), buffer, length );
    ret = pkt_send( sock, buf, sizeof(sock->ethhdr) + length );
    free(buf);
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
