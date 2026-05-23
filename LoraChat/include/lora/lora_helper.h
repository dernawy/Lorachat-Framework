#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>


#ifdef __cplusplus
extern "C" {
#endif /**< _cplusplus */

typedef enum {
    LORA_SEND_OK,
    LORA_SEND_FAIL,
    LORA_DELIVERY_OK,
    LORA_DELIVERY_FAIL,
    LORA_SEND_JSON_ERROR,
    LORA_SEND_MSG_LENGTH_ERROR,
    LORA_SEND_NOT_FOR_ME,
    LORA_SEND_APP_VER_NOT_EQUAL
} lora_send_t;

/**
* @brief Divide espnow data into multiple pipes
*/
typedef enum {
    LORA_TYPE_ACK,                 /**< For reliable data transmission */
    LORA_TYPE_OPEN_STREAM,                 /**< For reliable data transmission */
    LORA_TYPE_SEND_APP_STATUS,                 /**< For reliable data transmission */
    LORA_TYPE_GET_APP_STATUS,                 /**< For reliable data transmission */
    LORA_TYPE_DELIVERY_STATUS,     /**< Status packet for rapid upgrade of batch Device */
    LORA_TYPE_MESSAGE,
    LORA_TYPE_DATA                 /**< User-defined use */
} lora_type_t;

/**
* @brief Frame header of espnow
*/
typedef struct {
    uint8_t version;
    bool security;  /**< The payload data is encrypted if security is true */
    bool ack;  /**< Wait for the receiving device to return ack to ensure transmission reliability */
    uint8_t type;
} lora_frame_head_t;

#define LORA_FRAME_CONFIG_DEFAULT() { \
    .version  = 2, \
    .security = false, \
    .ack      = 1, \
    .type     = 0, \
}


typedef struct {
    uint8_t type;
    int msg_id;
    uint8_t dest_addr;
    uint8_t src_addr;
} lora_ack_data_t;





void lora_loop();
void init_send_settings();

String parseAppMessage(JsonDocument app_doc);
void getDeliveryStatus();
void sendAck(int ack, int type, int msg_id, byte dest_addr, byte src_addr, String sender, String receiver);

// This next function will be called by the TJpg_Decoder library during decoding of the jpeg file
// A copy of the decoded MCU block is grabbed for rendering so decoding can then continue while
// the MCU block is rendered on the TFT. Note: This function is called by processor 0




#ifdef __cplusplus
}
#endif /**< _cplusplus */