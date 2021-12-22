// include the library code:
#include <LiquidCrystal_74HC595.h>

// - Original code from the tutorial
// initialize the library with the numbers of the interface pins
// LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//
// - New code to support LiquidCrystal with 74HC595
LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

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

AceButton buttonOne(new ButtonConfig());
AceButton buttonTwo(new ButtonConfig());
AceButton buttonThree(new ButtonConfig());

// Buttons
void setupButtons();
void setButtonConfig(ButtonConfig *buttonConfig, ButtonConfig::EventHandler eventHandler);

void handleButtonOneEvent(AceButton *, uint8_t, uint8_t);
void handleButtonTwoEvent(AceButton *, uint8_t, uint8_t);
void handleButtonThreeEvent(AceButton *, uint8_t, uint8_t);

// Photocell
const int lightPin = A1;

// Led
const int greenLEDPin = 9;
const int redLEDPin = 6;
const int blueLEDPin = 10;

// Color
#include <NeoPixelBus.h>
const HsbColor led_colors[] = {
     HsbColor(0, 1, 1),
     HsbColor(0, 0.8, 1),
     HsbColor(0.125, 0.8, 1),
     HsbColor(0.125, 1, 1),
     HsbColor(0.25, 1, 1),
     HsbColor(0.25, 0.8, 1),
     HsbColor(0.375, 0.8, 1),
     HsbColor(0.375, 1, 1),
     HsbColor(0.5, 1, 1),
     HsbColor(0.5, 0.8, 1),
     HsbColor(0.5, 0, 1),
     HsbColor(0.625, 0.8, 1),
     HsbColor(0.625, 1, 1),
     HsbColor(0.75, 1, 1),
     HsbColor(0.75, 0.8, 1),
     HsbColor(0.875, 0.8, 1),
     HsbColor(0.875, 1, 1)
};
int color_index = 0;
const int nr_led_colors = 16;

// User Interface
enum menu_mode
{
    main_mode = 0,
    clock_mode = 1,
    brightness_mode = 2,
    seconds_mode = 3
};
menu_mode current_mode = menu_mode::main_mode;
bool it_is = true;

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
int auto_levels[16];
int auto_high = 15;
int auto_low = 3;
brightness current_brightness = brightness::automatic;

const long TIMEOUT = 10000; // in milliseconds
const long TIME_UPDATE_INTERVAL = 1; // in seconds
const long CLOCK_UPDATE_INTERVAL = 1; // in seconds

unsigned long last_press = 0;
unsigned long last_change = 0;
unsigned long lastClockUpdate = 0;
unsigned long lastShowTime = 0;


//-----------------------------------------------------
// Function Declarations
//-----------------------------------------------------

void update_display();
void show_hour();
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

// LCD utilities
void lcd_print_time(int col, int row, int hour);
void lcd_print_hour(int col, int row, int hour);

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

void setup()
{
    Serial.begin(9600);

   float step = pow(0.6, 1.0/ 3.0);

    for(int i=0; i<16; i++) {
        Serial.print(i);
        Serial.print(' ');
        Serial.print(step);
        Serial.print(' ');
        auto_levels[15-i] = (int)(255.0f * pow(step, i));
        Serial.println(auto_levels[15-i]);
    }


    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    Serial.println("Initialize RTC module");

    // Initialize DS3231
    clock.begin();

    setupButtons();

    // Manual (YYYY, MM, DD, HH, II, SS
    // clock.setDateTime(2016, 12, 9, 11, 46, 00);

    // Send sketch compiling time to Arduino
    // clock.setDateTime(__DATE__, __TIME__);

    pinMode(greenLEDPin, OUTPUT);
    pinMode(redLEDPin, OUTPUT);
    pinMode(blueLEDPin, OUTPUT);

    restore_settings();
    
    update_display();
}

void loop()
{
    // We need continuous updates for interactive modes
    switch(current_mode) {
    case clock_mode:
        lastClockUpdate = 0;
        lastShowTime = 0;
        break;
    }

    unsigned long current_time = millis();
    if((current_time - lastClockUpdate) > (CLOCK_UPDATE_INTERVAL * 1000) || last_press > lastClockUpdate) {
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
        if (last_press != 0 && current_time > (last_press + TIMEOUT) )
        {
            if (current_mode == brightness_mode) {
                current_brightness = automatic;
            }

            Serial.print("Timeout ");
            Serial.print(current_time);
            Serial.print(" ");
            Serial.println(last_press);
            lcd.clear();
            current_mode = main_mode;
            last_press = 0;

        }
        break;
    }

    if((millis() - lastShowTime) > (TIME_UPDATE_INTERVAL * 1000) || last_press > lastShowTime) {
        update_display();
        lastShowTime = millis();
    }


    buttonOne.check();
    buttonTwo.check();
    buttonThree.check();

    // delay(2);
}

