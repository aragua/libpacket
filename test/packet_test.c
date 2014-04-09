#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>

#include "packet.h"
#include "ethernet.h"

int main ( int argc, char **argv )
{
  int sock = 0;

  if ( argc <= 1 )
    {
      printf("Usage:\n\t%s <iface>\n", argv[0]);
      return EXIT_FAILURE;
    }


  /* test packet.c */
  {
    printf("Testing raw packet ... ");
    sock = pkt_socket( argv[1], ETH_P_IP );
    if ( sock < 0 )
      {
	perror("pkt_socket");
	return EXIT_FAILURE;
      }
    
    if ( pkt_send( sock, "It works!!!", 12 ) < 0 )
      {
	perror("pkt_send");
	return EXIT_FAILURE;
      }
    
    pkt_close( sock );
    printf("OK\n");
  }


  /* test ethernet.c */
  {
    struct ether_addr ehdr;
    int idx;

    printf("Testing ethernet packet ... ");
    sock = eth_socket( argv[1], ETH_P_IP );
    if ( sock < 0 )
      {
	perror("eth_socket");
	return EXIT_FAILURE;
      }

    for ( idx = 0 ; idx < ETH_ALEN ; idx++ )
      ehdr.ether_addr_octet[idx] = 0xff;
    
    if ( eth_sendto( sock, "It works!!!", 12, ehdr ) < 0 )
      {
	perror("eth_send");
	return EXIT_FAILURE;
      }
    
    eth_close( sock );
    printf("OK\n");
  }

  return EXIT_SUCCESS;
}
