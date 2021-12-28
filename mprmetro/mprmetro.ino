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

  DBG_INFO("Initialization done");
}

ArduinoClock clock;

void loop()
{
  model.clock.loop();

  if (! model.clock.updating()) {
    controller.check_interactive_mode();
    controller.check_timeout();
    controller.update_display();
    controller.check_buttons();
  }
}
