/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * llist.c --
 * 
 * Linked list for single threaded application
 */
#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#define TRUE 1
#define FALSE 0



int llist_init (llist_t *llistp) 
{

	*llistp = NULL;
  return 0;
}
int llist_insert_data (int index, void *datap, llist_t *llistp) 
{
  llist_node_t *cur, *prev, *new;

  int found = FALSE;

  for (cur=prev=*llistp; cur != NULL; prev=cur, cur=cur->nextp) {
    if (cur->index == index) {
      free(cur->datap);
      cur->datap = datap;
      found=TRUE;
      break;
    }
    else if (cur->index > index){
      break;
    }
  }
  if (!found) {
    new = (llist_node_t *)malloc(sizeof(llist_node_t));
    new->index = index;
    new->datap = datap;
    new->nextp = cur;
    if (cur==*llistp)
      *llistp = new;
    else
      prev->nextp = new;
  }
   return 0;
}
int llist_remove_data(int index, void **datapp, llist_t *llistp) 
{
  llist_node_t *cur, *prev;

  /* Initialize to "not found" */
  *datapp = NULL;

  for (cur=prev=*llistp; cur != NULL; prev=cur, cur=cur->nextp) {
    if (cur->index == index) {
      *datapp = cur->datap;
      prev->nextp = cur->nextp;
      free(cur);
      break;
    }
    else if (cur->index > index){
      break;
    }
  }

  return 0;
}

int llist_find_data(int index, void **datapp, llist_t *llistp)
{
  llist_node_t *cur, *prev;

  /* Initialize to "not found" */
  *datapp = NULL;

  /* Look through index for our entry */
  for (cur=prev=*llistp; cur != NULL; prev=cur, cur=cur->nextp) {
    if (cur->index == index) {
    	*datapp = cur->datap;
    	break;
    }
    else if (cur->index > index){
    	break;

    }

  }
  return 0;
}

void * llist_get_data(int index, void **datapp, llist_t *llistp)
{
  llist_node_t *cur, *prev;

  /* Initialize to "not found" */
  *datapp = NULL;

  /* Look through index for our entry */
  for (cur=prev=*llistp; cur != NULL; prev=cur, cur=cur->nextp) {
    if (cur->index == index) {
    	*datapp = cur->datap;
    	return *datapp;
    	break;
    }
    else if (cur->index > index){
    	break;

    }

  }
  return 0;
}

int llist_change_data(int index, void *datap, llist_t *llistp)
{
  llist_node_t *cur, *prev;
  int status = -1; /* assume failure */

  for (cur=prev=*llistp; cur != NULL; prev=cur, cur=cur->nextp) {
    if (cur->index == index) {
      cur->datap = datap;
      prev->nextp = cur->nextp;
      free(cur);
      status = 0;
      break;
    }
    else if (cur->index > index){
      break;
    }
  }

  return status;
}
int llist_show_tracker(llist_t *llistp)
{
  llist_node_t *cur;

  packet_tracker_entry *pktentry1;
  pktentry1 = (packet_tracker_entry*)malloc(sizeof(packet_tracker_entry));

  printf ("Paket Tracker Linked list contains : \n");
  for (cur=*llistp; cur != NULL; cur=cur->nextp) {

	pktentry1 =  cur->datap;
    printf("Paket_ID(Index): %i Nachbar ID: %i Target: %i (Ziel=1, Quelle=0)\n", ntohs(cur->index), pktentry1->neighbor_id, pktentry1->target);

  }

  return 0;
}

int llist_show_neighbors(llist_t *llistp)
{
  llist_node_t *cur;

  neighbors_entry *neighbor_connection_entry;
  neighbor_connection_entry = (neighbors_entry *)malloc(sizeof(neighbors_entry));

  printf ("Nachbar Linked list contains : \n");
  for (cur=*llistp; cur != NULL; cur=cur->nextp) {

	  neighbor_connection_entry =  cur->datap;
    printf("Neigbor_ID: %i  Neighbor-Connection: %i \n", cur->index, neighbor_connection_entry->neighbor_connection);

  }

  return 0;
}


