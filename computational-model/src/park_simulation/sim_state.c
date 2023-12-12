#include <stdio.h>
#include <stdlib.h>

#include "../models/model.h"
#include "sim_state.h"

struct sim_state *create_sim_state(struct park *park) {
    struct sim_state *retVal ;
    if((retVal = malloc(sizeof(struct sim_state))) == NULL) {
        fprintf(stderr, "Error allocating simulation state.\n") ;
        return NULL ;
    }

    retVal->park = park ;
    struct ride_state* rides = malloc(park->num_rides * sizeof(struct ride_state));
    if (rides == NULL) {
        free(retVal) ;
        fprintf(stderr, "Error allocating rides\n");
        return NULL;
    }
    for(int i = 0; i < park->num_rides; i++) {
        rides[i]->stat_vip_clients = 0;
        rides[i]->stat_normal_clients = 0;
        rides[i]->busy_servers = malloc(park->rides[i]->server_num);
        if (rides[i]->busy_servers == NULL) {
            fprintf(stderr, "Error allocating busy_servers for %d\n", i);
            free(rides);
            free(retVal);
            return NULL;
        }
    }
    
    retVal->stat_vip_clients = 0 ;
    retVal->stat_normal_clients = 0;
    
    retVal->total_clients_normal = 0;
    retVal->total_clients_vip = 0;
    retVal->total_service_time_normal = 0.0;
    retVal->total_service_time_vip == 0.0;
    retVal->total_delay_normal  = 0.0;
    retVal->total_delay_vip = 0.0;
}

void delete_sim_state(struct sim_state *state) {

}