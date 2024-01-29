#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../../computational-model/src/park_simulation/run_simulation.h"
#include "../../computational-model/src/park_simulation/sim_state.h"
#include "../../computational-model/src/lambda-evaluator/lambda_evaluator.h"
#include "../../computational-model/src/models/model.h"
#include "../../computational-model/src/deserializer/deserializer.h"
#include "simulation/simulation.h"

void *run_evaluator_on_thread(void *args) {
  struct ride *ride = (struct ride *)args;
  
  struct return_value* ret_val = run_lambda_evaluator(ride->expected_wait, 0.5, ride->mu, ride->server_num);
  
  ride->popularity = ret_val->lambda ;
  return NULL ;
}

void do_run(struct park *park) {
  pthread_t* pids = malloc(park->num_rides * sizeof(pthread_t));
  if (pids == NULL) {
    fprintf(stderr, "Error allocating memory for pids\n");
    free(park);
    exit(1);
  }

  for (int j = 0; j < park->num_rides; j++) {
    fprintf(stderr, "Started thread %d\n", j);
    int ret = pthread_create(&pids[j], NULL, run_evaluator_on_thread, (void *)&park->rides[j]);
    if (ret != 0) {
      fprintf(stderr, "Error starting thread for ride %d\n", j);
      free(park);
      free(pids);
      exit(1);
    }
  }

  for (int  j = 0 ; j < park->num_rides ; j++) {
      pthread_join(pids[j], NULL);
  }
  free(pids);
  fprintf(stderr, "Lambda evaluator finished\n");

  double total_lambda = 0 ;
  for (int j = 0; j < park->num_rides; j++) {
    total_lambda += park->rides[j].popularity; // contains the temporary value for lambda
  }

  double total_popularity ;
  total_lambda += total_lambda / (park->num_rides + 1);

  for(int j = 0 ; j < park->num_rides; j++) {
      park->rides[j].popularity = park->rides[j].popularity / total_lambda ;
      total_popularity += park->rides[j].popularity;
  }

  park->shows[0].popularity = 1 - total_popularity ;
  
  // Run production run
  struct simulation* sim = run_park_simulation_from_park(park, 0);
  if (sim == NULL) {
    fprintf(stderr, "Error running simulation\n");
    free(park);
    exit(1);
  }
  struct sim_state *state = (struct sim_state*) sim->state;
  for(int j = 0; j < park->num_rides; j++) {
    struct ride_state ride = state->rides[j];
    printf("\tRide: %d\n", j);
    printf("\t\tTotal Clients Normal: %d\n", ride.total_clients_normal);
    printf("\t\tTotal Clients VIP: %d\n", ride.total_clients_vip);
    printf("\t\tMean Normal lost delay: %f\n", ride.total_lost_normal_delay / ride.total_lost_normal) ;
    printf("\t\tMean VIP lost delay: %f\n", ride.total_lost_vip_delay / ride.total_lost_vip) ;
    printf("\t\tMean Normal delay: %f\n", ride.total_delay_normal / ride.total_clients_normal) ;
    printf("\t\tMean VIP delay: %f\n", ride.total_delay_vip / ride.total_clients_vip) ;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Incorrect syntax: %s <source.json\n", argv[0]);
    return 1;  
  }
  
  struct park *park = deserialize(argv[1]);
  if (park == NULL) {
    fprintf(stderr, "Error deserializing park from %s\n", argv[1]);
    return 1;
  }

  printf("Default run\n");
  do_run(park) ;
  
  for (int i = 0 ; i < park->num_disabled_rides ; i++) {
    struct park *newPark = malloc(sizeof(struct park)) ;
    if(newPark == NULL) {
        fprintf(stderr, "Error allocating memory for new park\n") ;
        return 1 ;
    }

    memcpy(newPark, park, sizeof(struct park)) ;

    struct ride* new_rides = realloc(newPark->rides, (newPark->num_rides + 1) * sizeof(struct ride));
    if (new_rides == NULL) {
      fprintf(stderr, "Error reallocating memory for new rides\n");
      free(newPark);
      return 1;
    }
    struct ride *subject_ride = &(newPark->disabled_rides[i]);
    new_rides[newPark->num_rides].name = subject_ride->name;
    new_rides[newPark->num_rides].popularity = subject_ride->popularity;
    new_rides[newPark->num_rides].server_num = subject_ride->server_num;
    new_rides[newPark->num_rides].mean_time = subject_ride->mean_time;
    new_rides[newPark->num_rides].distribution = subject_ride->distribution;
    new_rides[newPark->num_rides].mu = subject_ride->mu;
    new_rides[newPark->num_rides].sigma = subject_ride->sigma;
    new_rides[newPark->num_rides].expected_wait = subject_ride->expected_wait;
    newPark->num_rides += 1;

    printf("Added ride %d\n", i);
    do_run(newPark);
  }

  return 0;
}