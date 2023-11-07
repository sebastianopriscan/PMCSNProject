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

  unsigned int total_clients_normal;
  unsigned int total_clients_vip;
  double total_service_time_normal;
  double total_service_time_vip ;
  double total_delay_normal;
  double total_delay_vip;
} ;

void server_activate(struct simulation *sim, void *metadata) ;

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
  
  return val;
}

void destroy_state(struct simulation_state *state)
{
  free(state->servers_status) ;
  free(state) ;
}

// void server_wakeup(struct simulation *sim, void *metadata)
// {
//   int i = (int) metadata;
//   struct simulation_state *state = (struct simulation_state*) sim->state;
  
//   if(state->vip_Clients > 0)
//     state->vip_Clients -= 1;
//   else if (state->n_Clients > 0)
//     state->n_Clients -= 1;
//   else {
//     state->servers_status[i] = 0;
//     return;
//   }

//   event *event = createEvent(sim->clock, server_activate, NULL, metadata);
//   struct queue_list *queue_l = sim->queues->next;
//   for (int j = 0; j < i; j++)
//   {
//     queue_l = queue_l->next;
//   }
  
//   enqueue_event(queue_l->queue, event);
// }

void server_activate(struct simulation *sim, void *metadata)
{
  int i = (int) metadata;
  SelectStream(i +1) ;
  struct simulation_state *state = (struct simulation_state*) sim->state;

  double service_time = 0.0;
  
  if (state ->vip_Clients >0 || state->n_Clients > 0)
   service_time = Exponential(1/state->mu);

  if(state->vip_Clients > 0) {
    state->vip_Clients -= 1;
    state->total_delay_vip += sim->clock;
    state->total_service_time_vip += service_time;
    state->total_clients_vip += 1;
  }
  else if (state->n_Clients > 0) {
    state->n_Clients -= 1;
    state->total_service_time_normal += service_time ;
    state->total_clients_normal += 1;
    state->total_delay_normal += sim->clock ;
  }
  else {
    state->servers_status[i] = 0;
    return ;
  }

  state->servers_status[i] = 1;

  double next = sim->clock + service_time;

  event* event = createEvent(next, server_activate, NULL, metadata);
  add_event_to_simulation(sim, event, i+1);
}

void arrivalPayload(struct simulation *sim, void *metadata) {

    struct simulation_state *state = (struct simulation_state *)(sim->state) ;

    SelectStream(255);

    double p = Uniform(0.0, 1.0);
    if (p > 0.5) {
      state->vip_Clients += 1;
      state->total_delay_vip -= sim->clock;
    }else {
      state->n_Clients += 1 ;
      state->total_delay_normal -= sim->clock ;
    }

    for(int i = 0 ; i < state->n_of_Servers ; i++)
    {
      if(state->servers_status[i] == 0)
      {
        /*if(state->vip_Clients > 0)
          state->vip_Clients -= 1 ;
        else if (state->n_Clients > 0)
          state->n_Clients -= 1;*/
  
        event * event = createEvent(sim->clock, server_activate, NULL, (void *)i) ;
        add_event_to_simulation(sim, event, i+1);
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
  struct simulation_state* state = init_state(2, 2, 3, 0.3);
  PlantSeeds(12345);
  simulation *sim = create_simulation(sizeof(struct simulation_state) + sizeof(char) * 2, 3, 100000, (char *) state);
  event *event = createEvent(0.0, next_arrival, NULL, NULL);
  enqueue_event(sim->queues->queue, event);
  run_simulation(sim);
  destroy_state(state);
  state = (struct simulation_state*)(sim->state);
  printf("%6.2f, %6.2f, %6.2f, %6.2f, %d , %d\n", state->total_delay_normal, state->total_delay_vip, state->total_service_time_normal, state->total_service_time_vip, state->total_clients_normal, state->total_clients_vip);
  printf("Average Queue Time (normal): %6.2f\n", state->total_delay_normal / state->total_clients_normal);
  printf("Average Queue Time (vip): %6.2f\n", state->total_delay_vip / state->total_clients_vip);
  printf("Average Queue Time: %6.2f\n", (state->total_delay_normal + state->total_delay_vip) / (state->total_clients_normal + state->total_clients_vip));
  printf("Average Service Time (normal): %6.2f\n", state->total_service_time_normal / state->total_clients_normal);
  printf("Average Service Time (vip): %6.2f\n", state->total_service_time_vip / state->total_clients_vip);
  printf("Average Service Time: %6.2f\n", (state->total_service_time_normal + state->total_service_time_vip) / (state->total_clients_normal + state->total_clients_vip));
  return 0;
}