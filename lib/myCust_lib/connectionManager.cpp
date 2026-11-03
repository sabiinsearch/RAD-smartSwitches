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



#include <PubSubClient.h>   // for Mqtt
#include "secrets.h"             //  for AWS Certificates and Keys"

#include "app_config.h"     // for Custom Configration
#include "receiverBoard.h"

/*  */

Preferences preferences;

WiFiClientSecure net = WiFiClientSecure();

PubSubClient pub_sub_client(net);

WiFiManager wm; // WiFi Manager 

       
 String sub_topic = AWS_IOT_SUBSCRIBE_TOPIC;      
 String pub_topic = AWS_IOT_PUBLISH_TOPIC;

char server[50] = AWS_ENDPOINT;



/* constructor implementation */

connectionManager * const connectionManager_ctor(connectionManager * const me ) {
   
  //  initConfig(me);
   //resetWifi(me);       // To Reset Wifi
   me->client = pub_sub_client;
   connectWiFi(me);
   connectAWS(me);

   return me;
}

/* Function implementation */

void messageHandler(char* topic, byte* payload, unsigned int length)
  {
   Serial.print("incoming: ");
   Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
  }

void publishOnMqtt(char* data, connectionManager* con) {
   char jsonBuffer_to_cloud[512];
   strcpy(jsonBuffer_to_cloud, data);

  //  String pub_topic = "trucks/";
  //  pub_topic += THINGNAME;
  //  pub_topic += "/temparature";  // AWS_IOT_PUBLISH_TOPIC;
      
  // client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
   pub_sub_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer_to_cloud);

}

void connectAWS(connectionManager * con) {
  // Certificate validity checks require the ESP32 clock to be synchronized.
  time_t now = time(nullptr);
  while (now < 1700000000) {
    Serial.println("Waiting for NTP time before AWS TLS connection...");
    delay(1000);
    now = time(nullptr);
  }

// Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  pub_sub_client.setServer(AWS_ENDPOINT, 8883);

  // Increase buffer size for larger payloads (JSON with time)
 // pub_sub_client.setBufferSize(512);

  // Create a message handler
  pub_sub_client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  unsigned long lastErrorLog = 0;
  while (!pub_sub_client.connect(THINGNAME))
  {
    if (millis() - lastErrorLog >= 5000) {
      char tlsError[128] = {0};
      int tlsErrorCode = net.lastError(tlsError, sizeof(tlsError));
      Serial.printf("AWS connect failed: MQTT state=%d, TLS error=%d (%s)\n",
                    pub_sub_client.state(), tlsErrorCode,
                    tlsErrorCode ? tlsError : "none");
      lastErrorLog = millis();
    }
    delay(1000);
  }

  if (!pub_sub_client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
 // pub_sub_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
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
  pub_sub_client.loop();
}

void initWiFi() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  //wm.setWiFiAutoReconnect(true);
}



void reconnectWiFi(connectionManager  * con){
  bool res;
  res = wm.autoConnect("myContainer"); // anonymous ap
    if(!res) {
        con->Wifi_status = false;
        digitalWrite(WIFI_LED,HIGH);
        Serial.println("Failed to connect");
        delay(3000);
        delay(5000);
    } 
    else {
        //if you get here you have connected to the WiFi  
        digitalWrite(WIFI_LED,LOW);  
        con->Wifi_status = true;   
    }
}

bool connectWiFi(connectionManager * con) {
  bool res;
  digitalWrite(HEARTBEAT_LED,LOW);  
  wm.setConfigPortalTimeout(TIMEOUT_INTERVAL); // If no access point name has been previously entered disable timeout
  // wm.setConnectTimeout(TIMEOUT_INTERVAL);
  res = wm.autoConnect("GPS_Data"); // auto generated AP name from chipid
    while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

    if(res) {
      //if you get here you have connected to the WiFi         
        con->Wifi_status = true;
        digitalWrite(HEARTBEAT_LED,HIGH);
        digitalWrite(WIFI_LED,LOW);   
      //  Serial.println("Wifi connected...yeey :)");     
        initNTP();      
    }
    return res;
}

void resetWifi(connectionManager * con) {
    con->Wifi_status = false;
    // wm.setConnectTimeout(TIMEOUT_INTERVAL);
    wm.setConfigPortalTimeout(TIMEOUT_INTERVAL); // If no access point name has been previously entered disable timeout
    wm.resetSettings(); // reset settings - wipe stored credentials for testing, these are stored by the esp library
    digitalWrite(WIFI_LED,HIGH);
}



