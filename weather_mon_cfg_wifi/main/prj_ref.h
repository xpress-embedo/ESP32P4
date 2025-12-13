/*
 * prj_ref.h
 *
 *  Created on: 13-12-2025
 *      Author: abc@xyz
 */

#ifndef PRJ_REF_H
#define PRJ_REF_H

#ifdef __cplusplus
extern "C" {
#endif

// Public Macros
#define GUI_TASK_NAME                               "gui task"
#define GUI_TASK_STACK_SIZE                         (1024*8)
#define GUI_TASK_PRIORITY                           (5)
#define GUI_TASK_CORE                               (0)

#define WIFI_APP_TASK_NAME                          "wifi task"
#define WIFI_APP_TASK_SIZE                          (1024*4)
#define WIFI_APP_TASK_PRIORITY                      (5)
#define WIFI_APP_TASK_CORE                          (0)

// for this task is automatically created by the function "httpd_start" as per 
// my understanding, so the "not used" as of now can't be controlled, or atleast
// I don't know now
#define HTTP_SERVER_TASK_NAME                       "http server"   // not used
#define HTTP_SERVER_TASK_SIZE                       (1024*8)
#define HTTP_SERVER_TASK_PRIORITY                   (4)
#define HTTP_SERVER_TASK_CORE                                       // not used

#define HTTP_SERVER_MONITOR_TASK_NAME               "http monitor"
#define HTTP_SERVER_MONITOR_TASK_SIZE               (1024*4)
#define HTTP_SERVER_MONITOR_TASK_PRIORITY           (3)
#define HTTP_SERVER_MONITOR_TASK_CORE               (0)

#define INFLUX_DB_TASK_NAME                         "influx db"
#define INFLUX_DB_TASK_SIZE                         (1024*4)
#define INFLUX_DB_TASK_PRIORITY                     (4)
#define INFLUX_DB_TASK_CORE                         (0)

#define DHT_TASK_NAME                               "dht task"
#define DHT_TASK_SIZE                               (configMINIMAL_STACK_SIZE*3)
#define DHT_TASK_PRIORITY                           (1)
#define DHT_TASK_CORE                               (1)

// Data Structure


// Enumeration


// Public Function Definition


#ifdef __cplusplus
}
#endif

#endif //