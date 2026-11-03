
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "Arduino.h"

//#define UNIQUE_ID              "T4"  // Change this to a unique identifier for your device

/******************************* Do Not Edit / update these values      **************************************/
//  Mqtt Configurations 
    #define THINGNAME               "green-building-1"        // Change this
    #define AWS_ENDPOINT            "a2rtkre10vau7j-ats.iot.eu-north-1.amazonaws.com"   // AWS MQTT Broker URL  
    #define AWS_IOT_PUBLISH_TOPIC   "bems/green-building-1/"           //change this
    #define AWS_IOT_SUBSCRIBE_TOPIC "bems/green-building-1/tel"           //change this
    
    #define PUBLILISH_INTERVAL  1000       // seconds  // actually 16 seconds 
    #define TIMEOUT_INTERVAL    120       // seconds
   // #define DISPLAY_TIME        10000      // milliseconds 

   /******** Define  Commands   ************/ 
   
   #define AUTO        "A#"
   #define MANUAL      "M#"
   #define GRID        "1#" 
   #define BIOMASS     "2#" 
   #define SOLAR       "3#" 
   #define BESS        "4#"

#endif