#ifndef SIM_STATE_H
#define SIM_STATE_H

#include "../models/park.h"

struct ride_state {
  unsigned int stat_vip_clients;
  unsigned int stat_normal_clients;

  char *busy_servers ;
};

struct sim_state {
  struct park *park;

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
};

#endif