#include "controller.h"
#include "mprmetro.h"
#include "model.h"

Controller::Controller() :
  buttons(),
  current_mode(menu_mode::main_mode)
{
}

void Controller::begin()
{
  Serial.begin(9600);
}

void Controller::increase_clock(Model &model)
{
  switch(sub_mode) {
  case clock_hour:
    change_hour(model, 1, 0, 0, false);
    break;
  case clock_5min:
    change_hour(model, 0, 1, 0, false);
    break;
  case clock_min:
    change_hour(model, 0, 0, 1, false);
    break;    
  }
}

void Controller::decrease_clock(Model &model)
{
  switch(sub_mode) {
  case clock_hour:
    change_hour(model, -1, 0, 0, false);
    break;
  case clock_5min:
    change_hour(model, 0, -1, 0, false);
    break;
  case clock_min:
    change_hour(model, 0, 0, -1, false);
    break;    
  }
}

void Controller::reset_seconds(Model &model)
{
  change_hour(model, 0, 0, 0, true);
}

void Controller::change_hour(Model &model, int h, int min5, int m, bool reset)
{
  TimeMS dt = model.getDateTime();
  DBG_VERBOSE_F("change_hour from %d:%d.%d", dt.hour, dt.minute, dt.second);
  int hour = (int)dt.hour;
  int min = dt.minute + min5 * 5 + m;
  if (min > 59) {
    min = 60 - min;  
    hour += 1;
  }
  if (min < 0) {
    min = 60 + min;
    hour -= 1;
  }
  hour += h;
  if (hour > 23)
    hour = 0;
  if (hour < 0)
    hour = 23;    
  int sec = reset ? 0 : dt.second;
  model.setDateTime(hour, min, sec);
  DBG_VERBOSE_F("change_hour  to  %d:%d.%d", hour, min, sec);
  changed();
}

void Controller::setup()
{
  buttons.setup();
}

void Controller::check_buttons()
{
  buttons.check();

  model.clock.sleep(10);
}

void Controller::update_display()
{
  controller.display();
}

void Controller::display()
{
  last_show_time = millis();

  HsbColor current_color = model.compute_current_color();

  TimeMS dt = model.getDateTime();
  switch (current_mode)
  {
    case main_mode:
      view.display_hour(current_color, dt.hour, dt.minute, dt.second, model.get_config());
      break;
    case config_mode:
      if (model.get_config() & TWENTY_CONFIG) {
        view.display_hour(current_color, 12, 24, dt.second, model.get_config() | SECONDS_CONFIG);        
      }
      else {
        view.display_hour(current_color, 12, 44, dt.second, model.get_config() | SECONDS_CONFIG);        
      }
      break;
    case clock_mode:
      view.display_hour(current_color, dt.hour, dt.minute, dt.second, model.get_config() | SETCLOCK_CONFIG | SECONDS_CONFIG);
      break;
    case rgb_mode:
      view.show_hue(current_color, sub_mode);
      break;
    case brightness_mode:
      view.show_bar(current_color, model.get_brightness_low(), model.get_brightness_high(), sub_mode == set_low);
      break;
    case happiness_mode:
      view.show_happiness(current_color);
      break;
    case debug_mode:
      view.show_debug(dt.second);
      break;
  }
}  

void Controller::check_interactive_mode()
{
  // We need continuous updates for interactive modes
  switch (current_mode) {
    case clock_mode:
    case brightness_mode:
    case rgb_mode:
    case config_mode:
      DBG_TRACE("Interactive mode");
      last_show_time = 0;
      break;
  }
}

void Controller::check_timeout()
{
  bool store_settings = false;
  switch (current_mode)
  {
    case clock_mode:
      if (timed_out(2) )
      {
        DBG_VERBOSE("Timed out: clock");
        set_mode(main_mode);
        store_settings = true;
      }
      break;
    case debug_mode:
      if (timed_out(4) )
      {
        DBG_VERBOSE("Timed out: debug");
        set_mode(main_mode);
        store_settings = true;
      }
      break;
    case brightness_mode:
    case happiness_mode:
    case rgb_mode:
    case config_mode:
      if (timed_out() )
      {
        DBG_VERBOSE("Timed out: default");
        set_mode(main_mode);
        store_settings = true;
      }
      break;
  }

  if (store_settings) {
    reset_timeout();
    if (model.timed_out()) 
    {
      DBG_INFO("Time out. Store settings");
      model.store_settings();
      model.reset_timeout();
    }
  }
}

bool Controller::timed_out(int factor)
{
  return last_press != 0 && millis() > (last_press + factor * TIMEOUT);
}

void Controller::reset_timeout()
{
  last_press = 0;
}

void Controller::changed()
{
  last_press = millis();
}

void Controller::change_brightness(int adjust)
{
  if (sub_mode == set_high) {
    model.set_brightness_high(model.get_brightness_high() + adjust);
  }
  if (sub_mode == set_low) {
    model.set_brightness_low(model.get_brightness_low() + adjust);
  }
}

void Controller::adjust_color(int adjust)
{
  model.adjust_color(-1, sub_mode == set_sat);
}

void Controller::toggle_rgb(int update)
{
  model.adjust_color(update, false);
}

menu_mode Controller::mode()
{
  return current_mode;
}

submenu_mode Controller::submode()
{
  return sub_mode;
}

void Controller::set_mode(menu_mode menumode)
{
  DBG_DEBUG("menu mode: %d", menumode);
  current_mode = menumode;
  changed();
}

void Controller::set_mode(menu_mode menumode, submenu_mode submode)
{
  DBG_DEBUG("menu mode: %d submenu: %d", menumode, submode);
  current_mode = menumode;
  sub_mode = submode;
  changed();
}
