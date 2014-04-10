/* Raw packet socket implementation
 *
 * Fabien Lahoudere <fabienlahoudere.pro@gmail.com>
 */

#ifndef PACKET_SOCKET_H
#define PACKET_SOCKET_H

#include <netpacket/packet.h>
#include <linux/ip.h>

typedef struct pkt_ctx_s pkt_ctx_t;
struct pkt_ctx_s
{
    int sock;
    char * iface;
    int protocol;
    struct sockaddr_ll output;
    /* layer 2*/
    struct ether_header ethhdr;
    size_t ethhdr_len;
    /* layer 3*/
    struct iphdr iphdr;
    size_t iphdr_len;
};

pkt_ctx_t * pkt_socket( char * iface, int protocol );

int pkt_send( pkt_ctx_t * sock, void * buffer, int length );

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length );

void pkt_close( pkt_ctx_t * sock );

#endif /* PACKET_SOCKET_H */
