#include "tide_clock_firmware.h"
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip(NUM_TUBES, NEOPIXEL, NEO_GRB + NEO_KHZ800);

unsigned long mark = 0;
unsigned long animation_time = 0;
int display_mode = NORMAL;

void init_display() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(get_neopixel_brightness(false)); // Initally set to night-level brightness
}

uint16_t adj_neopixel_addr(uint16_t n) {
  uint16_t adj_n = n + (uint16_t) get_neopixel_offset();
  if (adj_n >= NUM_TUBES) adj_n -= NUM_TUBES;
  return adj_n;
}

void set_neopixel_color_rgb(uint16_t n, uint32_t c) {
  strip.setPixelColor(adj_neopixel_addr(n), c);
}

uint32_t get_pixel_color(uint16_t n) {
  return strip.getPixelColor(adj_neopixel_addr(n));
}

void set_neopixel_color_hue(uint16_t n, uint16_t hue, uint8_t saturation = 255, uint8_t value = 255) {
  uint32_t color = Adafruit_NeoPixel::ColorHSV(hue, saturation, value);
  set_neopixel_color_rgb(n, color);
}

void blank_display() {
  for (int i = 0; i < NUM_TUBES; i++) {
    set_dac_value(i, 0x00);
    set_neopixel_color_rgb(i, 0x000000);
  }
  strip.show();
}

#define DEPOISON_TIME_PER_TUBE 120000 // 2 minutes for each 6-tube group
int depoison_cycle = 0;
void init_depoison() {
  depoison_cycle = 0;
  blank_display();
}
void depoison() {
  unsigned long timestamp = millis();
  if (timestamp < mark) mark = timestamp;
  if (timestamp >= mark + DEPOISON_TIME_PER_TUBE) {
    depoison_cycle++;
    if (depoison_cycle >= 4) depoison_cycle = 0;
    blank_display();
    mark = timestamp;
  }
  
  for (int i = depoison_cycle; i < NUM_TUBES; i += 4) set_dac_value(i, 0xff);
}

float get_day_frac(time_t now) {
  struct tm* ltime = localtime(&now);
  return ((float)(ltime->tm_hour * 60 * 60 + ltime->tm_min * 60 + ltime->tm_sec))/(60 * 60);
}

#define DAY_COLOR   0xffff00
#define NIGHT_COLOR 0x0000ff
#define SUNRISE_SUNSET_COLOR 0xffbf00

void normal_display() {
  display_mode = NORMAL;
  for (int i = 0; i < NUM_TUBES; i++) set_dac_value_float(i, tide_heights_by_tube[i]);

  time_t now = time(nullptr);
  time_t sunrise = 0;
  time_t sunset = 0;
  get_sunrise_sunset(sunrise, sunset);
  int sunrise_led = int(round(get_day_frac(sunrise)));
  int sunset_led  = int(round(get_day_frac(sunset)));
  bool is_day = (sunrise < now && now < sunset);
  strip.setBrightness(get_neopixel_brightness(is_day));
  
  for (int i = 0; i < NUM_TUBES; i++) {
    if (i == sunrise_led || i == sunset_led) {
      set_neopixel_color_rgb(i, SUNRISE_SUNSET_COLOR);
    } else if (i > sunrise_led && i < sunset_led) {
      set_neopixel_color_rgb(i, DAY_COLOR);
    } else {
      set_neopixel_color_rgb(i, NIGHT_COLOR);
    }
  }
  strip.show();
}

void update_animation_time() {
  unsigned long now = millis();
  if (now > mark) {
    animation_time += (now - mark);
  }
  mark = now;
}

