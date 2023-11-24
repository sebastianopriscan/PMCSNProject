#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <simulation/simulation.h>
#include <rngs.h>
#include <rvgs.h>
#include <generic_queue.h>

struct double_value {
  double value ;
} ;

char *rides[12] = {"Alice In Wonderland",
                   "Casey Junior's Circus Tale",
                   "Dumbo",
                   "Small World",
                   "King Arthur",
                   "Mad tea party",
                   "Matterhorn",
                   "Mister toad",
                   "Peter Pan",
                   "Pinocchio",
                   "Snow White",
                   "Storybook" } ;

double expected_waits[12] = {34.0,
                            15.0,
                            28.0,
                            19.0,
                            18.0,
                            17.0,
                            52.0,
                            20.0,
                            39.0,
                            17.0,
                            22.0,
                            16.0} ;

double mus[12] = {4.0,
                  4.0,
                  2.0,
                  14.0,
                  3.0,
                  2.0,
                  4.0,
                  2.0,
                  3.0,
                  3.0,
                  2.0,
                  10.0} ;

int ridesNum[12] = {16,
                    20,
                    16,
                    16,
                    68,
                    18,
                    120,
                    32,
                    18,
                    24,
                    16,
                    14} ;

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
    if (p > 0.5) {
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
    destroy_state(state);
    return sim ;
}

double run_lambda_evaluator(double expected_wait, double threshold, double mu, int server_num) {
  double lambda = 3.1;
  double checker = 0.0;
  double result = 0.0;

  SelectStream(0);
  long seedLambda = 0;
  GetSeed(&seedLambda);

  SelectStream(1);
  long seedMu = 0 ;
  GetSeed(&seedMu) ;
  
  do {
    lambda += 0.005;
    struct simulation *sim = run_single_simulation(lambda, mu, server_num);
    struct simulation_state *state = (struct simulation_state*)(sim->state);
    double total_delay = state->total_delay_normal + state->total_delay_vip ;
    result = total_delay / (state->total_clients_normal + state->total_clients_vip);
    // if (result != 0.0)
    //   fprintf(stderr, "\tLambda: %.4f, Wait: %.6f; %d\n", lambda, result, result < expected_wait);
    checker = (result - expected_wait) > 0 ? result - expected_wait : expected_wait - result ;
    destroy_simulation(sim);
    
    if(checker > threshold && result < expected_wait) {
      SelectStream(0);
      PutSeed(seedLambda);
      SelectStream(1);
      PutSeed(seedMu);
    }
   } while (checker > threshold && result < expected_wait);

  //  SelectStream(0);
  //  PutSeed(seedLambda);
  //  SelectStream(1);
  //  PutSeed(seedMu);
   return lambda;
}

int main(void) {
  int numRuns = 400;
  int i = 0;
  // for(int i = 0; i < 12; i++)
  // {
    PlantSeeds(12345) ;
    double lambda = 0.0;
    for(int q = 0 ; q < numRuns ; q++)
    {
      double l = run_lambda_evaluator(expected_waits[i], 0.5, mus[i], 1);
      fprintf(stderr, "\tRun %d; Lambda: %.6f\n", q, l);
      lambda += l;
    }

    lambda = lambda/numRuns ;
     
    fprintf(stderr, "%s : Lambda: %6.6f, Expected Time %6.6f\n", rides[i], lambda, expected_waits[i]);
    printf("%.4f, %.4f\n", lambda, mus[i]);
    struct simulation *sim = run_single_simulation(lambda, mus[i], 1) ;
    struct simulation_state * state = (struct simulation_state *)sim->state;

    double wait_vip = state->total_delay_vip / state->total_clients_vip;
    double wait_normal = state->total_delay_normal / state->total_clients_normal;
    double wait = 0.5 * wait_vip + 0.5 * wait_normal;
    
    // double total_delay = state->total_delay_normal + state->total_delay_vip - total_arrival;
    // double wait = total_delay / (state->total_clients_normal + state->total_clients_vip);
    fprintf(stderr, "\tAverage Queue Time (vip): %6.6f\n", wait_vip);
    fprintf(stderr, "\tAverage Queue Time (normal): %6.6f\n", wait_normal);
    fprintf(stderr, "\tAverage Queue Time: %6.6f\n", wait);
    fprintf(stderr, "\tNumber of clients: %d\n", state->total_clients_normal + state->total_clients_vip);
  // }
  return 0;
}