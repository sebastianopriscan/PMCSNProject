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
  if(park->vip_tickets_percent != 0.1) {
    fprintf(stderr, "Vip Tickets: expected 0.5 Got: %f\n", park->vip_tickets_percent);
    return 1;
  }
  if(park->max_vip_tickets != 1) {
    fprintf(stderr, "Max Vip Tickets: expected 1 Got: %f\n", park->vip_tickets_percent);
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
  if(park->patience_enabled != 1) {
    fprintf(stderr, "Patience enabled: expected 1 Got: %d\n", park->patience_enabled);
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
  if(park->park_arrival_rate != 0.1) {
    fprintf(stderr, "Park Arrival Rate: expected 0.1 Got: %f\n", park->park_arrival_rate);
    return 1;
  }
  if(park->park_exit_rate != 0.01) {
    fprintf(stderr, "Park Exit Rate: expected 0.1 Got: %f\n", park->park_exit_rate);
    return 1;
  }
  if(park->until_end != 1) {
    fprintf(stderr, "Park Until End: expected true Got: %d\n", park->until_end);
    return 1;
  }
  if(park->improved_run != 1) {
    fprintf(stderr, "Improved run: expected 1 Got: %d\n", park->improved_run);
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
  if(park->rides[0].batch_size != 1) {
    fprintf(stderr, "Park Ride Batch size: expected 1 Got: %d\n", park->rides[0].batch_size);
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
  if(park->rides[0].expected_wait != 0.1) {
    fprintf(stderr, "Park ride expected_wait: expected 0.1. Got: %f\n", park->rides[0].expected_wait);
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
  if(park->shows[0].distribution != NORMAL_DISTRIB) {
    fprintf(stderr, "Park show mu: expected %d. Got: %d\n", NORMAL_DISTRIB, park->shows[0].distribution);
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