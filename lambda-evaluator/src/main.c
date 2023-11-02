#include <stdio.h>
#include <stdlib.h>
#include <simulation/simulation.h>
#include <rngs.h>
#include <rvgs.h>

struct simulation_state {
  double lmba ;
  double mu ;
  double sigma ;
  unsigned int n_Clients ;
  unsigned int vip_Clients ;
  unsigned int n_of_Servers ;
  unsigned char *servers_status ;

} ;

void server_activate(struct simulation *sim, void *metadata);

struct simulation_state *init_state(unsigned int servNum, double lmba, double mu, double sigma)
{
  struct simulation_state *val;
  if((val = malloc(sizeof(struct simulation_state))))  {
    perror("Failed to allocate simulation state");
    exit(1);
  }

  if ((val->servers_status = malloc(sizeof(char) * servNum))) {
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
  for (int i = 0; i < servNum; i++)
  {
    val->servers_status[i] = 0;
  }
  
  return val;
}

void destroy_state(struct simulation_state *state)
{
  free(state->servers_status) ;
  free(state) ;
}

void server_wakeup(struct simulation *sim, void *metadata)
{
  int i = (int) metadata;
  struct simulation_state *state = (struct simulation_state*) sim->state;
  
  if(state->vip_Clients > 0)
    state->vip_Clients -= 1;
  else if (state->n_Clients > 0)
    state->n_Clients -= 1;
  else {
    state->servers_status[i] = 0;
    return;
  }

  event *event = createEvent(sim->clock, server_activate, NULL, metadata);
  struct event_queue *queue = sim->queues->next;
  for (int j = 0; j < i; j++)
  {
    queue = sim->queues->next;
  }
  
  enqueue_event(queue, event);
}

void server_activate(struct simulation *sim, void *metadata)
{
  int i = (int) metadata;
  SelectStream(i +1) ;
  struct simulation_state *state = (struct simulation_state*) sim->state;
  state->servers_status[i] = 1;

  double service_time = Normal(state->mu, state->sigma);

  double next = sim->clock + service_time;

  event* event = createEvent(next, server_wakeup, NULL, metadata);
  struct event_queue *queue = sim->queues->next;
  for (int j = 0; j < i; j++)
  {
    queue = sim->queues->next;
  }
  
  enqueue_event(queue, event);
}

void arrivalPayload(struct simulation *sim, void *metadata) {

    struct simulation_state *state = (struct simulation_state *)(sim->state) ;

    SelectStream(255);

    double p = Uniform(0.0, 1.0);
    if (p > 0.5)
      state->vip_Clients += 1;
    else
      state->n_Clients += 1 ;

    for(int i = 0 ; i < state->n_of_Servers ; i++)
    {
      if(state->servers_status[i] == 0)
      {
        if(state->vip_Clients > 0)
          state->vip_Clients -= 1 ;
        else if (state->n_Clients > 0)
          state->n_Clients -= 1;
  
        event * event = createEvent(sim->clock, server_activate, NULL, i) ;
        struct event_queue *queue = sim->queues->next;
        for (int j = 0; j < i; j++)
        {
          queue = sim->queues->next;
        }
        
        enqueue_event(queue, event);
        break;
      }
    }
}
    
void next_arrival(struct simulation *sim, void *metadata) {

    SelectStream(0) ;

    double inter = Exponential(1/((struct simulation_state *)(sim->state))->lmba) ;

    double time = sim->clock + inter;

    event *event1 = createEvent(time, arrivalPayload, next_arrival, NULL) ;

    enqueue_event(sim->queues->queue, event1) ;
}

int main(void) {
  struct simulation_state* state = init_state(4, 2, 3, 0.3);
  PlantSeeds(12345);
  simulation *sim = create_simulation(sizeof(struct simulation_state) + sizeof(char) * 4, 5, 100000, state);
  event *event = createEvent(0.0, next_arrival, NULL, NULL);
  enqueue_event(sim->queues->queue, event);
  run_simulation(sim);
  return 0;
}