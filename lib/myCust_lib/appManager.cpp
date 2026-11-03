#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>
#include <time.h>


// Library for GPS

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

connectionManager conManagerr;

Preferences pref;



/* constructor implementation */

void appManager_ctor(appManager * const me) {

  // init board    
  initBoard();
    // Start I2C on custom pins (for ESP32)
 // Wire.begin(SDA, SCL);
  
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


 
void getDeviceData(appManager* appMgr) {

// check Serial and set data In Application Manager


}

void publishData(appManager* appMgr) {

      StaticJsonDocument<512> doc;  // JSON document for publishing data, increased size for time string
      
      char lat_Buff[20];
      char lng_Buff[20];
      char batt_Buff[20];
      char temp_Buff[20];
      // snprintf(lat_Buff, sizeof(lat_Buff), "%.1f", appMgr->mgr_gps.location.lat());
      // snprintf(lng_Buff, sizeof(lng_Buff), "%.1f", appMgr->mgr_gps.location.lng());
      // snprintf(batt_Buff, sizeof(batt_Buff), "%.1f", appMgr->batteryLevel);
      // snprintf(temp_Buff, sizeof(temp_Buff), "%.1f", appMgr->temperature);
      
      //  doc["UID"] = UNIQUE_ID;  // Add unique ID to the JSON document
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

 
 void initBoard() {  
  // Configuring Board pins
 
 

  Serial.println();
  
  Serial.println(F("Board Initialized."));

 }
 
 
 
 



 


 
