#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>
#include <time.h>

// libraries for Display
#include <Wire.h>

// Library for GPS
#include <TinyGPSPlus.h>
 #include "Preferences.h"


// Custom Libraries
//#include "app_config.h"
#include "appManager.h"
#include "connectionManager.h"

#include "receiverBoard.h"
// #include "sensor.h"
// Libraries for Load Cell
#include <Arduino.h> 
#include "EEPROM.h"
#include "Preferences.h"
// #include "HX711.h"
#include "soc/rtc.h"
#include "esp32-hal-cpu.h"

HardwareSerial Hw_Serial(2); // Use UART2 on ESP32 (GPIO 16 = RX2, GPIO 17 = TX2)

connectionManager conManagerr;

Preferences pref;

// The TinyGPSPlus object
TinyGPSPlus gps;


/* constructor implementation */

void appManager_ctor(appManager * const me) {

  // init board    
  initBoard();
    // Start I2C on custom pins (for ESP32)
 // Wire.begin(SDA, SCL);
  Hw_Serial.begin(9600, SERIAL_8N1, TX_GPS, RX_GPS); // Serial for GPS module on ESP32    
  Serial.println("NEO-6M GPS initialized. Waiting for satellite lock...");
 
  me->publish_check = false;
   me->conManager = connectionManager_ctor(&conManagerr);
  Serial.println("Connection Manager set with App Manager");
}
  
/* Function Implementation */

unsigned long int getBoard_ID() {
     return ESP.getEfuseMac();
}

void connectCloud(appManager* appMgr) {
      connectWiFi(appMgr->conManager);
}


void loop_mgr(appManager* appMgr) {
     loop_con(appMgr->conManager);
}


//function to get sensor data and update appManager

void getSensorData_print_update(appManager* appMgr) {
   
  // get data from sensors
/*
 
  if (updateNeeded) {

      // Increase size for time string
      StaticJsonDocument<512> doc;  
      
      doc["UID"] = UNIQUE_ID;
      
      // Get current time
      struct tm timeinfo;
      if(getLocalTime(&timeinfo)){
        char timeStringBuff[50];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
        doc["time"] = timeStringBuff;
      } else {
        doc["time"] = "NTP_SYNC_FAILED";
      }
      
      
      doc["humidity"] = hum_Buff;
      doc["temperature"] = temp_Buff;
      doc["Load"] = load_Buff;
    

      char jsonBuffer[512]; // Increased buffer size
      serializeJson(doc, jsonBuffer); // print to client

      if(!(appMgr->conManager->client.connected())) {

           connectAWS(appMgr->conManager); 
      }
         publishOnMqtt(jsonBuffer, appMgr->conManager);
         Serial.println("Published ");
         updateNeeded = false;
      
      
      // client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
      //appMgr->conManager-> client .publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  checkGyro(appMgr);
  displayDataOnScreen(appMgr);      

  } else {
      //Serial.println("No significant change in sensor data. Skipping publish.");
  }

    if(((millis()-displayOn_start) > DISPLAY_TIME) && screen_state) {
     
//     displayOn_start = 0; 
     displayOn_start = 0; // reset display timer
     displayOn = false;     // set Display off     
     screen_state = false;
     screen.clearDisplay();  // Clear Display
     screen.display();

    }   
*/ 
}


 void checkBatteryStatus(appManager* appMgr) {

  digitalWrite(BAT_SW_PIN, HIGH); // Turn on the battery voltage divider
  delay(50); // Wait for the voltage to stabilize
  
    // 3. Multisample to eradicate ADC noise spikes
  uint32_t pinMillivoltsSum = 0;
  const int samples = 64;
  
  for (int i = 0; i < samples; i++) {
    pinMillivoltsSum += analogReadMilliVolts(BAT_PIN);
    delay(1);
  }
  float pinMillivolts = (float)pinMillivoltsSum / samples;

  // Use the ESP32's calibrated ADC voltage, then undo the divider.
  float pinVoltage = pinMillivolts / 1000.0;
  float batteryVoltage = pinVoltage / voltageDividerRatio;

  Serial.print(F("Bat: "));
  Serial.print(batteryVoltage);

  // 6. Manual Calibration Factor
  // Measure your real battery using a voltmeter. If the serial output reads 4.0V 
  // but your multimeter says 4.15V, adjust this factor (e.g., 4.15 / 4.0 = 1.0375)
  
  batteryVoltage *= calibrationFactor;

  appMgr->batteryLevel = batteryVoltage; // Store the battery voltage in appManager
 
 // batteryPercent = constrain(batteryPercent, 0.0, 100.0);


   // Map the raw value to a 0 - 100% range
  float batteryPercent = ((appMgr->batteryLevel - BAT_MIN_THRESHOLD) / (BAT_MAX_THRESHOLD - BAT_MIN_THRESHOLD)) * 100.0; 
 // float batteryPercentage = map(appMgr->batteryLevel, BAT_MIN_THRESHOLD, BAT_MAX_THRESHOLD, 0, 100);
  // Serial.print("Bat_perc: ");
  // Serial.println(batteryPercent);

  // Check battery status and control LEDs
  if (appMgr->batteryLevel > BAT_MIN_THRESHOLD) {
    digitalWrite(BAT_HIGH_PIN, HIGH);
    digitalWrite(BAT_LOW_PIN, LOW);
   // Serial.println("Battery Status: High");

  } else if (appMgr->batteryLevel < BAT_MIN_THRESHOLD) {
    digitalWrite(BAT_HIGH_PIN, LOW);
    digitalWrite(BAT_LOW_PIN, HIGH);
   // Serial.println("Battery Status: Low");
  
  }

 }

 // Function to display GPS info

 void displayInfo(appManager* appMgr)
{ 
  if (appMgr->mgr_gps.location.isUpdated()) {
    Serial.print(F("Latitude: ")); 
    Serial.println(appMgr->mgr_gps.location.lat(), 6);
    Serial.print(F("Longitude: ")); 
    Serial.println(appMgr->mgr_gps.location.lng(), 6); 

    Serial.println("GPS Data Updated in appManager: ");
  } else {
//    Serial.print(F("Location: Not Available (Searching for satellites...)"));
  }

  //Serial.println();
  delay(1000);

          //          Serial.print("Published : ");
        //  Serial.println(appMgr->publish_check);
        
}