/**
 * Setup Event Handlers of a Button
 * @param buttonConfig
 * @param eventHandler
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
 * Setup all Buttons.
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
 * Handle clicks of 1. button from left.
 * @param button
 * @param eventType
 * @param buttonState
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
            Serial.println("From main mode to seconds mode");
            lcd.clear();
            current_mode = seconds_mode;
            break;
        case seconds_mode:
            Serial.println("From seconds mode to main mode");
            lcd.clear();
            current_mode = main_mode;
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
            Serial.println("From main mode to seconds mode");
            lcd.clear();
            current_mode = seconds_mode;
            break;
        case seconds_mode:
            Serial.println("From seconds mode to main mode");
            lcd.clear();
            current_mode = main_mode;
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
            lcd.clear();
            current_mode = clock_mode;
            break;
        case seconds_mode:
            Serial.println("From seconds mode to main mode");
            lcd.clear();
            current_mode = main_mode;
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
 * Handle clicks of 2. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonTwoEvent(AceButton *button, uint8_t eventType,
                          uint8_t buttonState)
{
    last_press = millis();

    switch (eventType)
    {
    case AceButton::kEventClicked:
        Serial.println("Button Two Clicked");
        switch(current_mode) {
        case main_mode:
            toggle_brightness();
            break;
        case clock_mode:
            Serial.println("increase_5min");
            increase_5min();
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
        switch(current_mode) {
        case clock_mode:
            Serial.println("increase_5min");
            increase_5min();
            increase_5min();
            break;
        }
        break;
    case AceButton::kEventLongPressed:
        Serial.println("Button Two Long Press");
        switch(current_mode) {
        case clock_mode:
            Serial.println("decrease_5min");
            decrease_5min();
        break;
        case main_mode: 
            Serial.println("From main mode to brightness mode");
            lcd.clear();
            current_mode = brightness_mode;
            current_brightness = set_high;
        break;
        case brightness_mode:
            Serial.println("From brightness mode to main mode");
            lcd.clear();
            current_mode = main_mode;
            current_brightness = brightness::automatic;
            last_change = millis();
        break;
        }
    }
}

/**
 * Handle clicks of 3. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonThreeEvent(AceButton *button, uint8_t eventType,
                            uint8_t buttonState)
{
    last_press = millis();

    switch (eventType)
    {
    case AceButton::kEventClicked:
        Serial.println("Button Three Clicked");
        switch(current_mode) {
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
        }
        break;
    case AceButton::kEventDoubleClicked:
        Serial.println("Button Three Double clicked");
        switch(current_mode) {
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
        switch(current_mode) {
        case main_mode:
            last_change = millis();
            lcd.clear();
            it_is = !it_is;
            break;
        case clock_mode:
            Serial.println("decrease_1min");
            decrease_1min();
            break;
        }
        break;
    }
}

void update_display()
{
    // Serial.println(current_brightness);
    // First determine the brightness and color of the display
    HsbColor current_color = led_colors[color_index];
    
    switch(current_brightness)
    {
        case automatic:
        {
            int reading = analogRead(lightPin);
            current_color.B = min(1.0f, reading / 1024.0f);
        } 
        break;
        case set_high:
            current_color.B = auto_levels[auto_high] / 255.0f;
            break;
        case set_low:
            current_color.B = auto_levels[auto_low] / 255.0f;
            break;
        default:
            current_color.B = current_brightness / 255.0f;
            break;
    }

    RgbColor rgb_color(current_color);
    analogWrite(redLEDPin, rgb_color.R);
    analogWrite(greenLEDPin, rgb_color.G);
    analogWrite(blueLEDPin, rgb_color.B);
    
    switch (current_mode)
    {
    case main_mode:
        show_hour();
        break;
    case clock_mode:
        show_time();
        break;
    case seconds_mode:
        show_seconds();
        break;
    case brightness_mode:
        show_bar();
        break;
    }

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd_print_hour(0, 1, dt.hour);
    lcd.setCursor(2, 1);
    lcd.print(':');
    lcd_print_time(3, 1, dt.minute);
    lcd.setCursor(5, 1);
    lcd.print(':');
    lcd_print_time(6, 1, dt.second);
}

void show_hour()
{
    int column = 0;
    if (it_is)
    {
        column = 8;
        lcd.setCursor(0, 0);
        lcd.print("Het is ");
    }

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd_print_hour(column, 0, dt.hour);
    lcd.setCursor(column + 2, 0);
    lcd.print(':');
    lcd_print_time(column + 3, 0, dt.minute);
    lcd.setCursor(5, 1);
}

void show_time()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd_print_hour(0, 0, dt.hour);
    lcd.setCursor(2, 0);
    lcd.print(':');
    lcd_print_time(3, 0, dt.minute);
    lcd.setCursor(5, 0);
    lcd.print(':');
    lcd_print_time(6, 0, dt.second);
}

void show_seconds()
{
    lcd_print_time(0, 0, dt.second);
}

void show_bar()
{
    for(int i=0; i<auto_low; ++i) {
        lcd.setCursor(i, 0);
        lcd.print(' ');
    }
    for(int i=auto_low; i<=auto_high; ++i) {
        lcd.setCursor(i, 0);
        lcd.print(char(255));
    }
    for(int i=auto_high+1; i<16; ++i) {
        lcd.setCursor(i, 0);
        lcd.print(' ');
    }
}

void increase_hour()
{
    dt = clock.getDateTime();
    uint8_t hour = dt.hour + 1;
    if (hour > 23)
        hour = 0;
    clock.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
}

void decrease_hour()
{
    dt = clock.getDateTime();
    int hour = (int)dt.hour - 1;
    if (hour < 0)
        hour = 23;
    clock.setDateTime(dt.year, dt.month, dt.day, hour, dt.minute, dt.second);
}

void increase_5min()
{
    dt = clock.getDateTime();
    int min = dt.minute + 5;
    if (min > 59)
        min = 60 - min;
    clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, dt.second);
}

void decrease_5min()
{
    dt = clock.getDateTime();
    int min = (int)dt.minute - 5;
    if (min < 0)
        min = 60 + min;
    clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, dt.second);
}

void increase_1min()
{
    dt = clock.getDateTime();
    int min = dt.minute + 1;
    if (min > 59)
        min = 0;
    clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, 0);
}

void decrease_1min()
{
    dt = clock.getDateTime();
    int min = (int)dt.minute - 1;
    if (min < 0)
        min = 59;
    clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, min, 0);
}

void lcd_print_hour(int col, int row, int hour)
{
    lcd.setCursor(col, row);
    if (hour < 10)
    {
        lcd.print(' ');
        lcd.setCursor(col + 1, row);
    }
    lcd.print(hour);
}

void lcd_print_time(int col, int row, int hour)
{
    lcd.setCursor(col, row);
    if (hour < 10)
    {
        lcd.print('0');
        lcd.setCursor(col + 1, row);
    }
    lcd.print(hour);
}

void toggle_brightness() 
{
    switch(current_brightness) {
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
    last_change = millis();
    Serial.println("toggle_rgb");
    color_index = color_index + 1;
    if (color_index > nr_led_colors)
      color_index = 0;
}

void increase_brightness(brightness high_or_low)
{
    last_change = millis();
    if (high_or_low == set_high) {
        auto_high = min(15, auto_high + 1);
    }
    if (high_or_low == set_low) {
        auto_low = min(auto_high-1, auto_low+1);
    }
}

void decrease_brightness(brightness high_or_low)
{
    last_change = millis();
    if (high_or_low == set_high) {
        auto_high = max(auto_low+1, auto_high - 1);
    }
    if (high_or_low == set_low) {
        auto_low = max(0, auto_low-1);
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
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif;
    Wire.endTransmission();

    return value;
}


void store_settings() 
{
  Serial.println("store_settings");
  writeRegister8(0x08, (byte)color_index);
  writeRegister8(0x09, (byte)it_is);
  writeRegister8(0x0A, (byte)auto_high);
  writeRegister8(0x0B, (byte)auto_low);

  int bm = max(0, current_brightness);
  writeRegister8(0x0C, (byte)bm);
  
}

void restore_settings()
{
  Serial.println("restore_settings");
  color_index = min(nr_led_colors, readRegister8(0x08));
  it_is = readRegister8(0x09);
  auto_high = min(15, readRegister8(0x0A));
  auto_low = min(auto_high-1, readRegister8(0x0B));
  byte bmb = readRegister8(0x0C);
  if (bmb == 0) 
    current_brightness = automatic;
  else
    current_brightness = bmb;
}
