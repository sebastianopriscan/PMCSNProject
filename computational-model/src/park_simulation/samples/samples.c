#include "samples.h"
#include <stdio.h>
#include "../sim_state.h"

void collect_stats(struct simulation *sim, void *metadata) {

    struct sim_state *sim_state = (struct sim_state *) sim->state ;

    double sum_delay_normal = 0.0;
    double sum_delay_vip = 0.0;
    int sum_normal = 0;
    int sum_vip = 0;
    int total_patience_losses = 0 ;

    for (int i = 0; i < sim_state->park->num_rides; i++) {
      struct ride_state ride = sim_state->rides[i];
      sum_delay_normal += ride.total_delay_normal;
      sum_delay_vip += ride.total_delay_vip;
      sum_normal += ride.total_arrived_normal;
      sum_vip += ride.total_arrived_vip;
      total_patience_losses += ride.total_lost_normal;
    }
    printf("%6.6f, %6.6f, %6.6f\n", sum_delay_normal / sum_normal, sum_delay_vip / sum_vip, ((float)total_patience_losses) / sum_normal);
}