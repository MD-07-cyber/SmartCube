#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "HUAWEI Y7 Pro 2019";
const char* password = "987654321";

// Server URL
String serverURL = "http://10.10.70.170:5000/analyze"; // Flask server URL



// Camera pin configuration for ESP32-CAM (AI-Thinker)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WiFi initialization
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Camera configuration
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

  // Frame size and buffer
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12; // Lower quality for less memory usage
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed! Error: 0x%x\n", err);
    while (true); // Stop execution
  }

  Serial.println("Camera initialized successfully.");
}

void loop() {
  // Attempt to capture a frame
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("Camera capture failed. Diagnosing issue...");

    // Check camera status
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
      Serial.println("Failed to retrieve camera sensor!");
    

    // Check memory status
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("Try reducing resolution or frame buffer count.");
  } else {
    Serial.println("Camera capture successful!");

    // Debugging memory usage
    if (fb->len > 0) { // Added a safety check
      Serial.printf("Frame size: %d bytes\n", fb->len);
    } else {
      Serial.println("Frame buffer length is zero!");
    }

    // Return the frame buffer to free memory
    esp_camera_fb_return(fb);
  }

  delay(2000); // Delay for readability
}}