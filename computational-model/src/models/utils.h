#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

struct double_value {
  double value;
};

extern struct sim_state;

enum distribution_type {
  NORMAL_DISTRIB,
  UNIFORM,
  EXPONENTIAL
};

double GetRandomFromDistributionType(int stream, enum distribution_type type, double mu, double sigma);
struct client *create_new_client(double clock, double end, struct sim_state* state, char until_end);

#endif