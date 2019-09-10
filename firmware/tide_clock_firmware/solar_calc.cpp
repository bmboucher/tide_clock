#include <Dusk2Dawn.h>

#define GMT_OFFSET -5
Dusk2Dawn crescentBeach(29.792338, -81.260130, GMT_OFFSET);

time_t get_sunriseset(const struct tm* ltime, bool sunrise) {
  int minutes = 0;
  if (sunrise) {
    minutes = crescentBeach.sunrise(ltime->tm_year, ltime->tm_mon, ltime->tm_mday, false);
  } else {
    minutes = crescentBeach.sunset(ltime->tm_year, ltime->tm_mon, ltime->tm_mday, false);    
  }
  
  struct tm tmp;
  tmp.tm_year = ltime->tm_year;
  tmp.tm_mon  = ltime->tm_mon;
  tmp.tm_mday = ltime->tm_mday;
  tmp.tm_hour = minutes/60;
  tmp.tm_min  = minutes%60;
  tmp.tm_sec  = 0;
  tmp.tm_isdst = 0;
  return mktime(&tmp);
}

void get_sunrise_sunset(time_t& sunrise, time_t& sunset, bool fwd_only) {
  time_t now = time(nullptr);
  struct tm* ltime = localtime(&now);
  
  struct tm min_ltime = *ltime;
  min_ltime.tm_hour -= fwd_only ? 0 : 12;
  time_t min_time = mktime(&min_ltime);
  
  struct tm max_ltime = *ltime;
  max_ltime.tm_hour += fwd_only ? 24 : 12;
  time_t max_time = mktime(&max_ltime);

  sunrise = get_sunriseset(&min_ltime, true);
  if (sunrise < min_time) sunrise = get_sunriseset(&max_ltime, true);

  sunset = get_sunriseset(&min_ltime, false);
  if (sunset < min_time) sunset = get_sunriseset(&max_ltime, false);
}
