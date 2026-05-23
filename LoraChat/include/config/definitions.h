#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include <ArduinoWebsockets.h>

using namespace websockets;

class definitions {

    public:

    String rssi     = "RSSI --";
    String packSize = "--";
    String packet;
    
    WebsocketsServer soc_server;
    WebsocketsClient soc_client;

    #define LORA_PUBLIC_SYNCWORD  0x34
    #define LORA_PRIVATE_SYNCWORD 0x12

    #define MY_DEFAULT_APP_ID "DEFV1-LORA-CHAT-0000" //default id will be changed after setup

    TaskHandle_t LORA_RECEIVE;

    int VERSION = 2;
    String APP_ID;
    bool APP_CONNECTED;
    String APP_CONNECTED_STR;
    bool LORA_CONNECTED;
    String LORA_CONNECTED_STR;

    String APP_VERSION;
    String DEVICE_ID;
    uint8_t ID;
    String DEVICE_NAME;
    bool SETUP_OK;
    String AP_SSID;
    String AP_PASS;
    String AP_MAC;
    String AP_IP;
    String AP_HOSTNAME;
    int AP_SSID_HIDDEN;
    int AP_CHANNEL;
    int AP_MAX_CLIENTS;
    bool WIFI_SETUP;
    bool START_WIFI;
    String WIFI_SSID;
    String WIFI_PASS;
    String WIFI_DNS_1;
    String WIFI_DNS_2;
    String WIFI_MAC;
    String WIFI_IP;

    

    String SOC_REMOTE_IP; // ip of client connected to server (PC, MOBILE ... etc)
    int ACK;
    int TYPE;
    int SECURITY;
    int IN_VERSION;
    byte LORA_LOCAL_ADDRESS;
    byte LORA_SENDER;
    byte LORA_RECEIVER;
    byte LORA_BROADCAST_ID = 0xFF;
    byte INCOMING_MSG_LENGTH;
    int MSG_ID;
    bool DELIVERY;
    double LORA_BAND;
    int8_t LORA_TX_POWER;
    String LORA_SYNC_WORD;
    int LORA_SF;
    bool LORA_ENABLE_CRS;
    bool LORA_ENABLE_INVERT_IQ;

    bool EVENT_CONNECTED;

    WebsocketsClient sockets_client;
    const char* websockets_host;
    uint16_t websockets_port;
    
    

    
};



#endif /* _DEFINITIONS_H_ */