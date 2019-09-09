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

extern ESP8266WebServer server;

String datestr(time_t now = 0);

/*** WiFi/server methods ***/
void connect_to_wifi();
void start_server();
void server_loop();

/*** Tide calculator ***/
void update_tides();
void publish_tides();
void get_tides_by_hour(float* tides);

/*** EEPROM configuration ***/
void setup_eeprom();
void init_config();
void publish_config();
byte get_dac_calibration(int dac_number, int cal_point);
void set_dac_calibration(int dac_number, int cal_point, byte cal_value);
int get_time_steps_offset();
void set_time_steps_offset(int offset);
void save_config();

/*** DACs ***/
void init_dacs();
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
