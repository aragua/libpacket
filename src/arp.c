#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>

#include "arp.h"
#include "ethernet.h"

int arp( char * iface, in_addr_t ip, struct ether_addr * eaddr )
{
  struct ether_addr ehdr;
  struct ifreq ifr;
  arp_t payload;
  int idx, sock;
  void * reply;

  sock = eth_socket( iface, ETH_P_ARP );
  if ( sock < 0 )
    {
      perror("eth_socket");
      return EXIT_FAILURE;
    }
  
  /* Fill arp packet payload */
  payload.hw_type = htons(0x1);
  payload.prot_type = htons(ETH_P_IP);
  payload.hw_addr_len = ETH_ALEN;
  payload.prot_addr_len = 4;
  payload.opcode = htons(0x0001);
  memset(&ifr, 0x00, sizeof(ifr));
  strcpy( ifr.ifr_name, iface );
  if ( ioctl( sock, SIOCGIFHWADDR, &ifr ) >= 0 )
    memcpy( payload.src_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN );
  if ( ioctl( sock, SIOCGIFADDR, &ifr ) >= 0 )
    memcpy( payload.src_ip, &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr, 4 );
  memcpy( payload.dest_ip, &ip, 4 );

  /* Arp packet must be broadcasted */
  for ( idx = 0 ; idx < ETH_ALEN ; idx++ )
    ehdr.ether_addr_octet[idx] = 0xff;

  if ( eth_sendto( sock, &payload, sizeof(arp_t), ehdr ) < 0 )
    {
      perror("eth_send");
      eth_close( sock );
      return EXIT_FAILURE;
    }
  
  reply = malloc(60);
  if ( !reply )
    {
      perror("eth_recv");
      eth_close( sock );
      return EXIT_FAILURE;
    }
    
  if ( eth_recv( sock, reply, 60 ) < 0 )
    {
      perror("eth_recv");
      free(reply);
      eth_close( sock );
      return EXIT_FAILURE;
    }
  
  /* todo check that it is really an arp reply */ 
  memcpy( eaddr->ether_addr_octet, reply + sizeof(struct ether_header) + 8 /*hwaddr offset*/, ETH_ALEN );

  free(reply);
  eth_close( sock );
  
  return 0;
}
