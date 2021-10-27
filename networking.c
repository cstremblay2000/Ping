/**
 * File:        networking.c
 * Author:      Chris Tremblay <cst1465@rit.edu>
 * Date:        10/06/2021, National Noodle Day!
 * Description:
 *      A libary of networking services
 */

#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#include "networking.h"
#include "err.h"
#include "set_uid.h"

// Description:
//      Create a sockaddr_in from an ip and port
//      Must be free'd
// Parameters:
//      ip   -> the ipv4 addr fromatted as X.X.X.X
//      port -> the port
// Returns:
//      The sockaddr_in
struct sockaddr_in *create_sockaddr( const char* ip, const int port ){
        // initial sockaddr_in
        struct sockaddr_in *sa_in;
        sa_in = (struct sockaddr_in*)malloc( sizeof( struct sockaddr_in ) );
        memset( sa_in, 0, sizeof(struct sockaddr_in ) );
        sa_in->sin_family = AF_INET;
        sa_in->sin_port = htons( port );
        sa_in->sin_addr.s_addr = inet_addr( ip );
        return (sa_in);
}

// Description:
//      Create a server Datagram Socket
// Returns:
//      the socket fd if successful, -1 if not
int create_socket( void ){
        // create socket
        int sockfd;
        if( (sockfd = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP )) < 0 ){
                        print_error( "Error creating socket" );
                        return( -1 );
        }

        // reuse socket address
        int enable = 1;
        if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR,
                                &enable, sizeof(int)) < 0 ){
                print_error( "Error setting SO_REUSEADDR option" );
                return( -1 );
       }

        // reuse socket port
        if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEPORT,
                                &enable, sizeof(int)) < 0 ){
                print_error( "Error setting SO_REUSEPORT option" );
                return ( -1 );
        }

        return (sockfd);
}

// Description:
//      Generate and internet checksum
// Parameters:
//      hdr -> the icmp header
//      the size of the header and data
// Returns:
//      The checksum 
u_short make_cksum(u_short *hdr, int size ){
        u_short *addr = (u_short*)hdr;
        register u_short cksum = 0;
        int sum = 0;
        while( size > 1 ){
                sum += *addr++;
                size -= 2;
        }
        if( size )
                sum += *(u_char*)addr;
        sum = (sum>>16) + (sum&0xffff);
        sum += (sum >>16);
        cksum = ~sum;
        return (cksum);
}

// Description:
//      Get a host name from an IP
// Parameters:
//      iphdr -> the iphdr from a host
// Returns:
//      the hostname
char *get_host( struct IP_HDR *iphdr ){
        struct hostent *h;
        struct in_addr ai = iphdr->src_addr;
        h = gethostbyaddr(&ai, sizeof(ai), AF_INET);
        static char buffer[150];
        if( h == NULL || h->h_name == NULL )
                return inet_ntoa( iphdr->src_addr );
        strcpy( buffer, h->h_name );
        return buffer;
}
