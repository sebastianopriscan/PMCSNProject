#include "behaviors.h"
#include <stdlib.h>

void show_server_activate(struct simulation* sim, void *metadata) {
  struct sim_state *state = (struct sim_state *) sim->state ;
  struct client_event *client_ev = (struct client_event *) metadata;
  
  // double show_end = state->park->shows[client_ev->selected_attraction_idx].length; TODO: implement show duration
  double show_end = 10.0;
  double patience = GetRandomFromDistributionType(0, NORMAL_DISTRIB, client_ev->client->patience_mu, client_ev->client->patience_mu*0.1);
  double next = show_end < patience ? show_end : patience;
  struct event *event = createEvent(sim->clock + next, choose_delay, NULL, client_ev->client);
  add_event_to_simulation(sim, event, 0);
}