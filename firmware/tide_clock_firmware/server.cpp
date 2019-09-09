#include "tide_clock_firmware.h"

#include <WiFiManager.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

void connect_to_wifi() {
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
  Serial.println(datestr());
}

void server_loop() {
  server.handleClient();
  MDNS.update();
}

void publish_status() {
  float bus_voltage = get_main_bus_voltage();
  String message = "{\"main_bus\":";
  message += String(bus_voltage, 2);
  message += ", \"current_time\":\"";
  message += datestr();
  message += "\", \"position\":";
  message += String(get_current_steps());
  message += "}";
  server.send(200, "application/json", message);
}

void start_server() {
  // Configure and start HTTP server
  server.on("/tides", publish_tides);
  server.on("/config", publish_config);
  server.on("/config/reset", []() {
    init_config();
    publish_config();
  });
  server.on("/status", publish_status);
  server.begin();
  Serial.println("HTTP server started");
}
