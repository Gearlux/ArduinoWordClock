#include "mprmetro.h"
#include "ledstrip.h"

#define NEOPIN 5

LedStrip::LedStrip(): NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(NR_LEDS, NEOPIN)
{
}

// Fill the dots one after the other with a color
void LedStrip::colorWipe(uint8_t wait) {
  DBG_INFO("colorWipe");
  HsbColor color(0, 1, 0.25);
  for (uint16_t i = 0; i < PixelCount(); i++) {
    color.H = (float)i / (float)PixelCount();
    SetPixelColor(i, color);
    Show();
    delay(wait);
  }
  delay(1000 * (long)wait);
}

void LedStrip::enable_led(LedWord  word, const HsbColor &color)
{
  if (word.firstPixelY % 2 == 0) {
    int start_index = word.firstPixelY * LED_ROW + word.firstPixelX + LED_OFFSET;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      SetPixelColor(start_index + i, color);
    }
  } else {
    int start_index = word.firstPixelY * LED_ROW + 10 - word.firstPixelX + LED_OFFSET;
    for (int i = 0; i < word.length; ++i) {
      SetPixelColor(start_index - i, color);
    }
  }
}

void LedStrip::updateColor(uint16_t index, const HsbColor &color, float fraction)
{
    RgbColor old_color = GetPixelColor(index);
    RgbColor rgb_color(color);
    old_color.R += fraction * rgb_color.R;
    old_color.G += fraction * rgb_color.G;
    old_color.B += fraction * rgb_color.B;
    SetPixelColor(index, old_color);
}

void LedStrip::enable_led(LedWord  word, const HsbColor &color, float fraction)
{
  if (fraction == 1.0f)
  {
    enable_led(word, color);
    return;
  }
    
  if (word.firstPixelY % 2 == 0) {
    int start_index = word.firstPixelY * LED_ROW + word.firstPixelX + LED_OFFSET;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      updateColor(start_index + i, color, fraction);
    }
  } else {
    int start_index = word.firstPixelY * LED_ROW + 10 - word.firstPixelX + LED_OFFSET;
    for (int i = 0; i < word.length; ++i) {
      updateColor(start_index - i, color, fraction);
    }
  }
}
