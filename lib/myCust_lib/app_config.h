#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "Arduino.h"

/******************************* MQTT Configurations for AWS IoT (certs_user) **************************************/
#define THINGNAME               "green-building-1"
#define AWS_ENDPOINT            "a2rtkre10vau7j-ats.iot.ap-south-1.amazonaws.com"
#define AWS_PORT                8883

// BEMS AWS IoT Topics (Policy: bems-device)
#define AWS_IOT_PUBLISH_TOPIC   "bems/green-building-1/tel"      // Telemetry published by device to cloud
#define AWS_IOT_SUBSCRIBE_TOPIC "bems/green-building-1/cmd"      // Commands subscribed by device from cloud
#define AWS_IOT_STATUS_TOPIC    "bems/green-building-1/status"   // Status (online/offline) published by device

#define PUBLILISH_INTERVAL      1000                             // loop cycles between publish
#define TIMEOUT_INTERVAL        120                              // seconds

/******** Define Commands ************/ 
#define AUTO        "A#"
#define MANUAL      "M#"
#define GRID        "1#" 
#define BIOMASS     "2#" 
#define SOLAR       "3#" 
#define BESS        "4#"

#endif
