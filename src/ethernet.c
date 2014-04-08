#include "ethernet.h"

struct ether_header ehdr;

int eth_socket( char * iface, int protocol )
{
  int ret = 0;

  ret = pkt_socket( iface, protocol );
  if ( ret >= 0 )
    {
      struct ifreq ifr;
      memset(&buffer, 0x00, sizeof(ifr));
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

  /* Todo : manage a pool of buffer of size MTU in order to avoid malloc for each packet */
  buf = malloc( sizeof(ehdr) + length );
  if ( !buf )
    return -1;
  memcpy( buf, ehdr, sizeof(ehdr) );
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
