#define FASTLED_RMT_BUILTIN_DRIVER
#define FASTLED_ALLOW_INTERRUPTS 1
#define FASTLED_RMT_MAX_CHANNELS 1

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "DFRobotDFPlayerMini.h"
#include <FastLED.h>
#include "AdafruitIO_WiFi.h"

// ==================== WiFi ====================
#define WIFI_SSID     "Redmi 12C"
#define WIFI_PASS     "Dini16254030@"

// ==================== Adafruit IO ====================
#define IO_USERNAME   "MadhuDissa"
#define IO_KEY        "aio_GeVD02lb3XX6NwrHfPp6n3S1mmDp"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Feeds
AdafruitIO_Feed *lightFeed = io.feed("Relay1");
AdafruitIO_Feed *commandFeed = io.feed("moodcube-command");

#define RELAY_PIN 19
bool systemActive = false;

// ==================== Mood Server ====================
const String serverURL = "http://dini-mood-box.loca.lt/get-mood";
const String client_id = "esp32_cam";

// ==================== TFT Pins ====================
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  15
#define BACKLIGHT_PIN 13
#define BUTTON_UP     12
#define BUTTON_DOWN   14
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ==================== DFPlayer ====================
#define DFPLAYER_RX 16
#define DFPLAYER_TX 17
HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;
bool dfplayerAvailable = false;

// ==================== LED Strip ====================
#define LED_PIN     21
#define NUM_LEDS    60
#define CHIPSET     WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// ==================== Water Sensors ====================
#define SENSOR1_PIN 32
#define SENSOR2_PIN 33
#define SENSOR3_PIN 34

// ==================== Mist Pins ====================
#define MIST_HAPPY 25
#define MIST_SAD   26
#define MIST_ANGRY 27

// ==================== Mood Enum ====================
enum Mood { NONE, HAPPY, SAD, NORMAL, ANGRY };
Mood currentMood = NONE;
bool moodDetected = false;

// ==================== Timers ====================
unsigned long lastMoodFetch = 0;
unsigned long moodFetchInterval = 10000;

unsigned long lastWaterUpdate = 0;
unsigned long waterUpdateInterval = 500;

unsigned long lastBrightnessCheck = 0;
unsigned long brightnessCheckInterval = 100;

int brightness = 128;

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // Attach feed callbacks
  lightFeed->onMessage(handleLightMessage);
  commandFeed->onMessage(handleCommandMessage);

  // Wait for connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to Adafruit IO");

  // Get last known states
  lightFeed->get();
  commandFeed->get();

  delay(2000); // Let callbacks update systemActive

  if (systemActive) {
    initializeMoodCube();
  }
}

// ==================== Loop ====================
void loop() {
  io.run(); // Required for Adafruit IO events

  if (!systemActive) {
    return; // Skip all cube functions if inactive
  }

  unsigned long now = millis();

  if (now - lastBrightnessCheck >= brightnessCheckInterval) {
    handleBrightnessButtons();
    lastBrightnessCheck = now;
  }

  if (now - lastMoodFetch >= moodFetchInterval) {
    fetchMoodFromServer();
    lastMoodFetch = now;
  }

  if (!moodDetected && now - lastWaterUpdate >= waterUpdateInterval) {
    showWaterLevels();
    lastWaterUpdate = now;
  }
}

// ==================== Feed Handlers ====================
void handleLightMessage(AdafruitIO_Data *data) {
  int value = data->toInt();
  Serial.print("Relay1 Feed: ");
  Serial.println(value);

  if (value == 1) {
    activateSystem();
  } else {
    deactivateSystem();
  }
}

void handleCommandMessage(AdafruitIO_Data *data) {
  String cmd = data->toString();
  Serial.print("Command Received: ");
  Serial.println(cmd);  // <-- Add this

  cmd.toLowerCase();

  if (cmd == "start") {
    Serial.println("Activating system...");
    activateSystem();
  } else if (cmd == "stop") {
    Serial.println("Deactivating system...");
    deactivateSystem();
  }
}