int wave_amplitude_rpm = 0;
int wave_phase_rpm = 10;
int num_waves = 2;
const float ANGLE_PER_TUBE = 2 * PI / NUM_TUBES;
void init_wave_animation() {
  if (random(0,2) == 0) {
    wave_amplitude_rpm = 0;
  } else {
    wave_amplitude_rpm = random(10,20);
  }
  wave_phase_rpm = random(5,25);
  num_waves = random(1,5);
}
void wave_animation() {
  update_animation_time();
  float amplitude = cos(float(animation_time) * wave_amplitude_rpm * 2 * PI / 60000);
  float phase = float(animation_time) * wave_phase_rpm * 2 * PI / 60000;
  for (int i = 0; i < NUM_TUBES; i++) {
    float angle = num_waves * ANGLE_PER_TUBE * i + phase;
    while (angle > 2 * PI) angle -= 2 * PI;
    set_dac_value_float(i, 0.5 + 0.5 * amplitude * cos(angle));
    float hue = (angle / (2 * PI)) * 65536.0;
    set_neopixel_color_hue(i, (uint16_t)hue);
  }
  strip.show();
}

#define ALTERNATING_SPEED 1000
void alternating_animation() {
  update_animation_time();
  float state = float(animation_time) / ALTERNATING_SPEED;
  
  while (state >= 2.0) state -= 2.0;
  float hue1_f = (state / 2.0) * 65535;
  float hue2_f = hue1_f + 32768;
  if (hue2_f >= 65536) hue2_f -= 65536;
  uint16_t hue1 = (uint16_t)hue1_f;
  uint16_t hue2 = (uint16_t)hue2_f;  
  
  if (state >= 1.0) state = 2.0 - state;
  for (int i = 0; i < NUM_TUBES; i++) {
    set_dac_value_float(i, (i % 2 == 0) ? state : 1.0 - state);
    set_neopixel_color_hue(i, (i % 2 == 0) ? hue1 : hue2);
  }
  strip.show();
}

#define RANDOMIZATION_DELAY 10
#define MAX_STEP 3
#define STEPS_PER_NEOPIXEL 20
#define NUM_RAND_NEOPIXELS 3
int rand_step_count = 0;
byte rand_state[NUM_TUBES];
void init_random_animation() {
  rand_step_count = 0;
  for (int i = 0; i < NUM_TUBES; i++) {
    rand_state[i] = random(get_dac_calibration(i, 0), 
                           get_dac_calibration(i, NUM_DAC_CALIBRATION_POINTS - 1) + 1);
    set_dac_value(i, rand_state[i]);
  }
}
void random_animation() {
  unsigned long timestamp = millis();
  if (timestamp < mark) mark = timestamp;
  if (timestamp < mark + RANDOMIZATION_DELAY) return;
  mark = timestamp;

  for (int i = 0; i < NUM_TUBES; i++) {
    rand_state[i] += random(-MAX_STEP, MAX_STEP + 1);
    rand_state[i] = max(get_dac_calibration(i, 0), rand_state[i]);
    rand_state[i] = min(get_dac_calibration(i, NUM_DAC_CALIBRATION_POINTS - 1), rand_state[i]);
    set_dac_value(i, rand_state[i]);    
  }

  rand_step_count++;
  if (rand_step_count >= STEPS_PER_NEOPIXEL) {
    for (int i = 0; i < NUM_TUBES; i++) set_neopixel_color_rgb(i, 0x000000);
    for (int i = 0; i < NUM_RAND_NEOPIXELS; i++) {
      set_neopixel_color_rgb(random(0, NUM_TUBES), 0xffffff);
    }
    strip.show();
    rand_step_count = 0;
  }
}

#define PULSE_FACTOR 0.4
#define PULSE_RPM 30
#define PULSE_THRESHOLD 0.1
#define PULSE_HUE_RPM 20
void pulse_animation() {
  update_animation_time();
  float pulse_center = float(animation_time) * PULSE_RPM * NUM_TUBES / 60000;
  float hue = float(animation_time) * PULSE_HUE_RPM / 60000;
  hue = hue - floor(hue);
  uint16_t hue_int = (uint16_t)(hue * 65535);
  for (int i = 0; i < NUM_TUBES; i++) {
    float x = (float)i - pulse_center;
    while (x < -NUM_TUBES/2) { x += NUM_TUBES; }
    while (x > NUM_TUBES/2) { x -= NUM_TUBES; }
    x *= PULSE_FACTOR;
    float y = exp(-(x * x));
    if (y < PULSE_THRESHOLD) {
      set_dac_value(i, 0x00);
      set_neopixel_color_rgb(i, 0x000000);
    } else {
      set_dac_value_float(i, y);
      set_neopixel_color_hue(i, hue_int, 255, (uint8_t)(y * 255));
    }
  }
  strip.show();
}

