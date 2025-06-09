#include <Arduino.h>

const int heartSensorPin = 14; // GPIO14
const int numSamples = 50;     // Moving average over the last 10 samples

int readings[numSamples];      // Circular buffer
int readIndex = 0;
int total = 0;
int average = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);       // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db); // Read up to ~3.3V

  // Initialize readings array
  for (int i = 0; i < numSamples; i++) {
    readings[i] = 0;
  }
}

void loop() {
  // Subtract the oldest reading
  total -= readings[readIndex];

  // Read the current sensor value
  readings[readIndex] = analogRead(heartSensorPin);

  // Add the new reading to total
  total += readings[readIndex];

  // Advance the index (loop around if needed)
  readIndex = (readIndex + 1) % numSamples;

  // Compute the average
  average = total / numSamples;

  Serial.println(average); // Smoothed signal
  delay(10);               // ~100 Hz sample rate
}