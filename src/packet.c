#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "packet.h"


/****************************** Buffer management *****************************/
#define DFLT_RX_STACK_ENTRIES 1024
#define DFLT_TX_STACK_ENTRIES 1024

static int dflt_rx_stack_entries = DFLT_RX_STACK_ENTRIES;
static int dflt_tx_stack_entries = DFLT_TX_STACK_ENTRIES;

static void free_buffers( pkt_ctx_t * sock );

int get_dflt_rx_stack_entries()
{
    return dflt_rx_stack_entries;
}

void set_dflt_rx_stack_entries( int rx )
{
    if ( rx > 0 )
        dflt_rx_stack_entries = rx;
}

int get_dflt_tx_stack_entries()
{
    return dflt_tx_stack_entries;
}

void set_dflt_tx_stack_entries( int tx )
{
    if ( tx > 0 )
        dflt_tx_stack_entries = tx;
}

static int alloc_buffers( pkt_ctx_t * sock, int rx, int tx )
{
    if ( sock )
    {
        int idx;

        sock->rx_buf_nbr = rx;
        sock->rx_stack = malloc( sock->rx_buf_nbr * sizeof(pkt_buf_pool_t) );
        if ( !sock->rx_stack )
            goto error;
        for ( idx = 0; idx < rx ; idx++ )
        {
            sock->rx_stack[idx].free = 1;
            sock->rx_stack[idx].buf = malloc(sock->mtu_size);
            if ( !sock->rx_stack[idx].buf )
                goto error;
        }

        sock->tx_buf_nbr = rx;
        sock->tx_stack = malloc( sock->rx_buf_nbr * sizeof(pkt_buf_pool_t) );
        if ( !sock->tx_stack )
            goto error;
        for ( idx = 0; idx < rx ; idx++ )
        {
            sock->tx_stack[idx].free = 1;
            sock->tx_stack[idx].buf = malloc(sock->mtu_size);
            if ( !sock->tx_stack[idx].buf )
                goto error;
        }
    }

    return EXIT_SUCCESS;
error:
    free_buffers(sock);
    return EXIT_FAILURE;
}

void * get_rx_buffer( pkt_ctx_t * sock )
{
    if ( sock && sock->rx_stack )
    {
        int idx;
        for ( idx = 0; idx < sock->rx_buf_nbr ; idx++ )
        {
            if ( sock->rx_stack[idx].free != 0 )
            {
                sock->rx_stack[idx].free = 0;
                return sock->rx_stack[idx].buf;
            }
        }
    }
    return NULL;
}

int free_rx_buffer( pkt_ctx_t * sock, void * buffer )
{
    if ( sock && sock->rx_stack && buffer )
    {
        int idx;
        for ( idx = 0; idx < sock->rx_buf_nbr ; idx++ )
        {
            if ( sock->rx_stack[idx].buf == buffer )
            {
                sock->rx_stack[idx].free = 1;
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_FAILURE;
}

void * get_tx_buffer( pkt_ctx_t * sock )
{
    if ( sock && sock->tx_stack )
    {
        int idx;
        for ( idx = 0; idx < sock->tx_buf_nbr ; idx++ )
        {
            if ( sock->tx_stack[idx].free != 0 )
            {
                sock->tx_stack[idx].free = 0;
                return sock->tx_stack[idx].buf;
            }
        }
    }
    return NULL;
}

int free_tx_buffer( pkt_ctx_t * sock, void * buffer )
{
    if ( sock && sock->tx_stack && buffer )
    {
        int idx;
        for ( idx = 0; idx < sock->tx_buf_nbr ; idx++ )
        {
            if ( sock->tx_stack[idx].buf == buffer )
            {
                sock->tx_stack[idx].free = 1;
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_FAILURE;
}

static void free_buffers( pkt_ctx_t * sock )
{
    if ( sock )
    {
        int idx;
        if ( sock->rx_stack )
        {
            for ( idx = 0; idx < sock->rx_buf_nbr ; idx++ )
            {
                if ( sock->rx_stack[idx].buf )
                    free( sock->rx_stack[idx].buf );
            }
            free(sock->rx_stack);
            sock->rx_stack = NULL;
        }
        if ( sock->tx_stack )
        {
            for ( idx = 0; idx < sock->tx_buf_nbr ; idx++ )
            {
                if ( sock->tx_stack[idx].buf )
                    free( sock->tx_stack[idx].buf );
            }
            free(sock->tx_stack);
            sock->tx_stack = NULL;
        }
    }
}
/************************** End of buffer management **************************/

pkt_ctx_t * pkt_socket( char * iface, int protocol )
{
    pkt_ctx_t * ret_sock;
    struct ifreq ifr;

    if ( !iface )
        return NULL;

    ret_sock = malloc(sizeof(pkt_ctx_t ));
    if ( !ret_sock )
        return NULL;

    memset( ret_sock, 0, sizeof(pkt_ctx_t ) );
    ret_sock->iface = strdup(iface);
    ret_sock->protocol = protocol;
    
    ret_sock->sock = socket(AF_PACKET, SOCK_RAW, htons(protocol) );
    if ( ret_sock->sock < 0)
    {
        perror("socket");
        return NULL;
    }

    /* Get interface index */
    memset(&ifr, 0, sizeof(ifr));
    strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

    if ( ioctl( ret_sock->sock, SIOCGIFINDEX, &ifr) == -1 )
    {
        printf("No such interface: %s\n", iface );
        close(ret_sock->sock);
        return NULL;
    }

    memset( &ret_sock->output, 0x00, sizeof(struct sockaddr_ll));
    ret_sock->output.sll_family = AF_PACKET;
    ret_sock->output.sll_ifindex   = ifr.ifr_ifindex;
    ret_sock->output.sll_protocol  = htons(protocol);

    /* Is the interface up? */
    ioctl( ret_sock->sock, SIOCGIFFLAGS, &ifr);
    if ( (ifr.ifr_flags & IFF_UP) == 0)
    {
        printf("Interface %s is down\n", iface );
        close(ret_sock->sock);
        return NULL;
    }

    /* Get mtu size in order to alloc buffers */
    memset(&ifr, 0, sizeof(ifr));
    strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
    if ( ioctl( ret_sock->sock, SIOCGIFMTU, &ifr) == -1 )
    {
        printf("No such interface: %s\n", iface );
        close(ret_sock->sock);
        return NULL;
    }
    ret_sock->mtu_size = ifr.ifr_mtu;

    if ( 0 )
    {
        /* allocate rx and tx buffers */
        if ( alloc_buffers( ret_sock,
                            dflt_rx_stack_entries,
                            dflt_tx_stack_entries) != EXIT_SUCCESS )
        {
            printf("Fail to alloc buffers\n" );
            pkt_close(ret_sock);
            return NULL;
        }
    }

    return ret_sock;
}

int pkt_send( pkt_ctx_t * sock, void * buffer, int length  )
{
    return sendto( sock->sock,
                  buffer,
                  length,
                  0,
                  (struct sockaddr *)&sock->output,
                  sizeof(sock->output));
}

int pkt_recv( pkt_ctx_t * sock, void * buffer, int length )
{
    return recv( sock->sock, buffer, length, 0 );
}

void pkt_close( pkt_ctx_t * sock )
{
    if ( sock )
    {
        close(sock->sock);
        free_buffers(sock);
        free(sock->iface);
        free(sock);
    }
}
