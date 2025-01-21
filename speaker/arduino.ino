#include "DFRobotDFPlayerMini.h"

HardwareSerial mySerial(2); // Use UART2 (TX=16, RX=17)
DFRobotDFPlayerMini myDFPlayer;

// Function to play music based on mood
void playMusicBasedOnMood(int mood) {
  switch(mood) {
    case 1: // Angry - Play track A.mp3
      myDFPlayer.play(1); 
      Serial.println("Playing Angry music");
      break;
    case 2: // Sad - Play track S.mp3
      myDFPlayer.play(2);
      Serial.println("Playing Sad music");
      break;
    case 3: // Happy - Play track H.mp3
      myDFPlayer.play(3);
      Serial.println("Playing Happy music");
      break;
    case 4: // Normal - Play track N.mp3
      myDFPlayer.play(4);
      Serial.println("Playing Normal music");
      break;
    default:
      Serial.println("Unknown mood, no music playing");
      break;
  }
}

void setup() {
  Serial.begin(115200); // For debugging
  mySerial.begin(9600, SERIAL_8N1, 17, 16); // DFPlayer TX=17, RX=16
  
  Serial.println("Testing DFPlayer Mini...");
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("Communication with DFPlayer Mini failed!");
    while (true); // Halt execution
  }
  Serial.println("DFPlayer Mini communication successful!");
  myDFPlayer.volume(20); // Set volume (0-30)
}

void loop() {
  // For example, assuming the mood is detected and represented by an integer
  int mood = 2; // Happy mood (could be dynamically set based on mood detection)

  playMusicBasedOnMood(mood); // Play music based on detected mood
  
  delay(5000); // Wait for 5 seconds before switching mood/music
}