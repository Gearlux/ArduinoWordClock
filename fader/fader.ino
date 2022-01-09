#include <NeoPixelBus.h>

#define DBG_ENABLE_DEBUG

#include "fdebug.hpp"

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ledstrip(20, 5);

#define FADE_STEPS 200
#define FADENR (FADE_STEPS * 3.8 / 2)
#define FADEDIFF (FADENR - FADE_STEPS)
#define STEP_SIZE 0.95

#define WORD_LENGTH 5

int seconds_counter = -1;

int previous_second = 0;
int second = 0;

unsigned long current_time;
unsigned long previous_time;

HsbColor pixelcolor(0, 1, 0.05);

DEBUG_INSTANCE(80, Serial);

void colorwipe(uint8_t wait)
{
  DBG_INFO("colorWipe");
  HsbColor color(0, 1, 0.25);
  for (uint16_t i = 0; i < ledstrip.PixelCount(); i++) {
    color.H = (float)i / (float)ledstrip.PixelCount();
    ledstrip.SetPixelColor(i, color);
    ledstrip.Show();
    delay(wait);
  }
  delay(1000 * (long)wait);
}

void setup() {
  Serial.begin(9600);
  
  // put your setup code here, to run once:
  ledstrip.Begin();
  ledstrip.Show();

  DBG_DEBUG("colorwipe");
  colorwipe(2);
  
  previous_time = millis();
  ledstrip.SetPixelColor(0, pixelcolor);
  DBG_DEBUG("init done");
}

void loop() {
  current_time = millis();

  if (current_time > previous_time + 1000 * WORD_LENGTH) {
    second += 1;
    previous_time = current_time;
    seconds_counter = FADENR;

    if (second > 20 / WORD_LENGTH)
      second = 0;
  }


  if (previous_second == second)
    return;

  // float down_fraction = pow(STEP_SIZE, FADE_STEPS - seconds_counter);
  // float up_fraction = pow(STEP_SIZE, seconds_counter);
  
  // float down_fraction = seconds_counter / (float)FADE_STEPS;
  // float up_fraction = (FADE_STEPS - seconds_counter) / (float)FADE_STEPS;

  float down_fraction = seconds_counter > FADEDIFF ? (seconds_counter - FADEDIFF) / (float)(FADE_STEPS) : 0;
  float up_fraction = seconds_counter < FADE_STEPS ? (FADE_STEPS - seconds_counter) / (float)(FADE_STEPS) : 0;
  // DBG_DEBUG("%d %d %d", seconds_counter, (int)(up_fraction * 1000), (int)(down_fraction * 1000));

  HsbColor up_color(pixelcolor);
  up_color.B *= up_fraction;
  HsbColor down_color(pixelcolor);
  down_color.B *= down_fraction;

  ledstrip.ClearTo(RgbColor(0,0,0));

  for(int i=0; i<WORD_LENGTH; i++) {
    ledstrip.SetPixelColor(WORD_LENGTH * second + i, up_color);
    ledstrip.SetPixelColor(WORD_LENGTH * previous_second + i, down_color);
  }
  ledstrip.Show();
  
  seconds_counter--;
  if (seconds_counter < 0)
    previous_second = second;

  delay(4);
}
