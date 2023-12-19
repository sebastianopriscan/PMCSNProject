#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

enum distribution_type {
  NORMAL_DISTRIB,
  UNIFORM,
  EXPONENTIAL
};

double GetRandomFromDistributionType(int stream, enum distribution_type type, double mu, double sigma);

#endif