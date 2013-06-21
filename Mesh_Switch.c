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
int   tcp_port  = TCP_PORT_DEFAULT;

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
		node_role = HOP;
		tcp_port = TCP_PORT_DEFAULT;
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

	 struct mesh_paket test_paket;
	 struct mesh_paket *paketptr;
	 paketptr = (mesh_paket*)malloc(sizeof(mesh_paket));
	 test_paket= getMesh_paket(paketptr);

	 return test_paket;
}

// Globale Variablen
llist_t ll_neighbors;
llist_t ll_tracker;
int neighbor_id;
struct neighbors_entry *ptrneighbors_entry;
struct neighbors_entry *ptrneighbors_entry1;
struct packet_tracker_entry *pktentry;
int valide_routen[2];
pthread_mutex_t *lock;

/* Child thread implementation ----------------------------------------- */
void *clean_packet_tracker(void * arg) {

	sleep(10);
	unsigned int paket_id = *(unsigned int *) arg;
	struct packet_tracker_entry *pktentry;
	pktentry = (packet_tracker_entry*) malloc(sizeof(packet_tracker_entry));
	pthread_mutex_lock(lock);
	if ((packet_tracker_entry*) llist_get_data(paket_id, (void*) pktentry,
			&ll_tracker) == 0) {

	} else {
		llist_show_tracker(&ll_tracker);
		llist_remove_data(paket_id, (void*) pktentry, &ll_tracker);
		llist_show_tracker(&ll_tracker);

	}

	pthread_mutex_unlock(lock);

	free(pktentry);
	pthread_exit(NULL );

}
/* Child thread implementation ----------------------------------------- */
void *connection_handler(void * arg)
{

	struct mesh_paket paket;
	int open = 1; // Für Loop-Steuerung
	pthread_t threads;
	struct mesh_paket paket_confirm;
	unsigned int connection_s = *(unsigned int *) arg;   // copy the socket

	printf("Neuer Thread gestartet für Connection ID: %i\n ", connection_s);

	// Neuer Nachbar Eintragen
	pthread_mutex_lock(lock);
	printf(
			"Neuer Nachbar in Liste eingetragen: Nachbar-ID %i Nachbar-Connection: %i\n",
			neighbor_id, connection_s);
	pthread_mutex_unlock(lock);
	struct neighbors_entry *ptrneighbors_entry3;
	ptrneighbors_entry3 = (neighbors_entry*) malloc(sizeof(neighbors_entry));

	int this_neighbor_id = neighbor_id;
	ptrneighbors_entry3->neighbor_connection = connection_s;
	pthread_mutex_lock(lock);
	llist_insert_data(this_neighbor_id, ptrneighbors_entry3, &ll_neighbors);
	llist_show_neighbors(&ll_neighbors);

	//Nachbar ID für nächsten Knoten erhöhen.
	neighbor_id++;
	pthread_mutex_unlock(lock);

	while (open) {

		//sleep(5);
		printf("Pthread ID: %i \n", (int) pthread_self());

		if (read_line(connection_s, (void*) &paket, 132) == NULL ) {
			printf("Kein korrektes Paket erhalten oder Verbindung wurde geschlossen. Inhalt verwerfen.  \n.");
			open = 0;

		} else {
			printf("Paket Typ %c mit ID %i erhalten. Target: %i \n",
					paket.paket_type, ntohs(paket.paket_id), paket.target);

			// Verbindungspaket erhalten. Type N
			if (paket.paket_type == 'N') {
				print_paket(paket);

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
				int sockfd = open_connection_to("127.0.0.1", (int) newcon.port);
				if (sockfd == 0) {
					printf(
							"Keine Verbindung zu Port %i möglich. N-Paket verwerfen \n",
							(int) newcon.port);

				} else {

					int status = pthread_create(&threads, NULL,
							connection_handler, &sockfd);

					if (status != 0) {

						printf(
								"Fehler beim Erstellen des Threads mit Connection: %i",
								sockfd);

					}

					else {
						printf("Verbindung hergestellt mit %i\n",
								(int) newcon.port);
					}

				}

			}

			if (paket.paket_type == 'C') {
				print_paket(paket);
				if ((node_role == ZIEL && paket.target == 1)
						|| (node_role == QUELLE && paket.target == 0)) {

					// Bestätigungspaket erstellen, wenn das target erreicht wurde
					printf(
							"Es wird ein Bestätigunspaket versendet für Paket ID: %i an Connection: %i \n",
							ntohs((int) paket.paket_id), connection_s);

					//Paket zwischenspeichern
					paket_confirm = paket;
					paket.paket_type = 'O';
					strcpy(paket.data, " ");

					if (send_all(connection_s, &paket, sizeof(paket_confirm))
							< 0) {
						printf("Fehler beim senden \n");
					}

					printf("Bestätigung gesendet an %i \n", connection_s);

					//Paket wiederherstellen
					paket.paket_type = paket_confirm.paket_type;
					strcpy(paket.data, paket_confirm.data);

				} else {
					//send_all(connection_s, &paket,sizeof(paket));
					int paket_id = (int) paket.paket_id;

					//Paket mit der Paket_ID auslesen
					pthread_mutex_lock(lock);
					if (!(packet_tracker_entry*) llist_get_data(paket_id,
							(void*) pktentry, &ll_tracker) == 0) {

						pktentry = (packet_tracker_entry*) llist_get_data(
								paket_id, (void*) pktentry, &ll_tracker);
						pthread_mutex_unlock(lock);

						printf(
								"Eintrag im Tracker gefunden, Paket verwerfnen: Paket_ID: %i Nachbar ID: %i Target: %i \n",
								ntohs(paket_id), pktentry->neighbor_id,
								pktentry->target);

					} else {
						pthread_mutex_unlock(lock);
						//llist_show_tracker(&ll_tracker);
						printf("Eintrag im Tracker nicht gefunden \n");
						pthread_mutex_lock(lock);
						printf("Valide Routen %i %i \n", paket.target,
								valide_routen[paket.target]);

						if (valide_routen[paket.target] > 0) {

							struct neighbors_entry *ptrneighbors_entry4;
							ptrneighbors_entry4 = (neighbors_entry*) malloc(
									sizeof(neighbors_entry));
							ptrneighbors_entry4 = llist_get_data(
									valide_routen[paket.target],
									(void*) ptrneighbors_entry4, &ll_neighbors);
							pthread_mutex_unlock(lock);
							int connection_fd1 =
									ptrneighbors_entry4->neighbor_connection;
							free(ptrneighbors_entry4);

							printf(
									"Paket Typ %c mit ID %i wird an valide Route via Connection %i versendet. \n",
									paket.paket_type, ntohs(paket.paket_id),
									connection_fd1);
							send_all(connection_fd1, &paket, sizeof(paket));
							printf("Paket an valide Route gesendet! \n");

							struct packet_tracker_entry *pktentry1;
							pktentry1 = (packet_tracker_entry*) malloc(
									sizeof(packet_tracker_entry));
							pktentry1->neighbor_id = this_neighbor_id;
							pktentry1->target = paket.target;
							pthread_mutex_lock(lock);
							llist_show_tracker(&ll_tracker);
							llist_insert_data(paket.paket_id, (void*) pktentry1,
									&ll_tracker);
							pthread_mutex_unlock(lock);
							free(pktentry1);

						}

						else {
							pthread_mutex_unlock(lock);
							// Route existiert nicht. Sende an alle Nachbarn
							printf(
									"Es existiert keine Route zu %i (Ziel=1, QUelle=0)\n",
									paket.target);
							pthread_mutex_lock(lock);
							llist_show_neighbors(&ll_neighbors);
							if (neighbor_id == 0) {
								printf("Keine Nachbarn vorhanden \n");
							}

							int i = 1;
							for (i = 1; i < neighbor_id; i++) {
								if ((neighbors_entry*) llist_get_data(i,
										(void*) ptrneighbors_entry,
										&ll_neighbors) == 0) {
									printf("Kein Eintrag mit Nachbar ID: %i \n",
											i);
									pthread_mutex_unlock(lock);

								} else {
									ptrneighbors_entry =
											(neighbors_entry*) llist_get_data(i,
													(void *) ptrneighbors_entry,
													&ll_neighbors);
									pthread_mutex_unlock(lock);

									if (ptrneighbors_entry->neighbor_connection
											== connection_s) {
										printf(
												"Paket wird nicht an Quelle zurückgeschickt \n");
									} else {

										int status =
												send_all(
														ptrneighbors_entry->neighbor_connection,
														&paket, sizeof(paket));
										printf(
												"Status nach sendeschlaufe %i \n",
												status);
										printf(
												"Paket Typ %c mit ID %i an Nachbar mit ID %i weitergeleitet via Connection %i. \n",
												paket.paket_type,
												ntohs(paket.paket_id), i,
												ptrneighbors_entry->neighbor_connection);

										pktentry->neighbor_id = i;

									}

								}
							}
						}

						pktentry->target = paket.target;
						pktentry->neighbor_id = this_neighbor_id;
						pthread_mutex_lock(lock);
						llist_insert_data(paket_id, pktentry, &ll_tracker);
						pthread_mutex_unlock(lock);

					}

				}

				unsigned int paket_id_del = paket.paket_id;
				int status = pthread_create(&threads, NULL,
						clean_packet_tracker, &paket_id_del);

				if (status != 0) {

					printf(
							"Fehler beim Erstellen des Paket Tracker Cleaner für Paket ID: %i",
							paket.paket_id);

				}

			}

			if (paket.paket_type == 'O') {

				int paket_id = (int) paket.paket_id;
				print_paket(paket);
				printf("Bestätigung für Paket %i erhalten \n", ntohs(paket_id));

				//Ist das Paket im Tracker vorhanden?
				pthread_mutex_lock(lock);
				llist_show_tracker(&ll_tracker);
				if ((packet_tracker_entry*) llist_get_data(paket_id,
						(void*) pktentry, &ll_tracker) == 0) {

					printf(
							"Passendes Paket zur Bestätigung im Tracker nicht gefunden \n");
					pthread_mutex_unlock(lock);

				} else {
					pktentry = (packet_tracker_entry*) llist_get_data(paket_id,
							(void*) pktentry, &ll_tracker);

					pthread_mutex_unlock(lock);
					//Dieses paket wurde in der Liste gefunden
					printf(
							"Eintrag im Tracker gefunden: Paket_ID: %i Nachbar ID: %i Target: %i \n",
							ntohs(paket_id), pktentry->neighbor_id,
							pktentry->target);

					int source_id = pktentry->neighbor_id;
					printf("Dieser Nachbar wird als SOurce_ID festelegt: %i \n",
							source_id);
					pthread_mutex_lock(lock);
					valide_routen[paket.target] = this_neighbor_id;

					printf(
							"Eintrag nach Bestätigungspaket in valide Routen Liste: Target: %i Nachbar ID: %i\n",
							paket.target, valide_routen[paket.target]);

					printf("Valide ROuten Target: %i über Nachbar: %i \n",
							paket.target, valide_routen[paket.target]);
					pthread_mutex_unlock(lock);
					struct neighbors_entry *ptrneighbors_entry7;
					ptrneighbors_entry7 = (neighbors_entry*) malloc(
							sizeof(neighbors_entry));
					pthread_mutex_lock(lock);
					ptrneighbors_entry7 = (neighbors_entry*) llist_get_data(
							pktentry->neighbor_id, (void *) ptrneighbors_entry7,
							&ll_neighbors);
					pthread_mutex_unlock(lock);
					send_all(ptrneighbors_entry7->neighbor_connection, &paket,
							sizeof(paket));
					pthread_mutex_lock(lock);

					llist_remove_data(paket_id, (void*) pktentry, &ll_tracker);
					llist_show_tracker(&ll_tracker);
					pthread_mutex_unlock(lock);
					free(ptrneighbors_entry7);

				}

			}

		}

	}

	disconnect_client(connection_s);
	printf("Verbindung getrennt von Connection %i Nachbar ID: %i \n"
			"Nachbarliste wird bereinigt \n", connection_s, this_neighbor_id);
	ptrneighbors_entry3->neighbor_connection = connection_s;
	pthread_mutex_lock(lock);
	llist_remove_data(this_neighbor_id, (void*) ptrneighbors_entry3,
			&ll_neighbors);
	llist_show_neighbors(&ll_neighbors);
	pthread_mutex_unlock(lock);
	free(ptrneighbors_entry3);
	pthread_exit(NULL );

}

