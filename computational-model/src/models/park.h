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
    int delay_enabled;
    enum distribution_type delay_distribution;
    double delay_mu;
    double delay_sigma;
    int patience_enabled;
    enum distribution_type patience_distribution;
    double patience_mu;
    double patience_sigma;
    int max_group_size;
    double park_arrival_rate;
    double park_next_reschedule_rate; // NOTE: check if it is useful
    double park_exit_rate;
    int num_rides;
    int num_shows;
    struct ride *rides;
    struct show *shows;
    int until_end;

    int num_disabled_rides;
    struct ride *disabled_rides;

    //Temporary
    double exit_probability ;
} ;

#endif