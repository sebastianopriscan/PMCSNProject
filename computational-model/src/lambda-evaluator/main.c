#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <simulation/simulation.h>
#include <rngs.h>
#include <rvgs.h>
#include <rvms.h>
#include <generic_queue.h>
#include <math.h>
#include "../deserializer/deserializer.h"
#include "../models/model.h"

struct return_value {
  double lambda;
  double wait_time;
};

struct simulation_state {
  double lmba ;
  double mu ;
  double sigma ;
  unsigned int n_Clients ;
  unsigned int vip_Clients ;
  unsigned int n_of_Servers ;
  unsigned char *servers_status ;

  unsigned int total_clients_normal;
  unsigned int total_clients_vip;
  double total_service_time_normal;
  double total_service_time_vip ;
  double total_delay_normal;
  double total_delay_vip;

  struct generic_queue_list *vip_arrivals;
  struct generic_queue_list *normal_arrivals;
} ;

struct simulation_state *init_state(unsigned int servNum, double lmba, double mu, double sigma)
{
  struct simulation_state *val;
  if((val = malloc(sizeof(struct simulation_state))) == NULL)  {
    perror("Failed to allocate simulation state");
    exit(1);
  }

  if ((val->servers_status = malloc(sizeof(char) * servNum)) == NULL) {
    perror("Failed to allocate servers_status");
    free(val);
    exit(1);
  }
  
  val->lmba = lmba;
  val->mu = mu;
  val->sigma = sigma ;
  val->n_Clients = 0 ;
  val->n_of_Servers = servNum ;
  val->vip_Clients = 0 ;
  val->total_clients_normal = 0;
  val->total_clients_vip = 0;
  val->total_delay_normal = 0.0;
  val->total_delay_vip = 0.0;
  val->total_service_time_normal = 0.0;
  val->total_service_time_vip = 0.0;

  for (int i = 0; i < servNum; i++)
  {
    val->servers_status[i] = 0;
  }

  val->vip_arrivals = create_queue_list() ;
  val->normal_arrivals = create_queue_list();

  if(val->vip_arrivals == NULL || val->normal_arrivals == NULL) {
    perror("Error in allocating vip or normal queues. Exiting...") ;
    exit(1) ;
  }
  
  return val;
}

void destroy_state(struct simulation_state *state)
{
  destroy_generic_queue_list(state->vip_arrivals);
  destroy_generic_queue_list(state->normal_arrivals);
  free(state->servers_status) ;
  free(state) ;
}

void server_activate(struct simulation *sim, void *metadata)
{
  int i = (int) metadata;
  SelectStream(i + 1) ;
  struct simulation_state *state = (struct simulation_state*) sim->state;

  double service_time = 0.0;
  
  if (state ->vip_Clients >0 || state->n_Clients > 0)
    service_time = Exponential(1.0 / state->mu) ;
  //  service_time = Normal(state->mu, state->sigma) ;

  if(state->vip_Clients > 0) {
    state->vip_Clients -= 1;
    struct double_value *value = (struct double_value *) generic_dequeue_element(state->vip_arrivals);
    if (value == NULL) {
      perror("Error while dequeue-ing from vip_arrivals. Exiting...");
      exit(1);
    }
    state->total_delay_vip += sim->clock - value->value;
    free(value) ;
    state->total_service_time_vip += service_time;
    state->total_clients_vip += 1;
  }
  else if (state->n_Clients > 0) {
    state->n_Clients -= 1;
    struct double_value *value = (struct double_value *) generic_dequeue_element(state->normal_arrivals);
    if (value == NULL) {
      perror("Error while dequeue-ing from normal_arrivals. Exiting...");
      exit(1);
    }
    state->total_delay_normal += sim->clock - value->value;
    free(value);
    state->total_service_time_normal += service_time ;
    state->total_clients_normal += 1;
  }
  else {
    state->servers_status[i] = 0;
    return ;
  }

  state->servers_status[i] = 1;

  double next = sim->clock + service_time;

  struct event* event = createEvent(next, server_activate, NULL, metadata);
  add_event_to_simulation(sim, event, i+1);
}

void arrivalPayload(struct simulation *sim, void *metadata) {

    struct simulation_state *state = (struct simulation_state *)(sim->state) ;

    SelectStream(255);

    struct double_value *value = malloc(sizeof(struct double_value));
    if(value == NULL) {
      perror("Error in allocating value in memory. Exiting...") ;
      exit(1) ;
    }
    value->value = sim->clock;

    double p = Uniform(0.0, 1.0);
    if (p <= 0.1) {
      state->vip_Clients += 1;
      if(generic_enqueue_element(state->vip_arrivals, value)) {
        perror("Error in enqueue-ing to vip_arrivals. Exiting...") ;
        exit(1) ;
      }

    }else {
      state->n_Clients += 1 ;
      if(generic_enqueue_element(state->normal_arrivals, value)) {
        perror("Error in enqueue-ing to normal_arrivals. Exiting...") ;
        exit(1) ;
      }
    }

    for(int i = 0 ; i < state->n_of_Servers ; i++)
    {
      if(state->servers_status[i] == 0)
      {
        struct event * event = createEvent(sim->clock, server_activate, NULL, (void *)i) ;
        add_event_to_simulation(sim, event, i+1);
        break;
      }
    }
}

void next_arrival(struct simulation *sim, void *metadata) {

    SelectStream(0) ;

    double inter = Exponential(1.0/((struct simulation_state *)(sim->state))->lmba) ;

    double time = sim->clock + inter;

    struct event *event1 = createEvent(time, arrivalPayload, next_arrival, NULL) ;

    add_event_to_simulation(sim, event1, 0);
}

