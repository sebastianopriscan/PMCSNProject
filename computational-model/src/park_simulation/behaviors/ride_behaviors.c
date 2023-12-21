#include "behaviors.h"
#include <stdlib.h>
#include <stdio.h>

void ride_server_activate(struct simulation *sim, void *metadata)
{
  struct ride_metadata *ride_meta = (struct ride_metadata *)metadata;
  struct sim_state *state = (struct sim_state*) sim->state;
  struct client *me;
  struct event *event;

  double service_time = 0.0;
  
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL || state->rides[ride_meta->ride_idx].vip_queue != NULL)
    service_time = GetRandomFromDistributionType(0, state->park->rides[ride_meta->ride_idx].distribution, state->park->rides[ride_meta->ride_idx].mu, state->park->rides[ride_meta->ride_idx].sigma);

  if(state->rides[ride_meta->ride_idx].vip_queue != NULL) {
    struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].vip_queue);
    me = value->client ;
    event = value->event ;
    if (value == NULL) {
      perror("Error while dequeue-ing from vip_arrivals. Exiting...");
      exit(1);
    }
    free(value) ;
  }
  else if (state->rides[ride_meta->ride_idx].normal_queue != NULL) {
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
    return ;
  }

  delete_event_from_simulation(sim, 0, event) ; //TODO: select correct index

  state->rides[ride_meta->ride_idx].busy_servers[ride_meta->server_idx] = 1;

  double next = sim->clock + service_time;

  struct event* next_server_activate_event = createEvent(next, ride_server_activate, NULL, metadata);
  add_event_to_simulation(sim, next_server_activate_event, 0);
  struct event* choose_delay_event = createEvent(next, choose_delay, NULL, me);
  add_event_to_simulation(sim, choose_delay_event, 0);
}