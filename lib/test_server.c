#include <stdlib.h>
#include <stdio.h>

#include "server.h"

void convert_ip_address (ip_addr_t* ipaddr, char* str_ipaddr, int ipver);

static void dump_ip_addr (ip_addr_t* ip_addr);

int
main (int argc, char** argv)
{
    char* str_ipv4 = "192.168.1.126";
    char* str_ipv6 = "123:4567:8:9:0:ab:c0:dfff";


    ip_addr_t ipv4, ipv6;

    printf ("IPv4, the string: %s \n", str_ipv4);
    convert_ip_address (&ipv4, str_ipv4, 4);
    dump_ip_addr (&ipv4);

    printf ("IPv6, the string: %s \n", str_ipv6);
    convert_ip_address (&ipv6, str_ipv6, 6);
    dump_ip_addr (&ipv6);

};

static void 
dump_ip_addr (ip_addr_t* ip_addr)
{
    printf ("ip_ver: %d \n", ip_addr->ip_ver);
    printf ("o1: %x \n", ip_addr->o1);
    printf ("o2: %x \n", ip_addr->o2);
    printf ("o3: %x \n", ip_addr->o3);
    printf ("o4: %x \n", ip_addr->o4);
};
