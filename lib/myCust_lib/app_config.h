
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "Arduino.h"

#define UNIQUE_ID              "T4"  // Change this to a unique identifier for your device

#define BAT_MIN_THRESHOLD       10  // Voltage threshold for high battery
#define BAT_MAX_THRESHOLD       15  // Voltage threshold for low battery

#define voltageDividerRatio     0.0909  // Adjust this based on your voltage divider circuit
#define referenceVoltage        3.6  // Reference voltage for ADC (ESP32 typically uses 3.3V)
#define adcResolution           4095 // 12-bit ADC resolution for ESP32
#define calibrationFactor       0.9563   // Adjust this based on your calibration


/******************************* Do Not Edit / update these values      **************************************/
//  Mqtt Configurations 
    #define THINGNAME               "truck-1"        // Change this
    #define AWS_ENDPOINT            "awmminjiukh34-ats.iot.ap-south-1.amazonaws.com"   // AWS MQTT Broker URL  
    #define AWS_IOT_PUBLISH_TOPIC   "trucks/truck-1/temperature"           //change this
    #define AWS_IOT_SUBSCRIBE_TOPIC "trucks/truck-1/temperature"           //change this
    
    #define PUBLILISH_INTERVAL  100       // seconds  // actually 16 seconds 
    #define TIMEOUT_INTERVAL    120       // seconds
   // #define DISPLAY_TIME        10000      // milliseconds  
#endif