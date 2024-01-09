#include <stdio.h>

#include "deserializer/deserializer.h"
#include "../src/models/model.h"
#include <string.h>

int main(void) {
  struct park *park = deserialize("../../../docs/json-input-template.json");
  if(park->simulation_time != 960.0) {
    fprintf(stderr, "Simulation Time: expected 960.0. Got: %f\n", park->simulation_time);
    return 1;
  }
  if(park->vip_tickets_percent != 0.5) {
    fprintf(stderr, "Vip Tickets: expected 0.5 Got: %f\n", park->vip_tickets_percent);
    return 1;
  }
  if(park->maintainance_cost_per_rides != 1) {
    fprintf(stderr, "Maintainance cost per rides: expected 1 Got: %d\n", park->maintainance_cost_per_rides);
    return 1;
  }
  if(park->maintainance_cost_per_shows != 1) {
    fprintf(stderr, "Maintainance cost per shows: expected 1 Got: %d\n", park->maintainance_cost_per_shows);
    return 1;
  }
  if(park->construction_cost_per_seat!= 1) {
    fprintf(stderr, "Construction cost per seat: expected 1 Got: %d\n", park->construction_cost_per_seat);
    return 1;
  }
  if(park->vip_ticket_price != 1) {
    fprintf(stderr, "VIP Ticket Price: expected 1 Got: %d\n", park->vip_ticket_price);
    return 1;
  }
  if(park->normal_ticket_price != 1) {
    fprintf(stderr, "Normal Ticket Price: expected 1 Got: %d\n", park->normal_ticket_price);
    return 1;
  }
  if(park->number_of_clients != 1) {
    fprintf(stderr, "Number of clients: expected 1 Got: %d\n", park->number_of_clients);
    return 1;
  }
  if(park->patience_distribution != NORMAL_DISTRIB) {
    fprintf(stderr, "Patiance Distribution: expected %d Got: %d\n", NORMAL_DISTRIB, park->patience_distribution);
    return 1;
  }
  if(park->patience_mu != 0.1) {
    fprintf(stderr, "Patience Mu: expected 0.1 Got: %f\n", park->patience_mu);
    return 1;
  }
  if(park->patience_sigma != 0.1) {
    fprintf(stderr, "Patience sigma: expected 0.1 Got: %f\n", park->patience_sigma);
    return 1;
  }
  if(park->max_group_size != 5) {
    fprintf(stderr, "Max Group Size: expected 5 Got: %d\n", park->max_group_size);
    return 1;
  }
  if(park->park_arrival_rate != 0.1) {
    fprintf(stderr, "Park Arrival Rate: expected 0.1 Got: %f\n", park->park_arrival_rate);
    return 1;
  }
  if(park->park_next_reschedule_rate != 0.1) {
    fprintf(stderr, "Park Next Reschedule Rate: expected 0.1 Got: %f\n", park->park_next_reschedule_rate);
    return 1;
  }
  if(park->park_exit_rate != 0.01) {
    fprintf(stderr, "Park Exit Rate: expected 0.1 Got: %f\n", park->park_exit_rate);
    return 1;
  }
  if(park->num_rides != 1) {
    fprintf(stderr, "Num Rides: expected 1 Got: %d\n", park->num_rides);
    return 1;
  }
  if(park->num_shows != 1) {
    fprintf(stderr, "Num shows: expected 1 Got: %d\n", park->num_shows);
    return 1;
  }
  if(strcmp(park->rides[0].name, "Name") != 0) {
    fprintf(stderr, "Park Ride Name: expected Name Got: %s\n", park->rides[0].name);
    return 1;
  }
  if(park->rides[0].popularity != 0.1) {
    fprintf(stderr, "Park Ride Popularity: expected 0.1 Got: %f\n", park->rides[0].popularity);
    return 1;
  }
  if(park->rides[0].server_num != 1) {
    fprintf(stderr, "Park Ride Server Num: expected 1 Got: %d\n", park->rides[0].server_num);
    return 1;
  }
  if(park->rides[0].mean_time != 0.1) {
    fprintf(stderr, "Park Ride Mean Time: expected 0.1 Got: %f\n", park->rides[0].mean_time);
    return 1;
  }
  if(park->rides[0].distribution != NORMAL_DISTRIB) {
    fprintf(stderr, "Park ride distribution: expected %d. Got: %d\n",NORMAL_DISTRIB, park->rides[0].distribution);
    return 1;
  }
  if(park->rides[0].mu != 0.1) {
    fprintf(stderr, "Park ride mu: expected 0.1. Got: %f\n", park->rides[0].mu);
    return 1;
  }
  if(park->rides[0].sigma != 0.1) {
    fprintf(stderr, "Park ride sigma: expected 0.1. Got: %f\n", park->rides[0].sigma);
    return 1;
  }
  if(strcmp(park->shows[0].name, "Name") != 0) {
    fprintf(stderr, "Park show name: expected Name. Got: %s\n", park->shows[0].name);
    return 1;
  }
  if(park->shows[0].popularity != 0.1) {
    fprintf(stderr, "Park show popularity: expected 0.1. Got: %f\n", park->shows[0].popularity);
    return 1;
  }
  if(park->shows[0].mean_time != 0.1) {
    fprintf(stderr, "Park show mean time: expected  Got: %f\n", park->shows[0].mean_time);
    return 1;
  }
  if(park->shows[0].distribution != NORMAL_DISTRIB) {
    fprintf(stderr, "Park show mu: expected %d. Got: %d\n", NORMAL_DISTRIB, park->shows[0].distribution);
    return 1;
  }
  if(park->shows[0].mu != 0.1) {
    fprintf(stderr, "Park show mu: expected 0.1. Got: %f\n", park->shows[0].mu);
    return 1;
  }
  if(park->shows[0].sigma != 0.1) {
    fprintf(stderr, "Show sigma: expected 0.1. Got: %f\n", park->shows[0].sigma);
    return 1;
  }
  if(park->shows[0].length != 30.0) {
    fprintf(stderr, "Show length: expected 30. Got: %f\n", park->shows[0].length);
    return 1;
  }
  if(park->shows[0].num_starting_times != 4) {
    fprintf(stderr, "Show num starting times: expected 4. Got: %d\n", park->shows[0].num_starting_times);
    return 1;
  }
  if(park->shows[0].starting_times[0] != 120.0 
    || park->shows[0].starting_times[1] != 180.0 
    || park->shows[0].starting_times[2] != 300.0 
    || park->shows[0].starting_times[3] != 360.0) {
    fprintf(stderr, "Show starting times: invalid values\n");
    return 1;
  }
  return 0;
}