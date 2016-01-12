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

#ifndef _ARP_H
#define _ARP_H

#include <stdint.h>
#include <netinet/in.h>
#include <net/ethernet.h>

typedef struct arp_s arp_t;

struct arp_s{
	uint16_t hw_type;                //hardware address type
	uint16_t prot_type;                //protocol adress type
	uint8_t  hw_addr_len;        //hardware address length
	uint8_t  prot_addr_len;        //Protokoll adress length
	uint16_t opcode;                        //Operation
	uint8_t  src_mac[ETH_ALEN];            //source MAC (Ethernet Address)
	uint8_t  src_ip[4];                    //source IP
	uint8_t  dest_mac[ETH_ALEN];            //destination MAC (Ethernet Address)
	uint8_t  dest_ip[4];                    //destination IP
	int8_t   fill[18];                                //Padding, ARP-Requests are quite small (<64)
} __attribute__((packed));

int arp( char * iface, in_addr_t ip, struct ether_addr * eaddr );

void show_arp( uint8_t * buffer, int len );

#endif /*_ARP_H */
