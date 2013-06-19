/*
*  C Implementation: Mesh Switch
*
* Description:
*
*
* Author: Patrice Keusch, 21.04.2013
*
*
*/
#include <stdlib.h>      // exit
#include <stdio.h>
#include <string.h>      // strlen
#include <unistd.h>      // getopt
#include <pthread.h>
#include "conn_server.h"
#include "conn_client.h"
#include "conn_io.h"     // send_all
#include "mesh_paket.h"
#include "getMesh_paket.h"
#include "llist.h"
#define TRUE 1
#define FALSE 0


void report_error( char* message ) {
  fprintf( stderr, "ERROR: %s\n", message );
}

// Roles that a node can play
#define HOP    0
#define ZIEL   1
#define QUELLE 2
#define TCP_PORT_DEFAULT 3333
short node_role = HOP;

#define NO_TCP_PORT -1
int   tcp_port  = NO_TCP_PORT;

void parse_options( int argc, char *argv[])
{
  char optchar;

  while( ( optchar = getopt( argc, argv, "-hqz" ) ) != -1 ) {
    switch( optchar ) {
      case 'z':
        node_role = ZIEL;
        break;
      case 'q':
        node_role = QUELLE;
        break;
      case 1: // optchar '-' will assign non-option to 1
        tcp_port = atoi(optarg);
        //printf("Port is used %d",tcp_port);
        break;
      case 'h':
		  printf("Usage: server port [-z|-q]\n");

		  exit( 0 );
		  break;
      case '?':
    	  printf("Usage: server port [-z|-q]\n");
    	  exit( 0 );
    	  break;
      default:
		//node_role = HOP;
		//tcp_port = TCP_PORT_DEFAULT;
    	  if (tcp_port == NO_TCP_PORT ) {
    	    report_error("no port provided");
    	    exit( -1);
    	  }
		break;
    }

    if (tcp_port == NO_TCP_PORT ) {
              printf("Usage: server port [-z|-q]\n");
              report_error("no port provided");
              exit( 0 );
    	}

  }

}

void print_paket(mesh_paket paket){

    printf("paketID: %i \n", ntohs(paket.paket_id));
    printf("Target: %u \n",paket.target);
    printf("Paket Typ : %c \n",paket.paket_type);
    printf("Paket Data : %s \n",paket.data);
    printf("Size of recieved Mesh Paket: %d \n",sizeof(paket));


}

void print_n_content(connection newcon){


	  printf("NewIP: %i.%i.%i.%i\n", newcon.ip[0], newcon.ip[1], newcon.ip[2], newcon.ip[3]);
	  printf("NewPort: %i\n", newcon.port);
	  printf("NewContent: %s\n", newcon.data);
}


mesh_paket initialize_test_paket(void){

	// Test Paket initialisieren
	 struct mesh_paket test_paket;
	 struct mesh_paket *paketptr;
	 paketptr = (mesh_paket*)malloc(sizeof(mesh_paket));
	 test_paket= getMesh_paket(paketptr);

	 return test_paket;
}

// Globale Variablen
llist_t ll_neighbors;
llist_t ll_valid_routes;
llist_t ll_tracker;
int neighbor_id;
struct neighbors_entry *ptrneighbors_entry;
struct neighbors_entry *ptrneighbors_entry1;
struct packet_tracker_entry *pktentry;
int valide_routen[2];

