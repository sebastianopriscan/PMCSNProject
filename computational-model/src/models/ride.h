#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef RIDE_H
#define RIDE_H

#include "utils.h"

enum distribution_type;

struct ride {
  const char *name;
  double popularity;
  int server_num;
  double mean_time;
  enum distribution_type distribution;
  double mu;
  double sigma;
};

#endif