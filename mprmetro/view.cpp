#include "view.h"
#include "ledstrip.h"
#include "dutch.h"
#include "mprmetro.h"

#define SECONDS_FADE_STEPS 25
#define FADE_STEPS 75

#define FADENR (FADE_STEPS * 6 / 5)
#define SECONDS_FADENR (SECONDS_FADE_STEPS * 3 / 2)

#define FADEDIFF (FADENR - FADE_STEPS)
#define SECONDS_FADEDIFF (SECONDS_FADENR - SECONDS_FADE_STEPS)

View::View(): counter(-1), seconds_counter(-1), last_change(0) {
  previous.hour = 255;
  previous.minute = 255;
  previous.second = 255;
}

void View::begin()
{
  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'
}

void View::setup()
{
  // Some example procedures showing how to display to the pixels:
  strip.colorWipe(2);
}

void View::display_hour(const HsbColor &color, uint8_t hour, uint8_t minute, uint8_t seconds, byte config)
{
  // DBG_DEBUG_F("lastpress %d", (int)controller.lastpress() != last_change);
  if (config == previous_config && controller.lastpress() == last_change) {
    if (config & SECONDS_CONFIG) {
      if (previous.hour == hour && previous.minute == minute && previous.second == seconds)
        return;

      if (previous.second != seconds) {
        if (seconds_counter < 0)
          seconds_counter = SECONDS_FADENR;
        else {
          seconds_counter--;
          if (seconds_counter < 0) {
            previous.second = seconds;
          }
        }
      }
    }
    else {
      if (previous.hour == hour && previous.minute == minute)
        return;
    }

    if (previous.minute != minute) {
      if (counter < 0)
        counter = FADENR;
      else {
        counter--;
        if (counter < 0) {
          previous.minute = minute;
          previous.hour = hour;
        }
      }
    }
  }
  else {
    previous_config = config;

    previous.hour = hour;
    previous.minute = minute;
    previous.second = seconds;

    seconds_counter = -1;
    counter = -1;
  }

  if (!(config & FADE_CONFIG)) {
    seconds_counter = -1;
    counter = -1;
  }

  last_change = controller.lastpress();

  DBG_TRACE_F("display hour %d:%d.%d %d", hour, minute, seconds, config);
  // DBG_DEBUG_F("%d %d %d", seconds, previous.second, seconds_counter);

  strip.ClearTo(RgbColor(0, 0, 0));

  uint8_t hour_index;
  uint8_t fiveMinutes;
  LedWord after;

  uint8_t prev_hour_index;
  uint8_t prev_fiveMinutes;
  LedWord prev_after;

  model.get_display_parameters(hour, minute, config, hour_index, fiveMinutes, after, config);

  if (counter >= 0)
    model.get_display_parameters(previous.hour, previous.minute, config, prev_hour_index, prev_fiveMinutes, prev_after, config);

  bool blink =  (millis() / BLINK_INTERVAL) % 2 == 0;

  // float fraction = pow(STEP_SIZE, FADE_STEPS - (counter / RATIO) ) / pow(STEP_SIZE, FADE_STEPS);
  // float fraction = pow( (float)FADE_STEPS - (float)((float)counter / (float)RATIO) ) / (float)FADE_STEPS;

  float up_fraction = 1.0;
  float down_fraction = 0.0;
  if (counter >= 0) {
    down_fraction = counter > FADEDIFF ? (counter - FADEDIFF) / (float)(FADE_STEPS) : 0;
    up_fraction = counter < FADE_STEPS ? (FADE_STEPS - counter) / (float)(FADE_STEPS) : 0;
  }

  float seconds_up_fraction = 1.0;
  float seconds_down_fraction = 0.0;
  if (seconds_counter >= 0) {
    seconds_down_fraction = seconds_counter > SECONDS_FADEDIFF ? (seconds_counter - SECONDS_FADEDIFF) / (float)(SECONDS_FADE_STEPS) : 0;
    seconds_up_fraction = seconds_counter < SECONDS_FADE_STEPS ? (SECONDS_FADE_STEPS - seconds_counter) / (float)(SECONDS_FADE_STEPS) : 0;
  }
  
  if (config & SECONDS_CONFIG) {
    if (seconds_counter < 0 || previous.second == seconds)
      display_seconds(color, seconds, config, 1.0);
    else {
      uint8_t seconds_x, seconds_y;
      uint8_t prev_seconds_x, prev_seconds_y;

      get_seconds_location(seconds, seconds_x, seconds_y);
      get_seconds_location(previous.second, prev_seconds_x, prev_seconds_y);

      if (seconds_x == prev_seconds_x && seconds_y == prev_seconds_y) {
        display_seconds(color, seconds, config, 1.0);
      } else {
        // DBG_DEBUG_F("%d s", (int)(fraction2 * 10000));
        display_seconds(color, previous.second, config, seconds_down_fraction);
        display_seconds(color, seconds, config, seconds_up_fraction);
      }
    }
  }

  bool display_after, display_before;
  bool display;
  bool displayhalf, prev_displayhalf;
  bool displayitis = config & ITIS_CONFIG;
  bool prev_displayitis = config & ITIS_CONFIG;
  
  if (counter < 0 || prev_fiveMinutes == fiveMinutes) {
    display = display_five_minutes(color, fiveMinutes, config, blink, 1.0, display_after, display_before, displayitis, displayhalf);
    if (display) {
      display_word(color, after, 1.0, display_after, display_before);
      if (displayitis) display_itis(color, 1.0);
      if (displayhalf) strip.enable_led(MINUTE_HALF, color, 1.0);
    }
  }
  else {
    bool prev_display_after, prev_display_before;

    display_five_minutes(color, prev_fiveMinutes, config, blink, down_fraction, prev_display_after, prev_display_before, prev_displayitis, prev_displayhalf);
    display = display_five_minutes(color, fiveMinutes, config, blink, up_fraction, display_after, display_before, displayitis, displayhalf);

    if (display) {
      if (displayitis == prev_displayitis) {
        if (displayitis)
          display_itis(color, 1.0);
      }
      else {
        if (displayitis) {
          display_itis(color, up_fraction);
        }
        else
          display_itis(color, down_fraction);
      }

      if (display_after == prev_display_after && display_before == prev_display_before) {
        display_word(color, after, 1.0, display_after, display_before);
      }
      else {
        display_word(color, after, up_fraction, display_after, display_before);
        display_word(color, after, down_fraction, prev_display_after, prev_display_before);
      }

      if (displayhalf == prev_displayhalf) {
        if (displayhalf) strip.enable_led(MINUTE_HALF, color, 1.0);
      }
      else {
        if (displayhalf) 
          strip.enable_led(MINUTE_HALF, color, up_fraction);
        else
          strip.enable_led(MINUTE_HALF, color, down_fraction);
      }
    }
  }


  // Always show this if setclock is on
  if ( (config & SETCLOCK_CONFIG) ) {
    strip.enable_led(PREFIX_IT, HsbColor(0, 1, 1));
    strip.enable_led(PREFIX_IS, HsbColor(0.66, 1, 1));
  }

  if (counter < 0 || prev_hour_index == hour_index)
    display_the_hour(color, hour_index, config, blink, 1.0f);
  else {
    display_the_hour(color, prev_hour_index, config, blink, down_fraction);
    display_the_hour(color, hour_index, config, blink, up_fraction);
  }

  if (counter < 0 || previous.minute == minute)
    display_dots(color, minute, config, blink, 1.0, 0.0);
  else {
    display_dots(color, minute, config, blink, up_fraction, down_fraction);
  }

  show_blacklight(color, config);

  strip.Show();
}

