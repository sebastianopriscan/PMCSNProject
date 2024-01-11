#include "behaviors.h"
#include <stdlib.h>

void reach_show(struct simulation* sim, void *metadata) {
  struct sim_state *state = (struct sim_state *) sim->state ;
  struct client_event *client_ev = (struct client_event *) metadata;
  
  double show_end = state->park->shows[client_ev->selected_attraction_idx - state->park->num_rides].length;
  double patience = GetRandomFromDistributionType(6, NORMAL_DISTRIB, client_ev->client->patience_mu, client_ev->client->patience_mu*0.1);
  patience = patience < 0 ? 0 : patience ;
  double next = show_end < patience ? show_end : patience;
  struct event *event = createEvent(sim->clock + next, choose_delay, NULL, client_ev->client);
  add_event_to_simulation(sim, event, 0);
  free(client_ev);
}

// NOTE: show_activate is scheduled at the start of the simulation
void show_activate(struct simulation * sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  int idx = (int) metadata;
  state->active_shows[idx] = 1;
  state->num_active_shows += 1;
  evaluate_attraction_probabilities(state);
  struct event* event = createEvent(sim->clock + state->park->shows[idx].length, show_deactivate, NULL, metadata);
  add_event_to_simulation(sim, event, 1);
}

void show_deactivate(struct simulation *sim, void *metadata) {
  struct sim_state *state = (struct sim_state *)sim->state;
  int idx = (int) metadata;
  state->active_shows[idx] = 0;
  state->num_active_shows -= 1;
  evaluate_attraction_probabilities(state);
}