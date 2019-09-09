#include "tide_clock_firmware.h"

void setup() {
  pinMode(MAIN_BUS, INPUT);
  pinMode(TMR1, OUTPUT);
  pinMode(TMR2, OUTPUT);
  pinMode(PHA_A, OUTPUT);
  pinMode(PHA_B, OUTPUT);
  pinMode(NEOPIXEL, OUTPUT);
  pinMode(HALL, INPUT);

  Serial.begin(115200);

  connect_to_wifi();
  update_tides();
  setup_eeprom();
  init_dacs();
  init_position();
  start_server();

  // Turn on status LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  update_time_position();
  update_tides();
  check_main_bus_voltage();
  server_loop();
}
