#include <stdio.h>
#include <stdlib.h>
#include "lambda_evaluator.h"
#include "../deserializer/deserializer.h"
#include "../models/model.h"
#include "rvms.h"
#include "rngs.h"
#include <math.h>

FILE *lambda_out ;

int main(int argc, char **argv) {

  if(argc != 4) {
    fprintf(stderr, "Usage: ./exName source lambda_out ride_idx") ;
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

  // printf("Name ; sample_lambda ; confidence_lambda ; sample_wait ; confidence_wait ; asked_wait ; lambda_sample_std ; wait_sample_std\n") ;

  fprintf(lambda_out, "Park name ; Lambda ; Wait\n") ;
  int numRuns = 64;
  int ride_idx = strtol(argv[3], NULL, 10);
  if (ride_idx >= park->num_rides) {
    fprintf(stderr, "Incorrect index %d; max value is: %d\n", ride_idx, park->num_rides - 1);
    exit(1);
  }
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
    struct return_value *r = run_lambda_evaluator(park->rides[ride_idx].expected_wait, 0.001, park->rides[ride_idx].mu, park->rides[ride_idx].server_num, park->rides[ride_idx].batch_size);
    lambda_diff = r->lambda - lambda_mean;
    lambda_sum += lambda_diff * lambda_diff * ((q + 1) - 1.0) / (q + 1);
    lambda_mean += lambda_diff / (q+1);
    wait_time_diff = r->wait_time - wait_time_mean;
    wait_time_sum += wait_time_diff * wait_time_diff * ((q + 1) - 1.0) / (q + 1);
    wait_time_mean += wait_time_diff / (q+1);

    fprintf(lambda_out, "%s ; %6.6f; %6.6f\n", park->rides[ride_idx].name, r->lambda, r->wait_time);
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

  printf("%s ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f ; %6.6f\n",
    park->rides[ride_idx].name, lambda, w_lambda, wait_time_mean, w_wait_time, park->rides[ride_idx].expected_wait, stdev_lambda, stdev_wait_time) ;
  return 0;
}