#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "DFRobotDFPlayerMini.h"

// TFT pin setup
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  15
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// DFPlayer setup on UART2
HardwareSerial mySerial(2); // TX=16, RX=17
DFRobotDFPlayerMini myDFPlayer;

int mood = 1;  // Start with mood 1

void setup() {
  Serial.begin(115200);

  // TFT Setup
  tft.begin();
  tft.setRotation(1);
  showMoodScreen(mood);

  // DFPlayer Setup
  mySerial.begin(9600, SERIAL_8N1, 17, 16); // RX=16, TX=17
  Serial.println("Initializing DFPlayer...");
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini not found!");
    while (true); // halt
  }
  myDFPlayer.volume(20); // Volume (0~30)
  Serial.println("DFPlayer Ready");

  playMusicBasedOnMood(mood);
}

void loop() {
  delay(6000); // Wait for 6 seconds
  mood++;
  if (mood > 4) mood = 1;

  showMoodScreen(mood);
  playMusicBasedOnMood(mood);
}

void showMoodScreen(int moodID) {
  String moodText, icon;
  uint16_t bg, boxColor, textColor;

  switch (moodID) {
    case 1:
      moodText = "HAPPY";
      icon = ":)";
      bg = ILI9341_GREEN;
      boxColor = ILI9341_WHITE;
      textColor = ILI9341_GREEN;
      break;
    case 2:
      moodText = "SAD";
      icon = ":(";
      bg = ILI9341_BLUE;
      boxColor = ILI9341_WHITE;
      textColor = ILI9341_BLUE;
      break;
    case 3:
      moodText = "NORMAL";
      icon = ":|";
      bg = ILI9341_YELLOW;
      boxColor = ILI9341_BLACK;
      textColor = ILI9341_ORANGE;
      break;
    case 4:
      moodText = "ANGRY";
      icon = ">:(";
      bg = ILI9341_RED;
      boxColor = ILI9341_WHITE;
      textColor = ILI9341_RED;
      break;
    default:
      moodText = "UNKNOWN";
      icon = "?";
      bg = ILI9341_BLACK;
      boxColor = ILI9341_WHITE;
      textColor = ILI9341_WHITE;
  }

  tft.fillScreen(bg);
  tft.fillRoundRect(30, 40, 180, 160, 10, boxColor);
  tft.drawRoundRect(30, 40, 180, 160, 10, textColor);

  tft.setTextColor(textColor);
  tft.setTextSize(2);
  tft.setCursor(80, 50);
  tft.println("Your Mood:");

  tft.setTextSize(4);
  tft.setCursor(100, 90);
  tft.println(icon);

  tft.setTextSize(3);
  tft.setCursor(80, 140);
  tft.println(moodText);
}

void playMusicBasedOnMood(int mood) {
  switch (mood) {
    case 1: // Happy - H.mp3
      myDFPlayer.play(3);
      Serial.println("Playing Happy (H.mp3)");
      break;
    case 2: // Sad - S.mp3
      myDFPlayer.play(2);
      Serial.println("Playing Sad (S.mp3)");
      break;
    case 3: // Normal - N.mp3
      myDFPlayer.play(4);
      Serial.println("Playing Normal (N.mp3)");
      break;
    case 4: // Angry - A.mp3
      myDFPlayer.play(1);
      Serial.println("Playing Angry (A.mp3)");
      break;
    default:
      Serial.println("Unknown mood, no music played.");
  }
}
