// Clock
#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

// Buttons
#include <AceButton.h>

using namespace ace_button;

#define BUTTON_ONE_PIN 2
#define BUTTON_TWO_PIN 3
#define BUTTON_THREE_PIN 4

const AceButton buttonOne(new ButtonConfig());
const AceButton buttonTwo(new ButtonConfig());
const AceButton buttonThree(new ButtonConfig());

void setButtonConfig(ButtonConfig *buttonConfig, ButtonConfig::EventHandler eventHandler);

void handleButtonOneEvent(AceButton *, uint8_t, uint8_t);
void handleButtonTwoEvent(AceButton *, uint8_t, uint8_t);
void handleButtonThreeEvent(AceButton *, uint8_t, uint8_t);

// Buttons
void setupButtons();

// Photocell
#define lightPin A1

// Color
#include <NeoPixelBus.h>
int color_index = 0;
HsbColor current_color(0, 1, 0);

// Function that selects and updates the current_color and the color_index
HsbColor update_color(int color_idx);

// User Interface
enum menu_mode
{
  main_mode = 0,
  clock_mode = 1,
  brightness_mode = 2,
  config_mode = 3
};
menu_mode current_mode = menu_mode::main_mode;

#define ITIS_CONFIG 0x01
#define TWENTY_CONFIG 0x02
#define OVER_CONFIG 0x04
#define SECONDS_CONFIG 0x08
#define DOT_CONFIG 0x10
#define BLACKLIGHT_CONFIG 0x20
#define SETCLOCK_CONFIG 0x40
#define SET_CONFIG 0x80
byte config = ITIS_CONFIG | TWENTY_CONFIG;

enum brightness
{
  very_high = 255,
  high = 153,
  medium = 91,
  low = 55,
  very_low = 33,
  automatic = -1,
  set_high = -2,
  set_low = -3
};

const float step = pow(0.6, 1.0 / 3.0);
int auto_high = 15;
int auto_low = 3;
brightness current_brightness = brightness::automatic;

#define TIMEOUT 10000 // in milliseconds
#define TIME_UPDATE_INTERVAL 1 // in seconds
#define CLOCK_UPDATE_INTERVAL 1 // in seconds

unsigned long last_press = 0;
unsigned long last_change = 0;
unsigned long lastClockUpdate = 0;
unsigned long lastShowTime = 0;

//-----------------------------------------------------
// Function Declarations
//-----------------------------------------------------
void update_display();

void show_seconds();
void show_bar();

void toggle_brightness();
void increase_brightness(brightness high_or_low);
void decrease_brightness(brightness high_or_low);
void toggle_rgb();

void increase_hour();
void decrease_hour();
void increase_5min();
void decrease_5min();
void increase_1min();
void decrease_1min();

void restore_settings();
void store_settings();

//
// Word Clock
//
// #include <Adafruit_NeoPixel.h>

class LedWord {
  public:
    LedWord(int firstPixelX, int firstPixelY, int length) : firstPixelX(firstPixelX), firstPixelY(firstPixelY), length(length) {}
  public:
    int firstPixelX;
    int firstPixelY;
    int length;
};

#define PREFIX_IT LedWord(0, 0, 3)
#define PREFIX_IS LedWord(4, 0, 2)

#define MINUTE_FIVE LedWord(7, 0, 4)
#define MINUTE_TEN LedWord(0, 1, 4)

#define INFIX_BEFORE LedWord(0, 3, 4)
#define INFIX_AFTER LedWord(7, 3, 4)

#define INFIX_AFTER2 LedWord(8, 2, 2)
#define MINUTE_TWENTY LedWord(0, 2, 7)
  
#define MINUTE_QUARTER LedWord(6, 1, 5)
#define MINUTE_HALF LedWord(0, 4, 4)

