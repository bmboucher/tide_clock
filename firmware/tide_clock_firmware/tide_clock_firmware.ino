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
  
  #ifdef MOTION
  init_position();
  #endif
  
  start_server();

  // Turn on status LED
  digitalWrite(LED_BUILTIN, LOW);
}

#define MAJOR_LOOP_DELAY 1000
unsigned long last_major_loop = 0;

void loop() {
  unsigned long timestamp = millis();
  if (timestamp < last_major_loop) last_major_loop = timestamp;
  if (timestamp >= last_major_loop + MAJOR_LOOP_DELAY) {    
    update_tides();
    check_main_bus_voltage();
    last_major_loop = timestamp;
  }

  #ifdef MOTION
  update_time_position();
  #endif
  
  server_loop();
}