struct simulation *run_single_simulation(double lambda, double mu, int server_num) {
    struct simulation_state* state = init_state(server_num, lambda, mu, 0.1 * mu);
    struct simulation *sim = create_simulation(server_num + 1, 960, (char *) state);
    struct event *event = createEvent(0.0, next_arrival, NULL, NULL);
    add_event_to_simulation(sim, event, 0);
    run_simulation(sim);
    return sim ;
}

struct return_value* run_lambda_evaluator(double expected_wait, double threshold, double mu, int server_num) {
  double lambda = 1.0;
  double checker = 0.0;
  double real_wait = 0.0;
  double delta = 3.0 ;

  double lambda_lower_bound = 0.0;
  double lambda_upper_bound = 0.0;

  SelectStream(0);
  long seedLambda = 0;
  GetSeed(&seedLambda);

  SelectStream(1);
  long seedMu = 0 ;
  GetSeed(&seedMu) ;
 
  double direction = 0 ;

  //find upper and lower bounds for lambda
  do {
    struct simulation *sim = run_single_simulation(lambda, mu, server_num);
    struct simulation_state *state = (struct simulation_state*)(sim->state);
    double total_delay = state->total_delay_normal + state->total_delay_vip ;
    real_wait = total_delay / (state->total_clients_normal + state->total_clients_vip);

    if (direction == 0) {
      if(real_wait > expected_wait) {
        lambda_upper_bound = lambda ;
        direction = -1.0 ;
      }
      else {
        lambda_lower_bound = lambda ;
        direction = 1.0 ;
      }
      lambda = lambda + direction*delta < 0 ? lambda + direction * delta / 2 : lambda + direction * delta ;
      continue;
    }

    if(direction == -1.0 && real_wait < expected_wait) {
      lambda_lower_bound = lambda;
      break;
    } else if (direction == 1.0 && real_wait > expected_wait) {
      lambda_upper_bound = lambda;
      break;
    }

    lambda = lambda + direction*delta < 0 ? lambda + direction * delta / 2 : lambda + direction * delta ;
    destroy_state(state);
    destroy_simulation(sim);
  } while(1);

  SelectStream(0);
  PutSeed(seedLambda);
  SelectStream(1);
  PutSeed(seedMu);

  //Newton-like approximation method
  do {
    lambda = (lambda_upper_bound + lambda_lower_bound) / 2.0;

    struct simulation *sim = run_single_simulation(lambda, mu, server_num);
    struct simulation_state *state = (struct simulation_state*)(sim->state);
    double total_delay = state->total_delay_normal + state->total_delay_vip ;
    real_wait = total_delay / (state->total_clients_normal + state->total_clients_vip);
    if(lambda_upper_bound - lambda_lower_bound < threshold)
      break;

    if (real_wait > expected_wait) {
      lambda_upper_bound = lambda;
    } else {
      lambda_lower_bound = lambda;
    }

    checker = (real_wait - expected_wait) > 0 ? real_wait - expected_wait : expected_wait - real_wait ;
    destroy_state(state);
    destroy_simulation(sim);
    
    if(checker > threshold) {
      SelectStream(0);
      PutSeed(seedLambda);
      SelectStream(1);
      PutSeed(seedMu);
    }
  } while (checker > threshold);

  fprintf(stderr, "Lambda: %6.6f; Wait Time: %6.6f\n", lambda, real_wait);
  printf("%6.6f\n", lambda);
  
  struct return_value *retVal ;
  if((retVal = malloc(sizeof( struct return_value))) == NULL) {
    exit(1) ;
  }
  retVal->lambda = lambda;
  retVal->wait_time = real_wait;
  return retVal;
}

int main(void) {
  struct park *park = deserialize("");
  if (park == NULL) {
    fprintf(stderr, "Error loading json\n");
    exit(1);
  }
  int numRuns = 1000;
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
      fprintf(stderr, "\tRun %d; ", q) ;
      struct return_value *r = run_lambda_evaluator(park->rides[i].expected_wait, 0.001, park->rides[i].mu, park->rides[i].server_num);
      lambda_diff = r->lambda - lambda_mean;
      lambda_sum += lambda_diff * lambda_diff * ((q + 1) - 1.0) / (q + 1);
      lambda_mean += lambda_diff / (q+1);
      wait_time_diff = r->wait_time - wait_time_mean;
      wait_time_sum += wait_time_diff * wait_time_diff * ((q + 1) - 1.0) / (q + 1);
      wait_time_mean += wait_time_diff / (q+1);

      free(r) ;
    }

    double u = 1.0 - 0.5 * (1.0 - 0.95);
    double t = idfStudent(numRuns - 1, u);

    lambda = lambda_mean;
    double stdev_lambda = sqrt(lambda_sum / numRuns);
    double w_lambda = t * stdev_lambda / sqrt(numRuns - 1);
    double stdev_wait_time = sqrt(wait_time_sum / numRuns);
    double w_wait_time = t * stdev_wait_time / sqrt(numRuns - 1);
    
    fprintf(stderr, "%s : Lambda: %6.6f, Expected Time %6.6f\n", park->rides[i].name, lambda, park->rides[i].expected_wait);
    fprintf(stderr, "Lambda: Expected value is in the interval: %6.6f +/- %6.6f; stdev: %6.6f\n", lambda_mean, w_lambda, stdev_lambda);
    fprintf(stderr, "Wait Time: Expected value is in the interval: %6.6f +/- %6.6f; stdev: %6.6f\n", wait_time_mean, w_wait_time, stdev_wait_time);

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