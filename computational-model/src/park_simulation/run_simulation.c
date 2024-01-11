#include "behaviors/behaviors.h"
#include "simulation/simulation.h"
#include "../models/model.h"
#include "../deserializer/deserializer.h"

#include <stdio.h>
#include <stdlib.h>

int run_park_simulation(const char *path) {
  struct park* park = deserialize(path);
  if (park == NULL) {
    return 1;
  }
  int num_queues = 2;
  for (int i = 0; i < park->num_rides; i++) {
    num_queues += park->rides[i].server_num;
  }


  struct sim_state* sim_state = create_sim_state(park);
  struct simulation* sim = create_simulation(num_queues, park->simulation_time, sim_state);

  struct event *first_arrival = createEvent(0, reach_park, next_reach, NULL) ;
  add_event_to_simulation(sim, first_arrival, 0);
  for (int i = 0; i < park->num_shows; i++){
    for(int j = 0 ; j < park->shows[i].num_starting_times; j++) {

        struct event *show_activate_event = createEvent(park->shows[i].starting_times[j], show_activate, NULL, (void *) i) ;
        add_event_to_simulation(sim, show_activate_event, 1) ;
    }
  }
  run_simulation(sim);
  printf("Number of total clients: %d\n", sim_state->total_clients);
  printf("Number of total clients exited: %d\n", sim_state->total_clients_exited);
  delete_sim_state(sim->state);
  destroy_simulation(sim);
  
  return 0;
}

// per ride: n server
// 1 coda per show (activate, deactivate)
// coda clienti (arrivi e del pool)

//Clienti             : 0
//Show                : 1
//Giostra i, server k : 2 + sum(j in 0..i-1){ride[j]*serverNum[j]} + k

// Streams
// 0: next arrival
// 1: patience_mu
// 2: client type
// 3: exit time
// 4: delay
// 5: popularity
// 6: patience