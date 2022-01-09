// Clock
#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

#define ITIS_CONFIG 0x01
#define TWENTY_CONFIG 0x02
#define OVER_CONFIG 0x04
#define SECONDS_CONFIG 0x08
#define DOT_CONFIG 0x10
#define BLACKLIGHT_CONFIG 0x20
#define SETCLOCK_CONFIG 0x40
#define FADE_CONFIG 0x80

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

    Serial.println("Initialize RTC module");

    // Initialize DS3231
    clock.begin();

    // Send sketch compiling time to Arduino
    clock.setDateTime(__DATE__, __TIME__);

    writeRegister8(0x08, (byte)0);
    writeRegister8(0x09, (byte)( ITIS_CONFIG | TWENTY_CONFIG | DOT_CONFIG | BLACKLIGHT_CONFIG | FADE_CONFIG));
    writeRegister8(0x0A, (byte)11);
    writeRegister8(0x0B, (byte)0);

    writeRegister8(0x0C, (byte)0);

    Serial.println("All settings stored");
}

void loop() {
  // put your main code here, to run repeatedly:

}
