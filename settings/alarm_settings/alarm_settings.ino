#define ALARM 0
#define WRITE 0

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

    Serial.print(reg); 
    Serial.print(' ');
    Serial.println(value);
    return value;
}

void setup() {
    Serial.begin(9600);

    Serial.println("Initialize RTC module");

    // Initialize DS3231
    clock.begin();

    // Send sketch compiling time to Arduino
    clock.setDateTime(__DATE__, __TIME__);

#if WRITE
    Serial.println("Writing Defaults");
    writeRegister8(0x08, (byte)0);
    writeRegister8(0x09, (byte)true);
    writeRegister8(0x0A, (byte)3);
    writeRegister8(0x0B, (byte)15);

    writeRegister8(0x0C, (byte)0);
#endif   

    Serial.println("Checking Defaults");
    if (readRegister8(0x08) != (byte)0)
      Serial.println("Error 0x08");
    if (readRegister8(0x09) != (byte)true)
      Serial.println("Error 0x09");
    if (readRegister8(0x0A) != (byte)3)
      Serial.println("Error 0x0A");
    if (readRegister8(0x0B) != (byte)15)
      Serial.println("Error 0x0B");
    if (readRegister8(0x0C) != (byte)0)
      Serial.println("Error 0x0C");

    Serial.println("Done");
}

void loop() {
  // put your main code here, to run repeatedly:

}
