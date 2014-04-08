/* Raw packet socket implementation
 *
 * Fabien Lahoudere <fabienlahoudere.pro@gmail.com>
 */

#ifndef PACKET_SOCKET_H
#define PACKET_SOCKET_H

int pkt_socket( char * iface, int protocol );

int pkt_send( int sock, void * buffer, int length );

int pkt_recv( int sock, void * buffer, int length );

void pkt_close( int sock );

#endif /* PACKET_SOCKET_H */
