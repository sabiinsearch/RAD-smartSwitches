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


// my libraries
#include "appManager.h"
#include "receiverBoard.h"

HardwareSerial mySerial(2);  // Use UART2 for Serial1

int publish_counter=0;
// my Managers
appManager managr;

// WiFiClientSecure net = WiFiClientSecure();
// PubSubClient client(net);

void checkSerialData(void *) {
  for (;;) {
    // Read data coming from the external device via UART2 and print to PC
    if (mySerial.available() > 0) {
      String incomingData = mySerial.readString();
      //incomingData.trim();
      Serial.print("Received via UART2: ");
      Serial.println(incomingData);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup()
{

  // Change from 2048 to 4096 or higher
//xTaskCreate(TaskFunction, "ScaleTask", 4096, NULL, 1, NULL);

  Serial.begin(115200);

  mySerial.begin(9600, SERIAL_8N1, RX_pin, TX_pin);  // Initialize Serial1 with custom pins

  //  Initiating Manager
  appManager_ctor(&managr);

  // Getting swich status and setting in  Application Manager for further action
  if (xTaskCreatePinnedToCore(checkSerialData, "Serial_Task", 4096, NULL, 1, NULL, 0) == pdPASS) {
    Serial.println("Task created");
  } else {
    Serial.println("Failed to create Serial_Task");
  }
   
  Serial.println("All Systems Initialized..");
 
}

void loop()
{
    
 /*
  getDeviceData(&managr);

  // Get Switches status and publish data to AWS IoT
   if(publish_counter>PUBLILISH_INTERVAL) {      
      publishData(&managr);
      publish_counter = 0;
   }
  loop_mgr(&managr);
  //Serial.println(F("==>> in loop() : main.cpp"));
  //delay(1000);
  */
}
