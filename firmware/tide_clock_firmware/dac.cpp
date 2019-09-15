#include "tide_clock_firmware.h"

#include <Wire.h>

// Buffer for DAC values prevents unnecessary I2C communication
byte prev_dac_value[NUM_TUBES];

void init_dacs() {
  Wire.begin();
  for (int i = 0; i < NUM_TUBES; i++) prev_dac_value[i] = 0x00;
}

#define PCA9540B_ADDR     0x70
#define PCA9540B_CTRL     0x04
#define DAC5574_BASE_ADDR 0x4c
#define DAC5574_CTRL      0x10

byte curr_i2c_channel = 0xff;
void select_i2c_channel(byte i2c_channel) {
  if (curr_i2c_channel == i2c_channel) return;
  Wire.beginTransmission(PCA9540B_ADDR);
  byte ctrl = PCA9540B_CTRL + i2c_channel;
  Wire.write(ctrl);
  Wire.endTransmission();
  curr_i2c_channel = i2c_channel;
}

const byte DAC_ADDRESSES[] PROGMEM = {
  0b01100, // i2c_channel = 0, A1 = 1, A0 = 1, channel = 0
  0b01110, // i2c_channel = 0, A1 = 1, A0 = 1, channel = 2
  0b01101, // i2c_channel = 0, A1 = 1, A0 = 1, channel = 1
  0b01111, // i2c_channel = 0, A1 = 1, A0 = 1, channel = 3
  0b00100, // i2c_channel = 0, A1 = 0, A0 = 1, channel = 0
  0b00110, // i2c_channel = 0, A1 = 0, A0 = 1, channel = 2
  0b00101, // i2c_channel = 0, A1 = 0, A0 = 1, channel = 1
  0b00111, // i2c_channel = 0, A1 = 0, A0 = 1, channel = 3
  0b01000, // i2c_channel = 0, A1 = 1, A0 = 0, channel = 0
  0b01010, // i2c_channel = 0, A1 = 1, A0 = 0, channel = 2
  0b01001, // i2c_channel = 0, A1 = 1, A0 = 0, channel = 1
  0b01011, // i2c_channel = 0, A1 = 1, A0 = 0, channel = 3
  0b11000, // i2c_channel = 1, A1 = 1, A0 = 0, channel = 0
  0b11010, // i2c_channel = 1, A1 = 1, A0 = 0, channel = 2
  0b11001, // i2c_channel = 1, A1 = 1, A0 = 0, channel = 1
  0b11011, // i2c_channel = 1, A1 = 1, A0 = 0, channel = 3
  0b10100, // i2c_channel = 1, A1 = 0, A0 = 1, channel = 0
  0b10110, // i2c_channel = 1, A1 = 0, A0 = 1, channel = 2
  0b10101, // i2c_channel = 1, A1 = 0, A0 = 1, channel = 1
  0b10111, // i2c_channel = 1, A1 = 0, A0 = 1, channel = 3
  0b11100, // i2c_channel = 1, A1 = 1, A0 = 1, channel = 0
  0b11110, // i2c_channel = 1, A1 = 1, A0 = 1, channel = 2
  0b11101, // i2c_channel = 1, A1 = 1, A0 = 1, channel = 1
  0b11111  // i2c_channel = 1, A1 = 1, A0 = 1, channel = 3
};

// DAC address byte is:
//   3 MSB "don't care"
//   1 i2c_channel
//   2 DAC A1/A0
//   2 DAC channel
void set_dac_by_addr(byte dac_addr, byte dac_value) {
  byte i2c_channel = (0x10 & dac_addr) >> 4;
  byte dac5574_addr = DAC5574_BASE_ADDR + ((0x0c & dac_addr) >> 2);
  byte dac_channel = 0x03 & dac_addr;
  
  select_i2c_channel(i2c_channel);
  Wire.beginTransmission(dac5574_addr);
  byte ctrl = DAC5574_CTRL + (dac_channel << 1);
  Wire.write(ctrl);
  Wire.write(dac_value);
  Wire.write(0x00);
  Wire.endTransmission();
}

byte get_dac_value(int dac_number) {
  return prev_dac_value[dac_number];
}
void set_dac_value(int dac_number, byte dac_value) {
  if (prev_dac_value[dac_number] == dac_value) return;
  byte dac_addr = pgm_read_byte(DAC_ADDRESSES + dac_number);
  set_dac_by_addr(dac_addr, dac_value);
  prev_dac_value[dac_number] = dac_value;
}

void set_dac_value_float(int dac_number, float dac_value) {
  byte dac_value_byte = 0x00;
  if (dac_value <= 0.0) {
    dac_value_byte = get_dac_calibration(dac_number, 0);
  } else if (dac_value >= 1.0) {
    dac_value_byte = get_dac_calibration(dac_number, NUM_DAC_CALIBRATION_POINTS - 1);    
  } else {
    int prev_pt = int(dac_value * (NUM_DAC_CALIBRATION_POINTS - 1));
    int next_pt = prev_pt + 1;
    float fraction = dac_value * (NUM_DAC_CALIBRATION_POINTS - 1) - prev_pt;
    float prev_val = get_dac_calibration(dac_number, prev_pt);
    float next_val = get_dac_calibration(dac_number, next_pt);
    dac_value_byte = int(prev_val + (next_val - prev_val) * fraction);    
  }
  set_dac_value(dac_number, dac_value_byte);
}
