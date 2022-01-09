#include "mprmetro.h"
#include <limits.h>

Model model;
View view;
Controller controller;

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

DEBUG_INSTANCE(BUF_SIZE, Serial);

void setup()
{
  controller.begin();
  view.begin();
  model.begin();

  controller.setup();
  model.setup();
  view.setup();

  // For testing
  // model.set_config(DOT_CONFIG);
  // model.setDateTime(3, 4, 58);
  // model.setDateTime(3, 9, 58);
  // model.setDateTime(3, 14, 58);
  // model.setDateTime(3, 19, 58);
  // model.setDateTime(3, 24, 58);
  // model.setDateTime(3, 29, 58);
  // model.setDateTime(3, 34, 58);
  // model.setDateTime(3, 39, 58);
  // model.setDateTime(3, 44, 58);
  // model.setDateTime(3, 49, 58);
  // model.setDateTime(3, 54, 58);
  // model.setDateTime(3, 59, 58);

  TimeMS current_time = model.getDateTime();
  DBG_DEBUG_F("Hour %d:%d.%d", current_time.hour, current_time.minute, current_time.second);
  DBG_INFO("Initialization done");
}

ArduinoClock clock;

void loop()
{
  controller.check_interactive_mode();
  controller.check_timeout();
  controller.update_display();
  controller.check_buttons();
}
