#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../computational-model/src/park_simulation/run_simulation.h"
#include "../../computational-model/src/park_simulation/sim_state.h"
#include "../../computational-model/src/lambda-evaluator/lambda_evaluator.h"
#include "../../computational-model/src/models/model.h"
#include "../../computational-model/src/deserializer/deserializer.h"
#include "simulation/simulation.h"
#include <rvms.h>
#include <math.h>

#define NUM_RUNS 64
#define CONFIDENCE 0.95

struct statistics_sample_means
{
  double mean_mean_delay_vip ;
  double sum_mean_delay_vip ;

  double mean_mean_delay_normal ;
  double sum_mean_delay_normal ;

  double mean_mean_delay ;
  double sum_mean_delay ;
  
  double mean_lost_normal;
  double sum_lost_normal;

  double mean_lost_vip;
  double sum_lost_vip;
};

struct statistics_sample_means* stats_means;

void run_evaluator_on_ride(void *args) {
  struct ride *ride = (struct ride *)args;
  
  struct return_value* ret_val = run_lambda_evaluator(ride->expected_wait, 0.5, ride->mu, ride->server_num, ride->batch_size);
  
  ride->popularity = ret_val->lambda ;
  free(ret_val) ;
  return ;
}

void do_run(struct park *park) {
  double mean_normal = 0.0;
  double sum_normal = 0.0;

  double mean_vip = 0.0;
  double sum_vip = 0.0;

  for (int i = 0; i < NUM_RUNS; i++) {
    for (int j = 0; j < park->num_rides; j++) {
      fprintf(stderr, "Started lambda evaluator for ride %d\n", j);
      run_evaluator_on_ride((void *)&park->rides[j]);
    }

    fprintf(stderr, "Lambda evaluator finished\n");

    double total_lambda = 0 ;
    for (int j = 0; j < park->num_rides; j++) {
      total_lambda += park->rides[j].popularity; // contains the temporary value for lambda
    }

    double total_popularity = 0.0;
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
      double delay_diff = 0.0;
      if (ride.total_arrived_normal + ride.total_arrived_vip != 0) 
        delay_diff = (ride.total_delay_normal + ride.total_delay_vip) / (ride.total_arrived_normal + ride.total_arrived_vip) - stats_means[j].mean_mean_delay;
      else
        fprintf(stderr, "Run %d: total_arrived_normal + total_arrived_vip is 0\n", i);
      
      stats_means[j].sum_mean_delay += delay_diff * delay_diff * ((i + 1) - 1.0) / (i + 1);
      stats_means[j].mean_mean_delay += delay_diff / (i + 1);
      
      double normal_delay_diff = 0.0;
      if (ride.total_arrived_normal != 0) 
        normal_delay_diff = (ride.total_delay_normal / ride.total_arrived_normal) - stats_means[j].mean_mean_delay_normal;
      else
        fprintf(stderr, "Run %d, ride %d: total_arrived_normal is 0\n", i, j);
      
      stats_means[j].sum_mean_delay_normal += normal_delay_diff * normal_delay_diff * ((i + 1) - 1.0) / (i + 1);
      stats_means[j].mean_mean_delay_normal += normal_delay_diff / (i + 1);
      
      double vip_delay_diff = 0.0;
      if (ride.total_arrived_vip != 0) 
        vip_delay_diff = (ride.total_delay_vip / ride.total_arrived_vip) - stats_means[j].mean_mean_delay_vip;
      else
        fprintf(stderr, "Run %d, ride %d: total_arrived_vip is 0\n", i, j);
      
      stats_means[j].sum_mean_delay_vip += vip_delay_diff * vip_delay_diff * ((i + 1) - 1.0) / (i + 1);
      stats_means[j].mean_mean_delay_vip += vip_delay_diff / (i + 1);

      double normal_lost_diff = 0.0;
      if (ride.total_arrived_normal != 0) 
        normal_lost_diff = (ride.total_lost_normal / ride.total_arrived_normal) - stats_means[j].mean_lost_normal;
      else
        fprintf(stderr, "Run %d, ride %d: total_arrived_normal is 0\n", i, j);
      
      stats_means[j].sum_lost_normal += normal_lost_diff * normal_lost_diff * ((i + 1) - 1.0) / (i + 1);
      stats_means[j].mean_lost_normal += normal_lost_diff / (i + 1);

      double vip_lost_diff = 0.0;
      if (ride.total_arrived_vip != 0)
        vip_lost_diff = (ride.total_lost_vip / ride.total_arrived_vip) - stats_means[j].mean_lost_vip;
      else
        fprintf(stderr, "Run %d, ride %d: total_arrived_vip is 0\n", i, j);
      
      stats_means[j].sum_lost_vip += vip_lost_diff * vip_lost_diff * ((i + 1) - 1.0) / (i + 1);
      stats_means[j].mean_lost_vip += vip_lost_diff / (i + 1);
    }
    double normal_diff = state->total_clients_normal - mean_normal;
    sum_normal += normal_diff * normal_diff * ((i + 1) - 1.0) / (i + 1);
    mean_normal += normal_diff / (i + 1);

    double vip_diff = state->total_clients_vip - mean_vip;
    sum_vip += vip_diff * vip_diff * ((i + 1) - 1.0) / (i + 1);
    mean_vip += vip_diff / (i + 1);

    delete_sim_state(state) ;
    destroy_simulation(sim) ;
  }
  double u = 1.0 - 0.5 * (1.0 - CONFIDENCE);
  double t = idfStudent(NUM_RUNS - 1, u);
  printf("Name, Mean Delay, stdev Delay, width Delay, Mean Normal Delay, stdev Normal Delay, width Normal Delay, Mean VIP Delay, stdev VIP Delay, width VIP Delay; Mean Lost Normal, stdev Normal Lost, width Normal Lost, VIP Lost Normal, stdev VIP Lost, width VIP Lost\n");
  for (int j = 0; j < park->num_rides; j++) {
    
    double stdev_delay = sqrt(stats_means[j].sum_mean_delay / NUM_RUNS);
    double w_delay = t * stdev_delay / sqrt(NUM_RUNS - 1);
    
    double stdev_delay_normal = sqrt(stats_means[j].sum_mean_delay_normal / NUM_RUNS);
    double w_delay_normal = t * stdev_delay_normal / sqrt(NUM_RUNS - 1);

    double stdev_delay_vip = sqrt(stats_means[j].sum_mean_delay_vip / NUM_RUNS);
    double w_delay_vip = t * stdev_delay_vip / sqrt(NUM_RUNS - 1);
    
    double stdev_lost_normal = sqrt(stats_means[j].sum_lost_normal / NUM_RUNS);
    double w_lost_normal = t * stdev_lost_normal / sqrt(NUM_RUNS - 1);

    double stdev_lost_vip = sqrt(stats_means[j].sum_lost_vip / NUM_RUNS);
    double w_lost_vip = t * stdev_lost_vip / sqrt(NUM_RUNS - 1);
    
    struct statistics_sample_means means = stats_means[j];
    printf("%s, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f\n",
            park->rides[j].name, 
            means.mean_mean_delay, stdev_delay, w_delay, 
            means.mean_mean_delay_normal, stdev_delay_normal, w_delay_normal, 
            means.mean_mean_delay_vip, stdev_delay_vip, w_delay_vip, 
            means.mean_lost_normal, stdev_lost_normal, w_lost_normal, 
            means.mean_lost_vip, stdev_lost_vip, w_lost_vip);
  }
  
  double stdev_normal = sqrt(sum_normal / NUM_RUNS);
  double w_normal = t * stdev_normal / sqrt(NUM_RUNS - 1);

  double stdev_vip = sqrt(sum_vip / NUM_RUNS);
  double w_vip = t * stdev_vip / sqrt(NUM_RUNS - 1);

  printf("\n\nMean Normal Clients, stdev Normal Clients, width Normal Clients, Mean VIP Clients, stdev VIP Clients, width VIP Clients\n");
  printf("%6.6f, %6.6f, %6.6f, %6.6f, %6.6f, %6.6f\n",
            mean_normal, stdev_normal, w_normal, 
            mean_vip, stdev_vip, w_vip);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Incorrect syntax: %s <source.json> <size> <sequence>\n", argv[0]);
    return 1;  
  }

  struct park *park = deserialize(argv[1]);
  if (park == NULL) {
    fprintf(stderr, "Error deserializing park from %s\n", argv[1]);
    return 1;
  }

  long size = strtol(argv[2], NULL, 10);

  stats_means = malloc(sizeof(struct statistics_sample_means) * (park->num_rides +size)) ;

  if(stats_means == NULL) {
    fprintf(stderr, "Error allocating statistics. Exiting...") ;
    return 1 ;
  }

  memset(stats_means, 0, sizeof(struct statistics_sample_means) * (park->num_rides + size)) ;

  if(size == 0) {
    fprintf(stderr, "Default run\n");
    do_run(park) ;
    return 0;
  }

  struct park *newPark = malloc(sizeof(struct park)) ;
  if(newPark == NULL) {
      fprintf(stderr, "Error allocating memory for new park\n") ;
      return 1 ;
  }

  memcpy(newPark, park, sizeof(struct park)) ;
  struct ride* new_rides = realloc(newPark->rides, (newPark->num_rides + size) * sizeof(struct ride));
  if (new_rides == NULL) {
    fprintf(stderr, "Error reallocating memory for new rides\n");
    free(newPark);
    return 1;
  }

  newPark->rides = new_rides ;

  for (int i = 0 ; i < size; i++) {
    long idx = strtol(argv[3 + i], NULL, 10);
    struct ride *subject_ride = &(newPark->disabled_rides[idx]);
    new_rides[newPark->num_rides + i].name = subject_ride->name;
    new_rides[newPark->num_rides + i].popularity = subject_ride->popularity;
    new_rides[newPark->num_rides + i].server_num = subject_ride->server_num;
    new_rides[newPark->num_rides + i].mean_time = subject_ride->mean_time;
    new_rides[newPark->num_rides + i].distribution = subject_ride->distribution;
    new_rides[newPark->num_rides + i].mu = subject_ride->mu;
    new_rides[newPark->num_rides + i].sigma = subject_ride->sigma;
    new_rides[newPark->num_rides + i].expected_wait = subject_ride->expected_wait;
    new_rides[newPark->num_rides + i].batch_size = subject_ride->batch_size ;
    fprintf(stderr, "Added ride %ld\n", idx);
  }
  newPark->num_rides += size;
  do_run(newPark);

  return 0;
}
