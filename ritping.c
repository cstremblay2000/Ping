/**
 * File:        ritping.c
 * Author:      Chris Tremblay <cst1465@rit.edu>
 * Date:        10/06/2021, National Noodle Day!
 * Description:
 *      An implementation of ping
 **/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <limits.h>

#include "networking.h"
#include "set_uid.h"
#include "err.h"

// The usage message
const char USAGE_MSG[] = 
"Usage: ./ritping [-c count] [-i wait] [-s size] host\n";

// the options for getopt
const char OPTIONS[] = "c:i:s:";

// usage flags msg
const char *USG_OPTS[] = { "Flag", "-c", "-i", "-s" };

// descriptions of flags
const char *USG_DESC[] = {"Descripton", "how many packets to send", 
        "how many second between packets", 
        "how many bytes packet is, 0<= s <= MAX_INT" };

// default values
const char *USG_DEF[] = {"Defaults", "infinte", "1s", "56" };

// init message
const char INIT_MSG[] = "PING %s (%s) %d(%d) bytes of data\n";

// const char echo resp message
const char REPLY_MSG[] = 
        "%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%0.2f ms\n";

// timeout message
const char TIMEOUT_MSG[] = 
        "From %s icmp_seq=%d Destination Host Unreachable\n";

// size variable
int SIZE = 56;

// default wait
int WAIT = 1;

// default count -1 for inf
int COUNT = -1;

// the socket 
int SOCKFD = -1;

// default ping timeout = 4s
int TIMEOUT = 4000;

// ping statisics 
int num_sent, num_recv = 0;

// SO THE HEADE CAN BE FREED
struct icmphdr *hdr;

// the thing the signal handler uses to break the while loop
int looping = 1;

// Description:
//      Signal handler to exit cleanly
// Parameters:
//      sig -> the signal number
void sig_handler( int sig ){
        if( sig == SIGINT || sig == SIGTERM )
                looping = 0;
        
}

// Descriptoin:
//      Print the usage message
void usage( void ){
        printf( "%s\n\n", USAGE_MSG );
        for( int i = 0; i < 4; i++ )
                printf( "\t%-10s %-10s %-50s\n", USG_OPTS[i], USG_DEF[i], 
                                USG_DESC[i] );
        return;
}

// Description:
//      Print and error nicely
// Parameters:
//      msg -> the message associated with error
void print_error( char *msg ){
        fprintf( stderr, "[error] %d <%s> : %s\n", errno, strerror( errno ), 
                        msg);
        return;
}

// Description:
//      Process the commandline args if applicable
void process_commandline( int argc, char **argv ){
        char opt;
        long t;
        while((opt = getopt( argc, argv, OPTIONS )) != -1 ){
                switch( opt ){
                        case 'c':
                                t = strtol( optarg, NULL, 10 );
                                if( t == LONG_MAX || t == LONG_MIN ){
                                        print_error("-c parameter");
                                        exit( EXIT_FAILURE );
                                }

                                if( t <= 0 ){
                                        print_error( "count must be >0" );
                                        exit( EXIT_FAILURE );
                                }

                                COUNT = (int)t;
                                break;
                        
                        case 'i':
                                t = strtol( optarg, NULL, 10 );
                                if( t == LONG_MAX || t == LONG_MIN ){
                                        print_error( "-i parameter" );
                                        exit( EXIT_FAILURE );
                                }
                                if( t <= 0 ){
                                        print_error( "wait must be >0" );
                                        exit( EXIT_FAILURE );
                                }
                                WAIT = (int)t;
                                break;
                        
                        case 's':
                                t = strtol( optarg, NULL, 10 );
                                if( t == LONG_MAX || t == LONG_MIN ){
                                        print_error("-s parameter" );
                                        exit( EXIT_FAILURE );
                                }
                                if( SIZE < 0 ){
                                        print_error( "size >0" );
                                        exit( EXIT_FAILURE );
                                }
                                SIZE = (int)t;
                                if( SIZE < 0 ){
                                        usage();
                                        exit( EXIT_FAILURE );
                                }
                                break;

                        case ':':
                                print_error( "missing parameter" );
                                usage();
                                exit( EXIT_FAILURE );
                                break;

                        case '?':
                                print_error( "check flags" );
                                exit( EXIT_FAILURE );

                        default:
                                print_error( "default error" );
                                exit( EXIT_FAILURE );
                }
        }
}

// Description:
//      Create and echo request
// Returns:
//      pointer to header, null on failure
struct ICMP_HDR *create_icmp_header( ){
        struct ICMP_HDR *ih = (struct ICMP_HDR*)malloc( sizeof(struct
                                ICMP_HDR));
        // make sure malloc worked
        if( ih == NULL ){
                print_error( "could malloc icmp hdr" );
                sig_handler( SIGINT );
        }
        
        // init fields
        memset( ih, 0, sizeof( struct ICMP_HDR ) );
        ih->type = ICMP_ECHOREQ;
        return (ih);
}

