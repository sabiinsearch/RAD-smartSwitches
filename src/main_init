// Libraries for AWS
#include "secrets.h"             //  for AWS Certificates and Keys
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// Others
// #include <nvs.h>
// #include <nvs_flash.h>


// my libraries
#include "appManager.h"
#include "receiverBoard.h"
#include "sensor.h"

// my Managers
appManager managr;


// DHT dht(DHTPIN, DHTTYPE);

// WiFiClientSecure net = WiFiClientSecure();
// PubSubClient client(net);



void setup()
{

  // Change from 2048 to 4096 or higher
//xTaskCreate(TaskFunction, "ScaleTask", 4096, NULL, 1, NULL);

  Serial.begin(115200);
 
    // Initiating Manager
  //Serial.println("Initializing App Manager..");
  appManager_ctor(&managr);
  
  Serial.println("All Systems Initialized..");

 
}

void loop()
{
    
  getSensorData_print_update(&managr);
  
  checkButtonPressed(&managr);
  loop_mgr(&managr);
  delay(100);
}