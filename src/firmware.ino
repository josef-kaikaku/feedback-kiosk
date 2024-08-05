#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include "config.h"


CRGB leds[NUM_LEDS];

// Setup NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Button states
int button1State = HIGH;
int button2State = HIGH;
int button3State = HIGH;
int button4State = HIGH;

// Last button states for debouncing
int lastButton1State = HIGH;
int lastButton2State = HIGH;
int lastButton3State = HIGH;
int lastButton4State = HIGH;

// Timing variables
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long lastDebounceTime4 = 0;
const unsigned long debounceDelay = 50; // 50 ms debounce delay

void setup() {
  Serial.begin(9600);

  Serial.println("Setting up button Pins...");
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.println("Setting up LED...");
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  delay(500);  // Small delay to ensure initialization
  FastLED.clear();
  FastLED.show();

  // Initialize NTP client
  Serial.println("Setting up Time Client...");
  timeClient.begin();
}

void loop() {
  timeClient.update();
  // Read the button state
  checkButton(button1Pin, lastButton1State, button1State, lastDebounceTime1, 0);
  checkButton(button2Pin, lastButton2State, button2State, lastDebounceTime2, 1);
  checkButton(button3Pin, lastButton3State, button3State, lastDebounceTime3, 2);
  checkButton(button4Pin, lastButton4State, button4State, lastDebounceTime4, 3);
}

// Function to check and debounce each button
void checkButton(int pin, int &lastState, int &currentState, unsigned long &lastDebounceTime, int buttonValue) {
  int reading = digitalRead(pin);

  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentState) {
      currentState = reading;

      if (currentState == LOW) {

        // Determine the color based on the button value
        CRGB color;
        switch (buttonValue) {
          case 3: color = CRGB::Green; break;
          case 2: color = CRGB::Yellow; break;
          case 1: color = CRGB::Orange; break;
          case 0: color = CRGB::Red; break;
          default: color = CRGB::Black; break;
        }

        // Perform breathing animation
        breath(color, 3, 1.5); // 3 breaths with 10ms delay

          // Turn off all LEDs after breathing animation
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        logButtonPress(buttonValue);
      }
    }
  }

  lastState = reading;
}

// Function to log button press to the cloud
void logButtonPress(int buttonValue) {

  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime(&rawtime);
  char buffer[30];
  strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", ti);
  String timestamp(buffer);

  String formattedTime = timeClient.getFormattedTime();
  Serial.println("Button " + String(buttonValue) + " Pressed at: " + formattedTime);
  sendToAirtable(timestamp, buttonValue);
}
// Function to send data to Airtable
void sendToAirtable(String timestamp, int rating) {
  WiFiClientSecure client;
  client.setInsecure();

  if (client.connect("api.airtable.com", 443)) {
    String url = String("/v0/") + airtableBaseId + "/" + airtableTableId;

    String jsonPayload = "{\"fields\": {\"Timestamp\": \"" + timestamp + "\", \"Rating\": " + String(rating) + "}}";

    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: api.airtable.com");
    client.println("Authorization: Bearer " + String(airtableToken));
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(jsonPayload.length()));
    client.println();
    client.println(jsonPayload);

    // Print debug information
    Serial.println("Sending data to the cloud...");
    Serial.println("Timestamp: " + timestamp);
    Serial.println("Rating: " + String(rating));

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
    }

    Serial.println("Data sent to Airtable");
    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  } else {
    Serial.println("Connection to Airtable failed");
  }
  client.stop();
}

// Function to perform breathing animation
void breath(CRGB color, int breaths, int delayTime) {
  for (int b = 0; b < breaths; b++) {
    // Increase brightness
    for (int brightness = 0; brightness < 255; brightness++) {
      fill_solid(leds, NUM_LEDS, color);
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(delayTime); // This delay controls the speed of dimming
    }
    // Decrease brightness
    for (int brightness = 255; brightness >= 0; brightness--) {
      fill_solid(leds, NUM_LEDS, color);
      FastLED.setBrightness(brightness);
      FastLED.show();
      delay(delayTime); // This delay controls the speed of dimming
    }
  }
}