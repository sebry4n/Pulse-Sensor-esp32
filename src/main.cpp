#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "POCO F4";
const char* password = "ryan1234";

const char* serverURL = "http://192.168.116.12:5000/numbers";

int lastValue = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the built-in LED pin
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(serverURL);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String newValue = http.getString();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, newValue);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }


      // Extract the value from the JSON object
      int value = doc[0]["num"].as<int>();
      Serial.print("Received value: ");
      Serial.println(value);

      if(lastValue == value)
      {
        ;
      }
      else if(value)
      {
        digitalWrite(LED_BUILTIN, LOW); // Turn on the LED
        Serial.println("LED ON");
        lastValue = value;
      } else {
        digitalWrite(LED_BUILTIN, HIGH); // Turn off the LED
        Serial.println("LED OFF");
        lastValue = value;
      }

    } else {
      Serial.print("Error code: ");
      Serial.println(httpCode);
    }


    http.end();
  }

  delay(5000); // Poll every 5 seconds
}