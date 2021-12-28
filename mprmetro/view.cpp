#include "view.h"
#include "ledstrip.h"
#include "dutch.h"
#include "mprmetro.h"

#define FADE_STEPS 25
#define STEP_SIZE 1.1
#define RATIO 5

View::View(): counter(-1), seconds_counter(-1) {
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
  if (config == previous_config) {
    if (config & SECONDS_CONFIG) {
      if (previous.hour == hour && previous.minute == minute && previous.second == seconds)
        return;    

      if (previous.second != seconds) {
        if (seconds_counter < 0)
          seconds_counter = FADE_STEPS;
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
        counter = FADE_STEPS * RATIO;
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
  float fraction = ( (float)FADE_STEPS - (float)((float)counter / (float)RATIO) ) / (float)FADE_STEPS;
  float fraction2 = pow(STEP_SIZE, FADE_STEPS - seconds_counter) / pow(STEP_SIZE, FADE_STEPS);
  
  if (counter < 0 || prev_fiveMinutes == fiveMinutes)
    display_five_minutes(color, fiveMinutes, after, config, blink, 1.0);
  else {
      display_five_minutes(color, prev_fiveMinutes, prev_after, config, blink, 1.0-fraction);
      display_five_minutes(color, fiveMinutes, after, config, blink, fraction);
  }
  
  display_itis(color, config, blink);

  if (counter < 0 || prev_hour_index == hour_index)
    display_the_hour(color, hour_index, config, blink, 1.0f);
  else {
    display_the_hour(color, prev_hour_index, config, blink, 1.0-fraction);
    display_the_hour(color, hour_index, config, blink, fraction);
  }

  if (counter < 0 || previous.minute == minute) 
    display_dots(color, minute, config, blink, 1.0);
  else {
    DBG_DEBUG_F("%d", (int)(fraction * 10000));
    display_dots(color, previous.minute, config, blink, 1.0-fraction);
    display_dots(color, minute, config, blink, fraction);
  }

  show_blacklight(color, config);

  if (config & SECONDS_CONFIG) {
    if (seconds_counter < 0 || previous.second == seconds)
      display_seconds(color, seconds, config, 1.0);
    else {
      uint8_t seconds_x, seconds_y;
      uint8_t prev_seconds_x, prev_seconds_y;
      
      get_seconds_location(seconds, seconds_x, seconds_y);  
      get_seconds_location(previous.second, seconds_x, seconds_y);  

      if (seconds_x == prev_seconds_x && seconds_y == prev_seconds_y) {
        display_seconds(color, seconds, config, 1.0);
      } else {
        // DBG_DEBUG_F("%d s", (int)(fraction2 * 10000));
        display_seconds(color, previous.second, config, 1.0-fraction2);
        display_seconds(color, seconds, config, fraction2);
      }
    }
  }

  strip.Show();
}

void View::display_five_minutes(const HsbColor &color, uint8_t fiveMinutes, const LedWord &after, byte config, bool blink, float fraction)
{
  bool display_5min = !(config & SETCLOCK_CONFIG) || (controller.submode() != clock_5min) || blink;
  if (display_5min) {
    switch (fiveMinutes) {
      case 0:
        strip.enable_led(SUFFIX_OCLOCK, color, fraction);
        break;
      case 1:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        strip.enable_led(after, color, fraction);
        break;
      case 2:
        strip.enable_led(MINUTE_TEN, color, fraction);
        strip.enable_led(after, color, fraction);
        break;
      case 3:
        strip.enable_led(MINUTE_QUARTER, color, fraction);
        strip.enable_led(after, color, fraction);
        break;
      case 4:
        if (config & TWENTY_CONFIG) {
          strip.enable_led(MINUTE_TWENTY, color, fraction);
          strip.enable_led(after, color, fraction);
        }
        else {
          strip.enable_led(MINUTE_TEN, color, fraction);
          strip.enable_led(INFIX_BEFORE, color, fraction);
          strip.enable_led(MINUTE_HALF, color, fraction);
        }
        break;
      case 5:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        strip.enable_led(INFIX_BEFORE, color, fraction);
        strip.enable_led(MINUTE_HALF, color, fraction);
        break;
      case 6:
        strip.enable_led(MINUTE_HALF, color, fraction);
        break;
      case 7:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        strip.enable_led(after, color, fraction);
        strip.enable_led(MINUTE_HALF, color, fraction);
        break;
      case 8:
        if (config & TWENTY_CONFIG) {
          strip.enable_led(MINUTE_TWENTY, color, fraction);
          strip.enable_led(INFIX_BEFORE, color, fraction);
        } 
        else {    
          strip.enable_led(MINUTE_TEN, color, fraction);
          strip.enable_led(after, color, fraction);
          strip.enable_led(MINUTE_HALF, color, fraction);
        }
        break;
      case 9:
        strip.enable_led(MINUTE_QUARTER, color, fraction);
        strip.enable_led(INFIX_BEFORE, color, fraction);
        break;
      case 10:
        strip.enable_led(MINUTE_TEN, color, fraction);
        strip.enable_led(INFIX_BEFORE, color, fraction);
        break;
      case 11:
        strip.enable_led(MINUTE_FIVE, color, fraction);
        strip.enable_led(INFIX_BEFORE, color, fraction);
        break;
    }
  }
}

void View::display_dots(const HsbColor &color, uint8_t minute, byte config, bool blink, float fraction)
{
  if (config & SETCLOCK_CONFIG)
    config |= DOT_CONFIG;
  if (!(config & SETCLOCK_CONFIG) || (controller.submode() != clock_min) || blink) 
    enableMinuteDots(minute, color, config, fraction);
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
    x = 10 -where;
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
  strip.enable_led(LedWord(x,y,1), second_color, fraction);
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

void View::display_itis(const HsbColor &color, byte config, bool blink)
{
  if (config & ITIS_CONFIG) {
    strip.enable_led(PREFIX_IT, color);
    strip.enable_led(PREFIX_IS, color);
  }
  if ( (config & SETCLOCK_CONFIG) && blink ){
    strip.enable_led(PREFIX_IT, HsbColor(0.125, 0.5, 1));
    strip.enable_led(PREFIX_IS, HsbColor(0.625, 0.5, 1));
  }
}

void View::show_blacklight(const HsbColor &color, byte config)
{
  if (config & BLACKLIGHT_CONFIG) {
    strip.enable_led(LedWord(11 + (LED_ROW-11)/2, 0, (LED_ROW-11) / 2), color);
    for (int r=1;r<8;++r) {
      strip.enable_led(LedWord(11 + r * LED_ROW, 0, LED_ROW-11), color);
    } 
    strip.enable_led(LedWord(11 + 8 * LED_ROW, 0, (LED_ROW-11) / 2), color);
  }
}

void View::enableMinuteDots(uint8_t minute, const HsbColor &color, byte config, float fraction)
{
   uint8_t minuteDots = minute % 5;
   if (config & DOT_CONFIG) {
      //Serial.print("enableMinuteDots ");
      //Serial.println(minuteDots);
      if (minuteDots > 0) {
        if (fraction == 1.0) 
          strip.SetPixelColor(NR_LEDS-3, color);
        else 
        {
          strip.updateColor(NR_LEDS-3, color, fraction);
        }
      }
      if (minuteDots > 1) 
        if (fraction == 1.0) 
          strip.SetPixelColor(NR_LEDS-2, color);
        else 
        {
          strip.updateColor(NR_LEDS-2, color, fraction);
        }
      if (minuteDots > 2) 
        if (fraction == 1.0) 
          strip.SetPixelColor(NR_LEDS-1, color);
        else 
        {
          strip.updateColor(NR_LEDS-1, color, fraction);
        }
      if (minuteDots > 3) 
        if (fraction == 1.0) 
          strip.SetPixelColor(NR_LEDS-4, color);
        else 
        {
          strip.updateColor(NR_LEDS-4, color, fraction);
        }
   }  
}

void View::show_bar(HsbColor color, int low, int high, bool set_low)
{
  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;

  view.strip.ClearTo(RgbColor(0, 0, 0));
  int brightness_low = model.get_brightness_low();
  int brightness_high = model.get_brightness_high();
  if (!set_low or blink) {
    color.B = model.compute_brightness(brightness_low);
    view.strip.enable_led(LedWord(0, brightness_low, 10-brightness_low), color);
  }  
  for(int i=brightness_low+1; i<brightness_high; ++i) {
    color.B = model.compute_brightness(i);
    view.strip.enable_led(LedWord(0, i, 10-i), color);
  }
  if (set_low or blink) {
    color.B = model.compute_brightness(brightness_high);
    view.strip.enable_led(LedWord(0, brightness_high, 10-brightness_high), color);
  }  
  view.strip.Show();
}

void View::show_hue(const HsbColor &color, submenu_mode sub_mode)
{
  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;
  view.strip.ClearTo(RgbColor(0, 0, 0));
  for(int i=0; i<NR_LED_COLORS; ++i) {
    view.strip.enable_led(LedWord(i,0,1), HsbColor((float)i / NR_LED_COLORS, 1, color.B));  
    view.strip.enable_led(LedWord(i,1,1), HsbColor((float)i / NR_LED_COLORS, 1, color.B));  

    view.strip.enable_led(LedWord(i,5,1), HsbColor(color.H, (float)i / (NR_LED_COLORS-1), color.B));  
    view.strip.enable_led(LedWord(i,6,1), HsbColor(color.H, (float)i / (NR_LED_COLORS-1), color.B));  
  }
  if (sub_mode != set_hue  || blink) {
    int hue = round(color.H * NR_LED_COLORS);
    view.strip.enable_led(LedWord(hue,2,1), HsbColor((float)hue / NR_LED_COLORS, 1, color.B));  
    view.strip.enable_led(LedWord(hue,3,1), HsbColor((float)hue / NR_LED_COLORS, 1, color.B));  
  }
  if (sub_mode != set_sat || blink) {
    int sat = round(color.S * (NR_LED_COLORS-1));
    view.strip.enable_led(LedWord(sat,7,1), HsbColor(color.H, (float)sat / (NR_LED_COLORS-1), color.B));  
    view.strip.enable_led(LedWord(sat,8,1), HsbColor(color.H, (float)sat / (NR_LED_COLORS-1), color.B));  
  }
  HsbColor currentcolor = color;
  int brightness_low = model.get_brightness_low();
  int brightness_high = model.get_brightness_high();
  for(int i=brightness_low; i<=brightness_high; ++i) {
    currentcolor.B = pow(step, i) / 255.0f;
    view.strip.enable_led(LedWord(i, 9, 1), currentcolor);
  }
  view.strip.Show();
}

void View::show_happiness(const HsbColor &color)
{
  bool gert =  ((millis() / 2000) % 4) == 0;
  view.strip.ClearTo(RgbColor(0, 0, 0));

  if (gert) {
      view.strip.enable_led(LedWord(3,0,1), color);
      view.strip.enable_led(LedWord(6,0,1), color);
      view.strip.enable_led(LedWord(4,1,2), color);
      view.strip.enable_led(LedWord(7,2,2), color);
      view.strip.enable_led(LedWord(10,4,1), color);
      view.strip.enable_led(LedWord(5,7,1), color);
      view.strip.enable_led(LedWord(6,9,2), color);
  } else {
      view.strip.enable_led(LedWord(4,3,3), color);
      view.strip.enable_led(LedWord(4,4,3), color);
      view.strip.enable_led(LedWord(4,5,3), color);    
  }
  
  view.strip.Show();
}

void View::show_debug(int seconds) {
  HsbColor color(0, 0, 1);
  int show = seconds % 14;
  view.strip.ClearTo(RgbColor(0, 0, 0));
  for (uint16_t i = 0; i < view.strip.PixelCount(); i++) {
    if ( (i % 14) == show) 
      view.strip.SetPixelColor(i, color);
  }
  view.strip.Show();
}
