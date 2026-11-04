// Libraries for AWS
#include "secrets.h"             // for AWS Certificates and Keys
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "app_config.h"          // for Custom Configuration

#include <nvs.h>
#include <nvs_flash.h>
#include <stdlib.h>
#include <SPI.h>

// Application libraries
#include "appManager.h"
#include "receiverBoard.h"

HardwareSerial mySerial(2);      // Use UART2 for communication with smart switches

int publish_counter = 0;
appManager managr;

void checkSerialData(void * pvParameters) {
  String jsonReceived = "{}";
  appManager* appMgr = (appManager*) pvParameters;
  while (1) {
    // Read data coming from the external device via UART2 and print to PC
    if (mySerial.available() > 0) {
      String incomingData = mySerial.readString();
      Serial.print("Received via UART2: ");
      Serial.println(incomingData);

      appMgr->device_status_msg = incomingData;  // Store the received data in appManager
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize UART2 with custom pins (RX_pin 16, TX_pin 17)
  mySerial.begin(9600, SERIAL_8N1, RX_pin, TX_pin);

  // Initialize Application Manager (WiFi & AWS IoT connection with certs_user)
  appManager_ctor(&managr);

  // Create Serial Task to monitor UART2
  if (xTaskCreatePinnedToCore(checkSerialData, "Serial_Task", 4096, &managr, 1, NULL, 0) == pdPASS) {
    Serial.println("Serial_Task created");
  } else {
    Serial.println("Failed to create Serial_Task");
  }
   
  Serial.println("All Systems Initialized..");
}

void loop() {
  // Service MQTT loop (process incoming AWS IoT commands and maintain TLS keepalive)
  loop_mgr(&managr);

  // Periodically publish device status to AWS IoT
  if (publish_counter > PUBLILISH_INTERVAL) {      
    managr.publish_check = true;
    publishData(&managr);
    managr.publish_check = false;
    publish_counter = 0;
  }
  publish_counter++;
  delay(10);
}
