// Clock
#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

// Buttons
#include <AceButton.h>

using namespace ace_button;

#define BUTTON_ONE_PIN 2
#define BUTTON_TWO_PIN 0
#define BUTTON_THREE_PIN 4
#define BUTTON_FOUR_PIN 3

// Photocell
#define lightPin A0

#define NEOPIN 5

AceButton buttonOne(new ButtonConfig(),BUTTON_ONE_PIN, 1);
AceButton buttonTwo(new ButtonConfig(),BUTTON_TWO_PIN, 1);
AceButton buttonThree(new ButtonConfig(),BUTTON_THREE_PIN, 1);
AceButton buttonFour(new ButtonConfig(),BUTTON_FOUR_PIN, 1);

void setButtonConfig(ButtonConfig *buttonConfig, ButtonConfig::EventHandler eventHandler);

void handleButtonOneEvent(AceButton *, uint8_t, uint8_t);
void handleButtonTwoEvent(AceButton *, uint8_t, uint8_t);
void handleButtonThreeEvent(AceButton *, uint8_t, uint8_t);
void handleButtonFourEvent(AceButton *, uint8_t, uint8_t);

// Buttons
void setupButtons();


// Color
#include <NeoPixelBus.h>
HsbColor current_color(0, 1, 0);

// Function that selects and updates the current_color and the color_index
HsbColor adjust_color(int update);

// User Interface
enum menu_mode
{
  main_mode = 0,
  clock_mode = 1,
  brightness_mode = 2,
  config_mode = 3,
  rgb_mode = 4,
  happiness_mode = 5,
  debug_mode = 6
};
enum submenu_mode {
  clock_hour,
  clock_5min,
  clock_min,
  set_high,
  set_low,
  set_hue,
  set_sat 
};
menu_mode current_mode = menu_mode::main_mode;
submenu_mode sub_mode;

#define ITIS_CONFIG 0x01
#define TWENTY_CONFIG 0x02
#define OVER_CONFIG 0x04
#define SECONDS_CONFIG 0x08
#define DOT_CONFIG 0x10
#define BLACKLIGHT_CONFIG 0x20
#define SETCLOCK_CONFIG 0x40
#define SECONDSBAR_CONFIG 0x80
byte config = ITIS_CONFIG | TWENTY_CONFIG;

enum brightness
{
  high,
  low,
  automatic
};

// const float step = exp( log(255) / 10);
// #define step 1.7404198104793391
// const float step = exp( log(255) / 9);
#define step 1.8509443151142613
#define MAX_BRIGHTNESS 9
#define LRD_SCALE 128
int brightness_high = MAX_BRIGHTNESS;
int brightness_low = 0;
brightness current_brightness = brightness::automatic;

#define TIMEOUT 60000 // in milliseconds
#define TIME_UPDATE_INTERVAL 250 // in milli seconds
#define CLOCK_UPDATE_INTERVAL 250 // in milli seconds
#define BLINK_INTERVAL 500 // in milli seconds

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
void show_hue();
void show_debug(int seconds);

void toggle_brightness();
void change_brightness(int update);
void toggle_rgb();

void change_hour(int h, int m5, int m, bool s);

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

#define INFIX_AFTER2 LedWord(9, 2, 2)
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

#define LED_OFFSET 0 // Normally 1
#define LED_ROW 13
#define NR_LEDS 133 // Normally 134
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NR_LEDS, NEOPIN);

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

void setup()
{
  Serial.begin(9600);

  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'

  // Some example procedures showing how to display to the pixels:
  colorWipe(RgbColor(0, 0, 255), 2); // Red

  Serial.println("Initialize RTC module");
  
  // Initialize DS3231
  clock.begin();

  Serial.println("Setup Buttons");
  setupButtons();

  // Manual (YYYY, MM, DD, HH, II, SS
  // clock.setDateTime(2016, 12, 9, 11, 46, 00);

  // Send sketch compiling time to Arduino
  // clock.setDateTime(__DATE__, __TIME__);


  update_display();

  restore_settings();
  Serial.println("Initialization done");
}

