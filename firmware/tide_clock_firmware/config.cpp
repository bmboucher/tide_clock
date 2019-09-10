#include "tide_clock_firmware.h"

#include <EEPROM.h>

#define DAC_CALIBRATION_BYTES (NUM_TUBES*NUM_DAC_CALIBRATION_POINTS)

#define CALIBRATION_STATUS_BYTE_ADDR  DAC_CALIBRATION_BYTES
#define TIME_STEPS_OFFSET_ADDR        (CALIBRATION_STATUS_BYTE_ADDR + 1)
#define NEOPIXEL_OFFSET_ADDR          (TIME_STEPS_OFFSET_ADDR + 2)
#define NEOPIXEL_BRIGHTNESS_ADDR      (NEOPIXEL_OFFSET_ADDR + 1)

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

int dac_cal_addr(int dac_number, int cal_point) {
  return dac_number * NUM_DAC_CALIBRATION_POINTS + cal_point;
}
byte get_dac_calibration(int dac_number, int cal_point) {
  return EEPROM.read(dac_cal_addr(dac_number, cal_point));
}
void set_dac_calibration(int dac_number, int cal_point, byte cal_value) {
  EEPROM.write(dac_cal_addr(dac_number, cal_point), cal_value);
}

int get_time_steps_offset() {
  int16_t short_offset;
  return EEPROM_readAnything(TIME_STEPS_OFFSET_ADDR, short_offset);
  return (int)short_offset;
}
void set_time_steps_offset(int offset) {
  int16_t short_offset = (int16_t)offset;
  EEPROM_writeAnything(TIME_STEPS_OFFSET_ADDR, short_offset);
}

byte get_neopixel_offset() {
  return EEPROM.read(NEOPIXEL_OFFSET_ADDR);
}
void set_neopixel_offset(byte offset) {
  EEPROM.write(NEOPIXEL_OFFSET_ADDR, offset);  
}

byte get_neopixel_brightness() {
  return EEPROM.read(NEOPIXEL_BRIGHTNESS_ADDR);
}
void set_neopixel_brightness(byte brightness) {
  EEPROM.write(NEOPIXEL_BRIGHTNESS_ADDR, brightness);  
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
  String message = "DAC calibrations:\n";
  for (int dac_number = 0; dac_number < NUM_TUBES; dac_number++) {
    message += "  DAC ";
    message += String(dac_number);
    message += ":\t";
    for (int cal_point = 0; cal_point < NUM_DAC_CALIBRATION_POINTS; cal_point++) {
      message += String(get_dac_calibration(dac_number, cal_point), DEC);
      message += " ";
    }
    message += "\n";
  }
  server.send(200, "text/plain", message);
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
  EEPROM.write(CALIBRATION_STATUS_BYTE_ADDR, 0x00);
  save_config();
}

void save_config() {
  EEPROM.commit();  
}
