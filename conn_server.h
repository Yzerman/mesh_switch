#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

int listen_on_port(int portno );
/*
 * return socket handle or 0
 */
int connect_with_client( int sockfd );
void disconnect_client( int sockfd );

#endif
