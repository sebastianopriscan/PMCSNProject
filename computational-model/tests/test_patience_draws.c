#include <stdio.h>
#include <stdlib.h>
#include <rngs.h>

#include "../src/park_simulation/behaviors/behaviors.h"
#include "../src/models/utils.h"

#define NUM_RUNS 100

int main(int argc, char **argv) {

    if(argc != 3) {
        fprintf(stderr, "Usage: ./exName meanTime multiplier\n") ;
        exit(1) ;
    }

    double mean_time = strtod(argv[1], NULL) ;
    double multiplier = strtod(argv[2], NULL) ;
    PlantSeeds(12345) ;

    for(int i = 0 ; i < NUM_RUNS ; i++) {
        double p = GetRandomFromDistributionType(1, NORMAL, 0.5, 0.1) ;
        double patience = GetRandomFromDistributionType(0, NORMAL_DISTRIB, mean_time + multiplier, mean_time * p);

        patience = patience < 0 ? -patience : patience ;

        printf("This client will wait for %6.6f minutes\n", patience) ;
    }
}