#define DBG_ENABLE_VERBOSE
#define DBG_ENABLE_DEBUG
#define DBG_ENABLE_INFO
#define DBG_ENABLE_WARNING
#define DBG_ENABLE_ERROR
// #define DBG_ENABLE_TRACE

#include "fdebug.hpp"

#define BUF_SIZE 80
extern ArduinoDebug<BUF_SIZE> Debug;

#include "model.h"
#include "view.h"
#include "controller.h"

// FIXME
// #define TIMEOUT 60000 // in milliseconds
#define TIMEOUT 60000
#define BLINK_INTERVAL 500 // in milli seconds

extern Model model;
extern View view;
extern Controller controller;
