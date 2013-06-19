/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>	// malloc
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>	// memset, memcpy
#include <unistd.h>	// write

/*
 * returns < 0 on error
 */
int listen_on_port(int portno )
{
     int sockfd;
     struct sockaddr_in serv_addr;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd == -1)
     {
       perror("ERROR opening socket");
       return -1;
     }
     /* fix for WAITing sockets on linux */
     int yes = 1;
     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
         &yes, sizeof(yes)) == -1)
     {
       perror("ERROR on setsockopt()");
     }
     memset((void *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
     {
       perror("ERROR on binding");
       return -2;
     }
     listen(sockfd,5); // there can be 5 pending connections
     return sockfd;
}

/*
 * returns < 0 on error
 */
int connect_with_client( int sockfd )
{
     socklen_t clilen;
     int newsockfd;
     struct sockaddr_in cli_addr;

     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd == -1 )
       perror("ERROR on binding");
     return newsockfd;
}

/*
 * returns != 0 on error
 */
int disconnect_client( int sockfd ) {
  char c;
  // TIME_WAIT problem:
  // http://www.linuxjournal.com/article/2333
  // http://www.developerweb.net/forum/showthread.php?t=2941
  while( read( sockfd, &c, 1) > 0 );
  if( close(sockfd) != 0 ) {
    perror("ERROR on connection closing");
    return -1;
  }
  return 0;
}

