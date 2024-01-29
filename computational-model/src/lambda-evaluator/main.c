#include <stdio.h>
#include "lambda_evaluator.h"
#include "../models/model.h"
#include "rvms.h"

FILE *lambda_out ;

int main(int argc, char **argv) {

  if(argc != 3) {
    fprintf(stderr, "Usage: ./exName source lambda_out") ;
    exit(1) ;
  }

  struct park *park = deserialize(argv[1]);
  if (park == NULL) {
    fprintf(stderr, "Error loading json\n");
    exit(1);
  }

  if((lambda_out = fopen(argv[2], "w+")) == NULL) {
    perror("Error opening lambda output file: ");
    exit(1);
  }

  printf("Name ; sample_lambda ; confidence_lambda ; sample_wait ; confidence_wait ; asked_wait ; lambda_sample_std ; wait_sample_std\n") ;

  fprintf(lambda_out, "Park name ; Lambda ; Wait\n") ;
  int numRuns = 10;
  for (int i = 0; i < park->num_rides; i++) {
    PlantSeeds(12345) ;
    double lambda = 0.0;
    double lambda_diff = 0.0;
    double lambda_mean = 0.0;
    double lambda_sum = 0.0;
    double wait_time = 0.0;
    double wait_time_diff = 0.0;
    double wait_time_mean = 0.0;
    double wait_time_sum = 0.0;
    for(int q = 0 ; q < numRuns ; q++)
    {
      struct return_value *r = run_lambda_evaluator(park->rides[i].expected_wait, 0.001, park->rides[i].mu, park->rides[i].server_num);
      lambda_diff = r->lambda - lambda_mean;
      lambda_sum += lambda_diff * lambda_diff * ((q + 1) - 1.0) / (q + 1);
      lambda_mean += lambda_diff / (q+1);
      wait_time_diff = r->wait_time - wait_time_mean;
      wait_time_sum += wait_time_diff * wait_time_diff * ((q + 1) - 1.0) / (q + 1);
      wait_time_mean += wait_time_diff / (q+1);

      fprintf(lambda_out, "%s ; %6.6f; %6.6f\n", park->rides[i].name, r->lambda, r->wait_time);
      fflush(lambda_out);

      free(r) ;
    }

    double u = 1.0 - 0.5 * (1.0 - 0.95);
    double t = idfStudent(numRuns - 1, u);

    lambda = lambda_mean;
    double stdev_lambda = sqrt(lambda_sum / numRuns);
    double w_lambda = t * stdev_lambda / sqrt(numRuns - 1);
    double stdev_wait_time = sqrt(wait_time_sum / numRuns);
    double w_wait_time = t * stdev_wait_time / sqrt(numRuns - 1);
    
    // fprintf(stderr, "%s : Lambda: %6.6f, Expected Time %6.6f\n", park->rides[i].name, lambda, park->rides[i].expected_wait);
    // fprintf(stderr, "Lambda: Expected value is in the interval: %6.6f +/- %6.6f; stdev: %6.6f\n", lambda_mean, w_lambda, stdev_lambda);
    // fprintf(stderr, "Wait Time: Expected value is in the interval: %6.6f +/- %6.6f; stdev: %6.6f\n", wait_time_mean, w_wait_time, stdev_wait_time);

    printf("%s ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f\n",
     park->rides[i].name, lambda, w_lambda, wait_time_mean, w_wait_time, park->rides[i].expected_wait, stdev_lambda, stdev_wait_time) ;

    // struct simulation *sim = run_single_simulation(lambda, mus[i], 1) ;
    // struct simulation_state * state = (struct simulation_state *)sim->state;

    // double wait_vip = state->total_delay_vip / state->total_clients_vip;
    // double wait_normal = state->total_delay_normal / state->total_clients_normal;
    // double total_delay = state->total_delay_normal + state->total_delay_vip ;
    // double wait = total_delay / (state->total_clients_normal + state->total_clients_vip);
    
    // double total_delay = state->total_delay_normal + state->total_delay_vip - total_arrival;
    // double wait = total_delay / (state->total_clients_normal + state->total_clients_vip);
    // fprintf(stderr, "\tAverage Queue Time (vip): %6.6f\n", wait_vip);
    // fprintf(stderr, "\tAverage Queue Time (normal): %6.6f\n", wait_normal);
    // fprintf(stderr, "\tAverage Queue Time: %6.6f\n", wait);
    // fprintf(stderr, "\tNumber of clients: %d\n", state->total_clients_normal + state->total_clients_vip);
  }
  return 0;
}