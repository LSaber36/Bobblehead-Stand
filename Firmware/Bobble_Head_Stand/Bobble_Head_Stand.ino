/* 
  Menu driven control of a sound board over UART.
  Commands for playing by # or by name (full 11-char name)
  Hard reset and List files (when not playing audio)
  Vol + and - (only when not playing audio)
  Pause, unpause, quit playing (when playing audio)
  Current play time, and bytes remaining & total bytes (when playing audio)

  Connect UG to ground to have the sound board boot into UART mode
*/

#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include <stdlib.h>
#include <time.h>

// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 10
#define SFX_RX 9
#define SFX_RST 3
#define SFX_ACT 12

#define TOUCH_PIN 15
#define TOUCH_VALUE_THRESH 700
#define NUM_BLINKS 2

Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial2, NULL, SFX_RST);

uint8_t totalNumFiles = 0;
uint32_t currentTouchState = false;
uint32_t oldTouchState = true;

void playStartupFile() {
  uint8_t file_index = 0;

  Serial.print("\nPlaying First File - Track #"); Serial.println(file_index);
  if (! sfx.playTrack(file_index) ) {
    Serial.println("Failed to play track");
    return;
  }
  // Wait for the audio to stop plyaing
  delay(200);
  while (!digitalRead(SFX_ACT));
}

void playRandomFile() {
  Serial.print("Number of files: "); Serial.println(totalNumFiles);

  uint8_t file_index = (rand() % (totalNumFiles-1)) + 1;

  Serial.print("\nPlaying Random File - Track #"); Serial.println(file_index);
  if (! sfx.playTrack(file_index) ) {
    Serial.println("Failed to play track");
    return;
  }
  // Wait for the audio to stop plyaing
  delay(200);
  while (!digitalRead(SFX_ACT));
}

void resetBoard() {
  // Custom function to reset the board without waiting for a response
  pinMode(SFX_RST, OUTPUT);
  digitalWrite(SFX_RST, LOW);
  delay(10);
  // Set the pin as an input, essentially buffering it from current draw
  pinMode(SFX_RST, INPUT);
  // Give the fx board time to boot
  delay(500);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(SFX_ACT, INPUT);

  Serial.begin(9600);  
  Serial2.begin(9600);

  // Set the RNG generator seed using the current time
  srand(time(NULL));

  // Call custom reset function for optimization
  // The caveat of this is that we aren't checking for a response, so it's technically not as robust
  resetBoard();

  totalNumFiles = sfx.listFiles();

  playStartupFile();
  delay(200);

  // Wait for the audio file to finish playing
  while (!digitalRead(SFX_ACT));

  // Use some fancy modulo magic to blink the led with minimal lines
  for (uint8_t i = 0; i < (NUM_BLINKS * 2); i++) {
    digitalWrite(LED_BUILTIN, (i % 2 == 0));
    delay(250);
  }
  Serial.println("Program Ready!");
}

void loop() {
  currentTouchState = (touchRead(TOUCH_PIN) >= TOUCH_VALUE_THRESH);

  if (currentTouchState != oldTouchState  &&  currentTouchState == true) {
    playRandomFile();
    // Use a little delay to make sure this isn't activated too much, kind of like button debouncing
    delay(100);
  }

  oldTouchState = currentTouchState;
}