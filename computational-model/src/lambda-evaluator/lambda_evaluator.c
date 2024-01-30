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

#include "lambda_evaluator.h"

struct simulation_state {
  int batch_size;
  double lmba ;
  double mu ;
  double sigma ;
  unsigned int n_Clients ;
  unsigned int vip_Clients ;
  unsigned int n_of_Servers ;

  unsigned int total_clients_normal;
  unsigned int total_clients_vip;
  double total_service_time_normal;
  double total_service_time_vip ;
  double total_delay_normal;
  double total_delay_vip;

  struct generic_queue_list *vip_arrivals;
  struct generic_queue_list *normal_arrivals;
} ;

struct simulation_state *init_state(unsigned int servNum, int batch_size, double lmba, double mu, double sigma)
{
  struct simulation_state *val;
  if((val = malloc(sizeof(struct simulation_state))) == NULL)  {
    perror("Failed to allocate simulation state");
    exit(1);
  }
  
  val->batch_size = batch_size;
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
  void *toFree ;
  do {
    toFree = generic_dequeue_element(state->vip_arrivals) ;
    free(toFree) ;
  } while(toFree != NULL) ;
  do {
    toFree = generic_dequeue_element(state->normal_arrivals) ;
    free(toFree) ;
  } while(toFree != NULL) ;

  destroy_generic_queue_list(state->vip_arrivals);
  destroy_generic_queue_list(state->normal_arrivals);
  free(state) ;
}

void server_activate(struct simulation *sim, void *metadata)
{
  int i = (int) metadata;
  SelectStream(i + 1) ;
  struct simulation_state *state = (struct simulation_state*) sim->state;

  double service_time = Normal(state->mu, state->sigma) ;

  int actual_served = 0;
  if(state->vip_Clients > 0) {
    for (int j = 0; j < state->batch_size; j++) {
      state->vip_Clients -= 1;
      struct double_value *value = (struct double_value *) generic_dequeue_element(state->vip_arrivals);
      if (value == NULL) {
        break;
      }
      state->total_delay_vip += sim->clock - value->value;
      free(value) ;
      state->total_service_time_vip += service_time;
      state->total_clients_vip += 1;
      actual_served ++;
    }
  }
  else if (state->n_Clients > 0) {
    for (int j = 0; j < state->batch_size - actual_served; j++) {
      state->n_Clients -= 1;
      struct double_value *value = (struct double_value *) generic_dequeue_element(state->normal_arrivals);
      if (value == NULL) {
        break;
      }
      state->total_delay_normal += sim->clock - value->value;
      free(value);
      state->total_service_time_normal += service_time ;
      state->total_clients_normal += 1;
    }
  }

  double next = sim->clock + service_time;

  struct event* event = createEvent(next, server_activate, NULL, metadata);
  int code = add_event_to_simulation(sim, event, i+1);
  if(code == 1) {
    free(event) ;

  }
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
}

void next_arrival(struct simulation *sim, void *metadata) {

    SelectStream(0) ;

    double inter = Exponential(1.0/((struct simulation_state *)(sim->state))->lmba) ;

    double time = sim->clock + inter;

    struct event *event1 = createEvent(time, arrivalPayload, next_arrival, NULL) ;

    int code = add_event_to_simulation(sim, event1, 0);
    if(code == 1) {
      free(event1) ;
    }
}

struct simulation *run_single_simulation(double lambda, double mu, int server_num, int batch_size) {
    struct simulation_state* state = init_state(server_num, batch_size, lambda, mu, 0.1 * mu);
    struct simulation *sim = create_simulation(server_num / batch_size + 1, 720, (char *) state);
    struct event *event = createEvent(0.0, next_arrival, NULL, NULL);
    add_event_to_simulation(sim, event, 0);
    for (int i = 0; i < server_num / batch_size; i++) {
      struct event *ride_event = createEvent(0.0, server_activate, NULL, (void *) i);
      add_event_to_simulation(sim, ride_event, 0);
    }
    run_simulation(sim);
    return sim ;
}

struct return_value* run_lambda_evaluator(double expected_wait, double threshold, double mu, int server_num, int batch_size) {
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
    struct simulation *sim = run_single_simulation(lambda, mu, server_num, batch_size);
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
      destroy_state(state);
      destroy_simulation(sim);
      continue;
    }

    if(direction == -1.0 && real_wait < expected_wait) {
      lambda_lower_bound = lambda;
      destroy_state(state);
      destroy_simulation(sim);
      break;
    } else if (direction == 1.0 && real_wait > expected_wait) {
      lambda_upper_bound = lambda;
      destroy_state(state);
      destroy_simulation(sim);
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

    struct simulation *sim = run_single_simulation(lambda, mu, server_num, batch_size);
    struct simulation_state *state = (struct simulation_state*)(sim->state);
    double total_delay = state->total_delay_normal + state->total_delay_vip ;
    real_wait = total_delay / (state->total_clients_normal + state->total_clients_vip);
    if(lambda_upper_bound - lambda_lower_bound < threshold) {
      destroy_state(state);
      destroy_simulation(sim);
      break;
    }
      
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
  
  struct return_value *retVal ;
  if((retVal = malloc(sizeof( struct return_value))) == NULL) {
    exit(1) ;
  }
  retVal->lambda = lambda;
  retVal->wait_time = real_wait;
  return retVal;
}