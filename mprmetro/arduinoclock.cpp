#include "mprmetro.h"
#include "arduinoclock.h"

#define NO_CPU

ArduinoClock::ArduinoClock() : ref_cpu_time(0), reference_second(255), extra_delay(0) {
  // Set reference year to zero to initialize it
  reference.year = 0;
}

void ArduinoClock::begin() {
  DS3231::begin();
}

bool ArduinoClock::updating() {
  return reference_second != 255;
}

void ArduinoClock::loop() {
  #ifndef NO_CPU
    if (reference.year == 0) {
      DBG_INFO_F("Reset arduino clock");
      reference = getDateTime();
      reference_second = reference.second;
    }
    if (reference_second == reference.second) {
      reference = getDateTime();
      if (reference_second != reference.second) {
        ref_cpu_time = millis();
        extra_delay = 0;
        reference_unixtime = reference.unixtime;
        DBG_INFO_F("Arduino clock is set %lu %d:%d.%d", ref_cpu_time, reference.hour, reference.minute, reference.second);
        reference_second = 255;
      }
    }
  #endif
}

TimeMS ArduinoClock::get_date_time()
{
#ifdef NO_CPU
  RTCDateTime current_time =  getDateTime();
  TimeMS result;
  result.hour = current_time.hour;
  result.minute = current_time.minute;
  result.second = current_time.second;
  result.ms = 0;
  return result;
#else
  if (ref_cpu_time == 0) {
    TimeMS result;
    result.hour = reference.hour;
    result.minute = reference.minute;
    result.second = reference.second;
    result.ms = 0;
    return result;
  }
  else {
    unsigned long current_millis = millis();
    // This occurs when the arduino clock overfloats
    if (current_millis < ref_cpu_time) {
      TimeMS result;
      RTCDateTime current_time =  getDateTime();

      result.hour = current_time.hour;
      result.minute = current_time.minute;
      result.second = current_time.second;
      result.ms = 0;

      ref_cpu_time = 0;
      reference.year = 0;

      return result;
    }
    // Here we can calculate the difference
    unsigned long diff = current_millis + extra_delay - ref_cpu_time;
    TimeMS result = get_time(reference_unixtime + (diff / 1000L));
    result.ms = diff % 1000L;
    if (diff > MAX_DIFF && reference_second == 255) {
      DBG_INFO_F("Reset internal clock %lu %lu %d:%d.%d", diff, current_millis, result.hour, result.minute, result.second);
      // Reset the internal clock
      reference.year = 0;
    }
    return result;
  }
#endif  
}

void ArduinoClock::set_date_time(uint8_t hour, uint8_t minute, uint8_t second)
{
  RTCDateTime dt = getDateTime();
  setDateTime(dt.year, dt.month, dt.day, hour, minute, second);
  reference = dt;
  reference.hour = hour;
  reference.minute = minute;
  reference.second = second;
  reference.year = 0;
}

TimeMS ArduinoClock::get_time(unsigned long t)
{
  TimeMS result;

  t -= 946681200;

  result.second = t % 60;
  t /= 60;
  result.minute = t % 60;
  t /= 60;
  result.hour = t % 24;

  return result;
}

void ArduinoClock::sleep(unsigned long ms)
{
  delay(ms);
  extra_delay += 2;
}
