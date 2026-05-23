#include <Arduino.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include "Application.h"
#include "heltec.h"


void setup() {
    // put your setup code here, to run once:
    app.init_application();
}

void loop() {

    if(app.SETUP_OK){

        app.loraRecive(LoRa.parsePacket());
    }
  	
    delay(5);
}