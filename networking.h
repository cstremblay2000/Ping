/**
 * File:        networking.c
 * Author:      Chris Tremblay <cst1465@rit.edu>
 * Date:        10/06/2021, National Noodle Day!
 * Description:
 *      A libary of networking services
 */

#ifndef NETWORKING_H_
#define NETWORKING_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

// ICMP ECHO REQ code
static const int ICMP_ECHOREQ = 8;

// ICMP ECHO RESP code
static const int ICMP_ECHOREP = 0;

// ip header
struct IP_HDR{
        u_char ver;
        u_char tos;
        short len;
        short id;
        short flags;
        u_char ttl;
        u_char protocol;
        u_short chksum;
        struct in_addr src_addr;
        struct in_addr dst_addr;
};

// the typedef for an icmp header
struct ICMP_HDR {
        u_char type;
        u_char code;
        u_short chksum;
        u_char id;
        u_char seq;
};

// the struct for an echo request
struct ECHO_REQ{
        struct icmphdr hdr;
        int time;
        //char data[];
};

struct ECHO_REP{
        struct IP_HDR ip_hdr;
        struct ECHO_REQ echo_req;
        char data[56];
};

// Description:
//      Create a server Datagram Socket
// Returns:
//      the socket fd if successful, -1 if not
int create_socket( void );

// Description:
//      Create a sockaddr_in from an ip and port
//      Must be free'd
// Parameters:
//      ip   -> the ipv4 addr fromatted as X.X.X.X
//      port -> the port
// Returns:
//      The sockaddr_in
struct sockaddr_in *create_sockaddr( const char* ip, const int port );

// Description:
//      Generate and internet checksum
// Parameters:
//      hdr -> the icmp header
//      the size of the header and data
// Returns:
//      The checksum
u_short make_cksum( u_short *hdr, int data_size );

// Description:
//      Get a host name from an IP
// Parameters:
//      iphdr -> the iphdr from a host
// Returns:
//      the hostname
char *get_host( struct IP_HDR *iphdr );

#endif