void get_setGPSdata(void * pvParameters) {
  
  appManager* appMgr = (appManager*) pvParameters;
  for(;;) {
    getGPSdata(appMgr);
    vTaskDelay(50 / portTICK_PERIOD_MS); // Delay for a short period to yield to other tasks
  }

  // set GPS data in appManager
//  appMgr->location = appMgr->mgr_gps.location.lat();
//  appMgr->longitude = appMgr->mgr_gps.location.lng(); 

  vTaskDelete(NULL); // Delete the task when done
}

void get_temp_data(appManager* appMgr) {

  const int samples = 16;
  uint32_t voltageSum = 0;
  
  for (int i = 0; i < samples; i++) {
    voltageSum += analogReadMilliVolts(TEMP_PIN);  

  }

  float voltage = (float)voltageSum / samples;

  // LM35 output is 10 mV per degree Celsius.
  float tempC = voltage / 10.0;

  Serial.print(F("  Temp : "));
  Serial.println(tempC);

  //float map(voltage, fromLow, fromHigh, toLow, toHigh);

  //long map(value, fromLow, fromHigh, toLow, toHigh);

  //Serial.print(F("Temp_v & Temp_C: "));
  //Serial.print(voltage);
  //Serial.print(F(" : "));
 // Serial.println(tempC);

  appMgr->temperature = tempC; // Store the temperature in appManager  

}

void publishData(appManager* appMgr) {

      StaticJsonDocument<512> doc;  // JSON document for publishing data, increased size for time string
      
      char lat_Buff[20];
      char lng_Buff[20];
      char batt_Buff[20];
      char temp_Buff[20];
      snprintf(lat_Buff, sizeof(lat_Buff), "%.1f", appMgr->mgr_gps.location.lat());
      snprintf(lng_Buff, sizeof(lng_Buff), "%.1f", appMgr->mgr_gps.location.lng());
      snprintf(batt_Buff, sizeof(batt_Buff), "%.1f", appMgr->batteryLevel);
      snprintf(temp_Buff, sizeof(temp_Buff), "%.1f", appMgr->temperature);
      
       doc["UID"] = UNIQUE_ID;  // Add unique ID to the JSON document
      // Get current time
      struct tm timeinfo;
      if(getLocalTime(&timeinfo)){
        char timeStringBuff[50];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
        doc["ts"] = timeStringBuff;
      } else {
        doc["ts"] = "NTP_SYNC_FAILED";
      } 
/* Comment one */      
      doc["temp"] = temp_Buff;      
      doc["bat"] = batt_Buff;      
      doc["lat"] = lat_Buff;
      doc["lng"] = lng_Buff;
      
      // doc["Alt"] = appMgr->mgr_gps.altitude.meters();
    

      char jsonBuffer[512]; // Increased buffer size
      serializeJson(doc, jsonBuffer); // print to client
      
//      Serial.print(F("publish_check: "));
//      Serial.println(appMgr->publish_check);
      // Publish only if publish_check is true
         if(appMgr->publish_check) {
    //      if(!(appMgr->conManager->client.connected())) {
           connectAWS(appMgr->conManager); 
           publishOnMqtt(jsonBuffer, appMgr->conManager);
           Serial.print(F("Published GPS data to AWS IoT "));
           Serial.println(jsonBuffer);
    //      }          
         }
}

 void getGPSdata(appManager* appMgr) {
 
  // Read incoming data from GPS module
//  Serial.print(F("Entering while loop to read GPS data on Core:1 >> "));
while(Hw_Serial.available() > 0) {  
      char c = Hw_Serial.read();   
      appMgr->mgr_gps.encode(c);
          //displayInfo(appMgr);
          Serial.print(F("."));
      //} // Feed byte to the parser
      
  }
  displayInfo(appMgr);
  // If 5 seconds pass with no data
  if (millis() > 5000 && appMgr->mgr_gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
    delay(5000);
  }

 }
 
 void initBoard() {  
  // Configuring Board pins

  pinMode(BAT_PIN, INPUT);

  pinMode(BAT_HIGH_PIN, OUTPUT);

  pinMode(BAT_LOW_PIN, OUTPUT);

  pinMode(BAT_SW_PIN, OUTPUT);

  pinMode(TEMP_PIN, INPUT);

  Serial.println();
  
  Serial.println(F("Board Initialized."));

 }
 
 
 
 



 


 