bool View::display_five_minutes(const HsbColor &color, uint8_t fiveMinutes, byte config, bool blink, float fraction, bool &display_after, bool &display_before, bool &displayitis, bool &displayhalf)
{
  bool display_5min = !(config & SETCLOCK_CONFIG) || (controller.submode() != clock_5min) || blink;
  
  display_after = false;
  display_before = false;
  displayhalf = false;
  
  if (display_5min) {
    switch (fiveMinutes) {
      case 0:
        strip.enable_led(SUFFIX_OCLOCK, color, fraction);
        displayitis = true;
        break;
      case 1:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        display_after = true;
        // strip.enable_led(after, color, fraction);
        break;
      case 2:
        strip.enable_led(MINUTE_TEN, color, fraction);
        // strip.enable_led(after, color, fraction);
        display_after = true;
        break;
      case 3:
        strip.enable_led(MINUTE_QUARTER, color, fraction);
        // strip.enable_led(after, color, fraction);
        display_after = true;
        break;
      case 4:
        if (config & TWENTY_CONFIG) {
          strip.enable_led(MINUTE_TWENTY, color, fraction);
          // strip.enable_led(after, color, fraction);
          display_after = true;
        }
        else {
          strip.enable_led(MINUTE_TEN, color, fraction);
          // strip.enable_led(INFIX_BEFORE, color, fraction);
          // strip.enable_led(MINUTE_HALF, color, fraction);
          display_before = true;
          displayhalf = true;
        }
        break;
      case 5:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        // strip.enable_led(INFIX_BEFORE, color, fraction);
        // strip.enable_led(MINUTE_HALF, color, fraction);
        displayhalf = true;
        display_before = true;
        break;
      case 6:
        // strip.enable_led(MINUTE_HALF, color, fraction);
        displayhalf = true;
        displayitis = true;
        break;
      case 7:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        // strip.enable_led(after, color, fraction);
        // strip.enable_led(MINUTE_HALF, color, fraction);
        displayhalf = true;
        display_after = true;
        break;
      case 8:
        if (config & TWENTY_CONFIG) {
          strip.enable_led(MINUTE_TWENTY, color, fraction);
          // strip.enable_led(INFIX_BEFORE, color, fraction);
          display_before = true;
        }
        else {
          strip.enable_led(MINUTE_TEN, color, fraction);
          // strip.enable_led(after, color, fraction);
          // strip.enable_led(MINUTE_HALF, color, fraction);
          display_after = true;
          displayhalf = true;
        }
        break;
      case 9:
        strip.enable_led(MINUTE_QUARTER, color, fraction);
        // strip.enable_led(INFIX_BEFORE, color, fraction);
        display_before = true;
        break;
      case 10:
        strip.enable_led(MINUTE_TEN, color, fraction);
        // strip.enable_led(INFIX_BEFORE, color, fraction);
        display_before = true;
        break;
      case 11:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        // strip.enable_led(INFIX_BEFORE, color, fraction);
        display_before = true;
        break;
    }
  }

  return display_5min;
}

