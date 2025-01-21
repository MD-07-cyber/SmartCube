#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// Wi-Fi credentials
const char* ssid = "Redmi 12C";
const char* password = "Dini16254030@";

// Server URL (replace with your server's IP address)
const char* serverUrl = "http://192.168.197.226:5000/mood";

// Timer variables
unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds in milliseconds

// Camera pin configuration for AI-Thinker ESP32-CAM
#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32-CAM...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // Full resolution
    config.jpeg_quality = 10;           // Lower value = higher quality
    config.fb_count = 2;                // Use 2 frame buffers
  } else {
    config.frame_size = FRAMESIZE_SVGA; // Lower resolution
    config.jpeg_quality = 12;           // Lower value = higher quality
    config.fb_count = 1;                // Use 1 frame buffer
  }

  // Initialize the camera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera initialization failed!");
    while (true); // Halt execution if camera fails to initialize
  }
  Serial.println("Camera initialized successfully!");
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if interval has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Serial.println("Capturing image...");
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed!");
      return;
    }

    Serial.println("Image captured, sending to server...");
    HTTPClient http;
    http.begin(serverUrl); // Initialize HTTP connection
    http.addHeader("Content-Type", "image/jpeg");
    http.setTimeout(20000); // Set timeout to 20 seconds

    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);

      // Parse "mood" field from response
     int startIndex = response.indexOf("\"mood\":\"");
      if (startIndex != -1) {
        startIndex += 8; // Move past the "mood":" part
        int endIndex = response.indexOf("\"", startIndex);
        if (endIndex != -1) {
          String mood = response.substring(startIndex, endIndex);
          Serial.println("Detected Mood: " + mood);
        } else {
          Serial.println("Error: Unable to parse mood from response.");
        }
      } 
    } else {
      Serial.println("Error in sending image: " + String(httpResponseCode));
      Serial.println("HTTP Error: " + http.errorToString(httpResponseCode));
    }

    http.end();
    esp_camera_fb_return(fb); // Release memory for the frame buffer
  }
}