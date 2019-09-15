#include "tide_clock_firmware.h"

#include <time.h>

#define MOTOR_REV_STEPS 513
#define MAX_RPM 10
#define GEAR_RATIO 84/16

const long MIN_MS_PER_STEP = ceil(60000/(MAX_RPM * MOTOR_REV_STEPS));
const int CLOCK_REV_STEPS = MOTOR_REV_STEPS*GEAR_RATIO;

int curr_phase = 0;
int step_count = 0;
int prev_hall_steps = 0;
unsigned long last_step_ms = 0;

const int PHASES[][2] = {{LOW,LOW},
                        {HIGH,LOW},
                        {HIGH,HIGH},
                        {LOW,HIGH}};
                        
void motor_step(int dir = 1) {
  curr_phase += dir;
  if (curr_phase < 0) curr_phase = 3;
  if (curr_phase > 3) curr_phase = 0;

  step_count += dir;
  while (step_count < 0) step_count += CLOCK_REV_STEPS;
  while (step_count >= CLOCK_REV_STEPS) step_count -= CLOCK_REV_STEPS;
  
  digitalWrite(PHA_A, PHASES[curr_phase][0]);
  digitalWrite(PHA_B, PHASES[curr_phase][1]);

  last_step_ms = millis();

  if (dir > 0) {
    if (digitalRead(HALL) == HIGH) {
      prev_hall_steps++;
    } else {
      if (prev_hall_steps > CLOCK_REV_STEPS / 4) {
        Serial.print("Hall sensor tripped at position ");
        Serial.println(step_count);
        Serial.print(" / time: ");
        Serial.print(datestr());
        Serial.println(" - resetting steps");
        step_count = 0;
      }
      prev_hall_steps = 0;
    }
  }
}

void init_position() {
  if (digitalRead(HALL) == LOW) {
    Serial.println("Hall sensor is already tripped - backing up");
    for (int i = 0; i < CLOCK_REV_STEPS / 4; i++) {
      motor_step(-1);
      delay(MIN_MS_PER_STEP);
    }
  }
  step_count = 0;
  int my_step_count = 0;
  while (digitalRead(HALL) == HIGH) {
    motor_step(1);
    my_step_count++;
    if (my_step_count >= CLOCK_REV_STEPS) {
      Serial.println("Full revolution completed without tripping Hall sensor!!!");
      break;
    }
    delay(MIN_MS_PER_STEP);
  }
  step_count = 0;
  prev_hall_steps = 0;
}

bool motion_enabled = true;
void set_motion_enabled(bool en) {
  motion_enabled = en;
}
bool is_motion_enabled() {
  return motion_enabled;
}

// Minimum number of back steps for which the cylinder will do a full revolution
// This prevents excessive retracing when the Hall sensor is tripped
#define MIN_BACK_MOTION 10 // Roughly 5 minutes

void update_time_position() {
  if (!motion_enabled) return;
  
  unsigned long timestamp = millis();
  if (timestamp < last_step_ms) last_step_ms = timestamp;
  if (timestamp < last_step_ms + MIN_MS_PER_STEP) return;
  
  time_t now = time(nullptr);
  struct tm * ltime = localtime(&now);
  float hr_frac = (float)(ltime->tm_hour * 3600 + ltime->tm_min * 60 + ltime->tm_sec) / (24*60*60);
  int tgt_steps = (int)(hr_frac * CLOCK_REV_STEPS) + get_time_steps_offset();
  while (tgt_steps < 0) tgt_steps += CLOCK_REV_STEPS;
  while (tgt_steps >= CLOCK_REV_STEPS) tgt_steps -= CLOCK_REV_STEPS;
  if (tgt_steps != step_count) {
    int back_steps = step_count - tgt_steps;
    while (back_steps < 0) back_steps += CLOCK_REV_STEPS;
    if (back_steps < MIN_BACK_MOTION) return;
    
    motor_step();
  }
}

int get_current_steps() {
  return step_count;
}
float get_position_time() {
  int time_diff = get_current_steps() - get_time_steps_offset();
  while (time_diff < 0) time_diff += CLOCK_REV_STEPS;
  while (time_diff >= CLOCK_REV_STEPS) time_diff -= CLOCK_REV_STEPS;
  return float(time_diff) * 24 / CLOCK_REV_STEPS;
}

#define LOCAL_MOVE_STEPS (MOTOR_REV_STEPS/2)
#define LOCAL_MOVE_SLOWDOWN 3
void seek_position(int position) {
  while (position < 0) position += CLOCK_REV_STEPS;
  while (position >= CLOCK_REV_STEPS) position -= CLOCK_REV_STEPS;
  int my_step_count = 0;
  while (my_step_count <= CLOCK_REV_STEPS + MOTOR_REV_STEPS && step_count != position) {
    motor_step();
    my_step_count++;
    int diff = position - step_count;
    while (diff < 0) diff += CLOCK_REV_STEPS;
    while (diff >= CLOCK_REV_STEPS) diff -= CLOCK_REV_STEPS;
    if (diff <= LOCAL_MOVE_STEPS) {
      delay(LOCAL_MOVE_SLOWDOWN * MIN_MS_PER_STEP);
    } else {
      delay(MIN_MS_PER_STEP);
    }
  }
}

void seek_position_relative(int pos_diff) {
  seek_position(step_count + pos_diff);
}

void seek_time(float hour) {
  hour -= floor(hour / 24) * 24;
  int tgt_steps = (int)(hour * CLOCK_REV_STEPS / 24) + get_time_steps_offset();
  seek_position(tgt_steps);
}
