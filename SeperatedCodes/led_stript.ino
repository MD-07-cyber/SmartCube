#include <FastLED.h>

#define LED_PIN 21
#define NUM_LEDS  60
#define BRIGHTNESS 100
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

int mood = 1;

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  setMoodColor(mood);
  FastLED.show();

  delay(2000);
  mood++;
  if (mood > 4) mood = 1;
}

void setMoodColor(int moodValue) {
  CRGB color;

  switch (moodValue) {
    case 1: color = CRGB::Green;  break;
    case 2: color = CRGB::Blue;   break;
    case 3: color = CRGB::Red;    break;
    case 4: color = CRGB::White;  break;
    default: color = CRGB::Black; break;
  }

  fill_solid(leds, NUM_LEDS, color);
}
