#ifndef BEHAVIORS_H
#define BEHAVIORS_H
#include <simulation/simulation.h>
#include "../../models/model.h"
#include "../sim_state.h"

#define CLIENT_QUEUE 0
#define SHOW_QUEUE 1
#define TOTAL_STANDARD_QUEUES 2

#define NEXT_ARRIVAL_STREAM 0
#define PATIENCE_MU_STREAM 1
#define CLIENT_TYPE_STREAM 2
#define EXIT_TIME_STREAM 3
#define DELAY_STREAM 4
#define POPULARITY_STREAM 5
#define PATIENCE_STREAM 6
#define RESERVED_STREAM 7
#define TOTAL_STREAMS 8

struct client_event {
  struct client *client;
  struct event *event;
  int selected_attraction_idx;
  // Statistics
  double arrival_time; //At a specific attraction
};

struct ride_metadata {
  int ride_idx ;
  int server_idx ;
  int queue_index;
};

// Client Behaviors

// metadata : unused
void reach_park(struct simulation *sim, void *metadata);

// metadata : unused
void next_reach(struct simulation *sim, void *metadata);

// metadata : struct client *
// void client_exit_trigger(struct simulation *sim, void *metadata) ;

// metadata :  struct client *
void choose_delay(struct simulation* sim, void *metadata);

// metadata: struct client *
void choose_attraction(struct simulation* sim, void *metadata);

// metadata: struct client_event *
void patience_lost(struct simulation* sim, void *metadata);

// Attraction Behaviors

// metadata: struct ride_metadata *
void ride_server_activate(struct simulation* sim, void *metadata);

// metadata: struct ride_metadata *
void ride_server_activate_validation(struct simulation* sim, void *metadata);

// metadata: struct client_event *
void reach_show(struct simulation* sim, void *metadata);

// metadata : int (show to activate)
void show_activate(struct simulation * sim, void *metadata);

// metadata : int (show to deactivate)
void show_deactivate(struct simulation *sim, void *metadata);
#endif