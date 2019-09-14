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
  String message = "{\"main_bus\":" + String(bus_voltage, 2);
  message += ", \"current_time\":\"" + datestr() + "\"";
  message += ", \"position\":" + String(get_current_steps());
  message += ", \"display_mode\":\"" + display_mode_name(get_display_mode()) + "\"";
  message += ", \"display_enabled\":" + String(is_display_enabled() ? "true" : "false");
  message += ", \"display_forced\":" + String(is_display_forced() ? "true" : "false");
  message += ", \"dac_values\":[";
  for (int i = 0; i < NUM_TUBES; i++) {
    message += String(get_dac_value(i));
    if (i < NUM_TUBES - 1) message += ",";
  }
  message += "], \"neopixel_colors\":[";
  for (int i = 0; i < NUM_TUBES; i++) {
    message += "\"0x" + String(get_pixel_color(i),HEX) + "\"";
    if (i < NUM_TUBES - 1) message += ",";
  }
  message += "]}";
  server.send(200, "application/json", message);
}

void seek_handler(bool absolute) {
  if (server.args() != 1) return;
  int pos = server.arg(0).toInt();
  if (absolute) {
    seek_position(pos);
  } else {
    seek_position_relative(pos);
  }
  server.send(200, "text/plain", String(get_current_steps()));
}

void set_time_zero() {
  set_time_steps_offset(get_current_steps());
  server.send(200, "text/plain", String(get_time_steps_offset()));
}

void set_dac_handler() {
  int dac_number = server.header("num").toInt();
  if (dac_number < 0 || dac_number >= NUM_TUBES) {
    server.send(400, "text/plain", "Invalid DAC number " + String(dac_number));
  } else {
    int dac_value = server.header("value").toInt();
    if (dac_value < 0 || dac_value >= 0xff) {
      server.send(400, "text/plain", "Invalid DAC value 0x" + String(dac_value, HEX));      
    } else {
      set_dac_value(dac_number, dac_value);
    
      String message = "DAC ";
      message += String(dac_number);
      message += " > 0x";
      message += String(dac_value, HEX);
      server.send(200, "text/plain", message);
    }
  }
}

void set_dac_calibrate_handler() {
  int dac_number = server.header("num").toInt();
  if (dac_number < 0 || dac_number >= NUM_TUBES) {
    server.send(400, "text/plain", "Invalid DAC number " + String(dac_number));
  } else {
    int dac_cal_pt = server.header("pt").toInt();
    if (dac_cal_pt < 0 || dac_cal_pt >= NUM_DAC_CALIBRATION_POINTS) {
      server.send(400, "text/plain", "Invalid DAC calibration point " + String(dac_cal_pt));      
    } else {
      byte cal_value = get_dac_value(dac_number);
      set_dac_calibration(dac_number, dac_cal_pt, cal_value);
    
      String message = "DAC " + String(dac_number);
      message += " CAL_PT " + String(dac_cal_pt);
      message += " > 0x";
      message += String(cal_value, HEX);
      server.send(200, "text/plain", message);
    }
  }  
}

void get_dac_calibrate_handler() {
  int dac_number = server.header("num").toInt();
  if (dac_number < 0 || dac_number >= NUM_TUBES) {
    server.send(400, "text/plain", "Invalid DAC number " + String(dac_number));
  } else {
    int dac_cal_pt = server.header("pt").toInt();
    if (dac_cal_pt < 0 || dac_cal_pt >= NUM_DAC_CALIBRATION_POINTS) {
      server.send(400, "text/plain", "Invalid DAC calibration point " + String(dac_cal_pt));      
    } else {
      byte cal_value = get_dac_calibration(dac_number, dac_cal_pt);
      set_dac_value(dac_number, cal_value);
    
      String message = "DAC " + String(dac_number);
      message += " CAL_PT " + String(dac_cal_pt);
      message += " > 0x";
      message += String(cal_value, HEX);
      server.send(200, "text/plain", message);
    }
  }  
}

void set_neopixel_brightness_handler() {
  String time_hdr = server.header("time");
  time_hdr.toLowerCase();
  bool is_day = (time_hdr.equals("day"));
  int brightness = server.header("value").toInt();
  if (brightness < 0 || brightness > 0xff) {
    server.send(400, "text/plain", "Invalid neopixel brightness level 0x" + String(brightness, HEX));
  } else {
    set_neopixel_brightness(is_day,  (byte)brightness);

    String message = is_day ? "Day" : "Night";
    message += " brightness > 0x" + String(brightness, HEX);
    server.send(200, "text/plain", message);
  }
}

void set_neopixel_offset_handler() {
  if (server.args() != 1) return;
  int offset = server.arg(0).toInt();
  while (offset < 0) offset += NUM_TUBES;
  while (offset >= NUM_TUBES) offset -= NUM_TUBES;
  set_neopixel_offset((byte)offset);

  server.send(200, "text/plain", String(offset));
}

void force_display_mode_handler(int mode) {
  force_display_mode(mode);  
  String message = "Setting display mode to ";
  server.send(200, "text/plain", "Setting display mode to " + display_mode_name(mode));
}

const char* HEADERS_TO_TRACK[] = {"num", "value", "pt", "time"};

void start_server() {
  // Configure and start HTTP server
  server.on("/tides", publish_tides);
  server.on("/config", publish_config);
  server.on("/config/reset", HTTP_POST, []() {
    init_config();
    publish_config();
  });
  server.on("/config/save", HTTP_POST, []() {
    save_config();
    publish_config();
  });
  server.on("/status", publish_status);
  server.on("/seek/abs", HTTP_POST, []() { seek_handler(true); });
  server.on("/seek/rel", HTTP_POST, []() { seek_handler(false); });
  server.on("/time_zero", HTTP_POST, set_time_zero);
  server.on("/time_zero", HTTP_GET, []() { 
    server.send(200, "text/plain", String(get_time_steps_offset()));
    seek_position(get_time_steps_offset()); 
  });
  server.on("/dac", HTTP_POST, set_dac_handler);
  server.on("/dac/calibrate", HTTP_POST, set_dac_calibrate_handler);
  server.on("/dac/calibrate", HTTP_GET, get_dac_calibrate_handler);
  server.on("/neopixel/brightness", HTTP_POST, set_neopixel_brightness_handler);
  server.on("/neopixel/offset", HTTP_POST, set_neopixel_offset_handler);
  server.on("/neopixel/offset", HTTP_GET, []() {
    server.send(200, "text/plain", String(get_neopixel_offset(), DEC));
    blink_time_zero_neopixel();
  });
  for (int mode = NORMAL; mode <= DEMO; mode++) {
    String endpoint = "/display/" + display_mode_name(mode);
    endpoint.toLowerCase();
    server.on(endpoint, HTTP_POST, [mode]() { force_display_mode_handler(mode); });
  }
  server.on("/display/release", HTTP_POST, []() {
    release_display();
    server.send(200, "text/plain", "Display released");
  });
  server.on("/display/enable", HTTP_POST, []() { 
    set_display_enabled(true); 
    server.send(200, "text/plain", "Display enabled");
  });
  server.on("/display/disable", HTTP_POST, []() { 
    set_display_enabled(false); 
    server.send(200, "text/plain", "Display disabled");
  });
  server.collectHeaders(HEADERS_TO_TRACK, sizeof(HEADERS_TO_TRACK)/sizeof(char*));
  server.begin();
  Serial.println("HTTP server started");
}
