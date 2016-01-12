/*

The MIT License (MIT)

Copyright (c) 2015 Fabien Lahoudere

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "tcp.h"

#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>

void show_tcp_header( const void * buffer, size_t len )
{
	struct tcphdr * hdr;

	if ( !buffer || len < sizeof(struct tcphdr) )
		return;

	hdr = (struct tcphdr *)buffer;

	printf("Tcp :\n"
		   "\tSrc port: %u, dest port: %u\n"
		   "\tSeq num: %08x, ack num: %08x\n"
		   "\thdr size:%d, flags: %s%s%s%s%s\n"
		   "\tWindow: %u, sum: %04x\n",
		   ntohs(hdr->th_sport),
		   ntohs(hdr->th_dport),
		   ntohl(hdr->th_seq),
		   ntohl(hdr->th_ack),
		   hdr->th_off * 4,
		   (hdr->th_flags & TH_FIN) != 0 ? "FIN,":"",
		   (hdr->th_flags & TH_SYN) != 0 ? "SYN,":"",
		   (hdr->th_flags & TH_RST) != 0 ? "RST,":"",
		   (hdr->th_flags & TH_PUSH) != 0 ? "PSH,":"",
		   (hdr->th_flags & TH_ACK) != 0 ? "ACK":"",
		   ntohs(hdr->th_win),
		   hdr->th_sum );
}