// Description:
//      Resolve an ip from a hostname
// Parameters:
//      host_name -> the host name to resolve
// Returns:
//      The ip_addr if possible, NULL if not
char *get_ip( char *host_name ){
        struct hostent *he;
        struct in_addr **addr_list;
        int i;

        if( (he=gethostbyname( host_name )) == NULL ){
                print_error( "error resolving ip" );
                return NULL;
        }

        addr_list = (struct in_addr**) he->h_addr_list;
        
        char buffer[3*4+4];
        for( i=0; addr_list[i] != NULL; i++ ){
                strcpy( buffer, inet_ntoa( *addr_list[i]) );
                char *ip = (char *)malloc( (strlen(buffer)+1)*sizeof(char));
                if( ip == NULL )
                        return NULL;
                strcpy( ip, buffer );
                return ip;
        }
        return NULL;
}

// Description:
//      The driver program
// Parameters:
//      argc -> number of argument
//      argv -> the arguments
// Note:
//      See usage message for details
int main( int argc, char **argv ){
        // drop privileges immediately
        uid_t ruid = getuid();
        drop_priv_temp( ruid );
        
        // check we at least have host name
        if( argc < 2 ){
                usage();
                exit( EXIT_FAILURE );
        }
        
        // extract commandline args
        process_commandline( argc, argv );

        // setup signal handlers
        signal( SIGINT, sig_handler );
        signal( SIGTERM, sig_handler );

        // create socket, using setuid
        restore_priv();
        SOCKFD = create_socket();
        drop_priv_temp( ruid );
        
        if( SOCKFD < 0 ){
                print_error( "error creating socket" );
                exit( EXIT_FAILURE );
        }

        // create destination address
        char *host_name = argv[argc-1];
        char *host_ip;
        if( inet_addr( host_name ) == INADDR_NONE ){
                host_ip = get_ip( host_name );
                if( host_ip == NULL ){
                        print_error( "host name resolve error" );
                        exit( EXIT_FAILURE );
                }
        } else {
                host_ip = host_name;
        }
        struct sockaddr_in *sa_in = create_sockaddr( host_ip, 0 ); 

        // initialize request
        hdr = (struct icmphdr*)malloc( SIZE + 64);
        if( hdr == NULL ){
                print_error( "error allocation icmpheadr" );
                exit( EXIT_FAILURE );
        }

        // set up icmp header
        memset( hdr, 0, 8+SIZE);
        uint16_t seq = 0;
        hdr->type = ICMP_ECHO;
        hdr->code = 0;

        // intialize poll structure
        struct pollfd pfd;
        pfd.fd = SOCKFD;
        pfd.events = POLLIN;
        int bytes;

        // start pinging
        int total_size = 8 + SIZE;
        int buffer[256];

        // start sending out packets
        printf( INIT_MSG, host_name, host_ip, SIZE, total_size+20 );
        struct timeval first, last;
        while( COUNT != 0 && looping){

                // init the check sum
                hdr->un.echo.sequence = htons( seq+1);
                hdr->checksum = 0;
                *(((u_int*)hdr)+2) = clock();
                hdr->checksum = make_cksum( (u_short*)hdr, total_size );

                // send it
                sleep( WAIT );
                gettimeofday( &first, NULL );
                bytes = sendto( pfd.fd, hdr, total_size, 
                                0, (struct sockaddr*)sa_in, 
                                sizeof( struct sockaddr ) );     
                num_sent++;
                // error writing out buffer
                if( bytes < 0 ){
                        free( hdr );
                        print_error( "error sending packet" );
                        exit( EXIT_FAILURE );
                }

                // listen for response
                bytes = poll( &pfd, 1, TIMEOUT );
                gettimeofday( &last, NULL );
                if( bytes < 0 ){
                        free( hdr );
                        print_error( "error polling" );
                        exit( EXIT_FAILURE );
                }

                if( bytes == 0 ){
                        printf( TIMEOUT_MSG, host_ip,hdr->un.echo.sequence );
                        fflush( stdout );
                }

                if( bytes != 0 && pfd.revents == POLLIN){
                        bytes = read( pfd.fd, buffer, 265 );
                        num_recv++;
                        struct ECHO_REP *e_rep = (struct ECHO_REP *)buffer;
                        int ttl = e_rep->ip_hdr.ttl;
                        double t = (double)(last.tv_usec-first.tv_usec)/1000;
                        printf( REPLY_MSG, bytes-20, 
                                get_host((struct IP_HDR*)&e_rep->ip_hdr), 
                                host_ip, 
                                ntohs(hdr->un.echo.sequence), ttl,
                                t );
                        fflush( stdout );
                }

                // dont touch
                if( COUNT > 0 )
                        COUNT--;
                seq++;
        }
        // done
        free( hdr );
        free( host_ip );
        close( SOCKFD );
        return 0;
}
