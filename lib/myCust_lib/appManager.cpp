#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Preferences.h"

// Custom Libraries
#include "appManager.h"
#include "connectionManager.h"
#include "receiverBoard.h"
#include "EEPROM.h"
#include "soc/rtc.h"
#include "esp32-hal-cpu.h"

connectionManager conManagerr;
Preferences pref;

/* constructor implementation */
void appManager_ctor(appManager * const me) {
  initBoard();
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
  // Can be used to process or query smart switch status
}

void publishData(appManager* appMgr) {
  StaticJsonDocument<512> doc;

  // Get current time
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    doc["ts"] = timeStringBuff;
  } else {
    doc["ts"] = "NTP_SYNC_FAILED";
  }

  // Include switch/device status received from UART2 if available
  if (appMgr->device_status_msg.length() > 0) {
    doc["device_status"] = appMgr->device_status_msg;
  }

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  // Publish only if publish_check is true
  if (appMgr->publish_check) {
    publishOnMqtt(jsonBuffer, appMgr->conManager);
    Serial.print(F("Published data to AWS IoT: "));
    Serial.println(jsonBuffer);
  }
}

void initBoard() {  
  Serial.println();
  Serial.println(F("Board Initialized."));
}
