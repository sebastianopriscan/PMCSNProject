#include "behaviors.h"
#include <stdlib.h>
#include <stdio.h>

#define show_log(log) (log & 0b0010) 

void reach_show(struct simulation* sim, void *metadata) {
  struct sim_state *state = (struct sim_state *) sim->state ;
  if(show_log(state->log))
    printf("launched reach_show at %f\n", sim->clock) ;
  struct client_event *client_ev = (struct client_event *) metadata;

  int show_index = client_ev->selected_attraction_idx - state->park->num_rides;
  state->shows[show_index].total_clients += 1;
  
  double next = state->park->shows[show_index].length;
  if (state->park->patience_enabled) {
    double patience = GetRandomFromDistributionType(PATIENCE_STREAM, NORMAL_DISTRIB, client_ev->client->patience_mu, client_ev->client->patience_mu*0.1);
    patience = patience < 0 ? -patience : patience ;
    next = next < patience ? next : patience;
  }
  
  state->shows[show_index].total_permanence += next;

  struct event *event = createEvent(sim->clock + next, choose_delay, NULL, client_ev->client);
  add_event_to_simulation(sim, event, CLIENT_QUEUE);
  free(client_ev);
}

// NOTE: show_activate is scheduled at the start of the simulation
void show_activate(struct simulation * sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  if (show_log(state->log))
    printf("launched show_activate at %f\n", sim->clock) ;
  int idx = (int) metadata;
  state->active_shows[idx] = 1;
  state->num_active_shows += 1;
  evaluate_attraction_probabilities(state);
  struct event* event = createEvent(sim->clock + state->park->shows[idx].length, show_deactivate, NULL, metadata);
  add_event_to_simulation(sim, event, SHOW_QUEUE);
}

void show_deactivate(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  if (show_log(state->log))
    printf("launched show_deactivate at %f\n", sim->clock) ;
  int idx = (int) metadata;
  state->active_shows[idx] = 0;
  state->num_active_shows -= 1;
  evaluate_attraction_probabilities(state);
}