#include <Arduino.h>
const int adcPin = 34; // Choose a safe ADC1 pin (GPIO 32-39)

// Resistor values
const float R1 = 10000.0; 
const float R2 = 1000.0;

// Conversion factor back to battery voltage
const float dividerRatio = (R1 + R2) / R2; // This equals 11.0

void setup() {
  Serial.begin(115200);
 analogReadResolution(12);
  // Use the ESP32's low-voltage ADC range for a ~342 mV sensor signal.
  analogSetPinAttenuation(adcPin, ADC_11db);
}

void loop() {
  // Read raw ADC value (0 - 4095)
  int rawADC = analogRead(adcPin);
  
  // Use the ESP32 ADC calibration for the configured 11 dB attenuation.
  //uint32_t pinMillivolts = analogReadMilliVolts(adcPin);
  float pinVoltage = (rawADC / 4095.0) * 3.3;
  
  // Calculate original battery voltage
  float batteryVoltage = pinVoltage * dividerRatio;
  
  Serial.print("Raw ADC counts: ");
  Serial.print(rawADC);
  Serial.print(" | Calibrated pin voltage: ");
  Serial.print(pinVoltage);
  Serial.print("V | Battery Volts: ");
  Serial.print(batteryVoltage);
  Serial.println("V");
  
  delay(1000);
}
