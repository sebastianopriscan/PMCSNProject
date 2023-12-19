#include "model.h"
#include <rngs.h>
#include <rvgs.h>

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