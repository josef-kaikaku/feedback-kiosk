#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* ssid = ""; // Fill in your WiFi SSID
const char* password = ""; // Fill in your WiFi password

// Airtable credentials
const char* airtableToken = ""; // Fill in your Airtable API token
const char* airtableBaseId = ""; // Fill in your Airtable Base ID
const char* airtableTableId = ""; // Fill in your Airtable Table ID

// Pin configurations
#define LED_PIN 14
#define NUM_LEDS 8
#define BUTTON1_PIN 23
#define BUTTON2_PIN 22
#define BUTTON3_PIN 21
#define BUTTON4_PIN 19

#endif // CONFIG_H