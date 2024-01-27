#ifndef SIM_STATE_H
#define SIM_STATE_H

#include "../models/model.h"
#include "generic_queue.h"

struct ride_state {
  // unsigned int stat_vip_clients;
  // unsigned int stat_normal_clients;
  // unsigned int total_clients_normal;
  // unsigned int total_clients_vip;
  // double total_service_time_normal;
  // double total_service_time_vip ;
  // double total_delay_normal;
  // double total_delay_vip;

  char *busy_servers ;
  struct generic_queue_list *vip_queue;
  struct generic_queue_list *normal_queue;

  // Statistics
  int total_clients_normal;
  int total_clients_vip;
  double total_delay_normal;
  double total_delay_vip;

  int total_lost_normal ;
  int total_lost_vip ;
  double total_lost_normal_delay ;
  double total_lost_vip_delay ;

  double global_service_mean; // Global
  double *servers_service_means; // Local server
  int *servers_served_clients; // Local server

  double first_arrival;
  double last_arrival ;
  double first_arrival_vip;
  double last_arrival_vip;
  double first_arrival_normal;
  double last_arrival_normal;
};

struct show_state {
  int total_clients ;
  // unsigned int stat_clients;
  // unsigned int total_clients;
  // double global_service_mean;
  // double total_delay;
  double total_permanence ;
};

struct sim_state {
  struct park *park;

  struct generic_queue_list *clients;

  // used when calculating tickets
  int available_vip_tickets ;
  int total_clients_normal; 
  int total_clients_vip;

  // Statistics
  // unsigned int total_clients;
  // unsigned int total_clients_exited;
  // unsigned int stat_vip_clients;
  // unsigned int stat_normal_clients;

  // double total_service_time_normal;
  // double total_service_time_vip ;
  // double total_delay_norm
  // double total_delay_vip;

  struct ride_state *rides ;
  struct show_state *shows;
  double rides_popularity_total ;
  int num_active_shows;
  double *popularities;
  char *active_shows;
  int clients_in_park;
  int clients_in_queue;

  // Statistics
  struct generic_queue_list *entrance_queue_arrival_times;
  double total_entrance_queue_times_delay;
  int total_clients_arrived ;

  int total_clients_exited;
  double total_permanence;
  int log; // 0b0001: client; 0b0010: show; 0b0100: ride; 0b1000: stats
};

struct sim_state *create_sim_state(struct park *park, int log) ;
void delete_sim_state(struct sim_state *state);
void evaluate_attraction_probabilities(struct sim_state *state);

#endif