#define LED_ONE    LedWord(7, 4, 3)
#define LED_TWO    LedWord(0, 5, 4)
#define LED_THREE  LedWord(7, 5, 4)
#define LED_FOUR   LedWord(0, 6, 4)
#define LED_FIVE   LedWord(4, 6, 4)
#define LED_SIX    LedWord(8, 6, 3)
#define LED_SEVEN  LedWord(0, 7, 5)
#define LED_EIGHT  LedWord(0, 8, 4)
#define LED_NINE   LedWord(6, 7, 5)
#define LED_TEN    LedWord(4, 8, 4)
#define LED_ELEVEN LedWord(8, 8, 3)
#define LED_TWELVE LedWord(0, 9, 6)

#define SUFFIX_OCLOCK LedWord(8, 9, 3)

#define NEOPIN 8
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(150, NEOPIN);

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

void setup()
{
  Serial.begin(9600);

  Serial.println("Initialize RTC module");
  
  // Initialize DS3231
  clock.begin();

  Serial.println("Setup Buttons");
  setupButtons();

  // Manual (YYYY, MM, DD, HH, II, SS
  // clock.setDateTime(2016, 12, 9, 11, 46, 00);

  // Send sketch compiling time to Arduino
  // clock.setDateTime(__DATE__, __TIME__);

  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'

  // Some example procedures showing how to display to the pixels:
  colorWipe(RgbColor(0, 0, 255), 5); // Red

  update_display();

  restore_settings();
  Serial.println("Initialization done");
}

// Fill the dots one after the other with a color
void colorWipe(RgbColor c, uint8_t wait) {
  Serial.println("colorWipe");
  for (uint16_t i = 0; i < strip.PixelCount(); i++) {
    strip.SetPixelColor(i, c);
    strip.Show();
    delay(wait);
  }
}

#define nr_led_colors 16
HsbColor update_color(int color_indx) {
  if (color_indx > nr_led_colors)
    color_indx = 0;
  if (color_indx < 0)
    color_indx = nr_led_colors;
  float h = 0.125 * (int)(color_indx / 2);
  float s = color_indx % 2 ? 0.85 : 1;
  color_index = color_indx;
  if (color_indx == nr_led_colors)
    current_color = HsbColor(0, 0, 1);
  else
    current_color = HsbColor(h, s, 1);
  return current_color;
}

void loop()
{
  // We need continuous updates for interactive modes
  switch (current_mode) {
    case clock_mode:
      lastClockUpdate = 0;
      lastShowTime = 0;
      break;
  }

  unsigned long current_time = millis();
  if ((current_time - lastClockUpdate) > (CLOCK_UPDATE_INTERVAL * 1000) || last_press > lastClockUpdate) {
    // For leading zero look to DS3231_dateformat example
    dt = clock.getDateTime();
    lastClockUpdate = millis();
  }

  if ( last_change != 0 && current_time > (last_change + TIMEOUT) ) {
    store_settings();
    last_change = 0;
  }

  switch (current_mode)
  {
    case brightness_mode:
    case clock_mode:
    case config_mode:
      if (last_press != 0 && current_time > (last_press + TIMEOUT) )
      {
        if (current_mode == brightness_mode) {
          current_brightness = automatic;
        }

        Serial.print("Timeout ");
        Serial.print(current_time);
        Serial.print(" ");
        Serial.println(last_press);
        current_mode = main_mode;
        last_press = 0;

      }
      break;
  }

  if ((millis() - lastShowTime) > (TIME_UPDATE_INTERVAL * 1000) || last_press > lastShowTime) {
    update_display();
    lastShowTime = millis();
  }

  buttonOne.check();
  buttonTwo.check();
  buttonThree.check();

  // delay(2);
}

