#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Server configuration
const char* ssid = "bababoiy";
const char* password = "sembarang";
const char* serverURL = "http://192.168.58.12:5000/data/publish_raw"; // Your server's endpoint

// pins
#define HEART_SENSOR_PIN 10

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("attempting to connect...");
  }
  Serial.println("\nConnected to WiFi!");

  pinMode(LED_BUILTIN, OUTPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON data
    StaticJsonDocument<200> doc;
    int valueToSend = analogRead(HEART_SENSOR_PIN);
    Serial.print(">raw:");
    Serial.println(valueToSend);
    doc["reading"] = valueToSend;

    String requestBody;
    serializeJson(doc, requestBody);

    // Send POST request
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      Serial.print("POST Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("Sent value: " + String(valueToSend));
      
      // Optional: LED ON when successful
      // digitalWrite(LED_BUILTIN, LOW);
      // delay(500);
      // digitalWrite(LED_BUILTIN, HIGH);

    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(100);
}
