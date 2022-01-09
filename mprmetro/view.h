#ifndef _VIEW_H_
#define _VIEW_H_

#include "ledstrip.h"
#include "controller.h"
#include "arduinoclock.h"

class View {
public:
  View();
  
  void begin();
  void setup();

  void display_hour(const HsbColor &color, uint8_t hour, uint8_t minute, uint8_t seconds, byte config);
  void enableMinuteDots(uint8_t minute, const HsbColor &color, byte config, float fraction);

  void show_bar(HsbColor color, int low, int high, bool set_low);

  void show_hue(const HsbColor &color, submenu_mode sub_mode);
  void show_happiness(const HsbColor &color);
  void show_debug(int seconds);

private:
  void display_five_minutes(const HsbColor &color, uint8_t fiveMinutes, const LedWord &after, byte config, bool blink, float fraction);
  void display_the_hour(const HsbColor &color, uint8_t hour_index, byte config, bool blink, float fraction);
  void display_itis(const HsbColor &color, byte config, bool blink);
  void display_seconds(const HsbColor &color, uint8_t seconds, byte config, float fraction);
  void display_dots(const HsbColor &color, uint8_t minute, byte config, bool blink, float fraction);

  void show_blacklight(const HsbColor &color, byte config);

  void get_seconds_location(uint8_t seconds, uint8_t &x, uint8_t &y);

public:
  // FIXME: should go private
  LedStrip strip;

  TimeMS previous;
  int counter;
  int seconds_counter;
  byte previous_config;
  unsigned long last_change;
};

#endif // _VIEW_H_
