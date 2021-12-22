#ifndef _ARDUINO_CLOCK_H_
#define _ARDUINO_CLOCK_H_

#include <DS3231.h>

#define MAX_DIFF 16384 // On my Arduino board, this allows a difference of max 25ms.

struct TimeMS
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t ms;
};

class ArduinoClock: private DS3231 
{
public:
  ArduinoClock();

  void begin();
  void loop();
  TimeMS get_date_time();
  void set_date_time(uint8_t hour, uint8_t minute, uint8_t second);

  bool updating();

private:
  TimeMS get_time(unsigned long t);
  
private:
  uint8_t reference_second;
  RTCDateTime reference;
  uint32_t reference_unixtime;
  unsigned long ref_cpu_time;
};

#endif // _ARDUINO_CLOCK_H_
