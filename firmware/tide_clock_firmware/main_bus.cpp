#include "tide_clock_firmware.h"

#define R1 200.0
#define R2 2.8
#define MAX_VIN 3.3
const float MAIN_BUS_FACTOR = MAX_VIN * (R1 + R2) / (1024 * R2);

#define MAIN_BUS_CUTOFF 150.0

float get_main_bus_voltage() {
  return float(analogRead(MAIN_BUS)) * MAIN_BUS_FACTOR;
}

bool main_bus_state = false;
void check_main_bus_voltage() {
  float voltage = get_main_bus_voltage();
  if (main_bus_state && voltage <= MAIN_BUS_CUTOFF) {
    stop_anode_timers();
    main_bus_state = false;
  } else if (!main_bus_state && voltage > MAIN_BUS_CUTOFF) {
    start_anode_timers();
    main_bus_state = true;    
  }
}