/**
   Setup Event Handlers of a Button
   @param buttonConfig
   @param eventHandler
*/
void setButtonConfig(ButtonConfig *buttonConfig, ButtonConfig::EventHandler eventHandler)
{
  buttonConfig->setEventHandler(eventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}

/**
   Setup all Buttons.
*/
void setupButtons()
{
  pinMode(BUTTON_ONE_PIN, INPUT);
  buttonOne.init(BUTTON_ONE_PIN, LOW);

  pinMode(BUTTON_TWO_PIN, INPUT);
  buttonTwo.init(BUTTON_TWO_PIN, LOW);

  pinMode(BUTTON_THREE_PIN, INPUT);
  buttonThree.init(BUTTON_THREE_PIN, LOW);

  setButtonConfig(buttonOne.getButtonConfig(), handleButtonOneEvent);
  setButtonConfig(buttonTwo.getButtonConfig(), handleButtonTwoEvent);
  setButtonConfig(buttonThree.getButtonConfig(), handleButtonThreeEvent);
}

/**
   Handle clicks of 1. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonOneEvent(AceButton *button, uint8_t eventType,
                          uint8_t buttonState)
{
  last_press = millis();
  switch (eventType)
  {
    case AceButton::kEventClicked:
      Serial.println("Button One Clicked");
      switch (current_mode)
      {
        case main_mode:
          Serial.println("Seconds mode");
          last_change = millis();
          config ^= SECONDS_CONFIG;
          break;
        case config_mode:
          last_change = millis();
          config ^= ITIS_CONFIG;
          break;
        case clock_mode:
          Serial.println("increase_hour");
          increase_hour();
          break;
        case brightness_mode:
          Serial.println("decrease_brightness");
          decrease_brightness(current_brightness);
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button One Double clicked");
      switch (current_mode)
      {
        case main_mode:
          Serial.println("Seconds mode");
          last_change = millis();
          config ^= TWENTY_CONFIG;
          break;
        case config_mode:
          break;
        case clock_mode:
          Serial.println("increase_hour");
          increase_hour();
          increase_hour();
          break;
        case brightness_mode:
          Serial.println("decrease_brightness");
          decrease_brightness(current_brightness);
          decrease_brightness(current_brightness);
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button One Long Press");
      switch (current_mode)
      {
        case main_mode:
          Serial.println("From main mode to clock mode");
          current_mode = clock_mode;
          break;
        case config_mode:
          last_change = millis();
          config ^= TWENTY_CONFIG;
          break;
        case clock_mode:
          Serial.println("decrease_hour");
          decrease_hour();
          break;
      }
      break;
  }
}

/**
   Handle clicks of 2. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonTwoEvent(AceButton *button, uint8_t eventType,
                          uint8_t buttonState)
{
  last_press = millis();

  switch (eventType)
  {
    case AceButton::kEventClicked:
      Serial.println("Button Two Clicked");
      switch (current_mode) {
        case main_mode:
          toggle_brightness();
          break;
        case clock_mode:
          Serial.println("increase_5min");
          increase_5min();
          break;
        case config_mode:
           last_change = millis();
           config ^= OVER_CONFIG;
           break;    
        case brightness_mode:
          Serial.print("switch brightness ");
          Serial.print(current_brightness);
          if (current_brightness == set_low)
          {
            current_brightness = set_high;
          }
          else
          {
            current_brightness = set_low;
          }
          Serial.print(" to ");
          Serial.println(current_brightness);
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button Two Double clicked");
      switch (current_mode) {
        case clock_mode:
          Serial.println("increase_5min");
          increase_5min();
          increase_5min();
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button Two Long Press");
      switch (current_mode) {
        case clock_mode:
          Serial.println("decrease_5min");
          decrease_5min();
          break;
        case main_mode:
          Serial.println("From main mode to brightness mode");
          current_mode = brightness_mode;
          current_brightness = set_high;
          break;
        case brightness_mode:
          Serial.println("From brightness mode to main mode");
          current_mode = main_mode;
          current_brightness = brightness::automatic;
          last_change = millis();
          break;
        case config_mode:
           last_change = millis();
           config ^= DOT_CONFIG;
           break;    
      }
  }
}

/**
   Handle clicks of 3. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonThreeEvent(AceButton *button, uint8_t eventType,
                            uint8_t buttonState)
{
  last_press = millis();

  switch (eventType)
  {
    case AceButton::kEventClicked:
      Serial.println("Button Three Clicked");
      switch (current_mode) {
        case main_mode:
          toggle_rgb();
          break;
        case clock_mode:
          Serial.println("increase_1min");
          increase_1min();
          break;
        case brightness_mode:
          Serial.println("increase_brightness");
          increase_brightness(current_brightness);
          break;
        case config_mode:
           last_change = millis();
           config ^= BLACKLIGHT_CONFIG;
           break;    
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button Three Double clicked");
      switch (current_mode) {
        case main_mode:
          toggle_rgb();
          toggle_rgb();
          break;
        case clock_mode:
          Serial.println("increase_1min");
          increase_1min();
          increase_1min();
          break;
        case brightness_mode:
          Serial.println("increase_brightness");
          increase_brightness(current_brightness);
          increase_brightness(current_brightness);
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button Three Long Press");
      switch (current_mode) {
        case main_mode:
          last_change = millis();
          current_mode = config_mode;
          break;
        case clock_mode:
          Serial.println("decrease_1min");
          decrease_1min();
          break;
        case config_mode:
          last_change = millis();
          current_mode = main_mode;
          break;
      }
      break;
  }
}

void update_display()
{
  Serial.println("update_display");
  
  // First determine the brightness and color of the display
  HsbColor currentcolor = current_color;

  switch (current_brightness)
  {
    case automatic:
      {
        int reading = analogRead(lightPin);
        currentcolor.B = min(1.0f, reading / 1024.0f);
      }
      break;
    case set_high:
      currentcolor.B = pow(step, 15 - auto_high); // auto_levels[auto_high] / 255.0f;
      break;
    case set_low:
      currentcolor.B = pow(step, 15 - auto_low); // auto_levels[auto_low] / 255.0f;
      break;
    default:
      currentcolor.B = current_brightness / 255.0f;
      break;
  }

  switch (current_mode)
  {
    case main_mode:
      display_hour(currentcolor, dt.hour, dt.minute, dt.second, config);
      break;
    case config_mode:
      display_hour(currentcolor, 12, 24, 45, config | SET_CONFIG);
      break;
    case clock_mode:
      display_hour(currentcolor, dt.hour, dt.minute, dt.second, config | SETCLOCK_CONFIG);
      break;
    case brightness_mode:
      // show_bar();
      break;
  }
}

void enable_led(LedWord  word, const HsbColor &color)
{
  if (word.firstPixelY % 2 == 0) {
    int start_index = word.firstPixelY * 15 + word.firstPixelX;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      strip.SetPixelColor(start_index + i, color);
    }
  } else {
    int start_index = word.firstPixelY * 15 + 10 - word.firstPixelX;
    for (int i = 0; i < word.length; ++i) {
      strip.SetPixelColor(start_index - i, color);
    }
  }
}

void enableMinuteDots(int minute, const HsbColor &color, byte config)
{
   int minuteDots = minute % 5;
   if (config & DOT_CONFIG) {
      Serial.print("enableMinuteDots ");
      Serial.println(minuteDots);
      if (minuteDots > 0) 
        strip.SetPixelColor(146, color);
      if (minuteDots > 1) 
        strip.SetPixelColor(147, color);
      if (minuteDots > 2) 
        strip.SetPixelColor(148, color);
      if (minuteDots > 3) 
        strip.SetPixelColor(149, color);
   }
   
}

void display_hour(const HsbColor &color, int hour, int minute, int seconds, byte config)
{
  Serial.print("display_hour ");
  Serial.print((int)config);
  Serial.println();
  
  strip.ClearTo(RgbColor(0, 0, 0));

  int fiveMinutes = minute / 5;

  LedWord after = INFIX_AFTER2;
  if (config & OVER_CONFIG)
    after = INFIX_AFTER;

  int threshold = 4;
  switch (fiveMinutes) {
    case 0:
      enable_led(SUFFIX_OCLOCK, color);
      config |= ITIS_CONFIG; 
      break;
    case 1:
      enable_led(MINUTE_FIVE, color);
      enable_led(after, color);
      break;
    case 2:
      enable_led(MINUTE_TEN, color);
      enable_led(after, color);
      break;
    case 3:
      enable_led(MINUTE_QUARTER, color);
      enable_led(after, color);
      break;
    case 4:
      if (config & TWENTY_CONFIG) {
        threshold = 5;
        enable_led(MINUTE_TWENTY, color);
        enable_led(after, color); 
      }
      else {
        enable_led(MINUTE_TEN, color);
        enable_led(INFIX_BEFORE, color);
        enable_led(MINUTE_HALF, color);
      }
      break;
    case 5:
      enable_led(MINUTE_FIVE, color);
      enable_led(INFIX_BEFORE, color);
      enable_led(MINUTE_HALF, color);
      break;
    case 6:
      enable_led(MINUTE_HALF, color);
      break;
    case 7:
      enable_led(MINUTE_FIVE, color);
      enable_led(after, color);
      enable_led(MINUTE_HALF, color);
      break;
    case 8:
      if (config & TWENTY_CONFIG) {
        enable_led(MINUTE_TWENTY, color);
        enable_led(INFIX_BEFORE, color);
      } 
      else {    
        enable_led(MINUTE_TEN, color);
        enable_led(after, color);
        enable_led(MINUTE_HALF, color);
      }
      break;
    case 9:
      enable_led(MINUTE_QUARTER, color);
      enable_led(INFIX_BEFORE, color);
      break;
    case 10:
      enable_led(MINUTE_TEN, color);
      enable_led(INFIX_BEFORE, color);
      break;
    case 11:
      enable_led(MINUTE_FIVE, color);
      enable_led(INFIX_BEFORE, color);
      break;
  }

  if (config & ITIS_CONFIG) {
    enable_led(PREFIX_IT, color);
    enable_led(PREFIX_IS, color);
  }
  if (config & SETCLOCK_CONFIG) {
    enable_led(PREFIX_IT, HsbColor(0.125, 0.5, 1));
    enable_led(PREFIX_IS, HsbColor(0.625, 0.5, 1));
  }


  int hourIndex = 0;
  if (fiveMinutes < threshold) {
    hourIndex = (hour + 11) % 12;
  } else {
    hourIndex = (hour) % 12;
  }
  // enable_led(HOURS[hourIndex], color);
  switch (hourIndex) {
    case 0: enable_led(LED_ONE, color); break;
    case 1: enable_led(LED_TWO, color); break;
    case 2: enable_led(LED_THREE, color); break;
    case 3: enable_led(LED_FOUR, color); break;
    case 4: enable_led(LED_FIVE, color); break;
    case 5: enable_led(LED_SIX, color); break;
    case 6: enable_led(LED_SEVEN, color); break;
    case 7: enable_led(LED_EIGHT, color); break;
    case 8: enable_led(LED_NINE, color); break;
    case 9: enable_led(LED_TEN, color); break;
    case 10: enable_led(LED_ELEVEN, color); break;
    case 11: enable_led(LED_TWELVE, color); break;
  }

  if (config & SECONDS_CONFIG) {
    HsbColor second_color = color;
    second_color.B = min(1, second_color.B * 2);
    /*
    if (config & SET_CONFIG) {
        enable_led(LedWord(5, 0, 5), second_color);
        enable_led(LedWord(10, 1, 1), second_color);
        enable_led(LedWord(10, 2, 1), second_color);
        enable_led(LedWord(10, 3, 1), second_color);
        enable_led(LedWord(10, 4, 1), second_color);
        enable_led(LedWord(10, 5, 1), second_color);
        enable_led(LedWord(10, 6, 1), second_color);
        enable_led(LedWord(10, 7, 1), second_color);
        enable_led(LedWord(10, 8, 1), second_color);
        enable_led(LedWord(0, 9, 11), second_color);
        enable_led(LedWord(0, 8, 1), second_color);
        enable_led(LedWord(0, 7, 1), second_color);
        enable_led(LedWord(0, 6, 1), second_color);
        enable_led(LedWord(0, 5, 1), second_color);
    }
    else {
    */
      if (seconds < 8) {
        int where = (int)round(seconds / 1.5f);
        enable_led(LedWord(5 + where, 0, 1), second_color);
      } else if (seconds < 23) {
        int where = (int)round((seconds - 7.5) / 1.666f);
        enable_led(LedWord(10, where, 1), second_color);
      } else if (seconds < 38) {
        int where = (int)round( (seconds - 22.5) / 1.5f);
        enable_led(LedWord(10 - where, 9, 1), second_color);
      } else if (seconds < 53) {
        int where = (int)round((seconds - 37.5) / 1.666f);
        enable_led(LedWord(0, 10-where, 1), second_color);
      } else {
        int where = (int)round( (seconds - 52.5) / 1.5f);
        enable_led(LedWord(where, 0, 1), second_color);
      }
    /*
    }
    */
  }

  enableMinuteDots(minute, color, config);

  if (config & BLACKLIGHT_CONFIG) {
    for (int r=0;r<10;++r) {
      enable_led(LedWord(11 + r * 15, 0, 4), color);
    } 
  }

  strip.Show();
}

