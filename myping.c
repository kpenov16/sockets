#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

//source https://www.stev.org/post/cgethostbynameexample


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

int main(int arg_count, char * args[]){
    
    if(arg_count != 2){
        printf("\n myping requires a parameter: ./myping mysmap.dk or 192.168.1.33");
        exit(EXIT_FAILURE);
    }

    printf("\n%s %s\n\n",args[0], args[1]);

    struct hostent * _hostent;
    _hostent = gethostbyname(args[1]);
    
    _print_host_entry(_hostent);







}