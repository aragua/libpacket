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
