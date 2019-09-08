#include <TidelibCrescentBeachMatanzasRiverICWWFlorida.h>
#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid = "ATTNzsAvGs";
const char* password = "g39i8fbeujzd";

TideCalc myTideCalc;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "EST5EDT", 1);
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  time_t now = time(nullptr);
  time_t now_local = mktime(localtime(&now));
  DateTime dt(now);
  float tide = myTideCalc.currentTide(dt);
  
  Serial.print(ctime(&now_local));
  Serial.print("Tide height: ");
  Serial.print(tide, 3);
  Serial.println(" ft.\n");
  delay(1000);
}
