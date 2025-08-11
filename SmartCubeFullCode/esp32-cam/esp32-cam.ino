#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include <WebServer.h>  // <-- Added for stream

// ====== Wi-Fi Credentials ======
const char* ssid = "Redmi 12C";
const char* password = "Dini16254030@";

// ====== Flask Server via LocalTunnel ======
const char* serverUrl = "http://dini-mood-box.loca.lt/mood?client_id=esp32_cam";

// ====== Timer Settings ======
unsigned long previousMillis = 0;
const long interval = 10000; // Capture every 10 seconds

// ====== AI Thinker ESP32-CAM Pin Definitions ======
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ====== Stream Server ======
WebServer server(80);  // Port 80 for live stream

// Stream handler
void handleStream() {
  WiFiClient client = server.client();

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (1) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ Camera capture failed for stream.");
      continue;
    }

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n");
    server.sendContent("Content-Length: " + String(fb->len) + "\r\n\r\n");
    server.sendContent((const char*)fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb);

    if (!client.connected()) break;
    delay(50);  // Frame rate ~20 fps
  }
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure Camera
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

  // Lower resolution for faster upload
  config.frame_size = FRAMESIZE_VGA;   // Use QVGA for smaller, faster
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Initialize Camera
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    while (true);
  }

  Serial.println("Camera ready.");

  // Start stream server
  server.on("/stream", HTTP_GET, handleStream);
  server.begin();
  Serial.println("📡 Stream ready. Access /stream in browser.");
}

// ====== Loop ======
void loop() {
  server.handleClient();  // Handle stream requests

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Serial.println("\nCapturing image...");

    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ Capture failed!");
      return;
    }

    Serial.println("✅ Image captured. Sending to Flask...");

    // Send image to server
    HTTPClient http;
    http.setTimeout(20000); // 20-second timeout

    if (http.begin(serverUrl)) {
      http.addHeader("Content-Type", "image/jpeg");

      int httpResponseCode = http.POST(fb->buf, fb->len);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("✅ Server response: " + response);
      } else {
        Serial.print("❌ POST failed. HTTP error: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("❌ HTTP connection failed.");
    }

    esp_camera_fb_return(fb); // Free memory
  }
}
