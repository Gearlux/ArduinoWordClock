#include <DS3231.h>
#include <Wire.h>

#include "mprmetro.h"
#include "dutch.h"

void writeRegister8(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(DS3231_ADDRESS);
#if ARDUINO >= 100
  Wire.write(reg);
  Wire.write(value);
#else
  Wire.send(reg);
  Wire.send(value);
#endif
  Wire.endTransmission();
}

uint8_t readRegister8(uint8_t reg)
{
  uint8_t value;
  Wire.beginTransmission(DS3231_ADDRESS);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 1);
  int cnt = 0;
  while (!Wire.available() && cnt < 100) {delay(2); cnt++;};
  if (cnt < 100) {
#if ARDUINO >= 100
  value = Wire.read();
#else
  value = Wire.receive();
#endif
  }
  else {
    DBG_WARNING("Reading of register failed");
  }
  Wire.endTransmission();
  return value;
}

Model::Model() :
  last_change(0),
  config(ITIS_CONFIG | TWENTY_CONFIG),
  current_color(0, 1, 0)
{
}

void Model::begin()
{
  DBG_INFO("Initialize RTC module");
  
  // Initialize DS3231
  clock.begin();
}

void Model::setup()
{
  // Manual (YYYY, MM, DD, HH, II, SS
  // clock.setDateTime(2016, 12, 9, 11, 46, 00);

  // Send sketch compiling time to Arduino
  clock.setDateTime(__DATE__, __TIME__);
  
  restore_settings();
}

bool Model::timed_out() 
{
  return last_change != 0 && millis() > (last_change + TIMEOUT);
}

void Model::reset_timeout()
{
  last_change = 0;
}

void Model::toggle_seconds() 
{
  config ^= SECONDS_CONFIG;
  model.changed();
}

void Model::toggle_itis() 
{
  config ^= ITIS_CONFIG;
  model.changed();
}

void Model::toggle_dots()
{
  config ^= DOT_CONFIG;
  model.changed();  
}

void Model::toggle_over()
{
  config ^= OVER_CONFIG;
  model.changed();
}

void Model::toggle_twenty()
{
  config ^= TWENTY_CONFIG;
  model.changed();
}

void Model::toggle_backlight()
{
  config ^= BLACKLIGHT_CONFIG;
  model.changed();
}

byte Model::get_config()
{
  return config;
}

void Model::changed()
{
  DBG_DEBUG("Model changed config %s", Debug.byte_to_binary(config) );
  last_change = millis();  
}

void Model::store_settings()
{
  DBG_INFO_F("store_settings");
  int hue = round(current_color.H * NR_LED_COLORS);
  writeRegister8(0x08, (byte)hue);
  writeRegister8(0x09, (byte)config);
  writeRegister8(0x0A, (byte)brightness_high);
  writeRegister8(0x0B, (byte)brightness_low);

  int bm = max(0, current_brightness);
  writeRegister8(0x0C, (byte)bm);

  int sat = round(current_color.S * (NR_LED_COLORS-1));
  writeRegister8(0x0D, (byte)sat);

  DBG_INFO("hue: %d config: %d brightness_high: %d low: %d current: %d sat: %d", hue, config, brightness_high, brightness_low, bm, sat);
}

void Model::restore_settings()
{
  DBG_INFO_F("restore_settings");
  int hue = min(NR_LED_COLORS-1, readRegister8(0x08));
  config = readRegister8(0x09);
  brightness_high = max(1,min(MAX_BRIGHTNESS, readRegister8(0x0A)));
  brightness_low = max(0,min(brightness_high-1, readRegister8(0x0B)));
  byte bmb = readRegister8(0x0C);
  if (bmb == 0)
    current_brightness = automatic;
  else
    current_brightness = static_cast<brightness>(bmb);
  int sat = min(NR_LED_COLORS-1, readRegister8(0x0D));

  DBG_INFO("hue: %d config: %d brightness_high: %d low: %d current: %d sat: %d", hue, config, brightness_high, brightness_low, bmb, sat);
  
  current_color.H = (float)hue / NR_LED_COLORS;
  current_color.S = (float)sat / (NR_LED_COLORS-1);
}

