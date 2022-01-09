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
    int start_index = word.firstPixelY * LED_ROW + word.firstPixelX;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      SetPixelColor(start_index + i, color);
    }
  } else {
    int start_index = word.firstPixelY * LED_ROW + 10 - word.firstPixelX;
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
    int start_index = word.firstPixelY * LED_ROW + word.firstPixelX;
    int i = 0;
    for (i = 0; i < word.length; i++) {
      updateColor(start_index + i, color, fraction);
    }
  } else {
    int start_index = word.firstPixelY * LED_ROW + 10 - word.firstPixelX;
    for (int i = 0; i < word.length; ++i) {
      updateColor(start_index - i, color, fraction);
    }
  }
}

// Delegates
void LedStrip::Begin()
{
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::Begin();
}

void LedStrip::Show()
{
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::Show();
}

void LedStrip::ClearTo(RgbColor color)
{
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::ClearTo(color);
}

#define DOT4 NR_LEDS - 4

void LedStrip::SetPixelColor(uint16_t indexPixel, RgbColor color)
{
#ifdef BOTTOM_TO_TOP
  uint16_t finalPixel = LED_OFFSET + 10 * LED_ROW - (LED_ROW - 10) - indexPixel;
  if (indexPixel == DOT4)
    finalPixel = 0;
  else if (indexPixel > DOT4)
    finalPixel = indexPixel -1;
  // DBG_DEBUG_F("%d -> %d", indexPixel, finalPixel);
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::SetPixelColor(finalPixel, color);
#else
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::SetPixelColor(indexPixel, color);
#endif
}

uint16_t LedStrip::PixelCount() const
{
  return NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>::PixelCount();
}
