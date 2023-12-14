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
        json_object_put(root);
        fprintf(stderr, "Error in opening file %s\n", file) ;
        return NULL;
    }

    struct park *park = malloc(sizeof(struct park));
    if (park == NULL) {
        json_object_put(root);
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

    park->simulation_time = json_object_get_double(simulation_time_json);
    park->vip_tickets_percent = json_object_get_double(vip_tickets_json);
    park->maintainance_cost_per_rides = json_object_get_int(maintenance_cost_per_rides_json);
    park->maintainance_cost_per_shows = json_object_get_int(maintenance_cost_per_shows_json);
    park->construction_cost_per_seat = json_object_get_int(construction_cost_per_seat_json);
    park->vip_ticket_price = json_object_get_int(vip_ticket_price_json);
    park->normal_ticket_price = json_object_get_int(normal_ticket_price_json);
    park->number_of_clients = json_object_get_int(number_of_clients_json);
    char* patience_distribution = json_object_get_string(patience_distribution_json);
    if(strcmp(patience_distribution, "normal") == 0) {
      park->patience_distribution = NORMAL_DISTRIB;
    } else if (strcmp(patience_distribution, "exponential") == 0) {
      park->patience_distribution = EXPONENTIAL;
    } else if (strcmp(patience_distribution, "equilikely") == 0)  {
      park->patience_distribution = EQUILIKELY ;
    } else {
      json_object_put(root);
      fprintf(stderr, "Error in parsing patience_distribution %s", patience_distribution);
      return NULL;
    }
    park->patience_mu = json_object_get_double(patience_mu_json);
    park->patience_sigma = json_object_get_double(patience_sigma_json);
    park->max_group_size = json_object_get_int(max_group_size_json);
    park->park_arrival_rate = json_object_get_double(park_arrival_rate_json);
    park->park_next_reschedule_rate = json_object_get_double(park_next_reschedule_rate_json);
    park->park_exit_probability = json_object_get_double(park_exit_probability_json);

    int rides_size = json_object_array_length(rides_json) ;
    json_object *curr_ride ;
    struct ride *park_rides = malloc(rides_size * sizeof(struct ride)) ;
    if(park_rides == NULL) {
        json_object_put(root);
        fprintf(stderr, "Error allocating rides space from file %s\n", file) ;
        return NULL ;
    }

    for(int i = 0; i < rides_size ; i++) {
        curr_ride = json_object_array_get_idx(rides_json, i) ;
        json_object *ride_name = json_object_object_get(curr_ride, "name") ;
        
        char *name = json_object_get_string(ride_name) ;
        if(name == NULL) {
          json_object_put(root);
            fprintf(stderr, "String field name is  null\n") ;
            return NULL ;
        }
        int name_len = strlen(name) ;

        char *name_Allocd ;

        if((name_Allocd = malloc(name_len +1)) == NULL) {
          json_object_put(root);
          fprintf(stderr, "Error allocating spacee for ride name %s\n", file) ;
          return NULL ;
        }

        strcpy(name_Allocd, name) ;

        park_rides[i].name = name_Allocd ;

        json_object *ride_server_num = json_object_object_get(curr_ride, "server_num") ;
        park_rides[i].server_num = json_object_get_int(ride_server_num) ;

        json_object *json_show_popularity = json_object_object_get(curr_ride, "popularity");
        park_rides[i].popularity = json_object_get_double(json_show_popularity);

        json_object *json_show_mean_time = json_object_object_get(curr_ride, "mean_time");
        park_rides[i].mean_time= json_object_get_double(json_show_mean_time);
        
        json_object *json_show_mu = json_object_object_get(curr_ride, "mu");
        park_rides[i].mu = json_object_get_double(json_show_mu);
        
        json_object *json_show_sigma = json_object_object_get(curr_ride, "sigma");
        park_rides[i].sigma = json_object_get_double(json_show_sigma);
        
        json_object *service_distribution_json = json_object_object_get(curr_ride, "service_distribution");
        char* service_distribution = json_object_get_string(service_distribution_json);
        if(strcmp(service_distribution, "normal") == 0) {
          park_rides[i].distribution = NORMAL_DISTRIB;
        } else if (strcmp(service_distribution, "exponential") == 0) {
          park_rides[i].distribution = EXPONENTIAL;
        } else if (strcmp(service_distribution, "equilikely") == 0)  {
          park_rides[i].distribution  = EQUILIKELY ;
        } else {
          json_object_put(root);
          fprintf(stderr, "Error in parsing service_distribution %s", patience_distribution);
          return NULL;
        }

    }
    int shows_size = json_object_array_length(shows_json) ;
    json_object *curr_show ;
    struct show *park_shows = malloc(shows_size * sizeof(struct show)) ;

    if(park_shows == NULL) {
        json_object_put(root);
        fprintf(stderr, "Error allocating shows space from file %s\n", file) ;
        return NULL ;
    }

    for (int i = 0; i < shows_size; i++) {
        curr_show = json_object_array_get_idx(shows_json, i);
        json_object *show_name = json_object_object_get(curr_show, "name");
        char *name = json_object_get_string(show_name);
        if (name == NULL) {
          json_object_put(root);
          fprintf(stderr, "String field name is null\n");
          return NULL;
        }
        int name_len = strlen(name);

        char *name_Allocd;
        if ((name_Allocd = malloc(name_len + 1)) == NULL) {
          json_object_put(root);
          fprintf(stderr, "Error allocating space for show name %s\n", file);
          return NULL;
        }
        strcpy(name_Allocd, name);
        park_shows[i].name = name_Allocd;
      
        json_object *json_show_popularity = json_object_object_get(curr_show, "popularity");
        park_shows[i].popularity = json_object_get_double(json_show_popularity);

        json_object *json_show_mean_time = json_object_object_get(curr_show, "mean_time");
        park_shows[i].mean_time= json_object_get_double(json_show_mean_time);
        
        json_object *json_show_mu = json_object_object_get(curr_show, "mu");
        park_shows[i].mu = json_object_get_double(json_show_mu);
        
        json_object *json_show_sigma = json_object_object_get(curr_show, "sigma");
        park_shows[i].sigma = json_object_get_double(json_show_sigma);
        
        json_object *service_distribution_json = json_object_object_get(curr_show, "service_distribution");
        char* service_distribution = json_object_get_string(service_distribution_json);
        if(strcmp(service_distribution, "normal") == 0) {
          park_shows[i].distribution = NORMAL_DISTRIB;
        } else if (strcmp(service_distribution, "exponential") == 0) {
          park_shows[i].distribution = EXPONENTIAL;
        } else if (strcmp(service_distribution, "equilikely") == 0)  {
          park_shows[i].distribution  = EQUILIKELY ;
        } else {
          json_object_put(root);
          fprintf(stderr, "Error in parsing service_distribution %s", patience_distribution);
          return NULL;
        }
    }

    park->num_rides = rides_size;
    park->num_shows = shows_size;
    park->rides = &park_rides ;
    park->shows = &park_shows ;
    double *popularities = malloc((park->num_rides + park->num_shows) * sizeof(double));
    if (popularities == NULL) {
      json_object_put(root);
      fprintf(stderr, "Error allocating popularities");
      return NULL;
    }
    double sum = park->rides[0].popularity;
    popularities[0] = park->rides[0].popularity;
    for (int i = 1; i < park->num_rides; i++) {
        sum += park->rides[i].popularity;
        popularities[i] = popularities[i-1] + park->rides[i].popularity;
    }
    sum += park->shows[0].popularity;
    popularities[park->num_rides] = popularities[park->num_rides-1] + park->shows[0].popularity;
    for (int i = 1; i < park->num_shows; i++) {
        sum += park->shows[i].popularity;
        popularities[park->num_rides + i] = popularities[park->num_rides + i-1] + park->shows[i].popularity;
    }
    if(sum - 1.0 > 0.05) {
      json_object_put(root);
      fprintf(stderr, "Invariant error: sum of popularity must be equal to 1\n");
      return NULL;
    }
    else {
      popularities[park->num_rides + park->num_shows -1] = 1.0 ;
    }
    park->popularities = popularities;

    json_object_put(root);
    return park;
}