#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <generic_queue.h>

#include "../models/model.h"
#include "sim_state.h"

void empty_generic_queue_list(struct generic_queue_list *queue) {
    void *payload ;

    do {
        payload = generic_dequeue_element(queue) ;
        if(payload != NULL)
            free(payload) ;
    } while(payload != NULL) ;
}

struct sim_state *create_sim_state(struct park *park, int log) {
    struct sim_state *retVal ;
    if((retVal = malloc(sizeof(struct sim_state))) == NULL) {
        fprintf(stderr, "Error allocating simulation state.\n") ;
        return NULL ;
    }
    //retVal->total_clients = 0;
    //retVal->total_clients_exited = 0;

    retVal->park = park ;
    struct ride_state* rides = malloc(park->num_rides * sizeof(struct ride_state));
    if (rides == NULL) {
        free(retVal) ;
        fprintf(stderr, "Error allocating rides\n");
        return NULL;
    }
    retVal->rides_popularity_total = 0.0 ;
    for(int i = 0; i < park->num_rides; i++) {
        rides[i].total_clients_normal = 0;
        rides[i].total_clients_reserved = 0;
        rides[i].total_clients_vip = 0;
        rides[i].total_arrived_normal = 0;
        rides[i].total_reservations = 0;
        rides[i].total_arrived_vip = 0;
        rides[i].total_delay_normal = 0.0;
        rides[i].total_delay_reserved = 0.0;
        rides[i].total_delay_vip = 0.0;
        rides[i].total_lost_normal = 0 ;
        rides[i].total_lost_vip = 0 ;
        rides[i].total_lost_normal_delay = 0.0 ;
        rides[i].total_lost_vip_delay = 0.0 ;
        rides[i].global_service_mean = 0.0;
        rides[i].first_arrival = 0.0 ;
        rides[i].last_arrival = 0.0;
        rides[i].first_arrival_vip = 0.0 ;
        rides[i].last_arrival_vip = 0.0;
        rides[i].first_arrival_normal = 0.0 ;
        rides[i].last_arrival_normal = 0.0;
        if (park->validation_run) {
            rides[i].servers_service_means = malloc(sizeof(double)*park->rides[i].server_num );
            rides[i].servers_served_clients = malloc(sizeof(int)*park->rides[i].server_num );
            rides[i].busy_servers = malloc(sizeof(char)*park->rides[i].server_num );
        } else {
            rides[i].servers_service_means = malloc(sizeof(double)*park->rides[i].server_num / park->rides[i].batch_size);
            rides[i].servers_served_clients = malloc(sizeof(int)*park->rides[i].server_num / park->rides[i].batch_size);
            rides[i].busy_servers = malloc(sizeof(char)*park->rides[i].server_num / park->rides[i].batch_size);
        }
        rides[i].vip_queue = create_queue_list();
        rides[i].normal_queue = create_queue_list();
        rides[i].real_reserved_queue = create_queue_list();
        rides[i].reserved_queue = create_queue_list();
        if (rides[i].busy_servers == NULL || rides[i].normal_queue == NULL || rides[i].vip_queue == NULL || 
            rides[i].real_reserved_queue == NULL || rides[i].reserved_queue == NULL || 
            rides[i].servers_service_means == NULL || rides[i].servers_served_clients == NULL) {
            fprintf(stderr, "Error allocating ride internal structure for %d\n", i);
            for(int j = 0; j < i ; j++)  {
                destroy_generic_queue_list(rides[i].vip_queue);
                destroy_generic_queue_list(rides[i].normal_queue);
                free(rides[i].busy_servers) ;
                free(rides[i].servers_service_means);
                free(rides[i].servers_served_clients);
            }
            free(rides);
            free(retVal);
            return NULL;
        }
        if (park->validation_run) {
            memset(rides[i].busy_servers, 0, park->rides[i].server_num );
            memset(rides[i].servers_service_means, 0, sizeof(double) * park->rides[i].server_num );
            memset(rides[i].servers_served_clients, 0, sizeof(int) * park->rides[i].server_num );
        } else {
            memset(rides[i].busy_servers, 0, park->rides[i].server_num / park->rides[i].batch_size);
            memset(rides[i].servers_service_means, 0, sizeof(double) * park->rides[i].server_num / park->rides[i].batch_size);
            memset(rides[i].servers_served_clients, 0, sizeof(int) * park->rides[i].server_num / park->rides[i].batch_size);
        }

        retVal->rides_popularity_total += park->rides[i].popularity ;
    }
    retVal->rides = rides;

    struct show_state* shows = malloc(park->num_shows * sizeof(struct show_state));
    if (shows == NULL) {
        for(int i = 0; i < park->num_rides; i++) {
            destroy_generic_queue_list(rides[i].vip_queue);
            destroy_generic_queue_list(rides[i].normal_queue);
            destroy_generic_queue_list(rides[i].reserved_queue);
            destroy_generic_queue_list(rides[i].real_reserved_queue);
            free(rides[i].busy_servers) ;
            free(rides[i].servers_service_means);
            free(rides[i].servers_served_clients);
        }
        free(retVal->rides);
        free(retVal);
        fprintf(stderr, "Error allocating shows\n");
        return NULL;
    }
    for(int i = 0; i < park->num_shows; i++) {
        shows[i].total_clients = 0;
        shows[i].total_permanence = 0.0;
    }
    retVal->shows = shows;
    retVal->clients = create_queue_list();
    if (retVal->clients == NULL) {
        for(int i = 0; i < park->num_rides; i++) {
            destroy_generic_queue_list(rides[i].vip_queue);
            destroy_generic_queue_list(rides[i].normal_queue);
            destroy_generic_queue_list(rides[i].reserved_queue);
            destroy_generic_queue_list(rides[i].real_reserved_queue);
            free(rides[i].busy_servers) ;
            free(rides[i].servers_service_means);
            free(rides[i].servers_served_clients);
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
            destroy_generic_queue_list(rides[i].vip_queue);
            destroy_generic_queue_list(rides[i].normal_queue);
            destroy_generic_queue_list(rides[i].reserved_queue);
            destroy_generic_queue_list(rides[i].real_reserved_queue);
            free(rides[i].busy_servers) ;
            free(rides[i].servers_service_means);
            free(rides[i].servers_served_clients);
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal);
        fprintf(stderr, "Error allocating active shows\n");
        return NULL;
    }
    memset(retVal->active_shows, 0, park->num_shows);
    
