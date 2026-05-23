#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>


#ifdef __cplusplus
extern "C" {
#endif /**< _cplusplus */

typedef enum {
    EVENT_SEND_OK,
    EVENT_SEND_NOT_CONNECTED,
    EVENT_SEND_KO

} t_event_send;




void start_wifi_ap();
void start_wifi_sta();
bool init_sockets();
void start_setup_server();
void start_production_server();
void start_control_server();

void socketServerListen();
bool socketServerPoll();


// This next function will be called by the TJpg_Decoder library during decoding of the jpeg file
// A copy of the decoded MCU block is grabbed for rendering so decoding can then continue while
// the MCU block is rendered on the TFT. Note: This function is called by processor 0






#ifdef __cplusplus
}
#endif /**< _cplusplus */