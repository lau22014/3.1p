/*
 * Arduino Nano 33 IoT Light Monitoring System
 * Features:
 * - Continuous light intensity measurement using BH1750 sensor
 * - WiFi connectivity for IFTTT integration
 * - Real-time light data printing with classification
 * - IFTTT alerts when light exceeds threshold
 */

#include <WiFiNINA.h>          // For WiFi connectivity
#include <ArduinoHttpClient.h> // For HTTP requests
#include <BH1750.h>           // Light sensor library
#include <Wire.h>             // I2C communication

// Network Configuration
char ssid[] = "TP-Link_57C4"; // WiFi SSID
char pass[] = "zxc123456";    // WiFi password

// IFTTT Webhooks Configuration
const String IFTTT_KEY = "bUM4fpo-1CxXsA3Y1jCPPP";
const String IFTTT_EVENT = "sunlight_alert";

// Global Objects
WiFiClient wifi;
HttpClient client = HttpClient(wifi, "maker.ifttt.com", 80);
BH1750 lightMeter;

// Light Monitoring Variables
unsigned long previousPrintTime = 0;
const long printInterval = 1000; // Data print interval (ms)

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to connect (debugging only)
  
  // Initialize I2C and sensor
  Wire.begin();
  if (!lightMeter.begin()) {
    Serial.println("Error: Failed to initialize BH1750 sensor!");
    while (1); // Halt execution if sensor fails
  }
  
  // Connect to WiFi
  connectWiFi();
}

void loop() {
  // Read current light level
  float lux = lightMeter.readLightLevel();
  
  // Print light data at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - previousPrintTime >= printInterval) {
    previousPrintTime = currentMillis;
    
    // Print timestamped light data
    Serial.print("[");
    Serial.print(currentMillis / 1000);
    Serial.print("s] Current light: ");
    Serial.print(lux);
    Serial.println(" lx");
    
    // Classify light intensity
    classifyLightLevel(lux);
  }

  // Trigger IFTTT if light exceeds threshold
  if (lux > 1000) {
    sendIFTTTAlert(lux);
    delay(100); // 5-minute cooldown period
  }
  
  delay(50); // Small delay to reduce CPU load
}

// Classifies and prints light level description
void classifyLightLevel(float lux) {
  if (lux > 40000) {
    Serial.println("  → Direct sunlight");
  } else if (lux > 1000) {
    Serial.println("  → Overcast day");
  } else if (lux > 300) {
    Serial.println("  → Office lighting");
  } else {
    Serial.println("  → Dim lighting");
  }
}

// Sends alert to IFTTT
void sendIFTTTAlert(float lightValue) {
  String url = "/trigger/" + IFTTT_EVENT + "/with/key/" + IFTTT_KEY;
  url += "?value1=" + String(lightValue);
  
  Serial.print("Sending IFTTT alert: ");
  Serial.println(url);
  
  // Send HTTP GET request
  client.get(url);
  
  // Print server response for debugging
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("HTTP Status: ");
  Serial.println(statusCode);
}

// Handles WiFi connection
void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected successfully!");
  Serial.print("Assigned IP: ");
  Serial.println(WiFi.localIP());
}