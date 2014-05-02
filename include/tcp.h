#ifndef _TCP_H
#define _TCP_H

#include <netinet/tcp.h>

void show_tcp_header( const void * buffer, size_t len );

#endif
