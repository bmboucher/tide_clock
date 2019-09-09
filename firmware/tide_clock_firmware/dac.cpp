#include "tide_clock_firmware.h"

#include <Wire.h>

void init_dacs() {
  Wire.begin();
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
  0x0C, // i2c_channel = 0, A1 = 1, A0 = 1, channel 0
  0x0D, // i2c_channel = 0, A1 = 1, A0 = 1, channel 1
  0x0E, // i2c_channel = 0, A1 = 1, A0 = 1, channel 2
  0x0F, // i2c_channel = 0, A1 = 1, A0 = 1, channel 3
  0x04, // i2c_channel = 0, A1 = 0, A0 = 1, channel 0
  0x05, // i2c_channel = 0, A1 = 0, A0 = 1, channel 1
  0x06, // i2c_channel = 0, A1 = 0, A0 = 1, channel 2
  0x07, // i2c_channel = 0, A1 = 0, A0 = 1, channel 3
  0x08, // i2c_channel = 0, A1 = 1, A0 = 0, channel 0
  0x09, // i2c_channel = 0, A1 = 1, A0 = 0, channel 1
  0x0A, // i2c_channel = 0, A1 = 1, A0 = 0, channel 2
  0x0B, // i2c_channel = 0, A1 = 1, A0 = 0, channel 3
  0x1C, // i2c_channel = 1, A1 = 1, A0 = 1, channel 0
  0x1D, // i2c_channel = 1, A1 = 1, A0 = 1, channel 1
  0x1E, // i2c_channel = 1, A1 = 1, A0 = 1, channel 2
  0x1F, // i2c_channel = 1, A1 = 1, A0 = 1, channel 3
  0x14, // i2c_channel = 1, A1 = 0, A0 = 1, channel 0
  0x15, // i2c_channel = 1, A1 = 0, A0 = 1, channel 1
  0x16, // i2c_channel = 1, A1 = 0, A0 = 1, channel 2
  0x17, // i2c_channel = 1, A1 = 0, A0 = 1, channel 3
  0x18, // i2c_channel = 1, A1 = 1, A0 = 0, channel 0
  0x19, // i2c_channel = 1, A1 = 1, A0 = 0, channel 1
  0x1A, // i2c_channel = 1, A1 = 1, A0 = 0, channel 2
  0x1B  // i2c_channel = 1, A1 = 1, A0 = 0, channel 3
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

void set_dac_value(int dac_number, byte dac_value) {
  byte dac_addr = pgm_read_byte(DAC_ADDRESSES + dac_number);
  set_dac_by_addr(dac_addr, dac_value);
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
    float fraction = dac_value * (NUM_DAC_CALIBRATION_POINTS - 1) - 1;
    float prev_val = get_dac_calibration(dac_number, prev_pt);
    float next_val = get_dac_calibration(dac_number, next_pt);
    dac_value_byte = int(prev_val + (next_val - prev_val) * fraction);    
  }
  set_dac_value(dac_number, dac_value_byte);
}
