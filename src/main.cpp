#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>

// WiFi setup
const char* ssid = "Rian";
const char* password = "rian5432";
const char* serverURL = "http://192.168.110.203:5000/data/publish_raw";

// Heart sensor pin
#define HEART_SENSOR_PIN 9

// OLED setup (SSH1106)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,     // No rotation
  /* clock=*/ 6, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE
);

// Graph buffer
#define GRAPH_WIDTH 128
#define GRAPH_HEIGHT 40
uint8_t graphData[GRAPH_WIDTH];
uint8_t graphIndex = 0;

void setup() {
  Serial.begin(115200);

  // OLED init
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);

  // WiFi init
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  pinMode(LED_BUILTIN, OUTPUT);
  analogReadResolution(12); // 0-4095
  analogSetAttenuation(ADC_11db); // full range 0-3.3V
  digitalWrite(LED_BUILTIN, HIGH);
}

void drawGraph() {
  u8g2.drawFrame(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT);

  for (int x = 1; x < GRAPH_WIDTH; x++) {
    int raw1 = graphData[(graphIndex + x - 1) % GRAPH_WIDTH];
    int raw2 = graphData[(graphIndex + x) % GRAPH_WIDTH];

    // Clamp ADC to max 4095
    raw1 = constrain(raw1, 0, 4095);
    raw2 = constrain(raw2, 0, 4095);

    // Scale to graph height
    int y1 = GRAPH_HEIGHT - (raw1 * GRAPH_HEIGHT / 500);
    int y2 = GRAPH_HEIGHT - (raw2 * GRAPH_HEIGHT / 500);

    // Clamp y1, y2 to inside frame (just in case)
    y1 = constrain(y1, 0, GRAPH_HEIGHT - 1);
    y2 = constrain(y2, 0, GRAPH_HEIGHT - 1);

    u8g2.drawLine(x - 1, y1, x, y2);
  }
}

void loop() {
  int valueToSend = analogRead(HEART_SENSOR_PIN);
  Serial.print(">raw: ");
  Serial.println(valueToSend);

  // Store value in buffer for graph
  graphData[graphIndex] = valueToSend;
  graphIndex = (graphIndex + 1) % GRAPH_WIDTH;

  // Send to server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    ArduinoJson::StaticJsonDocument<200> doc;
    doc["reading"] = valueToSend;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      Serial.print("POST Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  // Draw OLED
  u8g2.clearBuffer();
  u8g2.setCursor(3, 62);
  u8g2.print("HR Raw: ");
  u8g2.print(valueToSend);
  u8g2.setCursor(70, 54);
  u8g2.print("*built on");
  u8g2.setCursor(70, 62);
  u8g2.print(" trauma");
  drawGraph();
  u8g2.sendBuffer();
  delay(20);
}
