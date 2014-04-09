#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "ethernet.h"
#include "packet.h"

static struct ether_header ehdr;

int eth_socket( char * iface, int protocol )
{
  int ret = 0;

  ret = pkt_socket( iface, protocol );
  if ( ret >= 0 )
    {
      struct ifreq ifr;
      memset(&ifr, 0x00, sizeof(ifr));
      strcpy( ifr.ifr_name, iface );
      if ( ioctl( ret, SIOCGIFHWADDR, &ifr ) >= 0 )
	memcpy( ehdr.ether_shost, ifr.ifr_hwaddr.sa_data, ETH_ALEN );
      ehdr.ether_type = htons(protocol);
    }
  return ret;
}

int eth_sendto( int sock, void * buffer, int length, struct ether_addr eaddr )
{
  int ret = 0;
  char * buf;

  /* Todo : manage a pool of buffer in order to avoid malloc for each packet */
  buf = malloc( sizeof(ehdr) + length );
  if ( !buf )
    return -1;
  memcpy( buf, &ehdr, sizeof(ehdr) );
  memcpy( ((struct ether_header *)buf)->ether_dhost, eaddr.ether_addr_octet, ETH_ALEN );
  memcpy( buf + sizeof(ehdr), buffer, length );
  ret = pkt_send( sock, buf, sizeof(ehdr) + length );
  free(buf);
  return ret;
}

int eth_recv( int sock, void * buffer, int length )
{
  return pkt_recv( sock, buffer, length );
}

void eth_close( int sock )
{
  close(sock);
}
