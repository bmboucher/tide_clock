#include "tide_clock_firmware.h"

#define ON_TICKS 3025 // 9.68 ms
#define OFF_TICKS 100 // 0.32 ms

void ICACHE_RAM_ATTR tmr1off();
void ICACHE_RAM_ATTR tmr2on();
void ICACHE_RAM_ATTR tmr2off();

void ICACHE_RAM_ATTR tmr1on() {
  digitalWrite(TMR1, HIGH);
  timer1_attachInterrupt(tmr1off);
  timer1_write(ON_TICKS);
}

void ICACHE_RAM_ATTR tmr1off() {
  digitalWrite(TMR1, LOW);
  timer1_attachInterrupt(tmr2on);
  timer1_write(OFF_TICKS);
}

void ICACHE_RAM_ATTR tmr2on() {
  digitalWrite(TMR2, HIGH);
  timer1_attachInterrupt(tmr2off);
  timer1_write(ON_TICKS);
}

void ICACHE_RAM_ATTR tmr2off() {
  digitalWrite(TMR2, LOW);
  timer1_attachInterrupt(tmr1on);
  timer1_write(OFF_TICKS);
}

void start_anode_timers() {
  Serial.println("Starting anode timers");
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  tmr1on();
}

void stop_anode_timers() {
  Serial.println("Stopping anode timers");
  timer1_disable();
  digitalWrite(TMR1, LOW);
  digitalWrite(TMR2, LOW);
}
