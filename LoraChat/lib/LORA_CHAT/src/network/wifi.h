#ifndef WIFI_H
#define WIFI_H 

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "network/network_helper.h"
#include <ArduinoWebsockets.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"

class wifi {

    private:
        /* data */
    public:
        wifi(/* args */);
        ~wifi();

        void send_settings_to_server_calback();
        t_event_send send_event(String message, String event, uint32_t id);
        
        bool socketServerPoll();
        void serverPollLoop();
        void drawingTimeText(uint32_t deltaT, int yPos);

};





#endif /* WIFI_H */