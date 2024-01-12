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
  if (sim_state == NULL)
    exit(1);

  struct simulation* sim = create_simulation(num_queues, park->simulation_time, sim_state);
  if(sim == NULL) {
    fprintf(stderr, "Error allocating memory for simulation\n");
    exit(1);
  }

  struct event *first_arrival = createEvent(0, reach_park, next_reach, NULL) ;
  add_event_to_simulation(sim, first_arrival, CLIENT_QUEUE);
  for (int i = 0; i < park->num_shows; i++){
    for(int j = 0 ; j < park->shows[i].num_starting_times; j++) {

        struct event *show_activate_event = createEvent(park->shows[i].starting_times[j], show_activate, NULL, (void *) i) ;
        add_event_to_simulation(sim, show_activate_event, SHOW_QUEUE) ;
    }
  }
  run_simulation(sim);
  fflush(stdout);
  fprintf(stderr, "Total Entrance Queue Times Delay: %f\n", sim_state->total_entrance_queue_times_delay);
  fprintf(stderr, "Total Clients Arrived: %d\n", sim_state->total_clients_arrived);
  fprintf(stderr, "Total Clients Exited: %d\n", sim_state->total_clients_exited);
  fprintf(stderr, "Total Clients Normal: %d\n", sim_state->total_clients_normal);
  fprintf(stderr, "Total Clients VIP: %d\n", sim_state->total_clients_vip);
  fprintf(stderr, "Clients in Park: %d\n", sim_state->clients_in_park);
  fprintf(stderr, "Clients in Queue: %d\n", sim_state->clients_in_queue);
  for(int i = 0; i < sim_state->park->num_shows; i++) {
    fprintf(stderr, "\tShow %d\n", i);
    fprintf(stderr, "\t\tTotal Clients: %d\n", sim_state->shows[i].total_clients);
    fprintf(stderr, "\t\tTotal Permanence: %f\n", sim_state->shows[i].total_permanence);
    fprintf(stderr, "\t\tMean Permanence: %f\n", sim_state->shows[i].total_permanence / sim_state->shows[i].total_clients);
  }
  for(int i = 0; i < sim_state->park->num_rides; i++) {
    struct ride_state ride = sim_state->rides[i];
    fprintf(stderr, "\tRide: %d\n", i);
    fprintf(stderr, "\t\tTotal Clients Normal: %d\n", ride.total_clients_normal);
    fprintf(stderr, "\t\tTotal Clients VIP: %d\n", ride.total_clients_vip);
    fprintf(stderr, "\t\tTotal Lost Normal: %d\n", ride.total_lost_normal);
    fprintf(stderr, "\t\tTotal Lost VIP: %d\n", ride.total_lost_vip);
    fprintf(stderr, "\t\tTotal Delay Normal: %f\n", ride.total_delay_normal);
    fprintf(stderr, "\t\tTotal Delay VIP: %f\n", ride.total_delay_vip);
    fprintf(stderr, "\t\tTotal Service Time: %f\n", ride.global_service_mean);

    fprintf(stderr, "\t\tMean Normal lost delay: %f\n", ride.total_lost_normal_delay / ride.total_lost_normal) ;
    fprintf(stderr, "\t\tMean VIP lost delay: %f\n", ride.total_lost_vip_delay / ride.total_lost_vip) ;
    fprintf(stderr, "\t\tMean Normal delay: %f\n", ride.total_delay_normal / (ride.total_clients_normal - ride.total_lost_normal)) ;
    fprintf(stderr, "\t\tMean VIP delay: %f\n", ride.total_delay_vip / (ride.total_clients_vip - ride.total_lost_vip)) ;
    fprintf(stderr, "\t\tLambda: %f\n", 1 / (ride.last_arrival - ride.first_arrival));
    
    for(int j = 0; j < sim_state->park->rides[i].server_num; j++) {
      fprintf(stderr, "\t\tServer %d Mean Service Time (total clients served: %d); %f\n", j, ride.servers_served_clients[j], ride.servers_service_means[j]);
    }
  }
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