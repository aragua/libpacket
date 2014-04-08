#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>

#include "packet.h"

int main ( int argc, char **argv )
{
  int sock = 0;
  
  sock = pkt_socket( "enp10s0", ETH_P_IP );
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

  return EXIT_SUCCESS;
}
