#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netinet/ip_icmp.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <time.h> 
#include <sys/time.h>

//#define _POSIX_C_SOURCE 199309L
#define PORT 0

//ref.: https://www.stev.org/post/cgethostbynameexample

char * _get_name(int h_addrtype){
    if(h_addrtype == AF_INET)
       return "IPv4";
    else if (h_addrtype == AF_INET6)
        return "IPv6";
    else
        return "other type";    
}

void _print_host_entry(struct hostent * host_entry){
    /* Description of data base entry for a single host. 
    struct hostent
        char *h_name;		// Official name of host.
        char **h_aliases;	// Alias list.  
        int h_addrtype;		// Host address type. 
        int h_length;		// Length of address.  
        char **h_addr_list;	// List of addresses from name server.*/    
    printf("\nOfficial host name: %s\n",host_entry->h_name);
    printf("List of host aliases:\n");
    for(int i=0; host_entry->h_aliases[i] != NULL; i++) {
        printf("h_aliases[%d]: %s\n", 
                i,
                host_entry->h_aliases[i]);
    }
    printf("\nHost address type: %d %s\n",
            host_entry->h_addrtype, 
            _get_name(host_entry->h_addrtype));
    printf("\nLength of address: %d\n",host_entry->h_length);
    printf("List of host aliases:\n");
    for(int i=0; host_entry->h_addr_list[i] != NULL; i++) {
        printf("h_addr_list[%d]: %s\n", 
                i, 
        inet_ntoa( 
            (struct in_addr)*(struct in_addr *) host_entry->h_addr_list[i]
            )
                );
    }
}

//ref.: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/install.html
int _exit_signal = 0;
void _handle_inerupt(int signal){ 
    _exit_signal = 1;
    //we don't wait for another signal just exit 
} 

struct icmp_packet 
{ 
    struct icmphdr hdr; // https://sites.uclouvain.be/SystInfo/usr/include/netinet/ip_icmp.h.html 
    char msg[64 - sizeof(struct icmphdr)]; //the rest of the 64 bytes is the msg 
}; 

