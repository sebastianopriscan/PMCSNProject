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
  int batch_size;
  enum distribution_type distribution;
  double mu;
  double sigma;
  double expected_wait;
};

#endif