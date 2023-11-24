#ifndef SHOW_H
#define SHOW_H

#include "utils.h"

enum distribution_type;

struct show {
  const char *name;
  double popularity;
  double mean_time;
  enum distribution_type distribution;
  double mu;
  double sigma;
};

#endif SHOW_H