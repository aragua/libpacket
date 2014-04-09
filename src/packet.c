#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h> 
#include <netpacket/packet.h> 
#include <net/ethernet.h>
#include <net/if.h> 
#include <arpa/inet.h>

static struct sockaddr_ll output;

int pkt_socket( char * iface, int protocol )
{
  int ret_sock;
  struct ifreq ifr;
  
  if ( !iface )
    return EXIT_FAILURE;
  
  ret_sock = socket(AF_PACKET, SOCK_RAW, htons(protocol) );
  if ( ret_sock < 0)
    {
      perror("socket");
      return EXIT_FAILURE;
    }
  
  /* Get interface index */
  memset(&ifr, 0, sizeof(ifr));
  strncpy (ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
  ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
  
  if ( ioctl( ret_sock, SIOCGIFINDEX, &ifr) == -1 )
    {
      printf("No such interface: %s\n", iface );
      close(ret_sock);
      return EXIT_FAILURE;
    }

  memset( &output, 0x00, sizeof(struct sockaddr_ll));
  output.sll_family = AF_PACKET;
  output.sll_ifindex   = ifr.ifr_ifindex;
  output.sll_protocol  = htons(protocol);
  
  /* Is the interface up? */
  ioctl( ret_sock, SIOCGIFFLAGS, &ifr);
  if ( (ifr.ifr_flags & IFF_UP) == 0)
    {
      printf("Interface %s is down\n", iface );
      close(ret_sock);
      return EXIT_FAILURE;
    }
  
  return ret_sock;
}

int pkt_send( int sock, void * buffer, int length  )
{
  return sendto( sock, buffer, length, 0, (struct sockaddr *)&output, sizeof(output));
}

int pkt_recv( int sock, void * buffer, int length )
{
  return recv( sock, buffer, length, 0 );
}

void pkt_close( int sock )
{
  close(sock);
}