TimeMS Model::getDateTime()
{
  return model.clock.get_date_time();
}

void Model::setDateTime(uint8_t hour, uint8_t minute, uint8_t second) 
{
  clock.set_date_time(hour, minute, second); 
  changed();
}

void Model::toggle_brightness()
{
  DBG_DEBUG_F("toggle_brightness");
  
  switch (current_brightness) {
    case automatic:
      current_brightness = high;
      break;
    case high:
      current_brightness = low;
      break;
    case low:
      current_brightness = automatic;
      break;
  }

  DBG_DEBUG("Current brightness: %d", (int)current_brightness);
  model.changed();
}


int Model::get_brightness_high()
{
  return brightness_high;
}
int Model::get_brightness_low()
{
  return brightness_low;
}
int Model::set_brightness_high(int bh)
{
  brightness_high = max(brightness_low+1, min(MAX_BRIGHTNESS, bh));  
  return brightness_high;
}
int Model::set_brightness_low(int bl)
{
  brightness_low = max(0, min(brightness_high - 1, bl));
  return brightness_low;
}

float Model::compute_brightness(int value)
{
  return pow(step, value) / 255.0f;
}

void Model::set_current_brightness(brightness mode)
{
  current_brightness = mode;
  changed();
}

HsbColor Model::compute_current_color()
{
  HsbColor currentcolor = current_color;
  currentcolor.B = compute_current_brightness();
  return currentcolor;
}

float Model::compute_current_brightness()
{
  float result;
    // First determine the brightness and color of the display
  switch (current_brightness)
  {
    case automatic:
      {
        int reading = analogRead(lightPin);
        DBG_INFO_F("Reading %d", reading);
        float scale = brightness_low + max(0.0f, min(1.0f, reading / (float)LRD_SCALE)) * (brightness_high - brightness_low);
        result = pow(step, scale) / 255.0f;
      }
      break;
    case high:
      result = pow(step, brightness_high) / 255.0f; // auto_levels[auto_high] / 255.0f;
      break;
    case low:
      result = pow(step, brightness_low) / 255.0f; // auto_levels[auto_low] / 255.0f;
      break;
  }

  return result;
}

HsbColor Model::adjust_color(int update, bool sat) 
{
  if (sat) {
    int sat = round(current_color.S * (NR_LED_COLORS-1));
    sat = sat + update;
    sat = min(NR_LED_COLORS-1,max(0, sat));
    current_color.S = sat / (NR_LED_COLORS-1.0f);
  }
  else {
    int hue = round(current_color.H * NR_LED_COLORS);
    hue = hue + update;
    if (abs(update) == 1) {
      hue = min(NR_LED_COLORS-1,max(0,hue));
    }
    else {
      if (hue > NR_LED_COLORS-1) {
          hue = hue - NR_LED_COLORS;
      }
      if (hue < 0) {
          hue = NR_LED_COLORS + hue;
      }
    }
    current_color.H = (float)hue / NR_LED_COLORS;
  }
  return current_color;
}

void Model::get_display_parameters(uint8_t hour, uint8_t minute, byte config, uint8_t &hour_index, uint8_t &fiveMinutes, LedWord &after, byte &display_config)
{
  display_config = config;
  
  fiveMinutes = minute / 5;
  int threshold = 4;

  switch(fiveMinutes) {
  case 0:
    display_config |= ITIS_CONFIG; 
    break;
  case 4:
    if (config & TWENTY_CONFIG) 
      threshold = 5;
    break;
  default:
    break;
  }

  if (fiveMinutes < threshold) {
    hour_index = (hour + 11) % 12;
  } else {
    hour_index = (hour) % 12;
  }

  after = INFIX_AFTER2;
  if (config & OVER_CONFIG)
    after = INFIX_AFTER;
}
