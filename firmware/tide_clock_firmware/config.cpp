#include "tide_clock_firmware.h"

#include <EEPROM.h>

#define DAC_CALIBRATION_BYTES (NUM_TUBES*NUM_DAC_CALIBRATION_POINTS)

#define CALIBRATION_STATUS_BYTE_ADDR  DAC_CALIBRATION_BYTES
#define TIME_STEPS_OFFSET_ADDR        (CALIBRATION_STATUS_BYTE_ADDR + 1)
#define NEOPIXEL_OFFSET_ADDR          (TIME_STEPS_OFFSET_ADDR + 2)
#define NEOPIXEL_BRIGHTNESS_DAY_ADDR  (NEOPIXEL_OFFSET_ADDR + 1)
#define NEOPIXEL_BRIGHTNESS_NIGHT_ADDR (NEOPIXEL_BRIGHTNESS_DAY_ADDR + 1)

int dac_cal_addr(int dac_number, int cal_point) {
  return dac_number * NUM_DAC_CALIBRATION_POINTS + cal_point;
}
byte get_dac_calibration(int dac_number, int cal_point) {
  return EEPROM.read(dac_cal_addr(dac_number, cal_point));
}
void set_dac_calibration(int dac_number, int cal_point, byte cal_value) {
  EEPROM.write(dac_cal_addr(dac_number, cal_point), cal_value);
}

uint16_t read_eeprom_word(int addr) {
  uint16_t hi_byte = (uint16_t)EEPROM.read(addr);
  uint16_t lo_byte = (uint16_t)EEPROM.read(addr + 1);
  return (hi_byte << 8) + lo_byte;
}

void write_eeprom_word(int addr, uint16_t value) {
  EEPROM.write(addr, (byte)(value >> 8));
  EEPROM.write(addr + 1, (byte)(value & 0xff));
}

int get_time_steps_offset() {
  return (int)read_eeprom_word(TIME_STEPS_OFFSET_ADDR);
}
void set_time_steps_offset(int offset) {
  write_eeprom_word(TIME_STEPS_OFFSET_ADDR, (uint16_t)offset);
}

byte get_neopixel_offset() {
  return EEPROM.read(NEOPIXEL_OFFSET_ADDR);
}
void set_neopixel_offset(byte offset) {
  EEPROM.write(NEOPIXEL_OFFSET_ADDR, offset);  
}

byte get_neopixel_brightness(bool day) {
  if (day) {
    return EEPROM.read(NEOPIXEL_BRIGHTNESS_DAY_ADDR);    
  } else {
    return EEPROM.read(NEOPIXEL_BRIGHTNESS_NIGHT_ADDR);
  }
}
void set_neopixel_brightness(bool day, byte brightness) {
  if (day) {
    EEPROM.write(NEOPIXEL_BRIGHTNESS_DAY_ADDR, brightness);
  } else {
    EEPROM.write(NEOPIXEL_BRIGHTNESS_NIGHT_ADDR, brightness);
  }
}

void setup_eeprom() {
  // Setup EEPROM to store calibration values
  EEPROM.begin(256);
  byte cal_status = EEPROM.read(CALIBRATION_STATUS_BYTE_ADDR);
  if (cal_status != 0) {
    init_config();
  }
}

void publish_config() {
  String message = "{\"time_0_pos\":" + String(get_time_steps_offset());
  message += ", \"time_0_pixel\":" + String(get_neopixel_offset());
  message += ", \"brightness_day\":" + String(get_neopixel_brightness(true));
  message += ", \"brightness_night\": " + String(get_neopixel_brightness(false));
  message += ", \"dac_calibrations\":[";
  for (int dac_number = 0; dac_number < NUM_TUBES; dac_number++) {
    if (dac_number > 0) message += ",";
    message += "[";
    for (int cal_point = 0; cal_point < NUM_DAC_CALIBRATION_POINTS; cal_point++) {
      if (cal_point > 0) message += ",";
      message += String(get_dac_calibration(dac_number, cal_point), DEC);
    }
    message += "]";
  }
  message += "]}";
  server.send(200, "application/json", message);
}

#define MIN_DAC_VALUE 101
#define MAX_DAC_VALUE 225
void init_config() {
  Serial.println("Resetting configuration to defaults");
  for (int dac_number = 0; dac_number < NUM_TUBES; dac_number++) {
    for (int cal_pt = 0; cal_pt < NUM_DAC_CALIBRATION_POINTS; cal_pt++) {
      int dac_value = int(MIN_DAC_VALUE
                            + (MAX_DAC_VALUE - MIN_DAC_VALUE)*float(cal_pt)/(NUM_DAC_CALIBRATION_POINTS - 1));
      set_dac_calibration(dac_number, cal_pt, dac_value);
    }
  }
  set_time_steps_offset(0);
  set_neopixel_offset(0);
  set_neopixel_brightness(true, 0x40);
  set_neopixel_brightness(false, 0x20);
  EEPROM.write(CALIBRATION_STATUS_BYTE_ADDR, 0x00);
  save_config();
}

void save_config() {
  EEPROM.commit();  
}