#define DEMO_DELAY 15000
unsigned long demo_mark = 0;
int current_demo_animation = WAVE;
void init_demo() {
  demo_mark = millis();
  current_demo_animation = WAVE;
}
void demo_animation() {
  unsigned long timestamp = millis();
  if (timestamp < demo_mark || timestamp >= demo_mark + DEMO_DELAY) {
    demo_mark = timestamp;
    current_demo_animation++;
    if (current_demo_animation == DEMO) current_demo_animation = WAVE;
    switch (current_demo_animation) {
      case WAVE: init_wave_animation(); break;
      case RANDOM: init_random_animation(); break;
    }
  }

  switch (current_demo_animation) {
    case WAVE: wave_animation(); break;
    case ALTERNATING: alternating_animation(); break;
    case RANDOM: random_animation(); break;
    case PULSE: pulse_animation(); break; 
  }
}

void switch_to_display_mode(int mode) {
  if (display_mode == mode) return;
  display_mode = mode;
  mark = millis();
  animation_time = 0;
  switch (display_mode) {
    case DEPOISON: init_depoison(); break;
    case WAVE: init_wave_animation(); break;
    case RANDOM: init_random_animation(); break;
    case DEMO: init_demo(); break;
  }
  Serial.print("Switched to display mode ");
  Serial.println(display_mode_name(mode));
}

bool display_forced = false;
void force_display_mode(int mode) {
  display_forced = true;
  switch_to_display_mode(mode);
}
void release_display() {
  display_forced = false;
  switch_to_display_mode(NORMAL);
}

bool display_enabled = false;
void display_loop() {
  if (!display_enabled) return;
  
  if (!display_forced) {
    time_t now = time(nullptr);
    struct tm* ltime = localtime(&now);
    if (ltime->tm_min == 0 && ltime->tm_sec < 30) {
      if (display_mode == NORMAL || display_mode == DEPOISON) {
        switch_to_display_mode(random(WAVE, DEMO));
      }
    } else if (ltime->tm_hour == 2) {
      switch_to_display_mode(DEPOISON);
    } else {
      switch_to_display_mode(NORMAL);
    }
  }
  
  switch (display_mode) {
    case NORMAL: normal_display(); break;
    case DEPOISON: depoison(); break;
    case WAVE: wave_animation(); break;
    case ALTERNATING: alternating_animation(); break;
    case RANDOM: random_animation(); break;
    case PULSE: pulse_animation(); break;
    case DEMO: demo_animation(); break;
  }
}

void set_display_enabled(bool en) {
  display_enabled = en;
  if (!en) blank_display();
}

void blink_time_zero_neopixel() {
  for (int i = 0; i < NUM_TUBES; i++) {
    set_neopixel_color_rgb(i, 0x000000);
  }
  for (int i = 0; i < 4; i++) {
    set_neopixel_color_rgb(0, 0xffffff);
    strip.show();    
    delay(250);
    set_neopixel_color_rgb(0, 0x000000);
    strip.show();    
    delay(250);
  }
}

String display_mode_name(int mode) {
  switch (mode) {
    case NORMAL: return "NORMAL";
    case DEPOISON: return "DEPOISON";
    case WAVE: return "WAVE";
    case ALTERNATING: return "ALTERNATING";
    case RANDOM: return "RANDOM";
    case PULSE: return "PULSE";
    case DEMO: return "DEMO";
  }
}

int get_display_mode() {
  return display_mode;
}

bool is_display_forced() {
  return display_forced;
}

bool is_display_enabled() {
  return display_enabled;
}
