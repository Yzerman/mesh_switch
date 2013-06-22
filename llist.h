/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * llist.h --
 * 
 * Include file for linked list
 */
#include "pthread.h";

typedef struct llist_node {
  int index;
  void *datap;
  struct llist_node *nextp;
} llist_node_t;

typedef struct packet_tracker_entry {
  int dummy;
  int neighbor_id;
  int target;
} packet_tracker_entry;

typedef struct neighbors_entry{
  int dummy;
  int neighbor_connection;
} neighbors_entry;

typedef llist_node_t *llist_t;

int llist_init (llist_t *llistp);
int llist_insert_data (int index, void *datap, llist_t *llistp);
int llist_remove_data(int index, void **datapp, llist_t *llistp);
int llist_find_data(int index, void **datapp, llist_t *llistp);
void * llist_get_data(int index, void **datapp, llist_t *llistp);
int llist_change_data(int index, void *datap, llist_t *llistp);
int llist_show_tracker(llist_t *llistp);
int llist_show_neighbors(llist_t *llistp);
