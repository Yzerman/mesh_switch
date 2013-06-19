#include <stdio.h>
#include <string.h>	// memset, memcpy
#include <stdint.h>     // uint32_t
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>	// htonl
#include <unistd.h>     // close

#include <stdlib.h>     // malloc

/*
 * http://www.koders.com/c/fid3460285BA5BD8FCA8F1BAAF74DA6DCE90DB84A16.aspx?s=gethostbyname+connect
* in camserv\common\socket.c
*/

void show_ip( struct hostent* hst )
{
    char ip[INET_ADDRSTRLEN];
    bzero(ip,INET_ADDRSTRLEN);
    inet_ntop(AF_INET, hst->h_addr_list[0], ip, INET_ADDRSTRLEN);
    printf("IP: %s\n", ip);
}

/*
 * socket_resolv_hostname:  Resolve a string hostname to a internet
 *                          sockaddr structure
 *
 * Arguments:               hname = Hostname to resolve.
 *                          sin   = Sockaddr struct to place resolved host
 *
 * Return values:           Returns -1 on failure, 0 on succes.
 */
int resolv_hostname( const char *hname, struct sockaddr_in *sin ){
  struct hostent *hostp;
  unsigned long addr;

  addr = inet_addr( hname );
#ifdef INADDR_NONE
  if( addr != INADDR_NONE )
    memcpy( &sin->sin_addr, &addr, sizeof( addr ));
#else
  if (addr != (in_addr_t)-1)
    memcpy( &sin->sin_addr, &addr, sizeof( addr ));
#endif
  else {
    hostp = gethostbyname( hname );
    if( hostp == NULL ) {
      perror("ERROR resolving hostname");
      return -1;
    }
    else
      memcpy( &sin->sin_addr, hostp->h_addr, hostp->h_length );
  }
  return 0;
}

int open_connection_to( const char *remote_name, int remote_port ){
  int fd;
  struct sockaddr_in serv_addr;

  if( (fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0 ){
    perror( "ERROR opening socket" );
    return 0;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port   = htons( remote_port );

  if( resolv_hostname( remote_name, &serv_addr ) == -1 ){
    return 0;
  }

  if( connect( fd, (struct sockaddr *)&serv_addr, sizeof( serv_addr )) == -1 ){
    perror( "ERROR connecting" );
    return 0;
  }

  return fd;
}

void disconnect_server( int sockfd ) {
  if( close(sockfd) != 0 )
    perror("ERROR. Failed to close connection");
}

