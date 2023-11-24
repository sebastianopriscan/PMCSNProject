#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>

#include "deserializer.h"
#include "../models/model.h"

struct park;

struct park *deserialize(const char *file) {
    json_object *root = json_object_from_file(file);
    if (!root) {
        fprintf(stderr, "Error in opening file %s\n", file) ;
        return NULL;
    }

    struct park *park = malloc(sizeof(struct park));
    if (park == NULL) {
        fprintf(stderr, "Error when allocating memory for park\n") ;
        return NULL;
    }

    json_object *rides_json = json_object_object_get(root, "rides");
    json_object *shows_json = json_object_object_get(root, "shows");
    json_object *simulation_time_json = json_object_object_get(root, "simulation_time");
    json_object *vip_tickets_json = json_object_object_get(root, "vip_tickets");
    json_object *maintenance_cost_per_rides_json = json_object_object_get(root, "maintenance_cost_per_rides");
    json_object *maintenance_cost_per_shows_json = json_object_object_get(root, "maintenance_cost_per_shows");
    json_object *construction_cost_per_seat_json = json_object_object_get(root, "construction_cost_per_seat");
    json_object *vip_ticket_price_json = json_object_object_get(root, "vip_ticket_price");
    json_object *normal_ticket_price_json = json_object_object_get(root, "normal_ticket_price");
    json_object *number_of_clients_json = json_object_object_get(root, "number_of_clients");
    json_object *patience_distribution_json = json_object_object_get(root, "patience_distribution");
    json_object *patience_mu_json = json_object_object_get(root, "patience_mu");
    json_object *patience_sigma_json = json_object_object_get(root, "patience_sigma");
    json_object *max_group_size_json = json_object_object_get(root, "max_group_size");
    json_object *park_arrival_rate_json = json_object_object_get(root, "park_arrival_rate");
    json_object *park_next_reschedule_rate_json = json_object_object_get(root, "park_next_reschedule_rate");
    json_object *park_exit_probability_json = json_object_object_get(root, "park_exit_probability");

    double simulation_time = json_object_get_double(simulation_time_json);
    double vip_tickets = json_object_get_double(vip_tickets_json);
    int maintenance_cost_per_rides = json_object_get_int(maintenance_cost_per_rides_json);
    int maintenance_cost_per_shows = json_object_get_int(maintenance_cost_per_shows_json);
    int construction_cost_per_seat = json_object_get_int(construction_cost_per_seat_json);
    int vip_ticket_price = json_object_get_int(vip_ticket_price_json);
    int normal_ticket_price = json_object_get_int(normal_ticket_price_json);
    int number_of_clients = json_object_get_int(number_of_clients_json);
    char* patience_distribution = json_object_get_string(patience_distribution_json);
    enum distribution_type patience_distr;
    if(strcmp(patience_distribution, "normal") == 0) {
      patience_distr = NORMAL_DISTRIB;
    } else if (strcmp(patience_distribution, "exponential") == 0) {
      patience_distr = EXPONENTIAL;
    } else if (strcmp(patience_distribution, "equilikely") == 0)  {
      patience_distr = EQUILIKELY ;
    } else {
      fprintf(stderr, "Error in parsing patience_distribution %s", patience_distribution);
      return NULL;
    }
    double patience_mu = json_object_get_double(patience_mu_json);
    double patience_sigma = json_object_get_double(patience_sigma_json);
    int max_group_size = json_object_get_int(max_group_size_json);
    double park_arrival_rate = json_object_get_double(park_arrival_rate_json);
    double park_next_reschedule_rate = json_object_get_double(park_next_reschedule_rate_json);
    double park_exit_probability = json_object_get_double(park_exit_probability_json);
    //array_list *rides_array = json_object_get_array(rides_json);
    //array_list *shows_array = json_object_get_array(shows_json);


    if(park_rides == NULL) {
        fprintf(stderr, "Error allocating rides space from file %s\n", file) ;
        return NULL ;
    }

    int rides_size = json_object_array_length(rides_json) ;
    json_object *curr_ride ;
    struct ride *park_rides = malloc(rides_size * (struct ride)) ;

    for(int i = 0; i < rides_size ; i++) {
        curr_ride = json_object_array_get_idx(rides_json, i) ;
        json_object *ride_name = json_object_get(curr_ride, "name") ;
        
        char *name = json_object_get_string(ride_name) ;
        if(name == NULL) {
            fprintf(stderr, "String field name is  null\n") ;
            return NULL ;
        }
        int name_len = strlen(name) ;

        if(name )
    }
    int shows_size = json_object_array_length(shows_json) ;
    json_object *curr_show ;
    struct show *park_shows = malloc(shows_size * (struct show)) ;

    if(park_shows == NULL) {
        fprintf(stderr, "Error allocating shows space from file %s\n", file) ;
        return NULL ;
    }

    for (int i = 0; i < shows_size; i++) {
      curr_show = json_object_array_get_idx(shows_json, i);
      json_object *show_name = json_object_get(curr_show, "name");
    }

    //Loop and create data for rides/shows

    park->rides = &park_rides ;
    park->shows = &park_shows ;

    return park;
}