void increase_hour()
{
  last_change = millis();
  dt = clock.getDateTime();
  uint8_t hour = dt.hour + 1;
  if (hour > 23)
    hour = 0;
  clock.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
}

void decrease_hour()
{
  last_change = millis();
  dt = clock.getDateTime();
  int hour = (int)dt.hour - 1;
  if (hour < 0)
    hour = 23;
  clock.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
}

void increase_5min()
{
  last_change = millis();
  dt = clock.getDateTime();
  int min = dt.minute + 5;
  if (min > 59)
    min = 60 - min;
  clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, dt.second);
}

void decrease_5min()
{
  last_change = millis();
  dt = clock.getDateTime();
  int min = (int)dt.minute - 5;
  if (min < 0)
    min = 60 + min;
  clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, dt.second);
}

void increase_1min()
{
  last_change = millis();
  dt = clock.getDateTime();
  int min = dt.minute + 1;
  if (min > 59)
    min = 0;
  clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, 0);
}

void decrease_1min()
{
  last_change = millis();
  dt = clock.getDateTime();
  int min = (int)dt.minute - 1;
  if (min < 0)
    min = 59;
  clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, 0);
}

void toggle_brightness()
{
  Serial.println("toggle_brightness");
  
  switch (current_brightness) {
    case automatic:
      current_brightness = very_high;
      break;
    case very_high:
      current_brightness = high;
      break;
    case high:
      current_brightness = medium;
      break;
    case medium:
      current_brightness = low;
      break;
    case low:
      current_brightness = very_low;
      break;
    case very_low:
      current_brightness = automatic;
      break;
    default:
      current_brightness = automatic;
      break;
  }

  Serial.print("Current brightness ");
  Serial.println((int)current_brightness);
  last_change = millis();
}

