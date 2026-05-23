#ifndef APPLICATION_H
#define APPLICATION_H


#include "FilesPath.h"
#include "config/definitions.h"
#include "spiffs/SpiffsIo.h"
#include "network/network_helper.h"
#include "network/wifi.h"
#include "lora/lora_send.h"

class Application : public definitions, public SpiffsIo, public lora_send , public wifi {
    
    private:
        /* data */
    public:
        Application(/* args */);
        ~Application();

        void init_application();

};


extern Application APP;
extern Application app;

#endif /* APPLICATION_H */
