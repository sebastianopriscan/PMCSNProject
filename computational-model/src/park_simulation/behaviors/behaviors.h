#ifndef BEHAVIORS_H
#define BEHAVIORS_H
#include <simulation/simulation.h>
#include "../../models/model.h"
#include "../sim_state.h"

// Client Behaviors
void reach_park(struct simulation *sim, void *metadata);
void choose_attraction(struct simulation* sim, void *metadata);
void patience_lost(struct simulation* sim, void *metadata);

// Attraction Behaviors

#endif