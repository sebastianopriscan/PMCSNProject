#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "park_simulation/run_simulation.h"

// Usage: <program> --verbose 1000/100/10/1
int main(int argc, char **argv) {
  int log = 0b1000;
  if (argc == 3 && (!strcmp("--verbose", argv[1]) || !strcmp("-v", argv[1]))) {
    log = strtol(argv[2], NULL, 2);
    if (errno == ERANGE) {
      perror("Error converting verbose input to log");
      return ERANGE;
    }
  }
  run_park_simulation("../../../computational-model/tests/test.json", log) ;
  return 0;
}