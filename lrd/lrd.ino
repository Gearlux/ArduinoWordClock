// Clock
#include <Wire.h>
// Photocell
#define lightPin A0

#define step 1.8509443151142613
#define MAX_BRIGHTNESS 9
#define LRD_SCALE 128

#define TIMEOUT 60000 // in milliseconds
#define TIME_UPDATE_INTERVAL 250 // in milli seconds
#define CLOCK_UPDATE_INTERVAL 250 // in milli seconds
#define BLINK_INTERVAL 500 // in milli seconds

#define MAX_BRIGHTNESS 9
int brightness_high = MAX_BRIGHTNESS;
int brightness_low = 0;


//-----------------------------------------------------
// Function Declarations
//-----------------------------------------------------

void toggle_brightness();
void change_brightness(int update);

// ----------------------------------------------------
// Arduino
// ----------------------------------------------------

void setup()
{
  Serial.begin(9600);

  // restore_settings();
  Serial.println("Initialization done");
}

void loop()
{
  update_display();

  delay(200);
}


void update_display()
{
  //Serial.print("update_display brightness: ");
  
  int reading = analogRead(lightPin);
  float scale = brightness_low + max(0.0f, min(1.0f, reading / LRD_SCALE)) * (brightness_high - brightness_low);
  float B = pow(step, scale) / 255.0f;
  Serial.println(B);

}
