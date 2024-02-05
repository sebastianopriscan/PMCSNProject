#include "behaviors.h"
#include <generic_queue.h>
#include <rngs.h>
#include <stdio.h>
#include <stdlib.h>

#define client_log(log) (log & 0b0001)

int position_in_queue(void *first, void *second) {
  struct client_event *first_client = (struct client_event *) first ;
  struct client_event *second_client = (struct client_event *) second ;

  struct reservation *first_reservation;
  for(int i = 0; i < first_client->client->max_prenotations; i++) {
    if (first_client->client->active_reservations[i].expired || first_client->client->active_reservations[i].ride_idx != first_client->selected_attraction_idx)
      continue;
    first_reservation = &first_client->client->active_reservations[i];
  }
  
  struct reservation *second_reservation;
  for(int i = 0; i < second_client->client->max_prenotations ; i++) {
    if (second_client->client->active_reservations[i].expired || second_client->client->active_reservations[i].ride_idx != second_client->selected_attraction_idx)
      continue;
    second_reservation = &second_client->client->active_reservations[i];
  }

  if(first_reservation->clients_left < second_reservation->clients_left) {
    return 1 ;
  } else if (first_reservation->clients_left == second_reservation->clients_left) {
    return 0 ;
  } else {
    return -1 ;
  }
}

void patience_lost(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;

  if(client_log(state->log))
    printf("launched patience_lost at %f\n", sim->clock);
  struct client_event *client_event = (struct client_event *)metadata;
  struct client* client = client_event->client;
  client->lost_patience_times += 1;

  if (client_event->client->type == VIP) {
    generic_remove_element(state->rides[client_event->selected_attraction_idx].vip_queue, client_event);
    state->rides[client_event->selected_attraction_idx].total_lost_vip += 1;
    state->rides[client_event->selected_attraction_idx].total_lost_vip_delay += (sim->clock - client_event->arrival_time);
    state->rides[client_event->selected_attraction_idx].total_delay_vip += (sim->clock - client_event->arrival_time);
  } else {
    generic_remove_element(state->rides[client_event->selected_attraction_idx].normal_queue, client_event);
    state->rides[client_event->selected_attraction_idx].total_lost_normal += 1;
    state->rides[client_event->selected_attraction_idx].total_lost_normal_delay += (sim->clock - client_event->arrival_time);
    state->rides[client_event->selected_attraction_idx].total_delay_normal += (sim->clock - client_event->arrival_time);
  }
  free(client_event);
  struct event *event = createUndiscardableEvent(sim->clock, choose_delay, NULL, client);
  add_event_to_simulation(sim, event, CLIENT_QUEUE);
}