// ==================== Activation/Deactivation ====================
void activateSystem() {
  if (!systemActive) {
    digitalWrite(RELAY_PIN, HIGH);
    systemActive = true;
    initializeMoodCube();
    Serial.println("System Activated");
  }
}

void deactivateSystem() {
  if (systemActive) {
    // First stop all outputs cleanly
    FastLED.clear(true);        // Clear LEDs immediately
    allMistOff();               // Turn off mist pins
    if (dfplayerAvailable) {
      myDFPlayer.stop();        // Stop any playing sounds
    }
    tft.fillScreen(ILI9341_BLACK);  // Clear display
    analogWrite(BACKLIGHT_PIN, 0);  // Turn backlight off

    delay(100);                 // Small delay for peripherals to settle

    digitalWrite(RELAY_PIN, LOW);
    systemActive = false;

    Serial.println("System Deactivated");
  }
}

void shutdownMoodCube() {
  // This is called during deactivateSystem, you can keep it or merge as above
  // To be safe, clear everything again here too
  FastLED.clear(true);
  allMistOff();
  if (dfplayerAvailable) myDFPlayer.stop();
  tft.fillScreen(ILI9341_BLACK);
  analogWrite(BACKLIGHT_PIN, 0);
}

// ==================== Initialization ====================
void initializeMoodCube() {
  io.run(); if (!systemActive) return;

  tft.begin();
  tft.setRotation(1);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, brightness);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  drawStaticUI();
  io.run(); if (!systemActive) return;
  showDetectingScreen();
  io.run(); if (!systemActive) return;

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(150);
  setLEDSoftBlue();
  io.run(); if (!systemActive) return;

  mySerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini not found.");
    dfplayerAvailable = false;
  } else {
    myDFPlayer.volume(25);
    dfplayerAvailable = true;
  }

  pinMode(MIST_HAPPY, OUTPUT);
  pinMode(MIST_SAD, OUTPUT);
  pinMode(MIST_ANGRY, OUTPUT);
  allMistOff();

  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(SENSOR3_PIN, INPUT);

  Serial.println("Mood Cube Initialized");
}

// ==================== Mood Fetch ====================
void fetchMoodFromServer() {
  io.run(); 
  if (!systemActive) return;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return;
  }

  HTTPClient http;
  String url = serverURL + "?client_id=" + client_id;
  http.begin(url);
  int httpCode = http.GET();

  io.run();
  if (!systemActive) {
    http.end();
    return;
  }

  if (httpCode > 0) {
    String payload = http.getString();
    io.run();
    if (!systemActive) {
      http.end();
      return;
    }

    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      String mood = doc["mood"].as<String>();
      mood.toLowerCase();

      io.run();
      if (!systemActive) {
        http.end();
        return;
      }

      if (mood == "happy") setMood(HAPPY);
      else if (mood == "sad") setMood(SAD);
      else if (mood == "angry") setMood(ANGRY);
      else if (mood == "normal" || mood == "neutral") setMood(NORMAL);
      else setMood(NONE);
    }
  }
  http.end();
}

// ==================== Mood Functions ====================
void setMood(Mood mood) {
  currentMood = mood;

  if (mood == NONE) {
    moodDetected = false;
    showDetectingScreen();
    setLEDSoftBlue();
    allMistOff();
    return;
  }

  moodDetected = true;
  updateMoodDisplay(mood);
  io.run(); if (!systemActive) return;
  updateLEDsForMood(mood);
  io.run(); if (!systemActive) return;
  controlMist(mood);
  io.run(); if (!systemActive) return;

  if (dfplayerAvailable) playMoodSound(mood);
}

// ==================== TFT Display ====================
void drawStaticUI() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 200);
  tft.print("Brightness:");
  tft.drawRoundRect(130, 200, 100, 18, 5, ILI9341_WHITE);
}

