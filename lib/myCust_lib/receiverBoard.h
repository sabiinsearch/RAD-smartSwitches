#include <Arduino.h>

#ifndef __RECEIVER_BOARD_H__
#define __RECEIVER_BOARD_H__

// Sensors Config


#define RX_GPS                16
#define TX_GPS                17

// #define RGB LEDs
#define HEARTBEAT_LED       27         // Red
#define WIFI_LED            14         // Green
#define MQTT_LED            26         // Blue

//user-defined pins

#define BAT_PIN            34
#define BAT_HIGH_PIN       19
#define BAT_LOW_PIN        18
#define BAT_SW_PIN         5
#define TEMP_PIN           35

unsigned long int getBoard_ID();

#endif