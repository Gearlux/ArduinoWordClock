#include "mprmetro.h"
#include "buttons.h"
#include "controller.h"

void buttonOneClicked()
{
  DBG_DEBUG_F("Button One Clicked %d", controller.mode());
  switch (controller.mode())
  {
    case main_mode:
      model.toggle_seconds();
      break;
    case config_mode:
      model.toggle_itis();
      break;
    case clock_mode:
      controller.decrease_clock(model);
      break;
    case brightness_mode:
      controller.change_brightness(-1);
      break;
    case rgb_mode:
      controller.adjust_color(-1);
      break;
  }
}

void buttonOneLongPressed()
{
  DBG_DEBUG_F("Button One Long Press %d", controller.mode());
  switch (controller.mode())
  {
    case main_mode:
      controller.set_mode(clock_mode, clock_hour);
      break;
    case config_mode:
      model.toggle_dots();
      break;
  }
}

void buttonTwoClicked()
{
  DBG_DEBUG_F("Button Two Clicked %d", controller.mode());
  switch (controller.mode()) {
    case main_mode:
      model.toggle_brightness();
      break;
    case clock_mode:
      switch (controller.submode()) {
        case clock_hour:
          controller.set_mode(clock_mode, clock_5min);
          break;
        case clock_5min:
          controller.set_mode(clock_mode, clock_min);
          break;
        default:
          controller.set_mode(clock_mode, clock_hour);
          break;
      }
      break;
    case config_mode:
      model.toggle_over();
      break;
    case brightness_mode:
      if (controller.submode() == set_low)
      {
          controller.set_mode(brightness_mode, set_high);
      }
      else
      {
          controller.set_mode(brightness_mode, set_low);
      }
      break;
    case rgb_mode:
      if (controller.submode() == set_hue)
          controller.set_mode(rgb_mode, set_sat);
      else
          controller.set_mode(rgb_mode, set_hue);
      break;
  }
}

void buttonTwoLongPressed()
{
  DBG_DEBUG_F("Button Two Long Press %d", controller.mode());
  switch (controller.mode()) {
    case clock_mode:
      controller.reset_seconds(model);
      break;
    case main_mode:
      controller.set_mode(brightness_mode, set_high);
      break;
    case config_mode:
      model.toggle_twenty();
      break;
  }
}

void buttonThreeClicked()
{
  DBG_DEBUG_F("Button Three Clicked %d", controller.mode());
  switch (controller.mode()) {
    case main_mode:
      controller.toggle_rgb();
      break;
    case clock_mode:
      controller.increase_clock(model);
      break;
    case brightness_mode:
      controller.change_brightness(1);
      break;
    case config_mode:
      model.toggle_backlight();
      break;
    case rgb_mode:
      controller.adjust_color(1);
      break;
  }

}

void buttonThreeLongPressed()
{
  DBG_DEBUG_F("Button Three Long Press %d", controller.mode());
  switch (controller.mode()) {
    case main_mode:
      controller.set_mode(rgb_mode, set_hue);
      break;
    case clock_mode:
      // Do nothing
      break;
    case config_mode:
      controller.toggle_fader();
      break;
  }

}


void buttonFourClicked()
{
  DBG_DEBUG_F("Button Four Clicked %d", controller.mode());
  switch (controller.mode()) {
    case main_mode:
      controller.set_mode(happiness_mode);
      break;
    case happiness_mode:
      controller.set_mode(debug_mode);
      break;
    case debug_mode:
      controller.set_mode(happiness_mode);
      break;
  }
}

void buttonFourLongPressed()
{
  DBG_DEBUG_F("Button Four Long Press %d", controller.mode());
  switch (controller.mode()) {
    case main_mode:
      controller.set_mode(config_mode);
      break;
    case happiness_mode:
      controller.set_mode(config_mode);
      break;
    default:
      controller.set_mode(main_mode);
      break;
  }
}