void toggle_rgb()
{
  Serial.println("toggle_rgb");
  last_change = millis();
  update_color(color_index + 1);
}

void increase_brightness(brightness high_or_low)
{
  last_change = millis();
  if (high_or_low == set_high) {
    auto_high = min(15, auto_high + 1);
  }
  if (high_or_low == set_low) {
    auto_low = min(auto_high - 1, auto_low + 1);
  }
}

void decrease_brightness(brightness high_or_low)
{
  last_change = millis();
  if (high_or_low == set_high) {
    auto_high = max(auto_low + 1, auto_high - 1);
  }
  if (high_or_low == set_low) {
    auto_low = max(0, auto_low - 1);
  }
}

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
    Serial.println("Reading of register failed");
  }
  Wire.endTransmission();
  return value;
}


void store_settings()
{
  Serial.println("store_settings");
  writeRegister8(0x08, (byte)color_index);
  writeRegister8(0x09, (byte)config);
  writeRegister8(0x0A, (byte)auto_high);
  writeRegister8(0x0B, (byte)auto_low);

  int bm = max(0, current_brightness);
  writeRegister8(0x0C, (byte)bm);

}

void restore_settings()
{
  Serial.println("restore_settings");
  color_index = min(nr_led_colors, readRegister8(0x08));
  update_color(color_index);
  config = readRegister8(0x09);
  auto_high = min(15, readRegister8(0x0A));
  auto_low = min(auto_high - 1, readRegister8(0x0B));
  byte bmb = readRegister8(0x0C);
  if (bmb == 0)
    current_brightness = automatic;
  else
    current_brightness = (byte)bmb;
}
