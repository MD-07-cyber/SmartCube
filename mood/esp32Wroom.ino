#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Credentials
const char* ssid = "Redmi 12C";       // Replace with your WiFi SSID
const char* password = "Dini16254030@"; // Replace with your WiFi Password

// Server details
const String serverURL = "http://192.168.197.226:5000/get-mood"; // Replace with your Flask server URL
const String client_id = "esp32_cam"; // The client_id to retrieve the mood

// GPIO Pins for bulbs
const int bulb1 = 4; // Happy
const int bulb2 = 27; // Sad
const int bulb3 = 5; // Angry
const int bulb4 = 33; // Relaxed

void setup() {
  Serial.begin(115200);

  // Initialize GPIO pins as output
  pinMode(bulb1, OUTPUT);
  pinMode(bulb2, OUTPUT);
  pinMode(bulb3, OUTPUT);
  pinMode(bulb4, OUTPUT);

  // Turn off all bulbs initially
  digitalWrite(bulb1, LOW);
  digitalWrite(bulb2, LOW);
  digitalWrite(bulb3, LOW);
  digitalWrite(bulb4, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the URL with client_id
    String url = serverURL + "?client_id=" + client_id;
    http.begin(url); // Initialize HTTP request
    int httpResponseCode = http.GET(); // Perform GET request

    if (httpResponseCode > 0) {
      String payload = http.getString(); // Get the response payload
      Serial.println("Response: " + payload);

      // Extract mood from the response (assuming JSON format)
      if (payload.indexOf("happy") > -1) {
        setMood("happy");
      } else if (payload.indexOf("sad") > -1) {
        setMood("sad");
      } else if (payload.indexOf("angry") > -1) {
        setMood("angry");
      } else if (payload.indexOf("neutral") > -1) {
        setMood("neutral");
      } else {
        Serial.println("Mood not recognized");
        resetBulbs();
      }
    } else {
      Serial.println("Error in HTTP request");
    }
    http.end(); // Close the connection
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(5000); // Wait before the next request
}

void setMood(String mood) {
  // Turn off all bulbs
  resetBulbs();

  // Turn on the appropriate bulb
  if (mood == "happy") {
    digitalWrite(bulb1, HIGH);
    Serial.println("Happy mood detected: Bulb 1 ON");
  } else if (mood == "sad") {
    digitalWrite(bulb2, HIGH);
    Serial.println("Sad mood detected: Bulb 2 ON");
  } else if (mood == "angry") {
    digitalWrite(bulb3, HIGH);
    Serial.println("Angry mood detected: Bulb 3 ON");
  } else if (mood == "neutral") {
    digitalWrite(bulb4, HIGH);
    Serial.println("Relaxed mood detected: Bulb 4 ON");
  }
}

void resetBulbs() {
  // Turn off all bulbs
  digitalWrite(bulb1, LOW);
  digitalWrite(bulb2, LOW);
  digitalWrite(bulb3, LOW);
  digitalWrite(bulb4, LOW);
  Serial.println("All bulbs turned OFF");
}