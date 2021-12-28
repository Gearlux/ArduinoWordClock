#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#define BUTTON_ONE_PIN 2
#define BUTTON_TWO_PIN 0
#define BUTTON_THREE_PIN 4
#define BUTTON_FOUR_PIN 3

class Buttons {
public:
  virtual void setup() = 0;
  virtual void check() = 0;
};

#ifdef ACE
#include <AceButton.h>
using namespace ace_button;

class AceButtons: public Buttons 
{
public:
  AceButtons();
  void setup();
  void check();

private:
  AceButton buttonOne;
  AceButton buttonTwo;
  AceButton buttonThree;
  AceButton buttonFour;  
};

#else
#include <Button.h>
#include <ButtonEventCallback.h>
#include <MPR121Button.h>
#include <Adafruit_MPR121.h>

class MPRButtons : public Buttons
{
public:
  MPRButtons();
  void setup();
  void check();

private:
  Adafruit_MPR121 touchSensor;

  MPR121Button buttonOne;
  MPR121Button buttonTwo;
  MPR121Button buttonThree;
  MPR121Button buttonFour;
};

#endif

#endif // _BUTTONS_H_