void choose_attraction(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;

  if(client_log(state->log))
    printf("launched choose_attraction at %f\n", sim->clock);

  int selected_ride_idx = -1;
  double p = GetRandomFromDistributionType(POPULARITY_STREAM, UNIFORM, 0, 1);

  if (p <= state->popularities[0])
    selected_ride_idx = 0;
  else {
    for (int i = 1; i < state->park->num_rides + state->park->num_shows; i++) {
      if (state->popularities[i - 1] <= p && p <= state->popularities[i]) {
        selected_ride_idx = i;
        break;
      }
    }
  }

  if (selected_ride_idx == -1) {
    fprintf(stderr, "Fatal error in selecting ride (sim clock: %f), exiting...\n", sim->clock);
    exit(1);
  }

  struct client *me = (struct client *)metadata;

  struct client_event *client_ev = malloc(sizeof(struct client_event));
  if (client_ev == NULL) {
    fprintf(stderr, "Error allocating client_event\n");
    exit(1);
  }
  client_ev->client = me;
  client_ev->selected_attraction_idx = selected_ride_idx;
  client_ev->arrival_time = sim->clock ;
  client_ev->event = NULL;
  
  if (selected_ride_idx >= state->park->num_rides) {
    struct event *reach_show_event = createEvent(sim->clock, reach_show, NULL, client_ev);
    add_event_to_simulation(sim, reach_show_event, CLIENT_QUEUE);
    return;
  }

  // NOTE: check for patience sigma
  if (state->park->patience_enabled) {
    double mean_time = state->park->rides[selected_ride_idx].mu;
    double patience = GetRandomFromDistributionType(PATIENCE_STREAM, NORMAL_DISTRIB, mean_time + 60.0, mean_time * me->client_percentage);

    patience = patience < 0 ? -patience : patience ;

    if (state->park->improved_run) {
      for(int i = 0 ; i < me->max_prenotations; i++) {
        if (me->active_reservations[i].expired) 
          continue;

        struct reservation* res = &me->active_reservations[i];
        struct ride_state *ride_state = &state->rides[i];
        struct ride *ride = &state->park->rides[res->ride_idx];
        
        double estimation = (ride_state->vip_queue->size + res->clients_left) * ride->mu / ride->server_num;

        patience = estimation < patience ? estimation : patience ;
      }
    }

    struct event *lose_patience = createEvent(sim->clock + patience, patience_lost, NULL, client_ev);
    client_ev->event = lose_patience;
    int code = add_event_to_simulation(sim, lose_patience, TOTAL_STANDARD_QUEUES + selected_ride_idx);
    if(code == 1) {
      free(lose_patience) ;
      client_ev->event = NULL;
    }
  }

  if (me->type == VIP) {
    generic_enqueue_element(state->rides[selected_ride_idx].vip_queue, client_ev);
    state->rides[selected_ride_idx].total_arrived_vip += 1;
  } else {
    generic_enqueue_element(state->rides[selected_ride_idx].normal_queue, client_ev);
    state->rides[selected_ride_idx].total_arrived_normal += 1;
   }

  if(state->rides[selected_ride_idx].first_arrival == 0.0)
    state->rides[selected_ride_idx].first_arrival = sim->clock;

  if(state->rides[selected_ride_idx].first_arrival_normal == 0.0 && me->type == NORMAL)
    state->rides[selected_ride_idx].first_arrival_normal = sim->clock;

  if(state->rides[selected_ride_idx].first_arrival_vip == 0.0 && me->type == VIP)
    state->rides[selected_ride_idx].first_arrival_vip = sim->clock;

  if (!state->park->validation_run) 
    return;

  // Validation run code
  for (int i = 0; i < state->park->rides[selected_ride_idx].server_num; i++)
  {
    if (state->rides[selected_ride_idx].busy_servers[i] == 0) {
      
      int queue_index = 2 + i;
      for (int j = 0; j < selected_ride_idx; j++) {
        queue_index += state->park->rides[j].server_num;
      }
      
      struct ride_metadata *rideMetadata = malloc(sizeof(struct ride_metadata)) ;
      if(rideMetadata == NULL) {
        fprintf(stderr, "Error in allocating memory for ride metadata, exiting...\n") ;
        exit(1) ;
      }
      rideMetadata->ride_idx = selected_ride_idx;
      rideMetadata->server_idx = i;
      rideMetadata->queue_index = queue_index;

      state->rides[selected_ride_idx].busy_servers[i] = 1;

      struct event *activate_server = createUndiscardableEvent(sim->clock, ride_server_activate_validation, NULL, (void *) rideMetadata);
      add_event_to_simulation(sim, activate_server, queue_index);
      return ;
    }
  }
}

void reach_park(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  if(client_log(state->log))
    printf("launched reach_park at %f\n", sim->clock);
  
  if (state->clients_in_park == state->park->number_of_clients) {

    state->total_clients_arrived++ ; // Next arrival, client is added to the queue

    struct double_value *dv ;
    if((dv = malloc(sizeof(struct double_value))) == NULL) {
      fprintf(stderr, "Error in allocating double value struct. Exiting...\n") ;
      exit(1) ;
    }
    dv->value = sim->clock;
    generic_enqueue_element(state->entrance_queue_arrival_times, dv);
    state->clients_in_queue += 1;
    return;
  }
  if (state->clients_in_queue > 0) {
    struct double_value *dv = (struct double_value *) generic_dequeue_element(state->entrance_queue_arrival_times);
    state->clients_in_queue -= 1;
    state->total_entrance_queue_times_delay += (sim->clock - dv->value);
    free(dv);
  } else {
    state->total_clients_arrived += 1; //Next arrival, empty queue (and park not full) found
  }
  state->clients_in_park += 1;
  
  struct client *me = create_new_client(sim->clock, sim->simEnd, state, sim->until_end);

  int ret = generic_enqueue_element(state->clients, me);
  if (ret != 0) {
    fprintf(stderr, "Error enqueuing client\n");
    exit(1);
  }

  struct event *event = createUndiscardableEvent(sim->clock, choose_delay, NULL, me);
  add_event_to_simulation(sim, event, CLIENT_QUEUE);
}

