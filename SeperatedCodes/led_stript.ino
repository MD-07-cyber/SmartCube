#include <FastLED.h>

#define LED_PIN 13       // Pin connected to LED strip
#define NUM_LEDS 30      // Number of LEDs in the strip
#define BRIGHTNESS 100   // Brightness level (0-255)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

int mood = 1; // Example: set mood (1=Happy, 2=Sad, 3=Angry, 4=Normal)

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  setMoodColor(mood);
  FastLED.show();

  delay(2000); // just to simulate mood change
  mood++;
  if (mood > 4) mood = 1; // cycle through moods
}

void setMoodColor(int moodValue) {
  CRGB color;

  switch (moodValue) {
    case 1: color = CRGB::Green;  break; // Happy
    case 2: color = CRGB::Blue;   break; // Sad
    case 3: color = CRGB::Red;    break; // Angry
    case 4: color = CRGB::White;  break; // Normal
    default: color = CRGB::Black; break; // Off
  }

  fill_solid(leds, NUM_LEDS, color);
}
