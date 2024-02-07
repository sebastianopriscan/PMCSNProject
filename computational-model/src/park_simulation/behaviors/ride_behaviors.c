#include "behaviors.h"
#include <stdlib.h>
#include <stdio.h>

#define ride_log(log) (log & 0b0100) 

void ride_server_activate_validation(struct simulation *sim, void *metadata)
{
  struct ride_metadata *ride_meta = (struct ride_metadata *)metadata;
  struct sim_state *state = (struct sim_state*) sim->state;
  if (ride_log(state->log))
    printf("launched ride_server_activate at %f\n", sim->clock);
  struct client *me;
  struct event *event;
  int stream = ride_meta->queue_index + 5; // + 2 is already present when calculating queue_index

  double service_time = 0.0;
  
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL || state->rides[ride_meta->ride_idx].vip_queue->head != NULL) {
    
    struct ride ride = state->park->rides[ride_meta->ride_idx];
    service_time = GetRandomFromDistributionType(stream, ride.distribution, ride.mu, ride.mu * ride.sigma);
    service_time = service_time < 0 ? ride.mu : service_time ;
  }

  double arrival_time = 0.0;
  if(state->rides[ride_meta->ride_idx].vip_queue->head != NULL) {
    struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].vip_queue);
    if (value == NULL) {
      perror("Error while dequeue-ing from vip_arrivals. Exiting...");
      exit(1);
    }
    me = value->client ;
    event = value->event ;
    arrival_time = value->arrival_time;
    state->rides[ride_meta->ride_idx].total_delay_vip += (sim->clock - value->arrival_time);
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
    arrival_time = value->arrival_time;
    state->rides[ride_meta->ride_idx].total_delay_normal += (sim->clock - value->arrival_time) ;
    free(value);
  }
  else {
    state->rides[ride_meta->ride_idx].busy_servers[ride_meta->server_idx] = 0;
    free(ride_meta);
    return ;
  }

  state->rides[ride_meta->ride_idx].last_arrival = arrival_time;
  if (me->type == VIP) {
    state->rides[ride_meta->ride_idx].total_clients_vip += 1;
    state->rides[ride_meta->ride_idx].last_arrival_vip = arrival_time;
  }
  else {    
    state->rides[ride_meta->ride_idx].total_clients_normal += 1;
    state->rides[ride_meta->ride_idx].last_arrival_normal = arrival_time;
  }

  if (state->park->patience_enabled)
    delete_event_from_simulation(sim, CLIENT_QUEUE, event) ; //TODO: Check if client queue is too busy, and evaluate moving to dedicated queue

  state->rides[ride_meta->ride_idx].busy_servers[ride_meta->server_idx] = 1;

  double next = sim->clock + service_time;
  state->rides[ride_meta->ride_idx].global_service_mean += service_time;
  double old_sum = state->rides[ride_meta->ride_idx].servers_service_means[ride_meta->server_idx] * state->rides[ride_meta->ride_idx].servers_served_clients[ride_meta->server_idx];
  state->rides[ride_meta->ride_idx].servers_service_means[ride_meta->server_idx] = (old_sum + service_time) / (++state->rides[ride_meta->ride_idx].servers_served_clients[ride_meta->server_idx]);

  struct event* next_server_activate_event = createUndiscardableEvent(next, ride_server_activate_validation, NULL, metadata);
  add_event_to_simulation(sim, next_server_activate_event, ride_meta->queue_index);
  struct event* choose_delay_event = createUndiscardableEvent(next, choose_delay, NULL, me);
  add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
}

void ride_server_activate(struct simulation *sim, void *metadata)
{
  struct ride_metadata *ride_meta = (struct ride_metadata *)metadata;
  struct sim_state *state = (struct sim_state*) sim->state;
  if (ride_log(state->log))
    printf("launched ride_server_activate at %f\n", sim->clock);
  int stream = ride_meta->queue_index + 5; // + 2 is already present when calculating queue_index

  struct ride ride = state->park->rides[ride_meta->ride_idx];
  double service_time = GetRandomFromDistributionType(stream, ride.distribution, ride.mu, ride.mu * ride.sigma);
  service_time = service_time < 0 ? ride.mu : service_time ;
  double next = sim->clock + service_time;

  double arrival_time = 0.0;
  int actual_served = 0 ;
  if(state->rides[ride_meta->ride_idx].vip_queue->head != NULL) {
    for (int i = 0; i < ride.batch_size; i++) {
      struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].vip_queue);
      if (value == NULL) { // No more VIP in queue
        break;
      }
      arrival_time = value->arrival_time;
      state->rides[ride_meta->ride_idx].total_clients_vip += 1;
      state->rides[ride_meta->ride_idx].last_arrival_vip = arrival_time;
      state->rides[ride_meta->ride_idx].total_delay_vip += (sim->clock - value->arrival_time);
      
      if (state->park->patience_enabled)
        delete_event_from_simulation(sim, 2 + ride_meta->ride_idx, value->event) ; 
  
      struct event* choose_delay_event = createUndiscardableEvent(next, choose_delay, NULL, value->client);
      int code = add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
      if(code == 1) {
        free(choose_delay_event) ;
      }
      
      free(value) ;
      actual_served++ ;
    }
  }
  
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL) {
    for (int i = 0; i < ride.batch_size - actual_served; i++) {
      struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].normal_queue);
      if (value == NULL) {
        break;
      }
      arrival_time = arrival_time > value->arrival_time ? arrival_time : value->arrival_time;
      state->rides[ride_meta->ride_idx].total_clients_normal += 1;
      state->rides[ride_meta->ride_idx].last_arrival_normal = value->arrival_time;
      state->rides[ride_meta->ride_idx].total_delay_normal += (sim->clock - value->arrival_time) ;
      
      if (state->park->patience_enabled)
        delete_event_from_simulation(sim, 2 + ride_meta->ride_idx, value->event) ; 

      struct event* choose_delay_event = createUndiscardableEvent(next, choose_delay, NULL, value->client);
      int code = add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
      if(code == 1) {
        free(choose_delay_event) ;
      }

      free(value);
    }
  }

  state->rides[ride_meta->ride_idx].last_arrival = arrival_time;

  state->rides[ride_meta->ride_idx].global_service_mean += service_time;
  double old_sum = state->rides[ride_meta->ride_idx].servers_service_means[ride_meta->server_idx] * state->rides[ride_meta->ride_idx].servers_served_clients[ride_meta->server_idx];
  state->rides[ride_meta->ride_idx].servers_service_means[ride_meta->server_idx] = (old_sum + service_time) / (++state->rides[ride_meta->ride_idx].servers_served_clients[ride_meta->server_idx]);

  struct event* next_server_activate_event = createUndiscardableEvent(next, ride_server_activate, NULL, metadata);
  int code = add_event_to_simulation(sim, next_server_activate_event, ride_meta->queue_index);
  if(code == 1) {
    free(metadata) ;
    free(next_server_activate_event) ;
  }
}
