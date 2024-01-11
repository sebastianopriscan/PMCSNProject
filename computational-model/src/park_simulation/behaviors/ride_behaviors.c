#include "behaviors.h"
#include <stdlib.h>
#include <stdio.h>

void ride_server_activate(struct simulation *sim, void *metadata)
{
  printf("launched ride_server_activate at %f\n", sim->clock);
  struct ride_metadata *ride_meta = (struct ride_metadata *)metadata;
  struct sim_state *state = (struct sim_state*) sim->state;
  struct client *me;
  struct event *event;
  int stream = ride_meta->queue_index + 5; // + 2 is already present when calcudlating queue_index

  double service_time = 0.0;
  
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL || state->rides[ride_meta->ride_idx].vip_queue != NULL) {
    
    struct ride ride = state->park->rides[ride_meta->ride_idx];
    service_time = GetRandomFromDistributionType(stream, ride.distribution, ride.mu, ride.sigma);
  }

  if(state->rides[ride_meta->ride_idx].vip_queue->head != NULL) {
    struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].vip_queue);
    if (value == NULL) {
      perror("Error while dequeue-ing from vip_arrivals. Exiting...");
      exit(1);
    }
    me = value->client ;
    event = value->event ;
    free(value) ;
  }
  else if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL) {
    struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].normal_queue);
    if (value == NULL) {
      perror("Error while dequeue-ing from normal_arrivals. Exiting...");
      exit(1);
    }
    me = value->client;
    event = value->event;
    free(value);
  }
  else {
    state->rides[ride_meta->ride_idx].busy_servers[ride_meta->server_idx] = 0;
    free(ride_meta);
    return ;
  }

  delete_event_from_simulation(sim, 0, event) ; //TODO: Check if client queue is too busy, and evaluate moving to dedicated queue

  state->rides[ride_meta->ride_idx].busy_servers[ride_meta->server_idx] = 1;

  double next = sim->clock + service_time;

  struct event* next_server_activate_event = createEvent(next, ride_server_activate, NULL, metadata);
  add_event_to_simulation(sim, next_server_activate_event, ride_meta->queue_index);
  struct event* choose_delay_event = createEvent(next, choose_delay, NULL, me);
  add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
}