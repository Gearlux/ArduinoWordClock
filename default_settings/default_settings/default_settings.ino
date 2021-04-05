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

void setup() {
    Serial.begin(9600);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    Serial.println("Initialize RTC module");

    // Initialize DS3231
    clock.begin();

    // Send sketch compiling time to Arduino
    clock.setDateTime(__DATE__, __TIME__);

    writeRegister8(0x08, (byte)0);
    writeRegister8(0x09, (byte)true);
    writeRegister8(0x0A, (byte)3);
    writeRegister8(0x0B, (byte)15);

    writeRegister8(0x0C, (byte)0);

    lcd.setCursor(0,0);
    lcd.print("Time set");
    lcd.setCursor(0, 1);
    lcd.print("Defaults set");
}

void loop() {
  // put your main code here, to run repeatedly:

}