/* Child thread implementation ----------------------------------------- */
void *connection_handler(void * arg)
{

  unsigned int    connection_s;         //copy socket
  struct mesh_paket paket; /* we expect some line of text shorter than 132 chars */

  connection_s = *(unsigned int *)arg;        // copy the socket

  printf("Neuer Thread gestartet mit Connection ID: %i\n ", connection_s);

  // Neuer Nachbar Eintragen
  printf("Neuer Nachbar in Liste eingetragen: Nachbar-ID %i Nachbar-Connection: %i\n",
  						neighbor_id, connection_s);

  struct neighbors_entry *ptrneighbors_entry3;
  ptrneighbors_entry3 = (neighbors_entry*) malloc(
		sizeof(neighbors_entry));
  int this_neighbor_id =neighbor_id;
  llist_show_neighbors(&ll_neighbors);
  ptrneighbors_entry3->neighbor_connection = connection_s;
  llist_insert_data(this_neighbor_id, ptrneighbors_entry3,
		&ll_neighbors);
  llist_show_neighbors(&ll_neighbors);
  //Nachbar ID für nächsten Knoten erhöhen.
  neighbor_id++;
  printf("Nächste Nachbar_ID: %i \n", neighbor_id);
  int open=1;
	while (open){

		if (read_line(connection_s, (void*) &paket, 132) ==NULL ) {
			printf("Kein korrektes Paket erhalten \n. Inhalt verwerfen.");
		    open=0;

		}
		else{
			printf("Paket Typ %c mit ID %i erhalten. Target: %i \n",
			paket.paket_type, ntohs(paket.paket_id), paket.target);
			/* Einsatz prüfen B Paket
					 if(paket.paket_type == 'B'){
					 printf("Verbindung hergestellt mit %i\n", connection_s);
					 printf("Neuer Nachbar in Liste eingetragen: Nachbar-ID %i Nachbar-Connection: %i\n", neighbor_id, connection_s);
					 struct neighbors_entry *ptrneighbors_entry2;
					 ptrneighbors_entry2 = (neighbors_entry*)malloc(sizeof(neighbors_entry));
					 ptrneighbors_entry2->neighbor_connection=connection_s;
					 llist_insert_data(neighbor_id, ptrneighbors_entry2, &ll_neighbors);
					 llist_show_neighbors(&ll_neighbors);

					 //Nachbar ID für nächsten Knoten erhöhen.
					 neighbor_id++;
					 printf("Nächste Nachbar_ID: %i \n", neighbor_id);

					 }
					 */

					// Verbindungspaket erhalten. Type N
					if (paket.paket_type == 'N') {
						//print_paket(paket);

						//IP und Port auslesen vom Vom Dateninhalt
						unsigned char ip[4] = { paket.data[0], paket.data[1],
								paket.data[2], paket.data[3] };
						unsigned short port = (unsigned short) (paket.data[4] << 8)
								| paket.data[5];
						char data[122];
						int i;
						for (i = 6; i < 128; i++) {
							data[i - 5] = paket.data[i];
						}
						struct connection newcon = { { *ip }, { port }, { *data } };
						print_n_content(newcon);


						//Verbindung herstellen
						int sockfd;
						if ((sockfd = open_connection_to("127.0.0.1", (int) newcon.port))
								== 0) {
							report_error("couldn't open connection to destination");
							printf(
									"An diesem Port %i ist kein Knoten. Paket verwerfern \n ",
									(int) newcon.port);

						} else {
							printf("Verbindung hergestellt mit %i\n",
									(int) newcon.port);
							/* Bestätigungspaket bei Verbinen. EInsatz prüfen
							 struct mesh_paket paket_new_con;
							 paket_new_con.paket_type = 'B'; strcpy(paket_new_con.data, " ");
							 paket_new_con.paket_id=0;paket_new_con.target=0;
							 printf("Nachbar informiert über neue Verbindung \n");
							 print_paket(paket_new_con);

							 if(send_all(sockfd, &paket_new_con, sizeof(paket_new_con))<0){
							 printf("Fehler beim senden \n");
							 }
							 */

							printf(
									"Neuer Nachbar in Liste eingetragen: Nachbar-ID %i Nachbar-Connection: %i\n",
									neighbor_id, sockfd);

							ptrneighbors_entry1 = (neighbors_entry*) malloc(
									sizeof(neighbors_entry));
							ptrneighbors_entry1->neighbor_connection = sockfd;
							llist_insert_data(neighbor_id, ptrneighbors_entry1,
									&ll_neighbors);
							llist_show_neighbors(&ll_neighbors);

							//Nachbar ID für nächsten Knoten erhöhen.
							neighbor_id++;
							printf("Nächste Nachbar_ID: %i \n", neighbor_id);

							// Test Paket senden zum neuen Knoten. Code am schluss entfernen.
							//send_all(sockfd, &test_paket, sizeof(struct mesh_paket));
							//printf("Paket Typ %c mit ID %i versendet \n", test_paket.paket_type, ntohs(test_paket.paket_id));

						}

					}

					if (paket.paket_type == 'C') {

						if ((node_role == ZIEL && paket.target == 1)
								|| (node_role == QUELLE && paket.target == 0)) {

							//print_paket(paket);

							// Bestätigungspaket erstellen, wenn das target erreicht wurde
							printf(
									"Es wird ein Bestätigunspaket versendet für Paket ID: %i an Connection: %i \n",
									ntohs((int) paket.paket_id), connection_s);
							struct mesh_paket paket_confirm;
							//Paket zwischenspeichern
							paket_confirm = paket;
							paket.paket_type = 'O';
							strcpy(paket.data, " ");

							printf("Bestätigung senden an %i in 2 Sekunden \n",
									connection_s);
							sleep(2);
							if (send_all(connection_s, &paket,
									sizeof(paket_confirm)) < 0) {
								printf("Fehler beim senden \n");
							}

							printf("Bestätigung gesendet an %i \n", connection_s);
							//Paket wiederherstellen
							paket.paket_type=paket_confirm.paket_type;
							strcpy(paket.data, paket_confirm.data);


						} else {

							int paket_id = (int) paket.paket_id;
							/* Listen-Debugging

							 printf("Dieser Eintrag wird im Tracker gesucht: Paket_ID %i \n", ntohs(paket_id) );
							 int entry_there = (packet_tracker_entry*)llist_get_data(paket_id, (void*)pktentry, &ll_tracker);
							 printf("Dieser Eintrag wurde im Tracker gefunden: %i \n", entry_there);

							 */

							//Paket mit der Paket_ID auslesen
							if (!(packet_tracker_entry*) llist_get_data(paket_id,
									(void*) pktentry, &ll_tracker) == 0) {

								pktentry = (packet_tracker_entry*) llist_get_data(
										paket_id, (void*) pktentry, &ll_tracker);
								//Dieses paket wurde in der Liste gefunden
								printf(
										"Eintrag im Tracker gefunden, Paket verwerfnen: Paket_ID: %i Nachbar ID: %i Target: %i \n",
										ntohs(paket_id), ntohs(pktentry->neighbor_id),
										pktentry->target);
								//return 0; //Für Tests deaktivieren Muss aktiviert werden!!
							} else {

								printf("Eintrag im Tracker nicht gefunden \n");

								printf("Valide Routen %i %i \n", paket.target, valide_routen[paket.target]);

								if(valide_routen[paket.target]>0){
/*
									ptrneighbors_entry1 = llist_get_data(valide_routen[paket.target],(void*)ptrneighbors_entry1, ll_neighbors);

									int connection_fd1 = ptrneighbors_entry1->neighbor_connection;
									 printf("Eintrag in der validen Routen liste gefunden: Target (Index): %i Connection: %i \n",
											paket.target, connection_fd1);
*/

								   send_all(6, &paket,sizeof(paket));
								   printf("gesendet! \n");


								}

								else {
									// Route existiert nicht. Sende an alle Nachbarn
									printf(
											"Es existiert keine Route zu %i (Ziel=1, QUelle=0)\n",
											paket.target);

									llist_show_neighbors(&ll_neighbors);
									if (neighbor_id == 0) {
										printf("Keine Nachbarn vorhanden \n");
									}
									int i = 1;
									for (i = 1; i < neighbor_id; i++) {
										if ((neighbors_entry*) llist_get_data(i,
												ptrneighbors_entry, &ll_neighbors)
												== 0) {
											printf("Kein Eintrag mit Nachbar ID: %i \n",
													i);

										} else {
											ptrneighbors_entry =
													(neighbors_entry*) llist_get_data(i,
															ptrneighbors_entry,
															&ll_neighbors);

											if (ptrneighbors_entry->neighbor_connection
													== connection_s) {
												printf(
														"Paket wird nicht an Quelle zurückgeschickt \n");
											} else {

												int status =send_all(
														ptrneighbors_entry->neighbor_connection,
														&paket, sizeof(paket));
												printf("Status nach sendeschlaufe %i \n", status);
												printf("Paket Typ %c mit ID %i an Nachbar mit ID %i weitergeleitet via Connection %i. \n",
														paket.paket_type,
														ntohs(paket.paket_id), i,
														ptrneighbors_entry->neighbor_connection);
												read_line(ptrneighbors_entry->neighbor_connection, (void*) &paket, 132);

													/*
												if (read_line(ptrneighbors_entry->neighbor_connection, (void*) &paket, 132) ==NULL ) {
															printf("Kein korrektes Paket nach Weiterleitung erhalten \n. Inhalt verwerfen.");

												}
												else{
													printf("Bestätigung erhalten. Paket Typ %c mit ID %i erhalten. Target: %i \n",
																paket.paket_type, ntohs(paket.paket_id), paket.target);
													printf("Nachbar ID, von paketerhalt %i \n", ntohs(ptrneighbors_entry->neighbor_connection));


												}*/
												pktentry->neighbor_id = i;

											}

										}
									}
								}
								//Verarbeitetes Paket in Tracker hinzufügen
								pktentry->target = paket.target;
								//pktentry->neighbor_id = ptrneighbors_entry->neighbor_connection; //evtl. Lösung suchen
								llist_insert_data(paket_id, pktentry, &ll_tracker);
								llist_show_tracker(&ll_tracker);

								//clean Package tracker --> überprüfen bei vielen Paketen.
								//sleep(0.1);llist_remove_data(paket_id,(void*) pktentry,&ll_tracker);

							}

						}

					}

					if (paket.paket_type == 'O') {
						int paket_id = (int) paket.paket_id;
						print_paket(paket);
						printf("Bestätigung für Paket %i erhalten \n", ntohs(paket_id));

						//Ist das Paket im Tracker vorhanden?
						if ((packet_tracker_entry*) llist_get_data(paket_id,
								(void*) pktentry, &ll_tracker) == 0) {

							printf("Eintrag im Tracker nicht gefunden \n");

						} else {

							pktentry = (packet_tracker_entry*) llist_get_data(paket_id,
									(void*) pktentry, &ll_tracker);
							//Dieses paket wurde in der Liste gefunden
							printf(
									"Eintrag im Tracker gefunden: Paket_ID: %i Nachbar ID: %i Target: %i \n",
									ntohs(paket_id), ntohs(pktentry->neighbor_id),
									pktentry->target);

							int source_id = pktentry->neighbor_id;
							printf("Dieser Nachbar wird als SOurce_ID festelegt: %i \n",
									source_id);

							//struct valid_routes_entry *validRoutes_entry1;

							valide_routen[paket.target] = source_id;

							printf(
									"Eintrag nach Bestätigungspaket in valide Routen Liste: Target: %i Connection: %i\n",
									paket.target, valide_routen[paket.target]);

							printf("Valide ROuten Target: %i über Nachbar: %i \n", paket.target, valide_routen[paket.target]);
							// Paket aus Packet Tracker löschen
							llist_remove_data(paket_id, (void*) pktentry, &ll_tracker);
							llist_show_tracker(&ll_tracker);
							//Paket an source ID senden ?!?? WIeso

						}


					}

		}

		paket.paket_type='Z';

	}
	disconnect_client(connection_s);

	printf("Verbindung getrennt von Connection %i Nachbar ID: %i \n"
			"Nachbarliste bereinigen \n", connection_s, this_neighbor_id);
	ptrneighbors_entry3->neighbor_connection = connection_s;
	  llist_remove_data(this_neighbor_id, (void*)ptrneighbors_entry3,
			&ll_neighbors);
	  llist_show_neighbors(&ll_neighbors);

	pthread_exit(NULL );

}

