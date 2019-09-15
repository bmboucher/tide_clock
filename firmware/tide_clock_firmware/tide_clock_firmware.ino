#include "tide_clock_firmware.h"

void setup() {
  pinMode(MAIN_BUS, INPUT);
  pinMode(TMR1, OUTPUT);
  pinMode(TMR2, OUTPUT);
  pinMode(PHA_A, OUTPUT);
  pinMode(PHA_B, OUTPUT);
  pinMode(NEOPIXEL, OUTPUT);
  pinMode(HALL, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);

  connect_to_wifi();
  update_tides();
  setup_eeprom();
  init_dacs();
  init_display();
  
  init_position();  
  start_server();

  // Turn on status LED
  digitalWrite(LED_BUILTIN, LOW);
  init_loop_timer();
}

void loop() {
  update_tides();
  check_main_bus_voltage();
  update_time_position();
  display_loop();
  server_loop();
  mark_loop_timer();
}