// Fill the dots one after the other with a color
void colorWipe(RgbColor c, uint8_t wait) {
  Serial.println("colorWipe");
  HsbColor color(0, 1, 0.25);
  for (uint16_t i = 0; i < strip.PixelCount(); i++) {
    color.H = (float)i / (float)strip.PixelCount();
    strip.SetPixelColor(i, color);
    strip.Show();
    delay(wait);
  }
  delay(1000 * (long)wait);
}

#define nr_led_colors 11
HsbColor adjust_color(int update, bool sat = false) {
  if (sat) {
    int sat = round(current_color.S * (nr_led_colors-1));
    sat = sat + update;
    sat = min(nr_led_colors-1,max(0, sat));
    current_color.S = sat / (nr_led_colors-1.0f);
  }
  else {
    int hue = round(current_color.H * nr_led_colors);
    hue = hue + update;
    if (abs(update) == 1) {
      hue = min(nr_led_colors-1,max(0,hue));
    }
    else {
      if (hue > nr_led_colors-1) {
          hue = hue - nr_led_colors;
      }
      if (hue < 0) {
          hue = nr_led_colors + hue;
      }
    }
    current_color.H = (float)hue / nr_led_colors;
  }
  Serial.print("update_color ");
  Serial.println(current_color.H);
  return current_color;
}

void loop()
{
  // We need continuous updates for interactive modes
  switch (current_mode) {
    case clock_mode:
    case brightness_mode:
    case rgb_mode:
    case config_mode:
      lastClockUpdate = 0;
      lastShowTime = 0;
      break;
  }

  unsigned long current_time = millis();
  int scale = config & SECONDS_CONFIG ? 1 : 4;
  if ((current_time - lastClockUpdate) > (CLOCK_UPDATE_INTERVAL * scale) || last_press > lastClockUpdate) {
    // For leading zero look to DS3231_dateformat example
    dt = clock.getDateTime();
    lastClockUpdate = current_time;
  }

  if ( last_change != 0 && current_time > (last_change + TIMEOUT) ) {
    store_settings();
    last_change = 0;
  }

  switch (current_mode)
  {
    case brightness_mode:
      if (last_press != 0 && current_time > (last_press + TIMEOUT) )
      {
        Serial.print("Timeout ");
        Serial.print(current_time);
        Serial.print(" ");
        Serial.println(last_press);
        current_brightness = automatic;
        current_mode = main_mode;
        last_press = 0;
      }
      break;
    case happiness_mode:
    case rgb_mode:
    case config_mode:
    case debug_mode:
      if (last_press != 0 && current_time > (last_press + TIMEOUT) )
      {
        Serial.print("Timeout ");
        Serial.print(current_time);
        Serial.print(" ");
        Serial.println(last_press);
        current_mode = main_mode;
        last_press = 0;
      }
      break;
    case clock_mode:
      if (last_press != 0 && current_time > (last_press + 2 * TIMEOUT) )
      {
        Serial.print("Timeout ");
        Serial.print(current_time);
        Serial.print(" ");
        Serial.println(last_press);
        current_mode = main_mode;
        last_press = 0;
      }
      break;
  }

  if ((current_time - lastShowTime) > (TIME_UPDATE_INTERVAL * scale) || last_press > lastShowTime) {
    update_display();
    lastShowTime = current_time;
  }

  buttonOne.check();
  buttonTwo.check();
  buttonThree.check();
  buttonFour.check();
  
  delay(2);
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
  buttonConfig->setClickDelay(500);
}

