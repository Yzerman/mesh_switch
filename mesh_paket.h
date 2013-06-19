#include <stdio.h>
#include <stdlib.h>
 
 
typedef struct mesh_paket{
	unsigned short paket_id;
	unsigned char target;
	char paket_type;
	char data [128];
} mesh_paket;

typedef struct connection{
	unsigned char ip[4];
	unsigned short port;
	char data[122];

} connection;
 
