#include "mprmetro.h"
#include "arduinoclock.h"

#define NO_CPU

ArduinoClock::ArduinoClock() {
}

void ArduinoClock::begin() {
  DS3231::begin();
}

TimeMS ArduinoClock::get_date_time()
{
  RTCDateTime current_time =  getDateTime();
  TimeMS result;
  result.hour = current_time.hour;
  result.minute = current_time.minute;
  result.second = current_time.second;
  return result;
}

void ArduinoClock::set_date_time(uint8_t hour, uint8_t minute, uint8_t second)
{
  RTCDateTime dt = getDateTime();
  setDateTime(dt.year, dt.month, dt.day, hour, minute, second);
}

void ArduinoClock::sleep(unsigned long ms)
{
  delay(ms);
}