int main(int argc, char *argv[])
{
  int sock_fd, connection_fd, status =0;
  neighbor_id = 0;
  unsigned int ids;      // beinhaltet Thread Argumente
  pthread_t threads;
  pthread_attr_t attr;
  extern int optind;      // from unistd.h:getopt
  pthread_attr_init(&attr);
  parse_options( argc, argv );
  //Test Paket für Tests erstellen.
  //struct mesh_paket test_paket = initialize_test_paket();

  // Nachbar Liste initialisieren
  llist_init(&ll_neighbors);
  ptrneighbors_entry = (neighbors_entry*)malloc(sizeof(neighbors_entry));


  // Paket Tracker Liste initialisieren
  llist_init(&ll_tracker);
  pktentry = (packet_tracker_entry*)malloc(sizeof(packet_tracker_entry));

  // Test Eintrag in Nachbar Liste erstellen und einfügen
  ptrneighbors_entry->neighbor_connection = 9999;
  printf("Neuer Nachbar in Liste eingetragen: %i Nachbar-Connection: %i\n", neighbor_id, ptrneighbors_entry->neighbor_connection);
  llist_insert_data(neighbor_id, ptrneighbors_entry, &ll_neighbors);
  neighbor_id++;
  llist_show_neighbors(&ll_neighbors);

  // Test Einträge in den Listen.
  // Test Packet Tracker Entry erstellen und einfügen
  pktentry->neighbor_id = ntohs(9999);
  pktentry->target = 1;

  //printf("Test Paket Tracker Eintrag: Index %i Nachbar ID: %i Target: %i \n", 1, ntohs(pktentry->neighbor_id), pktentry->target);
  llist_insert_data(ntohs(9999), pktentry, &ll_tracker);
  llist_show_tracker(&ll_tracker);

  valide_routen[0] = 0;
  valide_routen[1] = 0;

    if( (sock_fd = listen_on_port(tcp_port) ) < 0 ) {
         report_error("failed to listen on the port");
         return sock_fd;
        }
    // Port und Node_Rolle anzeigen
    printf("Mesh_Switch %hi %i erstellt \n", node_role, tcp_port);

    while (TRUE)
     {

      if( (connection_fd = connect_with_client( sock_fd )) != 0){

      	ids = connection_fd;


  		status = pthread_create (                    /* Create a child thread        */
  				   &threads,                /* Thread ID (system assigned)  */
  				   &attr,                   /* Default thread attributes    */
  				   connection_handler,               /* Thread routine               */
  				   &ids);                   /* Arguments to be passed       */
  		if(status != 0){
  			printf("Fehler beim erstellen des Threads mit Connection: %i", ids);
  		}
      }

      else{
      	printf("Verbindung mit %i nicht möglich mit der neuen Verbindung \n", tcp_port);
      }

     }
    pthread_join( threads , NULL);
    disconnect_client(sock_fd);
    return TRUE;

}


