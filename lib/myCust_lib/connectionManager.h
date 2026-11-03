#ifndef __CONNNECTION_MANAGER_H__
#define __CONNNECTION_MANAGER_H__

#include "Arduino.h"
#include <WiFiManager.h>
#include "app_config.h"
#include <PubSubClient.h>   // for Mqtt


/*Connection Manager's attributes*/
typedef struct {

  WiFiManager wifi_manager;

  PubSubClient client;
  bool radio_status;  
  bool ble_status;
  bool Wifi_status;
  bool mqtt_status; 
//  struct configuration_t config; 

} connectionManager;


/*Connection Manager's operations*/
 
   connectionManager * const  connectionManager_ctor(connectionManager * const me);     // Constructor

  
   void checkConnections_and_reconnect(void *);
   void initWiFi();
   void initNTP();
   void mqtt_loop();
   bool connectMQTT(connectionManager*);
   bool connectWiFi(connectionManager*);
   void reconnectWiFi(connectionManager*);
   void resetWifi(connectionManager*);
   void connectAWS(connectionManager*);   
   void messageHandler(char*, byte*, unsigned int);
   void publishOnMqtt(char*, connectionManager*);  
   void loop_con(connectionManager*);


#endif