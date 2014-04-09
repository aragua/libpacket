#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"

int main ( int argc, char **argv )
{
  pkt_ctx_t * sock = 0;

  if ( argc <= 1 )
    {
      printf("Usage:\n\t%s <iface> [ip]\n", argv[0]);
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
    sock = eth_socket( argv[1], ETH_P_ALL );
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

  /* test ARP */
  if ( argc > 2 )
  {
    struct ether_addr eaddr;

    printf("Testing arp ... ");

    if ( arp( argv[1], inet_addr(argv[2]), &eaddr ) < 0 )
      {
	perror("arp");
	return EXIT_FAILURE;
      }

    printf("OK : %s is at %02x:%02x:%02x:%02x:%02x:%02x\n",
	   argv[2],
	   eaddr.ether_addr_octet[0],
	   eaddr.ether_addr_octet[1],
	   eaddr.ether_addr_octet[2],
	   eaddr.ether_addr_octet[3],
	   eaddr.ether_addr_octet[4],
	   eaddr.ether_addr_octet[5]);
  }

  /* test IP */
  if ( argc > 3 )
  {
    in_addr_t destaddr;

    printf("Testing ip packet ... ");
    destaddr = inet_addr(argv[3]);
    sock = ip_socket( argv[1], &destaddr, 0 );
    if ( sock < 0 )
      {
	perror("eth_socket");
	return EXIT_FAILURE;
      }
    if ( ip_sendto( sock, "It works!!!", 12, inet_addr(argv[2]) ) < 0 )
      {
	perror("ip_sendto");
	return EXIT_FAILURE;
      }
    
    ip_close( sock );
    printf("OK\n");
  }
  
  return EXIT_SUCCESS;
}
