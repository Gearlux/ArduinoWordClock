// Clock
#include <Wire.h>

#define NEOPIN 6

// Color
#include <NeoPixelBus.h>

#define LED_OFFSET 1
#define LED_ROW 13
#define NR_LEDS 134

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NR_LEDS, NEOPIN);

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

void setup()
{
  Serial.begin(9600);

  strip.Begin();
  strip.Show(); // Initialize all pixels to 'off'

  // Some example procedures showing how to display to the pixels:
  colorWipe(RgbColor(0, 0, 255), 2); // Red

  // restore_settings();
  Serial.println("Initialization done");
}

// Fill the dots one after the other with a color
void colorWipe(RgbColor c, uint8_t wait) {
  Serial.println("colorWipe");
  HsbColor color(0, 1, 0.25);
  for (uint16_t i = 0; i < strip.PixelCount(); i++) {
    color.H = (float)i / (float)strip.PixelCount();
    strip.SetPixelColor(i, color);
    strip.Show();
    delay(wait);
  }
  delay(1000 * (long)wait);
}

void loop()
{
  strip.ClearTo(RgbColor(0, 0, 0));
  colorWipe(RgbColor(0, 0, 255), 2); // Red
}
