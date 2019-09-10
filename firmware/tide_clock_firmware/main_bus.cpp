#include "tide_clock_firmware.h"

#define R1 200.0
#define R2 2.8
#define MAX_VIN 3.3
const float MAIN_BUS_FACTOR = MAX_VIN * (R1 + R2) / (1024 * R2);

#define MAIN_BUS_CUTOFF 150.0

#define MAIN_BUS_CHECK_DELAY 1000 // Limit analog reads to 1/sec
float main_bus_voltage = -1.0;
unsigned long last_main_bus_check = 0;

float get_main_bus_voltage() {
  unsigned long timestamp = millis();
  if (timestamp < last_main_bus_check) last_main_bus_check = timestamp;
  if (main_bus_voltage < 0 || timestamp >= last_main_bus_check + MAIN_BUS_CHECK_DELAY) {
    main_bus_voltage = float(analogRead(MAIN_BUS)) * MAIN_BUS_FACTOR;
    last_main_bus_check = timestamp;
  }
  return main_bus_voltage;
}

bool main_bus_state = false;
void check_main_bus_voltage() {
  float voltage = get_main_bus_voltage();
  if (main_bus_state && voltage <= MAIN_BUS_CUTOFF) {
    Serial.println("Bus voltage too low - shutting off display");
    stop_anode_timers();
    set_display_enabled(false);
    main_bus_state = false;
  } else if (!main_bus_state && voltage > MAIN_BUS_CUTOFF) {
    Serial.println("Bus voltage detected - turning on display");
    start_anode_timers();
    set_display_enabled(true);
    main_bus_state = true;    
  }
}