#ifdef ACE
/**
   Setup Event Handlers of a Button
   @param buttonConfig
   @param eventHandler
*/
void setButtonConfig(ButtonConfig *buttonConfig, ButtonConfig::EventHandler eventHandler)
{
  buttonConfig->setEventHandler(eventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  // buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setClickDelay(500);
}

void handleButtonOneEvent(AceButton *, uint8_t, uint8_t);
void handleButtonTwoEvent(AceButton *, uint8_t, uint8_t);
void handleButtonThreeEvent(AceButton *, uint8_t, uint8_t);
void handleButtonFourEvent(AceButton *, uint8_t, uint8_t);

AceButtons::AceButtons() :
  buttonOne(new ButtonConfig(),BUTTON_ONE_PIN, 1),
  buttonTwo(new ButtonConfig(),BUTTON_TWO_PIN, 1),
  buttonThree(new ButtonConfig(),BUTTON_THREE_PIN, 1),
  buttonFour(new ButtonConfig(),BUTTON_FOUR_PIN, 1)
{
  
}

void AceButtons::setup()
{
  pinMode(BUTTON_ONE_PIN, INPUT);
  pinMode(BUTTON_TWO_PIN, INPUT);
  pinMode(BUTTON_THREE_PIN, INPUT);
  pinMode(BUTTON_FOUR_PIN, INPUT);

  setButtonConfig(buttonOne.getButtonConfig(), handleButtonOneEvent);
  setButtonConfig(buttonTwo.getButtonConfig(), handleButtonTwoEvent);
  setButtonConfig(buttonThree.getButtonConfig(), handleButtonThreeEvent);
  setButtonConfig(buttonFour.getButtonConfig(), handleButtonFourEvent);

  buttonOne.init((uint8_t)BUTTON_ONE_PIN, HIGH);
  buttonTwo.init((uint8_t)BUTTON_TWO_PIN, HIGH);
  buttonThree.init((uint8_t)BUTTON_THREE_PIN, HIGH);
  buttonFour.init((uint8_t)BUTTON_FOUR_PIN, HIGH);
}

void AceButtons::check()
{
  buttonOne.check();
  buttonTwo.check();
  buttonThree.check();
  buttonFour.check();
}

/**
   Handle clicks of 1. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonOneEvent(AceButton *button, uint8_t eventType,
                          uint8_t buttonState)
{
  DBG_VERBOSE_F("Button One %d %d", eventType, buttonState);
  switch (eventType)
  {
    case AceButton::kEventClicked:
      buttonOneClicked();
      break;
//    case AceButton::kEventDoubleClicked:
//      buttonOneDoubleClicked();
//      break;
    case AceButton::kEventLongPressed:
      buttonOneLongPressed();
      break;
  }
}

/**
   Handle clicks of 2. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonTwoEvent(AceButton *button, uint8_t eventType,
                          uint8_t buttonState)
{
  DBG_VERBOSE_F("Button Two %d %d", eventType, buttonState);
  switch (eventType)
  {
    case AceButton::kEventClicked:
      buttonTwoClicked();
      break;
//    case AceButton::kEventDoubleClicked:
//      buttonTwoDoubleClicked();
//      break;
    case AceButton::kEventLongPressed:
      buttonTwoLongPressed();
      break;
  }
}

/**
   Handle clicks of 3. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonThreeEvent(AceButton *button, uint8_t eventType,
                            uint8_t buttonState)
{
  DBG_VERBOSE_F("Button Three %d %d", eventType, buttonState);
  switch (eventType)
  {
    case AceButton::kEventClicked:
      buttonThreeClicked();
      break;
//    case AceButton::kEventDoubleClicked:
//      buttonThreeDoubleClicked();
//      break;
    case AceButton::kEventLongPressed:
      buttonThreeLongPressed();
      break;
  }
}
/**
   Handle clicks of 3. button from left.
   @param button
   @param eventType
   @param buttonState
*/
void handleButtonFourEvent(AceButton *button, uint8_t eventType,
                           uint8_t buttonState)
{
  DBG_VERBOSE_F("Button Four %d %d", eventType, buttonState);
  switch (eventType)
  {
    case AceButton::kEventClicked:
      buttonFourClicked();
      break;
//    case AceButton::kEventDoubleClicked:
//      buttonFourDoubleClicked();
//      break;
    case AceButton::kEventLongPressed:
      buttonFourLongPressed();
      break;
  }
}
#else
void onButtonOnePressed(Button &btn)
{
  buttonOneClicked();
}
void onButtonTwoPressed(Button &btn)
{
  buttonTwoClicked();
}
void onButtonThreePressed(Button &btn)
{
  buttonThreeClicked();
}
void onButtonFourPressed(Button &btn)
{
  buttonFourClicked();
}

void onButtonOneHold(Button &btn)
{
  DBG_VERBOSE_F("onButtonOneHold");
  // Undo the press in main and config mode
  switch (controller.mode())
  {
    case main_mode:
    case config_mode:
      buttonOneClicked();
      break;
  }

  buttonOneLongPressed();
}
void onButtonTwoHold(Button &btn)
{
  DBG_VERBOSE_F("onButtonTwoHold");
  // Undo the press 
  switch (controller.mode()) {
    case clock_mode:
    case main_mode:
      buttonTwoClicked();
    case config_mode:
      buttonTwoClicked();
      break;
  }

  buttonTwoLongPressed();
}
void onButtonThreeHold(Button &btn)
{
  DBG_VERBOSE_F("onButtonThreeHold");
  // Undo the press
  switch (controller.mode()) {
    case main_mode:
      controller.toggle_rgb(-2);
      break;
    case config_mode:
      buttonThreeClicked();
      break;
  }
  
  buttonThreeLongPressed();
}
void onButtonFourHold(Button &btn)
{
  DBG_VERBOSE_F("onButtonFourHold");
  buttonFourLongPressed();
}

MPRButtons::MPRButtons() :
  touchSensor(),
  buttonOne(touchSensor, 0),
  buttonTwo(touchSensor, 1),
  buttonThree(touchSensor, 2),
  buttonFour(touchSensor, 3)
{
}

void MPRButtons::setup()
{
  touchSensor.begin();

  buttonOne.onPress(onButtonOnePressed);
  buttonTwo.onPress(onButtonTwoPressed);
  buttonThree.onPress(onButtonThreePressed);
  buttonFour.onPress(onButtonFourPressed);

  buttonOne.onHold(500, onButtonOneHold);
  buttonTwo.onHold(500, onButtonTwoHold);
  buttonThree.onHold(500, onButtonThreeHold);
  buttonFour.onHold(500, onButtonFourHold);
}

void MPRButtons::check()
{
  buttonOne.update();
  buttonTwo.update();
  buttonThree.update();
  buttonFour.update();
}
#endif
