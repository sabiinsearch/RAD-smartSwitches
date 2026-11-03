// Libraries for AWS
#include "secrets.h"             //  for AWS Certificates and Keys
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "app_config.h"     // for Custom Configration

#include <nvs.h>
#include <nvs_flash.h>
#include <stdlib.h>
#include <SPI.h>

// Others
#include <TinyGPSPlus.h>

// my libraries
#include "appManager.h"
#include "receiverBoard.h"
#include "sensor.h"

int publish_counter=0;
// my Managers
appManager managr;
TinyGPSPlus gps_;
//HardwareSerial Hw_Serial(2); // Use UART2 on ESP32 (GPIO 16 = RX2, GPIO 17 = TX2)

// DHT dht(DHTPIN, DHTTYPE);

// WiFiClientSecure net = WiFiClientSecure();
// PubSubClient client(net);

void display_test() {
  if (gps_.location.isValid()) {
    Serial.print(F("Latitude: "));
    Serial.println(gps_.location.lat(), 6);
    Serial.print(F("Longitude: "));
    Serial.println(gps_.location.lng(), 6);
  } else {
    Serial.print(F("Location: Not Available (Searching for satellites...)"));
  }
  Serial.println();
}

void setup()
{

  // Change from 2048 to 4096 or higher
//xTaskCreate(TaskFunction, "ScaleTask", 4096, NULL, 1, NULL);

  Serial.begin(115200);

  analogReadResolution(12); 
  analogSetAttenuation(ADC_2_5db); // Fixes the ceiling to ~1330mV max
    // 2. IMPORTANT: Change attenuation to 11dB to cleanly fit the 1.04V pin input
  analogSetPinAttenuation(BAT_PIN, ADC_11db);
  analogSetPinAttenuation(TEMP_PIN, ADC_0db);

  //
  appManager_ctor(&managr);
  // assinig the GPS task in second core of ESP32
  xTaskCreatePinnedToCore( get_setGPSdata, "GPS_Task", 4096, &managr, 1, NULL, 1);

     
    Serial.println("Task created ");
    // Initiating Manager
  
  
  Serial.println("All Systems Initialized..");
 
}

void loop()
{
    
  checkBatteryStatus(&managr);
  get_temp_data(&managr);

  // getSensorData_print_update(&managr);
   if(publish_counter>PUBLILISH_INTERVAL) {      
      managr.publish_check = true;
      publish_counter = 0;
   } else {      
      managr.publish_check = false;
   }
   publish_counter++;
  //  Serial.print("Publish Counter : ");
  //  Serial.print(publish_counter);
  //  Serial.print(F("\t appMgr->publish_check : "));
  //  Serial.println(managr.publish_check);
  // Serial.print(F("==>> in loop() : main.cpp"));
   publishData(&managr);

/*
 while (Serial2.available() > 0) {
    if (gps_.encode(Serial2.read())) {
      display_test();
    }
  }

  if (millis() > 5000 && gps_.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    delay(5000);
  }
 */  
  loop_mgr(&managr);
  //Serial.println(F("==>> in loop() : main.cpp"));
  //delay(1000);
}
