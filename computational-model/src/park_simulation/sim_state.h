#ifndef SIM_STATE_H
#define SIM_STATE_H

#include "../models/model.h"
#include <generic_queue.h>

struct ride_state {
  unsigned int stat_vip_clients;
  unsigned int stat_normal_clients;
  unsigned int total_clients_normal;
  unsigned int total_clients_vip;
  double total_service_time_normal;
  double total_service_time_vip ;
  double total_delay_normal;
  double total_delay_vip;

  char *busy_servers ;
};

struct show_state {
  unsigned int stat_clients;
  unsigned int total_clients;
  double total_service_time;
  double total_delay;
};

struct sim_state {
  struct park *park;

  struct generic_queue_list *clients;

  int available_vip_tickets ;

  // Statistics
  unsigned int stat_vip_clients;
  unsigned int stat_normal_clients;

  unsigned int total_clients_normal;
  unsigned int total_clients_vip;
  double total_service_time_normal;
  double total_service_time_vip ;
  double total_delay_normal;
  double total_delay_vip;

  struct ride_state *rides ;
  struct show_state *shows;
};

struct sim_state *create_sim_state(struct park *park) ;
void delete_sim_state(struct sim_state *state);

#endif