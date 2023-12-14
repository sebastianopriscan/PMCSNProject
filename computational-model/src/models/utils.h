#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H
#include <rngs.h>
#include <rvgs.h>

enum distribution_type {
  NORMAL_DISTRIB,
  UNIFORM,
  EXPONENTIAL
};

double GetRandomFromDistributionType(int stream, enum distribution_type type, double mu, double sigma) {
  SelectStream(stream);
  switch(type) {
    case NORMAL_DISTRIB:
      return Normal(mu, sigma);
      break;
    case UNIFORM:
      return Uniform(mu, sigma);
      break;
    case EXPONENTIAL:
      return Exponential(mu) ;
      break;
  }
}

#endif