// checksum calculation 
unsigned short _checksum(void * packet, int size) 
{   
    unsigned short * _2byte_ptr = packet; //unsigned short has 2 bytes 
    unsigned int _checksum = 0; //temporary checksum of 4 bytes 
                                //to be able to hold the carry of the addition 
    unsigned short result; //the real checksum is of 2 bytes in the header
  
    for ( _checksum = 0; size > 1; size -= 2 ){
        _checksum += *_2byte_ptr++; //the _2byte_ptr is incremented by 2 bytes (the size of the pointer type)
                                    //the 2 bytes are dereferenced and added to the 4 byte checksum,
                                    //the _checksum is of 4 bytes
                                    //so we have 2 bytes of place for the carry bits,
                                    //the packet struct should be small enought 
                                    //so we don't miss carry bits
    } 
         
    if ( size == 1 ){
        _checksum +=                  //if we have only one byte left from the msg
        *(unsigned char *)_2byte_ptr; //ou this is smart 
                                      //cast the 2 byte pointer to a 1 byte pointer 
                                      //when dereferencing the 1 byte pointer only the left most bits 
                                      //from the 2 bytes will be extracted and added to the checksum
    } 
    _checksum = (_checksum >> 16) //we want the carry bits
                                  //so we bitshift out the right 2 bytes
                                  //what is left is the carry if any
                + (_checksum & 0xFFFF); //we add the right 2 bytes with the carry 
    _checksum += (_checksum >> 16); //if there is a carry from the last addition
                                    //we need that as well, 
                                    //so we bitshift again the right 2 bytes
                                    //what is left is the carry and we add it to the sum 
    result = ~_checksum; //invert the bits 
                         //and cast to unsigned short of 2 bytes
                         //which need to be added to the icmp header 
    return result; 
} 
int main(int arg_count, char * args[]){
    
    if(arg_count != 2){
        printf("\n myping requires a parameter: ./myping mysmap.dk or 192.168.1.33");
        exit(EXIT_FAILURE);
    }

    printf("\n%s %s\n\n",args[0], args[1]);

    struct hostent * _hostent;
    _hostent = gethostbyname(args[1]); //the subrutine does a DNS lookup by host name or retruns the ip if ip is passed (in the returned structure) 
                      // https://www.ibm.com/support/knowledgecenter/ssw_aix_72/communicationtechref/gethostbyname.html 
		      // http://man7.org/linux/man-pages/man3/gethostbyname.3.html
    _print_host_entry(_hostent);

    if(_hostent == NULL){
        printf("\nNo host IP found via DNS lookup");
        exit(EXIT_FAILURE);
    }

/* Structure describing an Internet socket address.
struct sockaddr_in
    sin_family //IPv4 or IPv6 or other
    in_port_t sin_port;	     //Port number.
    struct in_addr sin_addr; // Internet address.
        //Internet address
        typedef uint32_t in_addr_t;
        struct in_addr
            in_addr_t s_addr;
    //Pad to size of `struct sockaddr'.
    unsigned char sin_zero[sizeof (struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof (in_port_t) -
			   sizeof (struct in_addr)];
*/

    struct sockaddr_in _sockaddr_in;
    _sockaddr_in.sin_family = _hostent->h_addrtype; //gethostbyname returns struct in network order, no need to reorder 
						    //https://stackoverflow.com/questions/30232733/gethostbyname-and-endianness-how-are-the-bytes-returned
    _sockaddr_in.sin_port = htons(PORT); //htons (host to network short) orders the bytes in the network order(big endian), if the system orders is different
    				         // https://stackoverflow.com/questions/19207745/htons-function-in-socket-programing 
    _sockaddr_in.sin_addr.s_addr = *(long *)_hostent->h_addr_list[0];
    //inet_ntoa( (struct in_addr)*(struct in_addr *) _hostent->h_addr_list[0]);
    
    //creating ICMP socket
    int _icmp_socket_fd;        
    if(_hostent->h_addrtype == AF_INET){ //its IPv4 address/network
        //_icmp_socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        _icmp_socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //we tell the socket that will be used with the ICMP 
		// http://man7.org/linux/man-pages/man2/socket.2.html -> https://pubs.opengroup.org/onlinepubs/009619199/syssocketh.htm 
		// -> http://man7.org/linux/man-pages/man3/getprotoent.3.html
		// http://man7.org/linux/man-pages/man5/protocols.5.html iana says its 1 , but here is a header netinet/in.h where it is defiend
		// this https://stackoverflow.com/questions/13543554/how-to-receive-icmp-request-in-c-with-raw-sockets says that we can send raw icmp datagram

        if(_icmp_socket_fd < 0){
            printf("\n_icmp_socket_fd is negative\n");
            exit(EXIT_FAILURE);
        }
        printf("\n_icmp_socket_fd is: %d\n", _icmp_socket_fd);
        
        //register a handler for kayboard interupt http://man7.org/linux/man-pages/man7/signal.7.html
        signal(2, _handle_inerupt);    //to exit the pign
    
        // https://pubs.opengroup.org/onlinepubs/009695399/functions/setsockopt.html
        // the java world https://docs.oracle.com/javase/8/docs/technotes/guides/net/socketOpt.html
        //configure socket details
        int ttl = 64;
        if(setsockopt(_icmp_socket_fd, 
                   IPPROTO_IP, IP_TTL, // setting the tll value in the ip protocol header https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/apis/ssocko.htm   
                   &ttl, sizeof(ttl)) != 0){
            printf("\nsetsockopt: Setting ttl to the IP protocol header failed!\n");
            exit(EXIT_FAILURE);           
        }else{
            printf("\nsetsockopt: TTL in IP header is set to: %d\n", ttl);
        }

        /* A time value that is accurate to the nearest
        microsecond but also has a range of years.
        struct timeval
            __time_t tv_sec;		// Seconds. 
            __suseconds_t tv_usec;	//Microseconds.  */
        
        //timeout on recieve reply 
        struct timeval _timeval;
        _timeval.tv_sec = 1; //wait 1 sec before timeout on recieve responce  
        _timeval.tv_usec = 0;

        //bear in mind this http://forums.codeguru.com/showthread.php?353217-example-of-SO_RCVTIMEO-using-setsockopt()
        setsockopt(_icmp_socket_fd,
                   SOL_SOCKET, SO_RCVTIMEO, 
                   (const void *) & _timeval, sizeof(struct timeval));
        //socket layer is set with recieve timeout https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/apis/ssocko.htm 
	
        /*
        struct icmp_packet //custom made 
            struct icmphdr hdr; 
            char msg[64 - sizeof(struct icmphdr)]; 

        struct icmphdr     //imported
            uint8_t type;  // message type
            uint8_t code;  // type sub-code
            uint16_t checksum;
            union un
                struct echo // echo datagram, we use this one 
                uint16_t	id;
                uint16_t	sequence;
                uint32_t    gateway; // gateway address
                struct frag //path mtu discovery
                uint16_t	__glibc_reserved;
                uint16_t	mtu; */

        struct icmp_packet _icmp_packet; 
        int _msg_count = 0;
        int _msg_count_recieved = 0;
        while (!_exit_signal){
            //write bytes containing '\0' end string char
            bzero(&_icmp_packet, sizeof(struct icmp_packet));

            _icmp_packet.hdr.type = ICMP_ECHO;
            //_icmp_packet.hdr.code is default to zero, no need to reassign
            _icmp_packet.hdr.un.echo.id = getpid(); 
            // https://stackoverflow.com/questions/25699779/should-i-write-id-to-icmphdr-id-field-when-using-icmp-sockets
            int i=0;
            for(;i<(sizeof _icmp_packet.msg)-1; i++){
                _icmp_packet.msg[i] = i + '0';
            }
            _icmp_packet.msg[i] = 0;

            _icmp_packet.hdr.un.echo.sequence = _msg_count++;
            _icmp_packet.hdr.checksum = _checksum(
                &_icmp_packet, sizeof(_icmp_packet)
            );            

            usleep(1000000);

            struct timespec _start_time;
            struct timespec _end_time;
            //clock_gettime(CLOCK_MONOTONIC, &_start_time);      
            clock_gettime(1, &_start_time);       
            int _packet_was_sent = 1;
            // https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/sndt.htm
            if(sendto(_icmp_socket_fd, 
                    &_icmp_packet, sizeof(_icmp_packet), 0,
                    (struct sockaddr *) &_sockaddr_in, sizeof(_sockaddr_in)) <= 0){
                    printf("\nSending _icmp_packet failed\n");
                    _packet_was_sent = 0;
            }

            // https://www.ibm.com/support/knowledgecenter/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rcvf.htm 
            // http://forums.codeguru.com/showthread.php?533915-How-are-ICMP-replies-associated-with-the-request-socket
            // And the real explanation: https://www.systutorials.com/docs/linux/man/2-recvfrom/ 
            // If src_addr is not NULL, and the underlying protocol provides the source address of the message, 
            // that source address is placed in the buffer pointed to by src_addr -> our _sockaddr_in_recv.
            struct sockaddr_in _sockaddr_in_recv;
            int _sockaddr_in_recv_size = sizeof(_sockaddr_in_recv);
            if(recvfrom(_icmp_socket_fd,
                        &_icmp_packet, sizeof(_icmp_packet),0,
                        (struct sockaddr *)&_sockaddr_in_recv, &_sockaddr_in_recv_size)
                        <= 0 && _msg_count > 1){
                printf("\nrecvfrom _icmp_packet failed\n");
            }else{
                //clock_gettime(CLOCK_MONOTONIC, &_start_time);      
                clock_gettime(1, &_end_time);
                double _time_left = 
                                ((double)(_end_time.tv_nsec - _start_time.tv_nsec))
                                    /1000000.; 
                long double _round_trip_time_msec = 
                                (_end_time.tv_sec - _start_time.tv_sec)*1000.
                                  + _time_left;
      
                if(_packet_was_sent){
                
                    if(!(_icmp_packet.hdr.type == 69
                        && _icmp_packet.hdr.code == 0)){
                        printf("Something went wrong: _icmp_packet.hdr.type: %d, _icmp_packet.hdr.code: %d",
                                _icmp_packet.hdr.type, _icmp_packet.hdr.code);
                    }else{                        
                        printf("got packet of 64 bytes from %s with IP: %s, _msg_count: %d, ttl: %d, _round_trip_time_msec: %Lf ms.\n",  
                            args[1], inet_ntoa((struct in_addr)*(struct in_addr *) _hostent->h_addr_list[0]), 
                            _msg_count, ttl, _round_trip_time_msec);
                        _msg_count_recieved++;
                    }   
                }            
            }


        }
                

    }



}








