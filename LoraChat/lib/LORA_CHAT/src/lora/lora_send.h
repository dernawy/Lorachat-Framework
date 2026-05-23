#ifndef LORA_SEND_H
#define LORA_SEND_H

#include "lora/lora_helper.h"


class lora_send {

    private:
        /* data */
    public:
        lora_send(/* args */);
        ~lora_send();

        void init_lora();
        void loraRecive(int packetSize);
        void loraReceiveTask();
        lora_send_t sendMessage(JsonDocument app_doc);
        void getAppStatus();
        void openStream(String status, String lora_status);
        void sendAppStatus(String app_status, String lora_status);

        
        
};





#endif /* TFT_LORA_H */