    // retVal->stat_vip_clients = 0 ;
    // retVal->stat_normal_clients = 0;
    
    retVal->total_clients_normal = 0;
    retVal->total_clients_vip = 0;
    // retVal->total_service_time_normal = 0.0;
    // retVal->total_service_time_vip = 0.0;
    // retVal->total_delay_normal  = 0.0;
    // retVal->total_delay_vip = 0.0;
    retVal->num_active_shows = 0;
    retVal->popularities = malloc((park->num_rides + park->num_shows) * sizeof(double));

    if(retVal->popularities == NULL) {
        fprintf(stderr, "Error in allocating popularities array\n") ;
        for(int i = 0; i < park->num_rides; i++) {
            destroy_generic_queue_list(rides[i].vip_queue);
            destroy_generic_queue_list(rides[i].normal_queue);
            destroy_generic_queue_list(rides[i].reserved_queue);
            destroy_generic_queue_list(rides[i].real_reserved_queue);
            free(rides[i].busy_servers) ;
            free(rides[i].servers_service_means);
            free(rides[i].servers_served_clients);
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal->active_shows);
        free(retVal);
        return NULL ;
    }
    evaluate_attraction_probabilities(retVal);

    retVal->clients_in_park = 0 ;
    retVal->clients_in_queue = 0 ;

    retVal->total_clients_arrived = 0 ;
    retVal->total_entrance_queue_times_delay = 0.0;
    
    retVal->entrance_queue_arrival_times = create_queue_list() ;

    if(retVal->entrance_queue_arrival_times == NULL) {
        fprintf(stderr, "Error in allocating arrival times queuee\n") ;
        for(int i = 0; i < park->num_rides; i++) {
            destroy_generic_queue_list(rides[i].vip_queue);
            destroy_generic_queue_list(rides[i].normal_queue);
            destroy_generic_queue_list(rides[i].reserved_queue);
            destroy_generic_queue_list(rides[i].real_reserved_queue);
            free(rides[i].busy_servers) ;
            free(rides[i].servers_service_means);
            free(rides[i].servers_served_clients);
        }
        free(retVal->rides);
        free(retVal->shows);
        free(retVal->active_shows);
        free(retVal->popularities);
        free(retVal);
        return NULL ;
    }

    retVal->total_clients_exited = 0;
    retVal->total_permanence = 0.0;
    retVal->log = log;

    return retVal;
}

void delete_sim_state(struct sim_state *state) {
    for (int i = 0; i < state->park->num_rides; i++) {
            empty_generic_queue_list(state->rides[i].vip_queue) ;
            empty_generic_queue_list(state->rides[i].normal_queue) ;
            empty_generic_queue_list(state->rides[i].real_reserved_queue) ;
            empty_generic_queue_list(state->rides[i].reserved_queue) ;
            destroy_generic_queue_list(state->rides[i].vip_queue);
            destroy_generic_queue_list(state->rides[i].normal_queue);
            destroy_generic_queue_list(state->rides[i].real_reserved_queue) ;
            destroy_generic_queue_list(state->rides[i].reserved_queue) ;
            free(state->rides[i].busy_servers) ;
            free(state->rides[i].servers_service_means);
            free(state->rides[i].servers_served_clients);
    }
    empty_generic_queue_list(state->clients) ;
    destroy_generic_queue_list(state->clients) ;
    free(state->rides);
    free(state->shows);
    free(state->active_shows);
    free(state->popularities);
    empty_generic_queue_list(state->entrance_queue_arrival_times) ;
    destroy_generic_queue_list(state->entrance_queue_arrival_times);
    free(state);
}

void evaluate_attraction_probabilities(struct sim_state *state) {
  double sum_open_popularities = state->rides_popularity_total;
  for (int i = 0; i < state->park->num_shows; i++) {
    if(state->active_shows[i] == 1) 
        sum_open_popularities += state->park->shows[i].popularity;
  }

  double residual = (1 - sum_open_popularities) / sum_open_popularities;
  state->popularities[0] = state->park->rides[0].popularity * (1 + residual);
  for (int i = 1; i < state->park->num_rides + state->park->num_shows; i++) {
    // If it is a show and it is deactivated, copy the previous value 
    if (i >= state->park->num_rides && state->active_shows[i - state->park->num_rides] == 0) {
        state->popularities[i] = state->popularities[i-1];
        continue;
    }
    double popularity = 0.0;
    if (i < state->park->num_rides)
        popularity = state->park->rides[i].popularity;
    else
        popularity = state->park->shows[i - state->park->num_rides].popularity;
    state->popularities[i] = state->popularities[i-1] + popularity * (1 + residual);
  }
}