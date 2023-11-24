#include <stdio.h>
#include <stdlib.h>
#include <json.h>

#include <simulation/simulation.h>
#include "deserializer/deserializer.h"

int main(void) {
  deserialize("docs/json-input-template.json");
  return 0;
}