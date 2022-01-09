#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "buttons.h"

// Forward declarations
class Model;

// User Interface
enum menu_mode
{
  main_mode = 0,
  clock_mode = 1,
  brightness_mode = 2,
  config_mode = 3,
  rgb_mode = 4,
  happiness_mode = 5,
  debug_mode = 6
};

enum submenu_mode {
  clock_hour,
  clock_5min,
  clock_min,
  set_high,
  set_low,
  set_hue,
  set_sat 
};

class Controller {
public:
  Controller();
  void begin();

  void increase_clock(Model &model);
  void decrease_clock(Model &model);  
  void reset_seconds(Model &model);

  void setup();
  void check_buttons();

  void update_display();
  void display();

  void check_interactive_mode();
  void check_timeout();

  bool timed_out(int factor=1);
  void reset_timeout();

  void change_brightness(int update);

  void adjust_color(int update);
  void toggle_rgb(int update=2);

  menu_mode mode();
  submenu_mode submode();
  unsigned long lastpress();
  void set_mode(menu_mode menumode);
  void set_mode(menu_mode menumode, submenu_mode submode);

private:
  void change_hour(Model &model, int h, int m5, int m, bool s);
  void changed();

private:
#ifdef ACE
  AceButtons buttons;
#else
  MPRButtons buttons;
#endif
  unsigned long last_show_time = 0;
  unsigned long last_press = 0;  

  menu_mode current_mode;
  submenu_mode sub_mode;
};

#endif // _CONTROLLER_H_