void showDetectingScreen() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(40, 80);
  tft.print("Detecting Mood...");
}

void updateMoodDisplay(Mood mood) {
  uint16_t bg;
  String label;
  switch (mood) {
    case HAPPY:  bg = ILI9341_YELLOW; label = "HAPPY"; break;
    case SAD:    bg = ILI9341_BLUE;   label = "SAD";   break;
    case ANGRY:  bg = ILI9341_RED;    label = "ANGRY"; break;
    case NORMAL: bg = ILI9341_GREEN;  label = "NORMAL"; break;
    default:     bg = ILI9341_BLACK;  label = "NONE"; break;
  }
  tft.fillScreen(bg);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);
  tft.setCursor(70, 80);
  tft.print(label);
}

// ==================== LED Strip ====================
void setLEDSoftBlue() {
  fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
  FastLED.show();
}

void updateLEDsForMood(Mood mood) {
  CRGB color;
  switch (mood) {
    case HAPPY:  color = CRGB::Yellow; break;
    case SAD:    color = CRGB::Blue;   break;
    case ANGRY:  color = CRGB::Red;    break;
    case NORMAL: color = CRGB::Green;  break;
    default:     color = CRGB(180, 200, 255); break;
  }
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

// ==================== DFPlayer ====================
void playMoodSound(Mood mood) {
  int track = 0;
  if (mood == ANGRY) track = 1;
  else if (mood == SAD) track = 2;
  else if (mood == HAPPY) track = 3;
  else if (mood == NORMAL) track = 4;

  if (track > 0) myDFPlayer.play(track);
}

// ==================== Mist ====================
void allMistOff() {
  digitalWrite(MIST_HAPPY, HIGH);
  digitalWrite(MIST_SAD, HIGH);
  digitalWrite(MIST_ANGRY, HIGH);
}

void controlMist(Mood mood) {
  allMistOff();
  if (mood == HAPPY || mood == NORMAL) digitalWrite(MIST_HAPPY, LOW);
  else if (mood == SAD) digitalWrite(MIST_SAD, LOW);
  else if (mood == ANGRY) digitalWrite(MIST_ANGRY, LOW);
}

// ==================== Brightness ====================
void handleBrightnessButtons() {
  bool changed = false;
  if (digitalRead(BUTTON_UP) == LOW) {
    brightness = min(brightness + 15, 255);
    analogWrite(BACKLIGHT_PIN, brightness);
    changed = true;
  }
  if (digitalRead(BUTTON_DOWN) == LOW) {
    brightness = max(brightness - 15, 0);
    analogWrite(BACKLIGHT_PIN, brightness);
    changed = true;
  }
  if (changed) updateBrightnessBar();
}

void updateBrightnessBar() {
  int width = map(brightness, 0, 255, 0, 98);
  tft.fillRect(131, 201, 98, 16, ILI9341_BLACK);
  tft.fillRect(131, 201, width, 16, ILI9341_GREEN);
}

// ==================== Water Levels ====================
void showWaterLevels() {
  int s1 = analogRead(SENSOR1_PIN);
  int s2 = analogRead(SENSOR2_PIN);
  int s3 = analogRead(SENSOR3_PIN);
  int l1 = map(s1, 0, 4095, 0, 100);
  int l2 = map(s2, 0, 4095, 0, 100);
  int l3 = map(s3, 0, 4095, 0, 100);
  drawWaterBar(270, l1);
  drawWaterBar(285, l2);
  drawWaterBar(300, l3);
}

void drawWaterBar(int x, int percent) {
  int height = map(percent, 0, 100, 0, 98);
  int y = 31 + (98 - height);
  tft.fillRect(x, 31, 10, 98, ILI9341_BLACK);
  tft.fillRect(x, y, 10, height, ILI9341_BLUE);
}