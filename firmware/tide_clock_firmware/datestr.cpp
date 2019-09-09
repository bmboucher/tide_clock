#include "tide_clock_firmware.h"

#include <time.h>

String datestr(time_t now) {
  if (now == 0) now = time(nullptr);
  char buffer[80];
  strftime(buffer, 80, "%a %b %d %Y %H:%M:%S", localtime(&now));
  return String(buffer);
}
