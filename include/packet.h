/* Raw packet socket implementation
 *
 * Fabien Lahoudere <fabienlahoudere.pro@gmail.com>
 */

#ifndef PACKET_SOCKET_H
#define PACKET_SOCKET_H

#include <netpacket/packet.h>
#include <linux/ip.h>

typedef struct pkt_buf_pool_s pkt_buf_pool_t;
struct pkt_buf_pool_s
{
    int free;
    void * buf;
};

typedef struct pkt_ctx_s pkt_ctx_t;
struct pkt_ctx_s
{
    int sock;
    char * iface;
    int mtu_size;
    int protocol;
    struct sockaddr_ll output;
    /* layer 2*/
    struct ether_header ethhdr;
    /* layer 3*/
    struct iphdr iphdr;
    off_t iphdr_offset;
    /* layer 4 & + */
    off_t data_offset;
    /* data buffers */
    int rx_buf_nbr;
    pkt_buf_pool_t * rx_stack;

    int tx_buf_nbr;
    pkt_buf_pool_t * tx_stack;
};

int get_dflt_rx_stack_entries();
void set_dflt_rx_stack_entries( int rx );
int get_dflt_tx_stack_entries();
void set_dflt_tx_stack_entries( int tx );

void * get_rx_buffer( pkt_ctx_t * sock );
int free_rx_buffer( pkt_ctx_t * sock, void * buffer );

void * get_tx_buffer( pkt_ctx_t * sock );
int free_tx_buffer( pkt_ctx_t * sock, void * buffer );

pkt_ctx_t * pkt_socket( char * iface, int protocol );

int pkt_send( pkt_ctx_t * sock, void * buffer, int length );

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length );

void pkt_close( pkt_ctx_t * sock );

#endif /* PACKET_SOCKET_H */
