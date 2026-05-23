#include "Application.h"
#include "heltec.h"

Application::Application(/* args */){}
Application::~Application(){}


void Application::init_application(){

    Serial.begin(115200);

    app.init_spiffs();

    app.init_lora();

    Serial.println("\nSETUP_OK: " + app.SETUP_OK);

    if(!app.SETUP_OK){
        start_wifi_ap();
        start_control_server();
    }
    else
    {
        
        start_wifi_ap();
        //init_sockets();
        
        if(app.WIFI_SETUP){
            
            if(!app.START_WIFI){
                start_wifi_sta();
            }
        }

        //app.openStream("OFFLINE", "ONLINE");

        start_control_server();

        
    }
    
    
}




















Application APP;
Application app;