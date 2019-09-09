#include "tide_clock_firmware.h"

#include <TidelibCrescentBeachMatanzasRiverICWWFlorida.h>

TideCalc myTideCalc;

int last_hour_mark = -1;
time_t hour_marks[NUM_TUBES];
float hour_tides[NUM_TUBES];

#define PAST_TIDES_OFFSET 11

void update_tides() {
  time_t now = time(nullptr);
  struct tm * ltime = localtime(&now);
  int current_hour = ltime->tm_hour;
  if (current_hour != last_hour_mark) {
    Serial.print("Recalculating tides\nCurrent time: ");
    Serial.println(datestr(now));
    last_hour_mark = current_hour;
    for (int hour_idx = 0; hour_idx < NUM_TUBES; hour_idx++) {
      ltime = localtime(&now);
      ltime->tm_hour = last_hour_mark + hour_idx - PAST_TIDES_OFFSET;
      ltime->tm_min = 0;
      ltime->tm_sec = 0;
      time_t mark = mktime(ltime);
      DateTime dt_mark(mark);
      float tide = myTideCalc.currentTide(dt_mark);
      hour_marks[hour_idx] = mark;
      hour_tides[hour_idx] = tide;

      Serial.print(datestr(mark));
      Serial.print(" - Tide = ");
      Serial.print(tide, 3);
      Serial.println(" ft");
    }
  }
}

void publish_tides() {
  String message = "";

  time_t now = time(nullptr);
  message += "Current time: ";
  message += datestr(now) + "\n";
  DateTime dt(now);
  float tide = myTideCalc.currentTide(dt);
  message += "Current tide: ";
  message += String(tide, 3);
  message += " ft\n\nTIDE TABLE\n----------\n";
  for (int idx = 0; idx < NUM_TUBES; idx++) {
    message += datestr(hour_marks[idx]);
    message += " > ";
    message += String(hour_tides[idx], 3);
    message += " ft\n";
    if (idx == PAST_TIDES_OFFSET) {
      message += "=============== NOW ===============\n";
    }
  }

  server.send(200, "text/plain", message);
}

void get_tides_by_hour(float* tides) {
  for (int i = 0; i < NUM_TUBES; i++) {
    struct tm * ltime = localtime(&hour_marks[i]);
    tides[ltime->tm_hour] = hour_tides[i];
  }
}
