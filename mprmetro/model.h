#ifndef _MODEL_H_
#define _MODEL_H_

#include "arduinoclock.h"
#include "mprmetro.h"

#define NR_LED_COLORS 11

#define ITIS_CONFIG 0x01
#define TWENTY_CONFIG 0x02
#define OVER_CONFIG 0x04
#define SECONDS_CONFIG 0x08
#define DOT_CONFIG 0x10
#define BLACKLIGHT_CONFIG 0x20
#define SETCLOCK_CONFIG 0x40

#define UPDATE_INTERVAL 250 // in milli seconds

enum brightness
{
  high,
  low,
  automatic
};

// Photocell
#define lightPin A0
#define LRD_SCALE 128

// const float step = exp( log(255) / 10);
// #define step 1.7404198104793391
// const float step = exp( log(255) / 9);
#define step 1.8509443151142613
#define MAX_BRIGHTNESS 9

class Model {
public:
  Model();
  
  void begin();
  void setup();

  bool timed_out();
  
  void reset_timeout();
  
  void toggle_seconds(); 
  void toggle_itis();
  void toggle_dots();
  void toggle_over();
  void toggle_twenty();
  void toggle_backlight();

  byte get_config();
  
  // FIXME
  void changed();

  void store_settings();
  void restore_settings();

  void toggle_brightness();
  
  int get_brightness_high();
  int get_brightness_low();
  int set_brightness_high(int bh);
  int set_brightness_low(int bl);

  float compute_brightness(int value);
  HsbColor compute_current_color();
  float compute_current_brightness();
  void set_current_brightness(brightness mode);
  
  TimeMS getDateTime();
  void setDateTime(uint8_t hour, uint8_t minute, uint8_t second);
  void get_display_parameters(uint8_t hour, uint8_t minute, byte config, uint8_t &hour_index, uint8_t &fiveminutes, LedWord &after, byte &display_config); 

  HsbColor Model::adjust_color(int update, bool sat);

public:
  ArduinoClock clock;

  // Mechanism to know if changes have occured, each change fills in the time stamp
  // If changes are reflected are persisted, this variable is cleared
  unsigned long last_change;

  // Configuration
  byte config;

  // Brightness settings
  int brightness_high;
  int brightness_low;

  brightness current_brightness;  

  HsbColor current_color;
};

#endif // _MODEL_H_
