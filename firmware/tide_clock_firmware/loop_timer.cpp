#include "tide_clock_firmware.h"

#define AVG_LOOPS 10
unsigned long last_loop = 0;
int curr_loop_time = 0;
unsigned long loop_times[AVG_LOOPS];

void init_loop_timer() {
  last_loop = micros();  
  for (int i = 0; i < AVG_LOOPS; i++) {
    loop_times[i] = 0;
  }
}
void mark_loop_timer() {
  unsigned long timestamp = micros();
  if (timestamp > last_loop) {
    loop_times[curr_loop_time] = (timestamp - last_loop);
    curr_loop_time++;
    if (curr_loop_time >= AVG_LOOPS) curr_loop_time = 0;
  }
  last_loop = timestamp;
}
unsigned long avg_loop_time() {
  unsigned long avg_time = 0;
  for (int i = 0; i < AVG_LOOPS; i++) {
    avg_time += loop_times[i];
  }
  avg_time /= AVG_LOOPS;
  return avg_time;
}
