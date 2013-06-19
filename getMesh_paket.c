/*
 * getMesh_paket.c
 *
 *  Created on: 21.04.2013
 *      Author: yzerman
 */

#include <stdio.h>
#include <stdlib.h>       // exit
#include <string.h>       // memset, memcpy, strlen
#include <unistd.h>       // getopt
#include "conn_client.h"  // open_connection_to
#include "conn_io.h"      // read_line
#include "mesh_paket.h"
mesh_paket getMesh_paket(mesh_paket paket){

	  paket.paket_id = ntohs(1);
	  paket.target = 0;
	  paket.paket_type = 'C';
	  strcpy(paket.data, "Hello");

	  return paket;

}
