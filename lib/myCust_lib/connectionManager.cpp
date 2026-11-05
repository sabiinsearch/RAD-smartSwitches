#include <ArduinoJson.h>
#include <time.h>
#include "connectionManager.h"
#include "Preferences.h"
#include "appManager.h"

// for WiFi and mqtt
#include <WiFiManager.h> 
#include "WiFi.h"
#include "WiFiGeneric.h"
#include "WiFiSTA.h"
#include "WiFiClientSecure.h"

#include <PubSubClient.h>        // for Mqtt
#include "secrets.h"             // for AWS Certificates and Keys
#include "app_config.h"          // for Custom Configuration
#include "receiverBoard.h"

extern HardwareSerial mySerial;  // UART2 for communication with smart switch controller

Preferences preferences;
WiFiClientSecure net = WiFiClientSecure();
PubSubClient pub_sub_client(net);
WiFiManager wm;                  // WiFi Manager 

String sub_topic = AWS_IOT_SUBSCRIBE_TOPIC;      
String pub_topic = AWS_IOT_PUBLISH_TOPIC;
char server[50] = AWS_ENDPOINT;

/* constructor implementation */
connectionManager * const connectionManager_ctor(connectionManager * const me ) {
   me->client = pub_sub_client;
   connectWiFi(me);
#if ENABLE_AWS_CONNECT
   connectAWS(me);
#endif
   return me;
}

/* Function implementation */

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming MQTT message on topic: ");
  Serial.println(topic);

  char payloadStr[256];
  unsigned int copyLen = (length < sizeof(payloadStr) - 1) ? length : (sizeof(payloadStr) - 1);
  memcpy(payloadStr, payload, copyLen);
  payloadStr[copyLen] = '\0';
  Serial.printf("Payload: %s\n", payloadStr);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (!err && doc.containsKey("message")) {
    const char* message = doc["message"];
    Serial.printf("Extracted command from JSON: %s\n", message);
    mySerial.println(message);  // Forward command to switch board via UART2
  } else {
    // Handle raw string command directly (e.g., "A#", "M#", "1#", "2#", "3#", "4#")
    Serial.printf("Forwarding raw command to UART2: %s\n", payloadStr);
    mySerial.println(payloadStr);
  }
}

void publishOnMqtt(char* data, connectionManager* con) {
  if (pub_sub_client.connected()) {
    pub_sub_client.publish(AWS_IOT_PUBLISH_TOPIC, data);
  } else {
    Serial.println("Cannot publish: AWS IoT MQTT client not connected");
  }
}

void connectAWS(connectionManager * con) {
  if (pub_sub_client.connected()) {
    return;
  }

  // Certificate validity checks require the ESP32 clock to be synchronized via NTP.
  time_t now = time(nullptr);
  int ntpWait = 0;
  while (now < 1700000000 && ntpWait < 15) {
    Serial.println("Waiting for NTP time before AWS TLS connection...");
    delay(1000);
    now = time(nullptr);
    ntpWait++;
  }

  // Configure WiFiClientSecure with AWS certificates from certs_user (via secrets.h)
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on AWS endpoint and port 8883
  pub_sub_client.setServer(AWS_ENDPOINT, AWS_PORT);
  pub_sub_client.setCallback(messageHandler);

  Serial.printf("Connecting to AWS IoT (%s:%d) as Thing: %s\n", AWS_ENDPOINT, AWS_PORT, THINGNAME);

  int retries = 0;
  while (!pub_sub_client.connect(THINGNAME) && retries < 5) {
    char tlsError[128] = {0};
    int tlsErrorCode = net.lastError(tlsError, sizeof(tlsError));
    Serial.printf("AWS connect attempt %d/5 failed: MQTT state=%d, TLS error=%d (%s)\n",
                  retries + 1, pub_sub_client.state(), tlsErrorCode,
                  tlsErrorCode ? tlsError : "none");
    retries++;
    delay(2000);
  }

  if (!pub_sub_client.connected()) {
    Serial.println("AWS IoT connection failed or timed out!");
    return;
  }

  Serial.println("AWS IoT Connected successfully!");

  // Subscribe to command topic. QoS 0 avoids PUBACK timing issues.
  // Note: subscribe() only SENDS the SUBSCRIBE packet - it does NOT wait for SUBACK.
  // We call pub_sub_client.loop() briefly after to process the SUBACK from AWS IoT.
  bool subSent = pub_sub_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC, 0);
  Serial.printf("[MQTT] SUBSCRIBE packet sent=%s to %s\n", subSent ? "OK" : "FAILED", AWS_IOT_SUBSCRIBE_TOPIC);

  // Process SUBACK by calling loop() for up to 1 second
  unsigned long subackWait = millis();
  while (millis() - subackWait < 1000) { pub_sub_client.loop(); delay(10); }
  Serial.printf("[MQTT] SUBACK wait done. MQTT state=%d\n", pub_sub_client.state());

  // Publish online status to status topic (bems/green-building-1/status)
  pub_sub_client.publish(AWS_IOT_STATUS_TOPIC, "online");
  Serial.printf("Published online status to: %s\n", AWS_IOT_STATUS_TOPIC);
}

void initNTP() {
  // Configure time using NTP
  // 19800 = GMT offset for IST (UTC +5:30)
  // 0 = Daylight offset in seconds
  // "pool.ntp.org" = NTP server
  configTime(19800, 0, "pool.ntp.org");
  Serial.println("NTP Configured");
}

void loop_con(connectionManager* con) {
#if ENABLE_AWS_CONNECT
  static unsigned long lastDiag = 0;
  if (WiFi.status() == WL_CONNECTED) {
    if (!pub_sub_client.connected()) {
      if (millis() - lastDiag > 5000) {
        lastDiag = millis();
        Serial.printf("[MQTT] Disconnected. State=%d. Will reconnect...\n", pub_sub_client.state());
      }
      static unsigned long lastReconnect = 0;
      if (millis() - lastReconnect > 10000) {
        lastReconnect = millis();
        Serial.println("MQTT connection lost. Reconnecting to AWS IoT...");
        connectAWS(con);
      }
    } else {
      if (millis() - lastDiag > 10000) {
        lastDiag = millis();
        Serial.printf("[MQTT] Connected OK. State=%d. Calling loop()...\n", pub_sub_client.state());
      }
      bool loopOk = pub_sub_client.loop();
      if (!loopOk) {
        Serial.println("[MQTT] loop() returned false - connection may have dropped!");
      }
    }
  } else {
    if (millis() - lastDiag > 5000) {
      lastDiag = millis();
      Serial.println("[WiFi] Not connected! Cannot run MQTT loop.");
    }
  }
#endif
}

void initWiFi() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
}

void reconnectWiFi(connectionManager * con) {
  bool res = wm.autoConnect("myContainer");
  if (!res) {
    con->Wifi_status = false;
    Serial.println("Failed to connect to WiFi");
    delay(5000);
  } else {
    con->Wifi_status = true;
  }
}

bool connectWiFi(connectionManager * con) {
  wm.setConfigPortalTimeout(TIMEOUT_INTERVAL);
  bool res = wm.autoConnect("RAD_SmartSwitch");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (res) {
    con->Wifi_status = true;
    Serial.println("\nWiFi connected.");
    initNTP();      
  }
  return res;
}

void resetWifi(connectionManager * con) {
  con->Wifi_status = false;
  wm.setConfigPortalTimeout(TIMEOUT_INTERVAL);
  wm.resetSettings();
}
