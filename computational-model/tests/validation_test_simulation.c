#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simulation/simulation.h>
#include "park_simulation/run_simulation.h"
#include "park_simulation/sim_state.h"
#include "models/model.h"
#include "deserializer/deserializer.h"

// Usage: <program> --single-queue or --single-server or --single
int main(int argc, char **argv) {
  struct park* park = deserialize("../../../computational-model/tests/validation_test.json");
  if (park == NULL)
    return 1;
  if (argc != 2) {
    printf("Incorrect usage: %s --single-queue or %s --single-server or --single\n", argv[0], argv[0]);
    return 1;
  }
  if(!strcmp("--single-queue", argv[1])) {
    // park->park_arrival_rate = 5.106;
    park->park_arrival_rate = 6.0;
    park->simulation_time = 50000.0;
    park->vip_tickets_percent = 0.0;
    for (int i = 0; i < park->num_rides; i++) {
      park->rides[i].popularity = 1.0 / (park->num_rides + park->num_shows) ;
    }
    park->rides[3].server_num = 100;
  } else if (!strcmp("--single-server", argv[1])) {
    park->park_arrival_rate = 0.0004;
    park->simulation_time = 1000000.0;
    park->until_end = 0;
    for (int i = 0; i < park->num_rides; i++) {
      park->rides[i].server_num = 1;
      park->rides[i].popularity = 1.0 / (park->num_rides + park->num_shows) ;
    }
  } else if (!strcmp("--single", argv[1])) {
    park->park_arrival_rate = 0.001;
    park->simulation_time = 1000000.0;
    park->vip_tickets_percent = 0.0;
    park->exit_probability = 0.002778;
    park->until_end = 0;
    for (int i = 0; i < park->num_rides; i++) {
      park->rides[i].server_num = 1;
      park->rides[i].popularity = 1.0 / (park->num_rides + park->num_shows) ;
    }
  }
  struct simulation *sim = run_park_simulation_from_park(park, 0) ;
  if (sim == NULL)
    return 1;
  struct sim_state *state = (struct sim_state *)sim->state;
  
  printf("Name, Normal Queue Time, VIP Queue Time, Lambda, Lambda Normal, Lambda VIP, Service Time 0, Service Time 1, VIP percent\n");
  for(int i = 0; i < state->park->num_rides; i++) {
    struct ride_state ride = state->rides[i];
    printf("%s, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, ", 
          state->park->rides[i].name, 
          ride.total_delay_normal / ride.total_clients_normal, 
          ride.total_delay_vip / ride.total_clients_vip,
          (ride.total_clients_normal + ride.total_clients_vip) / (ride.last_arrival - ride.first_arrival),
          (ride.total_clients_normal) / (ride.last_arrival_normal - ride.first_arrival_normal),
          (ride.total_clients_vip) / (ride.last_arrival_vip - ride.first_arrival_vip));

    for(int j = 0; j < state->park->rides[i].server_num; j++) {
      printf("%6.6f, ", ride.servers_service_means[j]);
    }
    printf("%6.6f\n", ((float)ride.total_clients_vip) / (ride.total_clients_normal + ride.total_clients_vip));
  }
  // printf("%6.6f\n", ((float) state->total_clients_vip) / (state->total_clients_normal + state->total_clients_vip));
  // printf("Mean Permanence: %6.6f\n", state->total_permanence / state->total_clients_exited);

  // printf("Total Entrance Queue Times Delay: %f\n", state->total_entrance_queue_times_delay);
  // printf("Total Clients Arrived: %d\n", state->total_clients_arrived);
  // printf("Total Clients Exited: %d\n", state->total_clients_exited);
  // printf("Total Clients Normal: %d\n", state->total_clients_normal);
  // printf("Total Clients VIP: %d\n", state->total_clients_vip);
  // printf("Clients in Park: %d\n", state->clients_in_park);
  // printf("Clients in Queue: %d\n", state->clients_in_queue);
  // for(int i = 0; i < state->park->num_shows; i++) {
  //   printf("\tShow %d\n", i);
  //   printf("\t\tMean Permanence: %f\n", state->shows[i].total_permanence / state->shows[i].total_clients);
  // }
  // for(int i = 0; i < state->park->num_rides; i++) {
  //   struct ride_state ride = state->rides[i];
  //   printf("\tRide: %s\n", state->park->rides[i].name);

  //   printf("\t\tMean Normal delay: %f\n", ride.total_delay_normal / ride.total_clients_normal) ;
  //   printf("\t\tMean VIP delay: %f\n", ride.total_delay_vip / ride.total_clients_vip) ;
  //   printf("\t\tLambda: %f\n", (ride.total_clients_normal + ride.total_clients_vip) / (ride.last_arrival - ride.first_arrival));
    
  //   for(int j = 0; j < state->park->rides[i].server_num; j++) {
  //     printf("\t\tServer %d Mean Service Time (total clients served: %d); %f\n", j, ride.servers_served_clients[j], ride.servers_service_means[j]);
  //   }
  // }

  delete_sim_state(state) ;
  destroy_simulation(sim) ;
  return 0;
}
