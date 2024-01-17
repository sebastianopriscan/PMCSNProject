#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <simulation/simulation.h>
#include "park_simulation/run_simulation.h"
#include "park_simulation/sim_state.h"

// Usage: <program> --verbose 1000/100/10/1
int main(int argc, char **argv) {
  int log = 0b1000;
  if (argc == 3 && (!strcmp("--verbose", argv[1]) || !strcmp("-v", argv[1]))) {
    log = strtol(argv[2], NULL, 2);
    if (errno == ERANGE) {
      perror("Error converting verbose input to log");
      return ERANGE;
    }
  }
  struct simulation *sim = run_park_simulation("../../../computational-model/tests/validation_test.json", log) ;
  if (sim == NULL)
    return 1;
  struct sim_state *state = (struct sim_state *)sim->state;

  printf("Total Entrance Queue Times Delay: %f\n", state->total_entrance_queue_times_delay);
  printf("Total Clients Arrived: %d\n", state->total_clients_arrived);
  printf("Total Clients Exited: %d\n", state->total_clients_exited);
  printf("Total Clients Normal: %d\n", state->total_clients_normal);
  printf("Total Clients VIP: %d\n", state->total_clients_vip);
  printf("Clients in Park: %d\n", state->clients_in_park);
  printf("Clients in Queue: %d\n", state->clients_in_queue);
  for(int i = 0; i < state->park->num_shows; i++) {
    printf("\tShow %d\n", i);
    printf("\t\tMean Permanence: %f\n", state->shows[i].total_permanence / state->shows[i].total_clients);
  }
  for(int i = 0; i < state->park->num_rides; i++) {
    struct ride_state ride = state->rides[i];
    printf("\tRide: %d\n", i);

    printf("\t\tMean Normal delay: %f\n", ride.total_delay_normal / ride.total_clients_normal) ;
    printf("\t\tMean VIP delay: %f\n", ride.total_delay_vip / ride.total_clients_vip) ;
    printf("\t\tLambda: %f\n", (ride.total_clients_normal + ride.total_clients_vip) / (ride.last_arrival - ride.first_arrival));
    
    for(int j = 0; j < state->park->rides[i].server_num; j++) {
      printf("\t\tServer %d Mean Service Time (total clients served: %d); %f\n", j, ride.servers_served_clients[j], ride.servers_service_means[j]);
    }
  }

  delete_sim_state(state) ;
  destroy_simulation(sim) ;
  return 0;
}