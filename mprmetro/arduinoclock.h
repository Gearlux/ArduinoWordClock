#ifndef _ARDUINO_CLOCK_H_
#define _ARDUINO_CLOCK_H_

#include <DS3231.h>

struct TimeMS
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

class ArduinoClock: public DS3231 
{
public:
  ArduinoClock();

  void begin();
  void sleep(unsigned long ms);

  TimeMS get_date_time();
  void set_date_time(uint8_t hour, uint8_t minute, uint8_t second);
};

#endif // _ARDUINO_CLOCK_H_