void next_reach(struct simulation *sim, void *metadata) {
  struct sim_state*state = (struct sim_state*)sim->state;
  // Arrivals are concentrated in the first 5 hours
  if (sim->clock > 300.0) {
    return;
  }
  if(client_log(state->log))
    printf("launched next_reach at %f\n", sim->clock);
  double next = GetRandomFromDistributionType(NEXT_ARRIVAL_STREAM, EXPONENTIAL, 1/(state->park->park_arrival_rate), 0);
  struct event* event = createEvent(sim->clock + next, reach_park, next_reach, NULL);
  int code = add_event_to_simulation(sim, event, CLIENT_QUEUE);
  if(code == 1) {
    free(event) ;
  }
}

void choose_delay(struct simulation* sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  if(client_log(state->log))
    printf("launched choose_delay at %f\n", sim->clock);
  struct client *client = (struct client *) metadata ;

  if (state->park->improved_run) {
    // Go to real reserved queue if there is no time
    for(int i = 0; i < client->max_prenotations; i++) {
      if (client->active_reservations[i].expired) 
        continue;
      int ride_idx = client->active_reservations[i].ride_idx;
      struct ride_state ride = state->rides[ride_idx];
      struct ride park_ride = state->park->rides[ride_idx];
      double estimation = (ride.vip_queue->size + client->active_reservations[i].clients_left) * park_ride.mu / park_ride.server_num;
      if (estimation < state->park->delay_mu) {
        struct client_event *client_ev = malloc(sizeof(struct client_event));
        client_ev->client = client;
        client_ev->arrival_time = sim->clock;
        client_ev->selected_attraction_idx = ride_idx;
        client_ev->event = NULL;        
        generic_insert_element(state->rides[client->active_reservations[i].ride_idx].real_reserved_queue, client_ev, position_in_queue);
        return;
      }
    }
  }

  double delay = 0.0;
  if(state->park->delay_enabled) {
    delay = GetRandomFromDistributionType(DELAY_STREAM, state->park->delay_distribution, state->park->delay_mu, state->park->delay_sigma);
    delay = delay < 0 ? -delay : delay ;
  }

  int condition  = sim->clock + delay > client->exit_time;
  if(state->park->validation_run)
    condition = GetRandomFromDistributionType(POPULARITY_STREAM, UNIFORM, 0, 1) < state->park->exit_probability;

  // Exit Condition
  if (condition) {
    state->total_clients_exited += 1;
    state->total_permanence += (sim->clock - client->arrival_time);
    generic_remove_element(state->clients, client);
    // Remove client from ride reservations
    for (int i = 0; i < state->park->num_rides; i++) {
      generic_remove_element(state->rides[i].reserved_queue, client);
    }
    free(client) ;
    state->clients_in_park -= 1;
    if (state->clients_in_queue > 0) {
      struct event* reach_park_event = createEvent(sim->clock, reach_park, NULL, NULL);
      add_event_to_simulation(sim, reach_park_event, CLIENT_QUEUE) ;
    }
    return ;
  }

  struct event *event = createUndiscardableEvent(sim->clock + delay, choose_attraction, NULL, metadata);
  add_event_to_simulation(sim, event, CLIENT_QUEUE);

  if (state->park->improved_run) {
    for(int j = 0 ; j < client->max_prenotations ; j++) {
      if(client->active_reservations[j].expired) {

        double p ;
        int selected_ride_idx ;
extract_ride:
        p = GetRandomFromDistributionType(RESERVED_STREAM, UNIFORM, 0, 1);
        if (p <= state->popularities[0])
          selected_ride_idx = 0;
        else {
          for (int i = 1; i < state->park->num_rides + state->park->num_shows; i++) {
            if (state->popularities[i - 1] <= p && p <= state->popularities[i]) {
              selected_ride_idx = i;
              break;
            }
          }
        }
    
        // Skip show
        if(selected_ride_idx >= state->park->num_rides) goto extract_ride ;
    
        // Skip ride with active reservations
        for(int i = 0 ; i < client->max_prenotations; i++) {
          if (client->active_reservations[i].ride_idx == selected_ride_idx 
          && !client->active_reservations[i].expired)
            goto extract_ride;
        }

        client->active_reservations[j].ride_idx = selected_ride_idx;
        client->active_reservations[j].clients_left = state->rides[selected_ride_idx].reserved_queue->size+1 ;
        client->active_reservations[j].expired = 0;
        generic_enqueue_element(state->rides[selected_ride_idx].reserved_queue, client) ;
        state->rides[selected_ride_idx].total_reservations += 1;
      }
    }
  }
}
