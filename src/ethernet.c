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
        ret->iphdr_offset = sizeof(struct ether_header);
    }
    return ret;
}

int eth_sendto( pkt_ctx_t * sock,
                void * buffer,
                int length,
                struct ether_addr eaddr )
{
    int ret = 0;
    char * buf;

    if ( !buffer )
        return EXIT_FAILURE;

    buf = malloc( sizeof(struct ether_addr) + length );
    if ( !buf )
        return -1;

    memcpy( buf + sizeof(struct ether_addr), buffer, length );

    ret = _eth_sendto( sock, buf, length, eaddr );

    free(buf);

    return ret;
}

int _eth_sendto( pkt_ctx_t * sock,
                void * buffer,
                int length,
                struct ether_addr eaddr )
{
    int ret = 0;

    if ( !buffer )
        return EXIT_FAILURE;

    memcpy( buffer, &sock->ethhdr, sizeof(sock->ethhdr) );
    memcpy( ((struct ether_header *)buffer)->ether_dhost,
            eaddr.ether_addr_octet,
            ETH_ALEN );
    ret = pkt_send( sock, buffer, sizeof(sock->ethhdr) + length );
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
