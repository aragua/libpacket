#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "ethernet.h"
#include "arp.h"
#include "ip.h"

static uint16_t ip_checksum( const void * data, size_t data_len )
{
    unsigned long sum = 0;
    const uint16_t *ip1;

    ip1 = data;
    while (data_len > 1)
    {
        sum += *ip1++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        data_len -= 2;
    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return(~sum);
}

pkt_ctx_t * ip_socket( char * iface, in_addr_t * ipaddr, int protocol )
{
    pkt_ctx_t * ret = NULL;

    ret = eth_socket( iface, ETH_P_IP );

    if ( ret && ipaddr )
    {
        ret->iphdr.version = 4;
        ret->iphdr.ihl = 5;
        ret->iphdr.tos = 16;
        ret->iphdr.tot_len = htons(20);
        ret->iphdr.id = 0;
        ret->iphdr.frag_off = 0;
        ret->iphdr.ttl = 64;
        ret->iphdr.protocol = protocol;
        ret->iphdr.check = 0;
        memcpy( &ret->iphdr.saddr, ipaddr, sizeof(in_addr_t) );
        memset( &ret->iphdr.daddr, 0, sizeof(in_addr_t) );
        ret->data_offset = ret->iphdr_offset + ret->iphdr.ihl * 4;
    }

    return ret;
}

int ip_sendto( pkt_ctx_t * sock, void * buffer, int length, in_addr_t ipaddr )
{
    int ret = 0;
    char * buf, *databuf;

    buf = malloc( sock->iphdr_offset + sizeof(sock->iphdr) + length );
    if ( !buf )
        return -1;

    /* copy ip data */
    databuf = buf + sock->data_offset;
    memcpy( databuf, buffer, length );

    ret = _ip_sendto( sock, buf, length, ipaddr );

    free(buf);

    return ret;
}

int _ip_sendto( pkt_ctx_t * sock, void * buffer, int length, in_addr_t ipaddr )
{
    int ret = 0;
    char *ipbuf;
    struct ether_addr eaddr;

    if ( arp( sock->iface, ipaddr, &eaddr ) < 0 )
    {
        perror("arp");
        return EXIT_FAILURE;
    }

    /* copy ip header */
    ipbuf = buffer + sock->iphdr_offset;
    memcpy( ipbuf, &sock->iphdr, sizeof(sock->iphdr) );
    memcpy( &((struct iphdr *)ipbuf)->daddr, &ipaddr, 4 );
    ((struct iphdr *)ipbuf)->tot_len += htons(length);
    ((struct iphdr *)ipbuf)->check += ip_checksum(ipbuf,((struct iphdr *)ipbuf)->ihl*4);

    ret = _eth_sendto( sock, buffer, sizeof(sock->iphdr) + length, eaddr );

    return ret;
}

int ip_recv( pkt_ctx_t * sock, void * buffer, int length )
{
    int ret = 0;
    void * buf;
    buf = malloc(sock->mtu_size);
    ret = eth_recv( sock, buf, length );
    if ( ret < sock->data_offset )
        return -1;
    memcpy( buf + sock->data_offset, buffer, ret - sock->data_offset );
    return ret - sock->data_offset;
}

void ip_close( pkt_ctx_t * sock )
{
    if ( sock )
    {
        eth_close(sock);
    }
}
