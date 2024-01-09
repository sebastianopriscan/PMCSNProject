#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <generic_queue.h>

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
    retVal->rides_popularity_total = 0.0 ;
    for(int i = 0; i < park->num_rides; i++) {
        rides[i].stat_vip_clients = 0;
        rides[i].stat_normal_clients = 0;
        rides[i].busy_servers = malloc(park->rides[i].server_num);
        rides[i].vip_queue = create_queue_list();
        rides[i].normal_queue = create_queue_list();
        if (rides[i].busy_servers == NULL || rides[i].normal_queue == NULL || rides[i].vip_queue == NULL) {
            fprintf(stderr, "Error allocating busy_servers for %d\n", i);
            for(int j = 0; j < i ; j++) 
                free(rides[i].busy_servers) ;
            free(rides);
            free(retVal);
            return NULL;
        }
        for(int j = 0 ; j < park->rides[i].server_num ; j++)
            rides[i].busy_servers = 0 ;

        retVal->rides_popularity_total += park->rides[i].popularity ;
    }
    retVal->rides = rides;

    struct show_state* shows = malloc(park->num_shows * sizeof(struct show_state));
    if (shows == NULL) {
        for(int i = 0; i < park->num_rides; i++) {
            free(retVal->rides[i].busy_servers);
            destroy_generic_queue_list(rides[i].normal_queue) ;
            destroy_generic_queue_list(rides[i].vip_queue) ;
        }
        free(retVal->rides);
        free(retVal);
        fprintf(stderr, "Error allocating shows\n");
        return NULL;
    }
    for(int i = 0; i < park->num_shows; i++) {
        shows[i].stat_clients = 0;
    }
    retVal->shows = shows;
    retVal->clients = create_queue_list();
    if (retVal->clients == NULL) {
        for(int i = 0; i < park->num_rides; i++) {
            free(retVal->rides[i].busy_servers);
            destroy_generic_queue_list(rides[i].normal_queue) ;
            destroy_generic_queue_list(rides[i].vip_queue) ;
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal);
        fprintf(stderr, "Error allocating clients generic queues\n");
        return NULL;
    }

    retVal->active_shows = malloc(park->num_shows);
    if (retVal->active_shows == NULL) {
        for(int i = 0; i < park->num_rides; i++) {
            free(retVal->rides[i].busy_servers);
            destroy_generic_queue_list(rides[i].normal_queue) ;
            destroy_generic_queue_list(rides[i].vip_queue) ;
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal);
        fprintf(stderr, "Error allocating active shows\n");
        return NULL;
    }
    memset(retVal->active_shows, 0, park->num_shows);
    
    retVal->stat_vip_clients = 0 ;
    retVal->stat_normal_clients = 0;
    
    retVal->total_clients_normal = 0;
    retVal->total_clients_vip = 0;
    retVal->total_service_time_normal = 0.0;
    retVal->total_service_time_vip = 0.0;
    retVal->total_delay_normal  = 0.0;
    retVal->total_delay_vip = 0.0;
    retVal->num_active_shows = 0;
    retVal->popularities = malloc((park->num_rides + park->num_shows) * sizeof(double));

    if(retVal->popularities == NULL) {
        fprintf(stderr, "Error in allocating popularities array\n") ;
        for(int i = 0; i < park->num_rides; i++) {
            free(retVal->rides[i].busy_servers);
            destroy_generic_queue_list(rides[i].normal_queue) ;
            destroy_generic_queue_list(rides[i].vip_queue) ;
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal->active_shows);
        free(retVal);
        fprintf(stderr, "Error allocating active shows\n");
        return NULL ;
    }

    return retVal;
}

void delete_sim_state(struct sim_state *state) {
    for (int i = 0; i < state->park->num_rides; i++) {
        free(state->rides[i].busy_servers);
        destroy_generic_queue_list(state->rides[i].vip_queue) ;
        destroy_generic_queue_list(state->rides[i].normal_queue) ;
    }
    destroy_generic_queue_list(state->clients) ;
    free(state->rides);
    free(state->shows);
    free(state->active_shows);
    free(state->popularities);
    free(state);
}

void evaluate_attraction_probabilities(struct sim_state *state) {
  double sum_open_popularities = state->rides_popularity_total;
  for (int i = 0; i < state->park->num_shows; i++) {
    if(state->active_shows[i] == 1) sum_open_popularities += state->park->shows[i].popularity;
  }

  double residual = (1 - sum_open_popularities) / sum_open_popularities;
  state->popularities[0] = state->park->rides[0].popularity * (1 + residual);
  for (int i = 1; i < state->park->num_rides + state->park->num_shows; i++) {
    // If it is a show and it is deactivated, copy the previous value 
    if (i >= state->park->num_rides && state->active_shows[i - state->park->num_rides] == 0) {
        state->popularities[i] = state->popularities[i-1];
        continue;
    }
    state->popularities[i] = state->popularities[i-1] + state->park->rides[i].popularity * (1 + residual);
  }
}