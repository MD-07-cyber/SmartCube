#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// Server URL
const char* serverUrl = "http://your-server-ip:5000/mood";

// Timer variables
unsigned long previousMillis = 0;
const long interval = 60000; // 1 minute in milliseconds

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Initialize the camera
  camera_config_t config;
  // Configure your camera settings here
  esp_camera_init(&config);
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if 1 minute has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Send image to server
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
      // Get response from server
      String response = http.getString();
      Serial.println("Server Response: " + response);

      // OPTIONAL: Parse response for the message field
      int startIndex = response.indexOf("\"message\":\"") + 10; // Find start of message
      int endIndex = response.indexOf("\"", startIndex); // Find end of message
      String message = response.substring(startIndex, endIndex);
      Serial.println("Detected Mood: " + message);
    } else {
      Serial.println("Error in sending image: " + String(httpResponseCode));
    }

    http.end();
    esp_camera_fb_return(fb);
  }
}
