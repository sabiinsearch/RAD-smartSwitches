#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

// #include "app_config.h"
// #include "receiverBoard.h"
 #include "connectionManager.h" 


/*Application Manager's attributes*/

typedef struct {

     connectionManager* conManager;     
     
        bool publish_check;      
        
} appManager;

void appManager_ctor(appManager * const me); // constructor

void initBoard(); 
void connectCloud(appManager*);
void broadcast_appMgr(appManager*);
void checkConnections_and_reconnect(void * pvParameters);
void loop_mgr(appManager*);
void get_temp_data(appManager* appMgr);



// functions to set LEDs as per status

/*

HEARTBEAT_LED             // Red
WIFI_LED                  // Blue
BLE_LED                   // Green
*/

#endif