/**
   Setup all Buttons.
*/
void setupButtons()
{
  pinMode(BUTTON_ONE_PIN, INPUT);
  pinMode(BUTTON_TWO_PIN, INPUT);
  pinMode(BUTTON_THREE_PIN, INPUT);
  pinMode(BUTTON_FOUR_PIN, INPUT);

  setButtonConfig(buttonOne.getButtonConfig(), handleButtonOneEvent);
  setButtonConfig(buttonTwo.getButtonConfig(), handleButtonTwoEvent);
  setButtonConfig(buttonThree.getButtonConfig(), handleButtonThreeEvent);
  setButtonConfig(buttonFour.getButtonConfig(), handleButtonFourEvent);

  buttonOne.init((uint8_t)BUTTON_ONE_PIN, HIGH);
  buttonTwo.init((uint8_t)BUTTON_TWO_PIN, HIGH);
  buttonThree.init((uint8_t)BUTTON_THREE_PIN, HIGH);
  buttonFour.init((uint8_t)BUTTON_FOUR_PIN, HIGH);
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
  Serial.print("Button One");
  Serial.print(eventType);
  Serial.println(buttonState);
  switch (eventType)
  {
    case AceButton::kEventClicked:
      Serial.println("Button One Clicked");
      switch (current_mode)
      {
        case main_mode:
          Serial.println("Seconds mode");
          last_change = last_press;
          config ^= SECONDS_CONFIG;
          break;
        case config_mode:
          last_change = last_press;
          config ^= ITIS_CONFIG;
          break;
        case clock_mode:
          Serial.println("decrease_clock");
          decrease_clock();
          break;
        case brightness_mode:
          Serial.println("decrease_brightness");
          last_change = last_press;
          change_brightness(-1);
          break;
        case rgb_mode:
          last_change = last_press;
          adjust_color(-1, sub_mode == set_sat);
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button One Double clicked");
      switch (current_mode)
      {
        case main_mode:
          Serial.println("Seconds mode");
          last_change = last_press;
          config ^= SECONDS_CONFIG;
          break;
        case config_mode:
          last_change = last_press;
          config ^= ITIS_CONFIG;
          break;
        case clock_mode:
          Serial.println("decrease_clock");
          decrease_clock();
          decrease_clock();
          break;
        case brightness_mode:
          Serial.println("decrease_brightness");
          last_change = last_press;
          change_brightness(-1);
          change_brightness(-1);
          break;
        case rgb_mode:
          last_change = last_press;
          adjust_color(-1, sub_mode == set_sat);
          adjust_color(-1, sub_mode == set_sat);
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
          sub_mode = clock_hour;
          break;
        case config_mode:
          last_change = last_press;
          config ^= DOT_CONFIG;
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
          last_change = last_press;
          toggle_brightness();
          break;
        case clock_mode:
          switch(sub_mode) {
            case clock_hour:
              sub_mode = clock_5min;
              break;
            case clock_5min:
              sub_mode = clock_min;
              break;
            default:
              sub_mode = clock_hour;
              break;
          }
          Serial.print("clock_mode sub_mode:");
          Serial.println(sub_mode);
          break;
        case config_mode:
           last_change = last_press;
           config ^= OVER_CONFIG;
           break;    
        case brightness_mode:
          Serial.print("switch brightness ");
          Serial.print(sub_mode);
          if (sub_mode == set_low)
          {
            sub_mode = set_high;
          }
          else
          {
            sub_mode = set_low;
          }
          Serial.print(" to ");
          Serial.println(sub_mode);
          break;
        case rgb_mode:
          if (sub_mode == set_hue)
            sub_mode = set_sat;
          else
            sub_mode = set_hue;
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button Two Double clicked");
      switch (current_mode) {
        case clock_mode:
          switch(sub_mode) {
            case clock_hour:
              sub_mode = clock_5min;
              break;
            case clock_5min:
              sub_mode = clock_min;
              break;
            default:
              sub_mode = clock_hour;
              break;
          }
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button Two Long Press");
      switch (current_mode) {
        case clock_mode:
          Serial.println("reset_seconds");
          change_hour(0,0,0,true);
          break;
        case main_mode:
          Serial.println("From main mode to brightness mode");
          current_mode = brightness_mode;
          sub_mode = set_high;
          break;
        case config_mode:
           last_change = last_press;
           config ^= TWENTY_CONFIG;
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
          last_change = last_press;
          toggle_rgb();
          break;
        case clock_mode:
          Serial.println("increase_clock");
          increase_clock();
          break;
        case brightness_mode:
          Serial.println("increase_brightness");
          last_change = last_press;
          change_brightness(1);
          break;
        case config_mode:
           last_change = last_press;
           config ^= BLACKLIGHT_CONFIG;
           break;    
        case rgb_mode:
          last_change = last_press;
          adjust_color(1, sub_mode == set_sat);
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button Three Double clicked");
      switch (current_mode) {
        case main_mode:
          last_change = last_press;
          toggle_rgb();
          toggle_rgb();
          break;
        case clock_mode:
          Serial.println("increase_clock");
          increase_clock();
          increase_clock();
          break;
        case brightness_mode:
          Serial.println("increase_brightness");
          last_change = last_press;
          change_brightness(1);
          change_brightness(1);
          break;
        case rgb_mode:
          last_change = last_press;
          adjust_color(1, sub_mode == set_sat);
          adjust_color(1, sub_mode == set_sat);
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button Three Long Press");
      switch (current_mode) {
        case main_mode:
          Serial.println("From main mode to rgb mode");
          current_mode = rgb_mode;
          sub_mode = set_hue;
          break;
        case clock_mode:
          // Do nothing
          break;
        case config_mode:
          last_change = last_press;
          config ^= SECONDSBAR_CONFIG;
          break;    
      }
      break;
  }
}

/**
   Handle clicks of 3. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonFourEvent(AceButton *button, uint8_t eventType,
                            uint8_t buttonState)
{
  last_press = millis();

  switch (eventType)
  {
    case AceButton::kEventClicked:
      Serial.println("Button Four Clicked");
      switch(current_mode) {
        case main_mode:
          current_mode = happiness_mode;
          break;
      }
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Button Four Double clicked");
      switch(current_mode) {
        case happiness_mode:
          current_mode = debug_mode;
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Button Four Long Press");
      switch(current_mode) {
        case main_mode:
          current_mode = config_mode;
          break; 
        default:
          current_mode = main_mode;
          break;
      }
      break;
  }
}

void update_display()
{
  //Serial.print("update_display brightness: ");
  
  // First determine the brightness and color of the display
  switch (current_brightness)
  {
    case automatic:
      {
        int reading = analogRead(lightPin);
        float scale = brightness_low + max(0.0f, min(1.0f, reading / (float)LRD_SCALE)) * (brightness_high - brightness_low);
        current_color.B = pow(step, scale) / 255.0f;
      }
      break;
    case high:
      current_color.B = pow(step, brightness_high) / 255.0f; // auto_levels[auto_high] / 255.0f;
      break;
    case low:
      current_color.B = pow(step, brightness_low) / 255.0f; // auto_levels[auto_low] / 255.0f;
      break;
  }
  //Serial.println(currentcolor.B);

  switch (current_mode)
  {
    case main_mode:
      display_hour(current_color, dt.hour, dt.minute, dt.second, config);
      break;
    case config_mode:
      if (config & TWENTY_CONFIG) {
        display_hour(current_color, 12, 24, dt.second, config | SECONDS_CONFIG);        
      }
      else {
        display_hour(current_color, 12, 44, dt.second, config | SECONDS_CONFIG);        
      }
      break;
    case clock_mode:
      display_hour(current_color, dt.hour, dt.minute, dt.second, config | SETCLOCK_CONFIG | SECONDS_CONFIG);
      break;
    case rgb_mode:
      show_hue();
      break;
    case brightness_mode:
      show_bar();
      break;
    case happiness_mode:
      show_happiness(current_color);
      break;
    case debug_mode:
      show_debug(dt.second);
      break;
  }
}

void enable_led(LedWord  word, const HsbColor &color, bool display=true)
{
  if (!display) return;
  if (word.firstPixelY % 2 == 0) {
    int start_index = word.firstPixelY * LED_ROW + word.firstPixelX + LED_OFFSET;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      strip.SetPixelColor(start_index + i, color);
    }
  } else {
    int start_index = word.firstPixelY * LED_ROW + 10 - word.firstPixelX + LED_OFFSET;
    for (int i = 0; i < word.length; ++i) {
      strip.SetPixelColor(start_index - i, color);
    }
  }
}

void enableMinuteDots(int minute, const HsbColor &color, byte config)
{
   int minuteDots = minute % 5;
   if (config & DOT_CONFIG) {
      //Serial.print("enableMinuteDots ");
      //Serial.println(minuteDots);
      if (minuteDots > 0) 
        strip.SetPixelColor(NR_LEDS-3, color);
      if (minuteDots > 1) 
        strip.SetPixelColor(NR_LEDS-2, color);
      if (minuteDots > 2) 
        strip.SetPixelColor(NR_LEDS-1, color);
      if (minuteDots > 3) 
        strip.SetPixelColor(NR_LEDS-4, color);
   }
   
}

void show_bar()
{
  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;
  HsbColor currentcolor = current_color;
  strip.ClearTo(RgbColor(0, 0, 0));
  if (sub_mode != set_low or blink) {
    currentcolor.B = pow(step, brightness_low) / 255.0f;
    enable_led(LedWord(0, brightness_low, 10-brightness_low), currentcolor);
  }  
  for(int i=brightness_low+1; i<brightness_high; ++i) {
    currentcolor.B = pow(step, i) / 255.0f;
    enable_led(LedWord(0, i, 10-i), currentcolor);
  }
  if (sub_mode != set_high or blink) {
    currentcolor.B = pow(step, brightness_high) / 255.0f;
    enable_led(LedWord(0, brightness_high, 10-brightness_high), currentcolor);
  }  
  strip.Show();
}

void show_hue()
{
  bool blink =  ((millis() / BLINK_INTERVAL) % 2) == 0;
  strip.ClearTo(RgbColor(0, 0, 0));
  for(int i=0; i<nr_led_colors; ++i) {
    enable_led(LedWord(i,0,1), HsbColor((float)i / nr_led_colors, 1, current_color.B));  
    enable_led(LedWord(i,1,1), HsbColor((float)i / nr_led_colors, 1, current_color.B));  

    enable_led(LedWord(i,5,1), HsbColor(current_color.H, (float)i / (nr_led_colors-1), current_color.B));  
    enable_led(LedWord(i,6,1), HsbColor(current_color.H, (float)i / (nr_led_colors-1), current_color.B));  
  }
  if (sub_mode != set_hue  || blink) {
    int hue = round(current_color.H * nr_led_colors);
    enable_led(LedWord(hue,2,1), HsbColor((float)hue / nr_led_colors, 1, current_color.B));  
    enable_led(LedWord(hue,3,1), HsbColor((float)hue / nr_led_colors, 1, current_color.B));  
  }
  if (sub_mode != set_sat || blink) {
    int sat = round(current_color.S * (nr_led_colors-1));
    enable_led(LedWord(sat,7,1), HsbColor(current_color.H, (float)sat / (nr_led_colors-1), current_color.B));  
    enable_led(LedWord(sat,8,1), HsbColor(current_color.H, (float)sat / (nr_led_colors-1), current_color.B));  
  }
  HsbColor currentcolor = current_color;
  for(int i=brightness_low; i<=brightness_high; ++i) {
    currentcolor.B = pow(step, i) / 255.0f;
    enable_led(LedWord(i, 9, 1), currentcolor);
  }
  strip.Show();
}

void show_happiness(const HsbColor &color)
{
  bool gert =  ((millis() / 2000) % 4) == 0;
  strip.ClearTo(RgbColor(0, 0, 0));

  if (gert) {
      enable_led(LedWord(3,0,1), color, true);
      enable_led(LedWord(6,0,1), color, true);
      enable_led(LedWord(4,1,2), color, true);
      enable_led(LedWord(7,2,2), color, true);
      enable_led(LedWord(10,4,1), color, true);
      enable_led(LedWord(5,7,1), color, true);
      enable_led(LedWord(6,9,2), color, true);
  } else {
      enable_led(LedWord(4,3,3), color, true);
      enable_led(LedWord(4,4,3), color, true);
      enable_led(LedWord(4,5,3), color, true);    
  }
  
  strip.Show();
}

void show_debug(int seconds) {
  HsbColor color(0, 0, 1);
  int show = seconds % 8;
  strip.ClearTo(RgbColor(0, 0, 0));
  for (uint16_t i = 0; i < strip.PixelCount(); i++) {
    if ( (i % 8) == show) 
      strip.SetPixelColor(i, color);
  }
  strip.Show();
}

void display_hour(const HsbColor &color, int hour, int minute, int seconds, byte config)
{
  //Serial.print("display_hour ");
  //Serial.print((int)config);
  //Serial.println();
  
  strip.ClearTo(RgbColor(0, 0, 0));

  int fiveMinutes = minute / 5;

  LedWord after = INFIX_AFTER2;
  if (config & OVER_CONFIG)
    after = INFIX_AFTER;

  int threshold = 4;
  bool blink =  (millis() / BLINK_INTERVAL) % 2 == 0;
  bool display_5min = !(config & SETCLOCK_CONFIG) || (sub_mode != clock_5min) || blink;
  switch (fiveMinutes) {
    case 0:
      enable_led(SUFFIX_OCLOCK, color, display_5min);
      config |= ITIS_CONFIG; 
      break;
    case 1:
      enable_led(MINUTE_FIVE, color, display_5min);
      enable_led(after, color);
      break;
    case 2:
      enable_led(MINUTE_TEN, color, display_5min);
      enable_led(after, color);
      break;
    case 3:
      enable_led(MINUTE_QUARTER, color, display_5min);
      enable_led(after, color);
      break;
    case 4:
      if (config & TWENTY_CONFIG) {
        threshold = 5;
        enable_led(MINUTE_TWENTY, color, display_5min);
        enable_led(after, color); 
      }
      else {
        enable_led(MINUTE_TEN, color, display_5min);
        enable_led(INFIX_BEFORE, color);
        enable_led(MINUTE_HALF, color);
      }
      break;
    case 5:
      enable_led(MINUTE_FIVE, color, display_5min);
      enable_led(INFIX_BEFORE, color);
      enable_led(MINUTE_HALF, color);
      break;
    case 6:
      enable_led(MINUTE_HALF, color, display_5min);
      break;
    case 7:
      enable_led(MINUTE_FIVE, color, display_5min);
      enable_led(after, color);
      enable_led(MINUTE_HALF, color);
      break;
    case 8:
      if (config & TWENTY_CONFIG) {
        enable_led(MINUTE_TWENTY, color, display_5min);
        enable_led(INFIX_BEFORE, color);
      } 
      else {    
        enable_led(MINUTE_TEN, color, display_5min);
        enable_led(after, color);
        enable_led(MINUTE_HALF, color);
      }
      break;
    case 9:
      enable_led(MINUTE_QUARTER, color, display_5min);
      enable_led(INFIX_BEFORE, color);
      break;
    case 10:
      enable_led(MINUTE_TEN, color, display_5min);
      enable_led(INFIX_BEFORE, color);
      break;
    case 11:
      enable_led(MINUTE_FIVE, color, display_5min);
      enable_led(INFIX_BEFORE, color);
      break;
  }

  if (config & ITIS_CONFIG) {
    enable_led(PREFIX_IT, color);
    enable_led(PREFIX_IS, color);
  }
  if ( (config & SETCLOCK_CONFIG) && blink ){
    enable_led(PREFIX_IT, HsbColor(0.125, 0.5, 1));
    enable_led(PREFIX_IS, HsbColor(0.625, 0.5, 1));
  }


  int hourIndex = 0;
  if (fiveMinutes < threshold) {
    hourIndex = (hour + 11) % 12;
  } else {
    hourIndex = (hour) % 12;
  }

  if ( !(config & SETCLOCK_CONFIG) || (sub_mode != clock_hour) || blink) {
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
  }

  if (config & SECONDS_CONFIG) {
    HsbColor second_color = color;
    second_color.B = min(1, second_color.B * 2);
    if (config & SECONDSBAR_CONFIG) {
      int where = min(4, seconds);
      enable_led(LedWord(5, 0, where+1), second_color);
      if (seconds >= 6) {
        int where = (seconds - 6) / 2;
        for(int r=0;r<=where;++r)
          enable_led(LedWord(10, r, 1), second_color);
      } 
      if (seconds >= 26) {
        int where = min(9, seconds - 25);
        enable_led(LedWord(10 - where, 9, where+1), second_color);
      } 
      if (seconds >= 36) {
        int where = (seconds - 36) / 2;
        for(int r=0; r<=where; ++r)
          enable_led(LedWord(0, 9-r, 1), second_color);
      } 
      if (seconds >= 56) {
        int where = (seconds - 55);
        enable_led(LedWord(0, 0, where+1), second_color);
      }
    } else {
      if (seconds < 6) {
        int where = min(4, seconds);
        enable_led(LedWord(5 + where, 0, 1), second_color);
      } else if (seconds < 26) {
        int where = (seconds - 6) / 2;
        enable_led(LedWord(10, where, 1), second_color);
      } else if (seconds < 36) {
        int where = min(9, seconds - 25);
        enable_led(LedWord(10 - where, 9, 1), second_color);
      } else if (seconds < 56) {
        int where = (seconds - 36) / 2;
        enable_led(LedWord(0, 9-where, 1), second_color);
      } else {
        int where = (seconds - 55);
        enable_led(LedWord(where, 0, 1), second_color);
      }
    }
  }

  if (config & SETCLOCK_CONFIG)
    config |= DOT_CONFIG;
  if (!(config & SETCLOCK_CONFIG) || (sub_mode != clock_min) || blink) 
    enableMinuteDots(minute, color, config);

  if (config & BLACKLIGHT_CONFIG) {
    enable_led(LedWord(11 + (LED_ROW-11)/2, 0, (LED_ROW-11) / 2), color);
    for (int r=1;r<8;++r) {
      enable_led(LedWord(11 + r * LED_ROW, 0, LED_ROW-11), color);
    } 
    enable_led(LedWord(11 + 8 * LED_ROW, 0, (LED_ROW-11) / 2), color);
  }

  strip.Show();
}

void increase_clock()
{
  switch(sub_mode) {
  case clock_hour:
    change_hour(1, 0, 0, false);
    break;
  case clock_5min:
    change_hour(0, 1, 0, false);
    break;
  case clock_min:
    change_hour(0, 0, 1, true);
    break;    
  }
}

void decrease_clock()
{
  switch(sub_mode) {
  case clock_hour:
    change_hour(-1, 0, 0, false);
    break;
  case clock_5min:
    change_hour(0, -1, 0, false);
    break;
  case clock_min:
    change_hour(0, 0, -1, true);
    break;    
  }
}

void change_hour(int h, int min5, int m, bool reset)
{
  last_change = millis();
  dt = clock.getDateTime();
  int hour = (int)dt.hour;
  int min = dt.minute + min5 * 5 + m;
  if (min > 59) {
    min = 60 - min;  
    hour += 1;
  }
  if (min < 0) {
    min = 60 + min;
    hour -= 1;
  }
  hour += h;
  if (hour > 23)
    hour = 0;
  if (hour < 0)
    hour = 23;    
  int sec = reset ? 0 : dt.second;
  clock.setDateTime(dt.year, dt.month, dt.day, hour, min, sec);
  
}

void toggle_brightness()
{
  Serial.println("toggle_brightness");
  
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

  Serial.print("Current brightness ");
  Serial.println((int)current_brightness);
  last_change = millis();
}

void toggle_rgb()
{
  Serial.println("toggle_rgb");
  last_change = millis();
  adjust_color(2, false);
}

void change_brightness(int adjust)
{
  last_change = millis();
  if (sub_mode == set_high) {
    brightness_high = max(brightness_low+1, min(MAX_BRIGHTNESS, brightness_high + adjust));
  }
  if (sub_mode == set_low) {
    brightness_low = max(0, min(brightness_high - 1, brightness_low + adjust));
  }
  Serial.print(brightness_low); Serial.print(' '); Serial.println(brightness_high);
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
  int hue = round(current_color.H * nr_led_colors);
  writeRegister8(0x08, (byte)hue);
  writeRegister8(0x09, (byte)config);
  writeRegister8(0x0A, (byte)brightness_high);
  writeRegister8(0x0B, (byte)brightness_low);

  int bm = max(0, current_brightness);
  writeRegister8(0x0C, (byte)bm);

  int sat = round(current_color.S * (nr_led_colors-1));
  writeRegister8(0x0D, (byte)sat);

  Serial.print(hue);
  Serial.print(' ');
  Serial.println(sat);
}

void restore_settings()
{
  Serial.println("restore_settings");
  int hue = min(nr_led_colors-1, readRegister8(0x08));
  config = readRegister8(0x09);
  brightness_high = max(1,min(MAX_BRIGHTNESS, readRegister8(0x0A)));
  brightness_low = max(0,min(brightness_high-1, readRegister8(0x0B)));
  byte bmb = readRegister8(0x0C);
  if (bmb == 0)
    current_brightness = automatic;
  else
    current_brightness = static_cast<brightness>(bmb);
  int sat = min(nr_led_colors-1, readRegister8(0x0D));
  Serial.print(hue);
  Serial.print(' ');
  Serial.println(sat);
  current_color.H = (float)hue / nr_led_colors;
  current_color.S = (float)sat / (nr_led_colors-1);
}
