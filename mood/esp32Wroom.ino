#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Redmi 12C";
const char* password = "Dini16254030@";

// Server URL
const char* serverUrl = "http://192.168.197.226:5000/get-mood";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Client 2...");

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
}

void loop() {
  Serial.println("Requesting mood result from server...");

  HTTPClient http;
  http.begin(serverUrl); // Initialize HTTP connection
  int httpResponseCode = http.GET();

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
    Serial.println("Error in retrieving mood: " + String(httpResponseCode));
    Serial.println("HTTP Error: " + http.errorToString(httpResponseCode));
  }

  http.end();
  delay(10000); // Delay before the next request
}