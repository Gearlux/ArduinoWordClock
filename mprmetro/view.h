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
  void enableMinuteDots(uint8_t minute, const HsbColor &color, byte config, float up_fraction, float down_fraction);

  void show_bar(HsbColor color, int low, int high, bool set_low);

  void show_hue(const HsbColor &color, submenu_mode sub_mode);
  void show_happiness(const HsbColor &color);
  void show_debug(int seconds);

private:
  bool display_five_minutes(const HsbColor &color, uint8_t fiveMinutes, byte config, bool blink, float fraction, bool &display_after, bool &display_before, bool &display_itis, bool &display_half);
  void display_word(const HsbColor &color, const LedWord &after, float fraction, bool display_after, bool display_before);
  void display_half(const HsbColor &color, float fraction);
  void display_the_hour(const HsbColor &color, uint8_t hour_index, byte config, bool blink, float fraction);
  void display_itis(const HsbColor &color, float fraction);
  void display_seconds(const HsbColor &color, uint8_t seconds, byte config, float fraction);
  void display_dots(const HsbColor &color, uint8_t minute, byte config, bool blink, float up_fraction, float down_fraction);

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