void View::display_word(const HsbColor &color, const LedWord &after, float fraction, bool display_after, bool display_before)
{
  if (display_after) {
    strip.enable_led(after, color, fraction);
  }
  if (display_before) {
    strip.enable_led(INFIX_BEFORE, color, fraction);
  }
}

void View::display_dots(const HsbColor &color, uint8_t minute, byte config, bool blink, float up_fraction, float down_fraction)
{
  if (config & SETCLOCK_CONFIG)
    config |= DOT_CONFIG;
  if (!(config & SETCLOCK_CONFIG) || (controller.submode() != clock_min) || blink)
    enableMinuteDots(minute, color, config, up_fraction, down_fraction);
}

void View::get_seconds_location(uint8_t seconds, uint8_t &x, uint8_t &y)
{
  if (seconds < 6) {
    int where = min(4, seconds);
    x = 5 + where;
    y = 0;
  } else if (seconds < 26) {
    int where = (seconds - 6) / 2;
    x = 10;
    y = where;
  } else if (seconds < 36) {
    int where = min(9, seconds - 25);
    x = 10 - where;
    y = 9;
  } else if (seconds < 56) {
    int where = (seconds - 36) / 2;
    x = 0;
    y = 9 - where;
  } else {
    int where = (seconds - 55);
    x = where;
    y = 0;
  }
}

