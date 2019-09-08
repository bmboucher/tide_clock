#include <Wire.h>

#define DAC5574_ADDR 0x4f
#define TMR1_PIN 3
#define TMR2_PIN 2
#define TMR_DELAY 10000 //20us = 50Hz
#define TMR_GAP 100

bool tmr1_active = true;
unsigned long tmr_mark = 0;

unsigned long last_dac_set = 0;

const uint8_t MIN_DAC_VALUE = 0;
const uint8_t MAX_DAC_VALUE = 255;
#define DAC_DELAY 10
uint8_t curr_dac_value = MIN_DAC_VALUE;
uint8_t curr_dac_step = 1;

void setup()
{
  pinMode(TMR1_PIN, OUTPUT);
  pinMode(TMR2_PIN, OUTPUT);
  
  digitalWrite(TMR1_PIN, LOW);
  digitalWrite(TMR2_PIN, LOW);
  
  Serial.begin(9600);
  delay(500);
  Serial.println("\nIN-9 Driver Test Shield\n");
  
  Wire.begin();

  for (byte channel = 0; channel < 4; channel++) {
    set_dac_value(channel, curr_dac_value);
  }

  tmr_mark = micros();
  digitalWrite(TMR1_PIN, HIGH);

  last_dac_set = millis();
}


void loop()
{
  unsigned long timestamp = micros();
  if (timestamp < tmr_mark || timestamp >= tmr_mark + TMR_DELAY) {
    if (tmr1_active) {
      digitalWrite(TMR1_PIN, LOW);
      delayMicroseconds(TMR_GAP);
      digitalWrite(TMR2_PIN, HIGH);
    } else {
      digitalWrite(TMR2_PIN, LOW);
      delayMicroseconds(TMR_GAP);
      digitalWrite(TMR1_PIN, HIGH);      
    }
    tmr1_active = !tmr1_active;
    tmr_mark = timestamp;
  }

  timestamp = millis();
  if (timestamp < last_dac_set || timestamp >= last_dac_set + DAC_DELAY) {
    curr_dac_value += curr_dac_step;
    if (curr_dac_value >= MAX_DAC_VALUE) {
      curr_dac_value = MAX_DAC_VALUE;
      curr_dac_step = -1;
    } else if (curr_dac_value <= MIN_DAC_VALUE) {
      curr_dac_value = MIN_DAC_VALUE;
      curr_dac_step = 1;
    }
    
    for (byte channel = 0; channel < 4; channel++) {
      set_dac_value(channel, curr_dac_value);
    }        
    last_dac_set = timestamp;
  }
}

void set_dac_value(byte channel, byte value) {
  Wire.beginTransmission(DAC5574_ADDR);
  byte ctrl = 0x10 | (channel << 1);
  Wire.write(ctrl);    // Control byte
  Wire.write(value);   // MSB = value
  Wire.write(0x00);    // LSB = don't care
  Wire.endTransmission();
  
  //Serial.print("Set DAC channel ");
  //Serial.print(int(channel));
  //Serial.print(" to value ");
  //Serial.println(int(value));
}
