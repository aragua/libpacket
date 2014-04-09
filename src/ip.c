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
    }

  return ret;
}

int ip_sendto( pkt_ctx_t * sock, void * buffer, int length, in_addr_t ipaddr )
{
  int ret = 0;
  char * buf;
  struct ether_addr eaddr;
  
  if ( arp( sock->iface, ipaddr, &eaddr ) < 0 )
    {
      perror("arp");
      return EXIT_FAILURE;
    }

  /* Todo : manage a pool of buffer in order to avoid malloc for each packet */
  buf = malloc( sizeof(sock->iphdr) + length );
  if ( !buf )
    return -1;
  memcpy( buf, &sock->iphdr, sizeof(sock->iphdr) );
  memcpy( &((struct iphdr *)buf)->daddr, &ipaddr, 4 );
  ((struct iphdr *)buf)->tot_len += htons(length);
  memcpy( buf + sizeof(sock->iphdr), buffer, length );
  ((struct iphdr *)buf)->check += ip_checksum(buf,((struct iphdr *)buf)->ihl*4);
  ret = eth_sendto( sock, buf, sizeof(sock->iphdr) + length, eaddr );
  free(buf);
  return ret;
}

int ip_recv( pkt_ctx_t * sock, void * buffer, int length )
{
  return eth_recv( sock, buffer, length );
}

void ip_close( pkt_ctx_t * sock )
{
  if ( sock )
    {
      eth_close(sock);
    }
}
