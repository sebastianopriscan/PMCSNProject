#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef PARK_H
#define PARK_H

#include "utils.h"

struct park {
    double vip_tickets_percent ;
    double simulation_time ;
    int maintainance_cost_per_rides ;
    int maintainance_cost_per_shows ;
    int construction_cost_per_seat ;
    int vip_ticket_price ;
    int normal_ticket_price ;
    int number_of_clients ;
    enum distribution_type delay_distribution;
    double delay_mu;
    double delay_sigma;
    enum distribution_type patience_distribution;
    double patience_mu;
    double patience_sigma;
    int max_group_size;
    double park_arrival_rate;
    double park_next_reschedule_rate;
    double park_exit_probability;
    int num_rides;
    int num_shows;
    struct ride *rides;
    struct show *shows;

    double *popularities ;
} ;

#endif