int main(int argc, char *argv[])
{
  int sock_fd, connection_fd, status =0;

  lock = (pthread_mutex_t *)malloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( lock, NULL);
  neighbor_id = 0;
  unsigned int thread_args;
  pthread_t threads;
  pthread_attr_t attr;
  extern int optind;
  pthread_attr_init(&attr);
  parse_options( argc, argv );

  // Nachbar / Paket Tracker Liste initialisieren
  llist_init(&ll_neighbors);
  ptrneighbors_entry = (neighbors_entry*)malloc(sizeof(neighbors_entry));
  llist_init(&ll_tracker);
  pktentry = (packet_tracker_entry*)malloc(sizeof(packet_tracker_entry));
  // Dummy Einträge in den Listen.
  // Dummy Packet Tracker Entry erstellen und einfügen
  ptrneighbors_entry->neighbor_connection = 9999;
  llist_insert_data(neighbor_id, ptrneighbors_entry, &ll_neighbors);
  neighbor_id++;
  pktentry->neighbor_id = ntohs(9999);
  pktentry->target = 1;
  llist_insert_data(ntohs(9999), pktentry, &ll_tracker);
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
    	  llist_show_neighbors(&ll_neighbors);
    	  thread_args = connection_fd;


  		status = pthread_create (
  				   &threads,
  				   &attr,
  				   connection_handler,
  				   &thread_args);
  		if(status != 0){
  			printf("Fehler beim erstellen des Threads mit Connection: %i", thread_args);
  		}
      }

      else{
      	printf("Verbindung mit %i nicht möglich mit der neuen Verbindung \n", tcp_port);
      }

     }
    pthread_join( threads , NULL);
    disconnect_client(sock_fd);
    free(ptrneighbors_entry);
    free(pktentry);
    free(lock);

    return TRUE;

}


