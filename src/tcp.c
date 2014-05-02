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
