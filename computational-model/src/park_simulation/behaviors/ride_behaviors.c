#include "behaviors.h"
#include <stdlib.h>
#include <stdio.h>

#define ride_log(log) (log & 0b0100)

void reduce_clients_ahead(struct generic_queue_list* queue, int ride_idx, int processed_clients) {

  struct generic_queue_node *node = queue->head ;
  while (node != NULL)
  {
    struct client *client = (struct client *) node->data ;

    for (int i = 0; i < client->max_prenotations; i++) {
      if (client->active_reservations[i].expired || client->active_reservations[i].ride_idx != ride_idx) 
        continue;
      
      client->active_reservations[i].clients_left -= processed_clients;
    }
    
    node = node->next;
  }
}

void ride_server_activate_validation(struct simulation *sim, void *metadata)
{
  struct ride_metadata *ride_meta = (struct ride_metadata *)metadata;
  struct sim_state *state = (struct sim_state*) sim->state;
  if (ride_log(state->log))
    printf("launched ride_server_activate at %f\n", sim->clock);
  struct client *me;
  struct event *event;
  int stream = ride_meta->queue_index + TOTAL_STREAMS - TOTAL_STANDARD_QUEUES;

  double service_time = 0.0;
  
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL || state->rides[ride_meta->ride_idx].vip_queue->head != NULL) {
    
    struct ride ride = state->park->rides[ride_meta->ride_idx];
    service_time = GetRandomFromDistributionType(stream, ride.distribution, ride.mu, ride.sigma);
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
  int stream = ride_meta->queue_index + TOTAL_STREAMS - TOTAL_STANDARD_QUEUES;

  struct ride ride = state->park->rides[ride_meta->ride_idx];
  double service_time = GetRandomFromDistributionType(stream, ride.distribution, ride.mu, ride.sigma);
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
        delete_event_from_simulation(sim, CLIENT_QUEUE, value->event) ; //TODO: Check if client queue is too busy, and evaluate moving to dedicated queue
  
      struct event* choose_delay_event = createUndiscardableEvent(next, choose_delay, NULL, value->client);
      int code = add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
      if(code == 1) {
        free(choose_delay_event) ;
      }
      
      free(value) ;
      actual_served++ ;
    }
  }
  int reserved_served = 0;
  if (state->park->improved_run && state->rides[ride_meta->ride_idx].real_reserved_queue->head != NULL) {
    for (int k = 0; k < ride.batch_size - actual_served; k++) {
      //Get an effective client in reserved queue
      struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].real_reserved_queue);
      if (value == NULL) { //No more clients in physical reserved queue (so virtual reservations are all invalid)
        struct client *content ;
        do {
          content = generic_dequeue_element(state->rides[ride_meta->ride_idx].reserved_queue);
          if(content == NULL) break;
          for (int i = 0; i < content->max_prenotations; i++) {
            if(content->active_reservations[i].expired || content->active_reservations[i].ride_idx != ride_meta->ride_idx)
              continue;
            content->active_reservations[i].expired = 1;
          }
        } while(content != NULL);

        break;
      }
      int expired_clients = 1 ;
      // Get reservation from client dequeued from real queue
      struct reservation *res_in_queue = NULL; // First client in queue
      for (int j = 0; j < value->client->max_prenotations; j++) {
        if (value->client->active_reservations[j].ride_idx == ride_meta->ride_idx && !value->client->active_reservations[j].expired) {
          res_in_queue = &value->client->active_reservations[j];
          break;
        }
      }
      //Expire all non-arrived clients with higher priority
      struct client *content ;
      do {
        content = generic_dequeue_element(state->rides[ride_meta->ride_idx].reserved_queue) ;
        if (content == NULL || content == value->client) 
          break;

        for(int i = 0; i < content->max_prenotations; i++) {
          struct reservation *res = &content->active_reservations[i]; // First reserved client (not necessarily in queue)
          if (res->expired || res->ride_idx != ride_meta->ride_idx) 
            continue;
          
          if (res->clients_left < res_in_queue->clients_left) {
            res->expired = 1 ;
            res->ride_idx = -1;
            expired_clients++;
          }
        }
      } while(1) ;

      reduce_clients_ahead(state->rides[ride_meta->ride_idx].reserved_queue, ride_meta->ride_idx, expired_clients);

      arrival_time = arrival_time > value->arrival_time ? arrival_time : value->arrival_time;
      state->rides[ride_meta->ride_idx].total_clients_reserved += 1;
      state->rides[ride_meta->ride_idx].total_delay_reserved += (sim->clock - value->arrival_time) ;

      res_in_queue->expired = 1;
      res_in_queue->ride_idx = -1;

      struct event* choose_delay_event = createUndiscardableEvent(next, choose_delay, NULL, value->client);
      int code = add_event_to_simulation(sim, choose_delay_event, CLIENT_QUEUE);
      if(code == 1) {
        free(choose_delay_event) ;
      }

      free(value);
      reserved_served++;
    }
  }
  if (state->rides[ride_meta->ride_idx].normal_queue->head != NULL) {
    for (int i = 0; i < ride.batch_size - actual_served - reserved_served; i++) {
      struct client_event *value = (struct client_event *) generic_dequeue_element(state->rides[ride_meta->ride_idx].normal_queue);
      if (value == NULL) {
        break;
      }
      arrival_time = arrival_time > value->arrival_time ? arrival_time : value->arrival_time;
      state->rides[ride_meta->ride_idx].total_clients_normal += 1;
      state->rides[ride_meta->ride_idx].last_arrival_normal = value->arrival_time;
      state->rides[ride_meta->ride_idx].total_delay_normal += (sim->clock - value->arrival_time) ;
      
      if (state->park->patience_enabled)
        delete_event_from_simulation(sim, CLIENT_QUEUE, value->event) ; //TODO: Check if client queue is too busy, and evaluate moving to dedicated queue

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