#ifndef __READLINE_H_
#define __READLINE_H_
/* 
 * A simple wrapper around "fgets". Read a newline terminated
 * line from fd.  See 'man 2 fgets'
 */
char *read_line(int socket_fd, char *s, int max_size);

/*
 * A simple wrappter around "write", that tries to send out all bytes
 * given to it. Returns -1 on error.
 */
int send_all(int socket_fd, void* bytes, size_t len);
#endif
