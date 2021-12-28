/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2020 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-Debug/graphs/contributors.
 */

#ifndef FDEBUG_HPP_
#define FDEBUG_HPP_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>

#include <stdarg.h>
#include <stdlib.h>

/**************************************************************************************
 * DEFINE
 **************************************************************************************/

#ifdef DBG_ENABLE_ERROR
#  define DBG_ERROR(fmt, ...) Debug.print(DebugLevel::Error, fmt, ## __VA_ARGS__)
#else
#  define DBG_ERROR(fmt, ...)
#endif

#ifdef DBG_ENABLE_WARNING
#  define DBG_WARNING(fmt, ...) Debug.print(DebugLevel::Warning, fmt, ## __VA_ARGS__)
#else
#  define DBG_WARNING(fmt, ...)
#endif

#ifdef DBG_ENABLE_INFO
#  define DBG_INFO(fmt, ...) Debug.print(DebugLevel::Info, fmt, ## __VA_ARGS__)
#  define DBG_INFO_F(fmt, ...) Debug.print_f(DebugLevel::Info, F(fmt), ## __VA_ARGS__)
#else
#  define DBG_INFO(fmt, ...)
#  define DBG_INFO_F(fmt, ...)
#endif

#ifdef DBG_ENABLE_DEBUG
#  define DBG_DEBUG(fmt, ...) Debug.print(DebugLevel::Debug, fmt, ## __VA_ARGS__)
#  define DBG_DEBUG_F(fmt, ...) Debug.print_f(DebugLevel::Debug, F(fmt), ## __VA_ARGS__)
#else
#  define DBG_DEBUG(fmt, ...)
#  define DBG_DEBUG_F(fmt, ...)
#endif

#ifdef DBG_ENABLE_VERBOSE
#  define DBG_VERBOSE(fmt, ...) Debug.print(DebugLevel::Verbose, fmt, ## __VA_ARGS__)
#  define DBG_VERBOSE_F(fmt, ...) Debug.print_f(DebugLevel::Verbose, F(fmt), ## __VA_ARGS__)
#else
#  define DBG_VERBOSE(fmt, ...)
#  define DBG_VERBOSE_F(fmt, ...)
#endif

#ifdef DBG_ENABLE_TRACE
#  define DBG_TRACE(fmt, ...) Debug.print(DebugLevel::Trace, fmt, ## __VA_ARGS__)
#  define DBG_TRACE_F(fmt, ...) Debug.print_f(DebugLevel::Trace, F(fmt), ## __VA_ARGS__)
#else
#  define DBG_TRACE(fmt, ...)
#  define DBG_TRACE_F(fmt, ...)
#endif


/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class DebugLevel
{
  Error, Warning, Info, Debug, Verbose, Trace
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

template<size_t N>
class ArduinoDebug
{

public:

  ArduinoDebug(Stream & stream) : _stream{stream} { }

  void print(DebugLevel const lvl, char * fmt, ...)
  {
    print(lvl);

    va_list args;
    va_start(args, fmt);
    print(fmt, args);
    va_end(args);

    print("\r\n");
  }

  void print_f(DebugLevel const lvl, const __FlashStringHelper * fmt, ...)
  {
    String fmt_str(fmt);

    print(lvl);
  
    va_list args;
    va_start(args, fmt);
    print(fmt_str.c_str(), args);
    va_end(args);
  
    print("\r\n");
  }

  static const char *byte_to_binary(byte x)
  {
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
  }
  
private:

  Stream & _stream;

  void print(char const * fmt, va_list args)
  {
    char buf[N] = {0};
    vsnprintf(buf, N, fmt, args);
    print(buf);
  }

  void print(DebugLevel const lvl)
  {
    switch(lvl)
    {
    case DebugLevel::Error:   print("[E] "); break;
    case DebugLevel::Warning: print("[W] "); break;
    case DebugLevel::Info:    print("[I] "); break;
    case DebugLevel::Debug:   print("[D] "); break;
    case DebugLevel::Verbose: print("[V] "); break;
    case DebugLevel::Trace:   print("[T] "); break;
    }
  }

  void print(char const * msg)
  {
    for (size_t i = 0; i < min(N, strlen(msg)); i++)
      _stream.write(static_cast<uint8_t>(msg[i]));
  }
};

/**************************************************************************************
 * MACRO MAGIC
 **************************************************************************************/

#define DEBUG_INSTANCE(BUF_SIZE,OUT_STREAM) \
  static ArduinoDebug<BUF_SIZE> Debug(OUT_STREAM)

#endif /* FDEBUG_HPP_ */
