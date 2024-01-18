#ifndef RUN_SIMULATION_H
#include "../models/model.h"

// Simulation and state must be deleted by caller
struct simulation* run_park_simulation_from_park(struct park*park, int log);
struct simulation* run_park_simulation(const char* path, int log);

#endif