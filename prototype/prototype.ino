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

// Temperature
int tempPin = 0; // Analog 0

// Buttons
#include <AceButton.h>

using namespace ace_button;

#define BUTTON_ONE_PIN 2
#define BUTTON_TWO_PIN 3
#define BUTTON_THREE_PIN 4
#define BUTTON_FOUR_PIN 5

AceButton buttonOne(new ButtonConfig());
AceButton buttonTwo(new ButtonConfig());
AceButton buttonThree(new ButtonConfig());
AceButton buttonFour(new ButtonConfig());

// Buttons
void setupButtons();
void setButtonConfig(ButtonConfig* buttonConfig, ButtonConfig::EventHandler eventHandler);

void handleButtonOneEvent(AceButton*, uint8_t, uint8_t);
void handleButtonTwoEvent(AceButton*, uint8_t, uint8_t);
void handleButtonThreeEvent(AceButton*, uint8_t, uint8_t);
void handleButtonFourEvent(AceButton*, uint8_t, uint8_t);

// Photocell
const int lightPin = A1;

// Led
const int greenLEDPin = 9; 
const int redLEDPin = 6; 
const int blueLEDPin = 10;

// Color
int red = 0;
int green = 0;
int blue = 255;

//-----------------------------------------------------
// Function Declarations
//-----------------------------------------------------

// LCD utilities
void lcd_print_time(int col, int row, int hour);
void lcd_print_hour(int col, int row, int hour);

void setup()
{
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Hello, World!");   
  
  Serial.println("Initialize RTC module");
  
  // Initialize DS3231
  clock.begin();

  setupButtons();
  
  // Manual (YYYY, MM, DD, HH, II, SS
  // clock.setDateTime(2016, 12, 9, 11, 46, 00);
  
  // Send sketch compiling time to Arduino
  // clock.setDateTime(__DATE__, __TIME__);   

  pinMode(greenLEDPin,OUTPUT);
  pinMode(redLEDPin,OUTPUT);
  pinMode(blueLEDPin,OUTPUT);

  analogWrite(redLEDPin, red);
  analogWrite(greenLEDPin, green);
  analogWrite(blueLEDPin, blue);

}

void loop()
{
  int tempReading = analogRead(tempPin);
  // This is OK
  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit

  dt = clock.getDateTime();
  // For leading zero look to DS3231_dateformat example

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd_print_hour(0,1,dt.hour);
  lcd.setCursor(2,1);
  lcd.print(':');
  lcd_print_time(3,1,dt.minute);
  lcd.setCursor(5,1);
  lcd.print(':');
  lcd_print_time(6,1, dt.second);

  lcd.setCursor(10, 1);
  lcd.print((int)tempC);
  lcd.setCursor(12, 1);
  lcd.print('C');
  // To erase the space
  lcd.print(' ');

  int reading  = analogRead(lightPin);
  int scale = 1024 / reading;

  analogWrite(redLEDPin, red / scale);
  analogWrite(greenLEDPin, green / scale);
  analogWrite(blueLEDPin, blue / scale);


  buttonOne.check();
  buttonTwo.check();
  buttonThree.check();
  buttonFour.check();

  delay(2);
}

/**
 * Setup Event Handlers of a Button
 * @param buttonConfig
 * @param eventHandler
 */
void setButtonConfig(ButtonConfig* buttonConfig, ButtonConfig::EventHandler eventHandler) {
    buttonConfig->setEventHandler(eventHandler);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
}

/**
 * Setup all four Buttons.
 */
void setupButtons() {
    pinMode(BUTTON_ONE_PIN, INPUT);
    buttonOne.init(BUTTON_ONE_PIN, LOW);

    pinMode(BUTTON_TWO_PIN, INPUT);
    buttonTwo.init(BUTTON_TWO_PIN, LOW);

    pinMode(BUTTON_THREE_PIN, INPUT);
    buttonThree.init(BUTTON_THREE_PIN, LOW);

    pinMode(BUTTON_FOUR_PIN, INPUT);
    buttonFour.init(BUTTON_FOUR_PIN, LOW);

    setButtonConfig(buttonOne.getButtonConfig(), handleButtonOneEvent);
    setButtonConfig(buttonTwo.getButtonConfig(), handleButtonTwoEvent);
    setButtonConfig(buttonThree.getButtonConfig(), handleButtonThreeEvent);
    setButtonConfig(buttonFour.getButtonConfig(), handleButtonFourEvent);
}

/**
 * Handle clicks of 1. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonOneEvent(AceButton* button, uint8_t eventType,
                           uint8_t buttonState) {
    switch (eventType) {
        case AceButton::kEventClicked:
            Serial.println("Button One Clicked");
            break;
        case AceButton::kEventDoubleClicked:
            Serial.println("Button One Double clicked");
            break;
        case AceButton::kEventLongPressed:
            Serial.println("Button One Long Press");
            break;
    }
}

/**
 * Handle clicks of 2. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonTwoEvent(AceButton* button, uint8_t eventType,
                           uint8_t buttonState) {
    switch (eventType) {
        case AceButton::kEventClicked:
            Serial.println("Button Two Clicked");
            break;
        case AceButton::kEventDoubleClicked:
            Serial.println("Button Two Double clicked");
            break;
        case AceButton::kEventLongPressed:
            Serial.println("Button Two Long Press");
            break;
    }
}

/**
 * Handle clicks of 3. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonThreeEvent(AceButton* button, uint8_t eventType,
                           uint8_t buttonState) {
    switch (eventType) {
        case AceButton::kEventClicked:
            Serial.println("Button Three Clicked");
            break;
        case AceButton::kEventDoubleClicked:
            Serial.println("Button Three Double clicked");
            break;
        case AceButton::kEventLongPressed:
            Serial.println("Button Three Long Press");
            break;
    }
}

/**
 * Handle clicks of 4. button from left.
 * @param button
 * @param eventType
 * @param buttonState
 */
void handleButtonFourEvent(AceButton* button, uint8_t eventType,
                           uint8_t buttonState) {
    switch (eventType) {
        case AceButton::kEventClicked:
            Serial.println("Button Four Clicked");
            break;
        case AceButton::kEventDoubleClicked:
            Serial.println("Button Four Double clicked");
            break;
        case AceButton::kEventLongPressed:
            Serial.println("Button Four Long Press");
            break;
    }
}

void lcd_print_hour(int col, int row, int hour) {
  lcd.setCursor(col, row);
  if (hour < 10) {
    lcd.print(' ');
    lcd.setCursor(col+1, row);
  }
  lcd.print(hour);
}

void lcd_print_time(int col, int row, int hour) {
  lcd.setCursor(col, row);
  if (hour < 10) {
    lcd.print('0');
    lcd.setCursor(col+1, row);
  }
  lcd.print(hour);
}
