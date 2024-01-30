#ifndef LAMBDA_EVALUATOR_H
#define LAMBDA_EVALUATOR_H

struct return_value {
  double lambda;
  double wait_time;
};

struct return_value* run_lambda_evaluator(double expected_wait, double threshold, double mu, int server_num, int batch_size);

#endif