#include <ESP8266WebServer.h>
#include <WString.h>
#include <stdint.h>

#define NUM_TUBES 24
#define NUM_DAC_CALIBRATION_POINTS 8

// Pin definitions
#define MAIN_BUS A0
#define TMR1     D4
#define TMR2     D3
#define PHA_A    D8
#define PHA_B    D6
#define NEOPIXEL D5
#define HALL     D7

// Display modes
#define NORMAL      1
#define DEPOISON    2
#define WAVE        3
#define ALTERNATING 4
#define RANDOM      5
#define PULSE       6
#define DEMO        7

extern ESP8266WebServer server;

String datestr(time_t now = 0);

/*** WiFi/server methods ***/
void connect_to_wifi();
void start_server();
void server_loop();

/*** Tide calculator ***/
extern float tide_heights_by_tube[NUM_TUBES];
void update_tides();
void publish_tides();

/*** Solar calculator ***/
void get_sunrise_sunset(time_t& sunrise, time_t& sunset, bool fwd_only = false);

/*** EEPROM configuration ***/
void setup_eeprom();
void init_config();
void publish_config();
byte get_dac_calibration(int dac_number, int cal_point);
void set_dac_calibration(int dac_number, int cal_point, byte cal_value);
int get_time_steps_offset();
void set_time_steps_offset(int offset);
byte get_neopixel_offset();
void set_neopixel_offset(byte offset);
byte get_neopixel_brightness(bool day);
void set_neopixel_brightness(bool day, byte brightness);
void save_config();

/*** DACs ***/
void init_dacs();
byte get_dac_value(int dac_number);
void set_dac_value(int dac_number, byte dac_value);
void set_dac_value_float(int dac_number, float dac_value);

/*** Main bus ***/
float get_main_bus_voltage();
void check_main_bus_voltage();

/*** Anode timers ***/
void start_anode_timers();
void stop_anode_timers();

/*** Motion ***/
void init_position();
void update_time_position();
int get_current_steps();
void seek_position(int position);
void seek_position_relative(int pos_diff);

/*** Display (IN-9s + neopixels) ***/
void init_display();
void display_loop();
void set_display_enabled(bool en);
void force_display_mode(int mode);
void release_display();
bool is_display_forced();
bool is_display_enabled();
void blink_time_zero_neopixel();
String display_mode_name(int mode);
int get_display_mode();
uint32_t get_pixel_color(uint16_t n);