void View::display_seconds(const HsbColor &color, uint8_t seconds, byte config, float fraction)
{
  HsbColor second_color = color;
  second_color.B = min(1, second_color.B * 2);
  uint8_t x, y;
  get_seconds_location(seconds, x, y);
  strip.enable_led(LedWord(x, y, 1), second_color, fraction);
}

void View::display_the_hour(const HsbColor &color, uint8_t hour_index, byte config, bool blink, float fraction)
{
  if ( !(config & SETCLOCK_CONFIG) || (controller.submode() != clock_hour) || blink) {
    switch (hour_index) {
      case 0: strip.enable_led(LED_ONE, color, fraction); break;
      case 1: strip.enable_led(LED_TWO, color, fraction); break;
      case 2: strip.enable_led(LED_THREE, color, fraction); break;
      case 3: strip.enable_led(LED_FOUR, color, fraction); break;
      case 4: strip.enable_led(LED_FIVE, color, fraction); break;
      case 5: strip.enable_led(LED_SIX, color, fraction); break;
      case 6: strip.enable_led(LED_SEVEN, color, fraction); break;
      case 7: strip.enable_led(LED_EIGHT, color, fraction); break;
      case 8: strip.enable_led(LED_NINE, color, fraction); break;
      case 9: strip.enable_led(LED_TEN, color, fraction); break;
      case 10: strip.enable_led(LED_ELEVEN, color, fraction); break;
      case 11: strip.enable_led(LED_TWELVE, color, fraction); break;
    }
  }
}

void View::display_itis(const HsbColor &color, float fraction)
{
  strip.enable_led(PREFIX_IT, color, fraction);
  strip.enable_led(PREFIX_IS, color, fraction);
}

void View::show_blacklight(const HsbColor &color, byte config)
{
  if (config & BLACKLIGHT_CONFIG) {
    strip.enable_led(LedWord(11 + (LED_ROW - 11) / 2, 0, (LED_ROW - 11) / 2), color);
    for (int r = 1; r < 8; ++r) {
      strip.enable_led(LedWord(11 + r * LED_ROW, 0, LED_ROW - 11), color);
    }
    strip.enable_led(LedWord(11 + 8 * LED_ROW, 0, (LED_ROW - 11) / 2), color);
  }
}

void View::enableMinuteDots(uint8_t minute, const HsbColor &color, byte config, float up_fraction, float down_fraction)
{
  if (config & DOT_CONFIG) {
    uint8_t minuteDots = minute % 5;
    uint8_t previousMinuteDots = previous.minute % 5;

    //Serial.print("enableMinuteDots ");
    //Serial.println(minuteDots);
    if (minuteDots == 0) {
      if (up_fraction != 1.0) {
        strip.updateColor(NR_LEDS - 1, color, down_fraction);
        strip.updateColor(NR_LEDS - 2, color, down_fraction);
        strip.updateColor(NR_LEDS - 3, color, down_fraction);
        strip.updateColor(NR_LEDS - 4, color, down_fraction);
      }
    }
    if (minuteDots > 0) {
      if (up_fraction == 1.0 || minuteDots > 1)
        strip.SetPixelColor(NR_LEDS - 3, color);
      else
      {
        strip.updateColor(NR_LEDS - 3, color, up_fraction);
      }
    }
    if (minuteDots > 1) {
      if (up_fraction == 1.0 || minuteDots > 2)
        strip.SetPixelColor(NR_LEDS - 2, color);
      else
      {
        strip.updateColor(NR_LEDS - 2, color, up_fraction);
      }
    }
    if (minuteDots > 2) {
      if (up_fraction == 1.0 || minuteDots > 3)
        strip.SetPixelColor(NR_LEDS - 1, color);
      else
      {
        strip.updateColor(NR_LEDS - 1, color, up_fraction);
      }
    }
    if (minuteDots > 3) {
      if (up_fraction == 1.0)
        strip.SetPixelColor(NR_LEDS - 4, color);
      else
      {
        strip.updateColor(NR_LEDS - 4, color, up_fraction);
      }
    }
  }
}

