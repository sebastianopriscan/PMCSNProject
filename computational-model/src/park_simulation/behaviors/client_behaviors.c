#include "behaviors.h"
#include <generic_queue.h>
#include <math.h>
#include <rngs.h>
#include <stdio.h>
#include <stdlib.h>

struct client_event {
  struct client *client;
  struct event *event;
  int selected_ride_idx;
};

void patience_lost(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  struct client_event *client_event = (struct client_event *)metadata;

  if (client_event->client->type == VIP) {
    generic_remove_element(state->rides[client_event->selected_ride_idx].vip_queue, client_event);
  } else {
    generic_remove_element(state->rides[client_event->selected_ride_idx].normal_queue, client_event);
  }
  struct event *event = createEvent(sim->clock, choose_attraction, NULL, client_event->client);
  add_event_to_simulation(sim, event, 0);
}

void choose_attraction(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;

  int selected_ride_idx = -1;
  double p = GetRandomFromDistributionType(2, UNIFORM, 0, 1);

  if (p < state->park->popularities[0])
    selected_ride_idx = 0;
  else {
    for (int i = 1; i < state->park->num_rides + state->park->num_shows; i++) {
      if (state->park->popularities[i - 1] < p &&
          p < state->park->popularities[i]) {
        selected_ride_idx = i;
      }
    }
  }

  if (selected_ride_idx == -1) {
    fprintf(stderr, "Fatal error in selecting ride, exiting...\n");
    exit(1);
  }

  struct client *me = (struct client *)metadata;

  struct client_event *client_ev = malloc(sizeof(struct client_event));
  if (client_ev == NULL) {
    fprintf(stderr, "Error allocating client_event\n");
    exit(1);
  }
  client_ev->client = me;
  client_ev->selected_ride_idx = selected_ride_idx;

  // NOTE: check for patience sigma
  double patience = GetRandomFromDistributionType(0, NORMAL_DISTRIB, me->patience_mu, me->patience_mu * 0.1);

  struct event *lose_patience = createEvent(sim->clock + patience, patience_lost, NULL, client_ev);
  client_ev->event = lose_patience;
  add_event_to_simulation(sim, lose_patience, 1);
  if (me->type == VIP) {
    generic_enqueue_element(state->rides[selected_ride_idx].vip_queue, client_ev);
  } else {
    generic_enqueue_element(state->rides[selected_ride_idx].normal_queue, client_ev);
  }
}

void reach_park(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;

  struct client *me = malloc(sizeof(struct client));
  if (me == NULL) {
    fprintf(stderr, "Error in allocating client size\n");
    exit(1);
  }

  double patience_mu = GetRandomFromDistributionType(0, state->park->patience_distribution, state->park->patience_mu, state->park->patience_sigma);
  double p = GetRandomFromDistributionType(1, UNIFORM, 0, 1);

  me->patience_mu = patience_mu;
  if (p > state->park->vip_tickets_percent) {
    me->type = NORMAL;
    state->total_clients_normal++;
    double iptr;
    if (modf(state->total_clients_normal / state->park->vip_tickets_percent, &iptr) < 0.01 ||
        modf(state->total_clients_normal / state->park->vip_tickets_percent, &iptr) > 0.89) {
      state->available_vip_tickets++;
    }
  } else {
    if (state->available_vip_tickets > 0) {
      state->available_vip_tickets--;
      me->type = VIP;
      state->total_clients_vip += 1;
    }
  }

  int ret = generic_enqueue_element(state->clients, me);
  if (ret != 0) {
    fprintf(stderr, "Error enqueuing client\n");
    exit(1);
  }

  /*
    TODO : Check if adding a delay in going to a ride/show affects theoretical
    results
  */
  struct event *event = createEvent(sim->clock, choose_attraction, NULL, me);
  add_event_to_simulation(sim, event, 0);
}
