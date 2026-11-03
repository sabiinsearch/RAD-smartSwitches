#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

// #include "app_config.h"
// #include "receiverBoard.h"
 #include "connectionManager.h"
 #include <TinyGPSPlus.h>
 


/*Application Manager's attributes*/

typedef struct {

     connectionManager* conManager;     
     
        bool publish_check;
        TinyGPSPlus mgr_gps;        
        float location;
        float longitude;
        float batteryLevel;
        float temperature;
        
     // float prev_temp;
     // float prev_load;
     // float load_threshold;
     
} appManager;

void appManager_ctor(appManager * const me); // constructor

void initBoard(); 
void printOnScreen(int, int, int, int, String); 
void connectCloud(appManager*);
void broadcast_appMgr(appManager*);
void checkConnections_and_reconnect(void * pvParameters);
void setBoardWithLC(appManager*);
void getSensorData_print_update(appManager*);
void loop_mgr(appManager*);
void getGPSdata(appManager* appMgr);
void displayInfo(appManager* appMgr);
void publishData(appManager* appMgr);
void updateGPSdata(appManager* appMgr);
void get_setGPSdata(void * pvParameters);
void get_temp_data(appManager* appMgr);
void checkBatteryStatus(appManager* appMgr);


// functions to set LEDs as per status

/*

HEARTBEAT_LED             // Red
WIFI_LED                  // Blue
BLE_LED                   // Green
*/

#endif