#include "tide_clock_firmware.h"

//#define MOTION

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
  
  #ifdef MOTION
  init_position();
  #endif
  
  start_server();

  // Turn on status LED
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  update_tides();
  check_main_bus_voltage();

  #ifdef MOTION
  update_time_position();
  #endif

  display_loop();
  server_loop();
}