void View::show_bar(HsbColor color, int low, int high, bool set_low)
{
  last_change = millis();
  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;

  view.strip.ClearTo(RgbColor(0, 0, 0));
  int brightness_low = model.get_brightness_low();
  int brightness_high = model.get_brightness_high();
  if (!set_low or blink) {
    color.B = model.compute_brightness(brightness_low);
    view.strip.enable_led(LedWord(0, brightness_low, 10 - brightness_low), color);
  }
  for (int i = brightness_low + 1; i < brightness_high; ++i) {
    color.B = model.compute_brightness(i);
    view.strip.enable_led(LedWord(0, i, 10 - i), color);
  }
  if (set_low or blink) {
    color.B = model.compute_brightness(brightness_high);
    view.strip.enable_led(LedWord(0, brightness_high, 10 - brightness_high), color);
  }
  view.strip.Show();
}

void View::show_hue(const HsbColor &color, submenu_mode sub_mode)
{
  last_change = millis();

  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;
  view.strip.ClearTo(RgbColor(0, 0, 0));
  for (int i = 0; i < NR_LED_COLORS; ++i) {
    view.strip.enable_led(LedWord(i, 0, 1), HsbColor((float)i / NR_LED_COLORS, 1, color.B));
    view.strip.enable_led(LedWord(i, 1, 1), HsbColor((float)i / NR_LED_COLORS, 1, color.B));

    view.strip.enable_led(LedWord(i, 5, 1), HsbColor(color.H, (float)i / (NR_LED_COLORS - 1), color.B));
    view.strip.enable_led(LedWord(i, 6, 1), HsbColor(color.H, (float)i / (NR_LED_COLORS - 1), color.B));
  }
  if (sub_mode != set_hue  || blink) {
    int hue = round(color.H * NR_LED_COLORS);
    view.strip.enable_led(LedWord(hue, 2, 1), HsbColor((float)hue / NR_LED_COLORS, 1, color.B));
    view.strip.enable_led(LedWord(hue, 3, 1), HsbColor((float)hue / NR_LED_COLORS, 1, color.B));
  }
  if (sub_mode != set_sat || blink) {
    int sat = round(color.S * (NR_LED_COLORS - 1));
    view.strip.enable_led(LedWord(sat, 7, 1), HsbColor(color.H, (float)sat / (NR_LED_COLORS - 1), color.B));
    view.strip.enable_led(LedWord(sat, 8, 1), HsbColor(color.H, (float)sat / (NR_LED_COLORS - 1), color.B));
  }
  HsbColor currentcolor = color;
  int brightness_low = model.get_brightness_low();
  int brightness_high = model.get_brightness_high();
  for (int i = brightness_low; i <= brightness_high; ++i) {
    currentcolor.B = pow(step, i) / 255.0f;
    view.strip.enable_led(LedWord(i, 9, 1), currentcolor);
  }
  view.strip.Show();
}

void View::show_happiness(const HsbColor &color)
{
  last_change = millis();

  bool gert =  ((millis() / 2000) % 4) == 0;
  view.strip.ClearTo(RgbColor(0, 0, 0));

  if (gert) {
    view.strip.enable_led(LedWord(3, 0, 1), color);
    view.strip.enable_led(LedWord(6, 0, 1), color);
    view.strip.enable_led(LedWord(4, 1, 2), color);
    view.strip.enable_led(LedWord(7, 2, 2), color);
    view.strip.enable_led(LedWord(10, 4, 1), color);
    view.strip.enable_led(LedWord(5, 7, 1), color);
    view.strip.enable_led(LedWord(6, 9, 2), color);
  } else {
    view.strip.enable_led(LedWord(4, 3, 3), color);
    view.strip.enable_led(LedWord(4, 4, 3), color);
    view.strip.enable_led(LedWord(4, 5, 3), color);
  }

  view.strip.Show();
}

void View::show_debug(int seconds) {
  last_change = millis();

  HsbColor color(0, 0, 1);
  int show = seconds % 14;
  view.strip.ClearTo(RgbColor(0, 0, 0));
  for (uint16_t i = 0; i < view.strip.PixelCount(); i++) {
    if ( (i % 14) == show)
      view.strip.SetPixelColor(i, color);
  }
  view.strip.Show();
}
