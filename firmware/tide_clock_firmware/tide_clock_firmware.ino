#include <WiFiManager.h>
#include <TidelibCrescentBeachMatanzasRiverICWWFlorida.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <time.h>

TideCalc myTideCalc;

int last_hour_mark = -1;
time_t hour_marks[24];
float hour_tides[24];

ESP8266WebServer server(80);

String lst_str(time_t now = 0) {
  if (now == 0) now = time(nullptr);
  char buffer[80];
  strftime(buffer, 80, "%a %b %d %Y %H:%M:%S", localtime(&now));
  return String(buffer);
}

void update_tides() {
  time_t now = time(nullptr);
  struct tm * ltime = localtime(&now);
  int current_hour = ltime->tm_hour;
  if (current_hour != last_hour_mark) {
    Serial.print("Recalculating tides\nCurrent time: ");
    Serial.println(lst_str(now));
    last_hour_mark = current_hour;
    for (int hour_idx = 0; hour_idx < 24; hour_idx++) {
      ltime = localtime(&now);
      ltime->tm_hour = last_hour_mark + hour_idx - 11;
      ltime->tm_min = 0;
      ltime->tm_sec = 0;
      time_t mark = mktime(ltime);
      DateTime dt_mark(mark);
      float tide = myTideCalc.currentTide(dt_mark);
      hour_marks[hour_idx] = mark;
      hour_tides[hour_idx] = tide;

      Serial.print(lst_str(mark));
      Serial.print(" - Tide = ");
      Serial.print(tide, 3);
      Serial.println(" ft");
    }
  }
}

void publishTides() {
  String message = "";

  time_t now = time(nullptr);
  message += "Current time: ";
  message += lst_str(now) + "\n";
  DateTime dt(now);
  float tide = myTideCalc.currentTide(dt);
  message += "Current tide: ";
  message += String(tide, 3);
  message += " ft\n\nTIDE TABLE\n----------\n";
  for (int idx = 0; idx < 24; idx++) {
    message += lst_str(hour_marks[idx]);
    message += " > ";
    message += String(hour_tides[idx], 3);
    message += " ft\n";
  }

  server.send(200, "text/plain", message);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFiManager wifiManager;
  wifiManager.autoConnect("TideClockAP");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Startup MDNS so that URLs start with http://tideclock.local/
  if (MDNS.begin("tideclock")) {
    Serial.println("MDNS responder started");  
  }

  // Setup time from NTP server
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "EST5EDT", 1);
  Serial.print("\nWaiting for time");
  while (time(nullptr) < 1546300800) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.print("Current time: ");
  Serial.println(lst_str());

  // Calculate tide levels at each hour
  update_tides();

  // Configure and start HTTP server
  server.on("/tides", publishTides);
  server.begin();
  Serial.println("HTTP server started");

  // Turn on status LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  // Recalculate tide levels if an hour mark has passed
  update_tides();

  // Server loop
  server.handleClient();
  MDNS.update();
}
