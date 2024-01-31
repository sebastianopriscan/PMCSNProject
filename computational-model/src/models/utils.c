#include <stdlib.h>
#include <stdio.h>

#include "model.h"
#include "../park_simulation/sim_state.h"
#include "../park_simulation/behaviors/behaviors.h"
#include <rngs.h>
#include <rvgs.h>
#include <math.h>

double GetRandomFromDistributionType(int stream, enum distribution_type type, double mu, double sigma) {
  SelectStream(stream);
  switch(type) {
    case NORMAL_DISTRIB:
      return Normal(mu, sigma);
      break;
    case UNIFORM:
      return Uniform(mu, sigma);
      break;
    case EXPONENTIAL:
      return Exponential(mu) ;
      break;
  }
}

struct client *create_new_client(double clock, double end, struct sim_state* state, char until_end) {
    struct client *me = malloc(sizeof(struct client));
  if (me == NULL) {
    fprintf(stderr, "Error in allocating client size\n");
    exit(1);
  }

  double patience_stdev_percentage = GetRandomFromDistributionType(PATIENCE_MU_STREAM, state->park->patience_distribution, state->park->patience_mu, state->park->patience_sigma);
  double p = GetRandomFromDistributionType(CLIENT_TYPE_STREAM, UNIFORM, 0, 1);
  double exit_time = GetRandomFromDistributionType(EXIT_TIME_STREAM, EXPONENTIAL, 1/(state->park->park_exit_rate), 0);

  patience_stdev_percentage = patience_stdev_percentage < 0 ? -patience_stdev_percentage : patience_stdev_percentage ;
  patience_stdev_percentage = patience_stdev_percentage > 1 ? 1 : patience_stdev_percentage;

  if (until_end) {
    me->exit_time = clock + exit_time;
  } else {
    me->exit_time = clock + exit_time > end ? end : clock + exit_time;
  }
  me->client_percentage = patience_stdev_percentage;
  if (p < state->park->vip_tickets_percent && state->total_clients_vip < state->park->max_vip_tickets) {
      me->type = VIP;
      state->total_clients_vip += 1;
  } else {
    me->type = NORMAL;
    state->total_clients_normal++;
  }

  me->lost_patience_times = 0 ;
  me->arrival_time = clock;

  return me;
}