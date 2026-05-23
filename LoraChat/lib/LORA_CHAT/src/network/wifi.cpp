#include "wifi.h"
#include <stdlib.h>
#include "Application.h"

AsyncWebServer server(80);
AsyncWebServer production_server(80);

AsyncWebServer  Control_server(81); /* To control the device after setup */
static AsyncWebSocket ControlServerSockets("/device_control_ws");
static AsyncEventSource ControlServerEvents("/device_control_events");

String command                 = "";

typedef struct {
    uint32_t client_id;
    size_t data_len;
    void *data;
    void *handle;
} sockets_incom_message_t;

wifi::wifi(/* args */){}
wifi::~wifi(){}

void addCrossHeader(){
    
    const char *link = "";

   
       
    link = "http://192.168.4.1"; //"10.10.10.11";
    

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); /* Must add this header to prevent javascript errors if used remote host "192.168.4.1" */
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, X-Amz-Date, Authorization, X-Api-Key, X-Amz-Security-Token"); /* Must add this header to prevent javascript errors if used remote host */
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"); /* Must add this header to prevent javascript errors if used remote host */
    //DefaultHeaders::Instance().addHeader("Access-Control-Expose-Headers", "STREAM_FLAG");
}

void socketEvents(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

    String response;
    String _lora_connected;

    if(type == WS_EVT_CONNECT){

        app.APP_CONNECTED     = true;
        app.APP_CONNECTED_STR = "ONLINE";

        if(app.LORA_CONNECTED){
           app.LORA_CONNECTED_STR = "ONLINE";
        }
        else
        {
            app.LORA_CONNECTED_STR = "ONLINE";
        }

        //app.sendAppStatus("ONLINE", _lora_connected);

        app.SOC_REMOTE_IP = client->remoteIP().toString();

        //client connected
        printf("\nWS: [CLIENT CONNECTED] - URL [%s] - CLIENT ID [%u] - CLIENT IP [%s]\n", server->url(), client->id(), app.SOC_REMOTE_IP);

        //app.send_event("Sockets Connected", "sockets_connect", client->id());

        

    }
    else if(type == WS_EVT_DISCONNECT){

        app.APP_CONNECTED     = false;
        app.APP_CONNECTED_STR = "OFFLINE";

        if(app.LORA_CONNECTED){
            app.LORA_CONNECTED_STR = "ONLINE";
        }
        else
        {
            app.LORA_CONNECTED_STR = "OFFLINE";
        }

        // Send broad cast
        //app.sendAppStatus("OFFLINE", _lora_connected);

    }
    else if(type == WS_EVT_PONG){

    }
    else if(type == WS_EVT_DATA){

        //data packet
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        
        if(info->final && info->index == 0 && info->len == len){
           
            if(info->opcode == WS_TEXT){
                
                String message = (char *) data;
                
                printf("\nWS: [GOT MESSAGE] - URL [%s] - CLIENT ID [%u]\n", server->url(), client->id());

                JsonDocument doc;
                JsonDocument array;
                JsonDocument data_doc;
                String QUESTION            = "--";
                String ACK                 = "--";
                String TYPE                = "--";
                String SENDER_ID           = "--";
                String RECEIVER_ID         = "--";
                String RECEIVER_DATA       = "--";
                String TIME                = "--";
                String MESSAGE             = "--";

                deserializeJson(doc, message);

                QUESTION       = doc["QUESTION"].as<String>();
                ACK            = doc["ACK"].as<String>();
                TYPE           = doc["TYPE"].as<String>();
                SENDER_ID      = doc["SENDER_ID"].as<String>();
                TIME           = doc["TIME"].as<String>();

                if(QUESTION.equals("GET_STATUS")){

                    if(TYPE.equals("LORA_TYPE_DEVICE_STATUS")){

                        if(SENDER_ID.equals(app.DEVICE_ID)){

                            RECEIVER_DATA    = doc["RECEIVER_DATA"].as<String>();

                            DeserializationError error = deserializeJson(data_doc, RECEIVER_DATA);

                            if (error) {
                                Serial.print(F("[NODES.JSON FILE] -> deserializeJson() failed: "));
                                Serial.println(error.f_str());
                                return;
                            }

                            JsonVariant data_array = data_doc["DATA"];
                            int array_size         = data_array.size();

                            for(int i = 0; i < array_size; i++){

                                Serial.println("\nSENDING APP STATUS TO - NAME: [" + data_array[i]["NAME"].as<String>() + "] - ID: [" +  data_array[i]["ID"].as<String>() + "]");
                                
                                app.getAppStatus();                                
                                delay(200);

                            }

                        }

                    }

                }
                
                if(QUESTION.equals("SEND_MESSAGE")){

                    RECEIVER_ID    = doc["RECEIVER_ID"].as<String>();
                    MESSAGE        = doc["MESSAGE"].as<String>();
                    
                    // Get data from application, send it by lora to other side
                    if(app.sendMessage(doc) != LORA_SEND_FAIL){
                        response = "{";
                        response += "\"SOCKETS\":";
                        response += "{\"RESPONSE\":\"CHAT_MESSAGE_SENT_OK\"";
                        response += ",\"MESSAGE\":\"Your message sent successfully\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"SOCKETS\":";
                        response += "{\"RESPONSE\":\"CHAT_MESSAGE_SENT_KO\"";
                        response += ",\"MESSAGE\":\"Failed while sending message\"}";
                        response += "}";
                    }

                    client->text(response);

                    Serial.println("\nData received from APPLICATION:\n");
                    Serial.print("QUESTION:    ");Serial.println(QUESTION);
                    Serial.print("ACK     :    ");Serial.println(ACK);
                    Serial.print("TYPE    :    ");Serial.println(TYPE);
                    Serial.print("SENDER_ID:   ");Serial.println(SENDER_ID);
                    Serial.print("RECEIVER_ID: ");Serial.println(RECEIVER_ID);
                    Serial.print("TIME:        ");Serial.println(TIME);
                    Serial.print("MESSAGE:     ");Serial.println(MESSAGE);
                    Serial.println();
                }  

                
            }
        }
    }
    else
    {

    }
}

void start_wifi_ap(){

    Serial.println("\nStart wifi accesspoint");

    if(!WiFi.softAP(app.AP_SSID, app. AP_PASS, app.AP_CHANNEL, app.AP_SSID_HIDDEN, app.AP_MAX_CLIENTS)){
        
        Serial.println("WIFI AP - ERROR");
        return;
        
    }

    app.AP_IP = WiFi.softAPIP().toString();
    app.setString("AP_IP", app.AP_IP.c_str());

    app.AP_MAC = WiFi.softAPmacAddress();
    app.setString("AP_MAC", app.AP_MAC.c_str());
    
    Serial.print("\nAP IP:  ");Serial.println(app.AP_IP);
    Serial.print("AP MAC: ");Serial.println(app.AP_MAC);
    Serial.println();
}

void start_wifi_sta(){

    Serial.println("\nStart wifi station");

    WiFi.begin(app.WIFI_SSID, app.WIFI_PASS, 1);

    while(WiFi.status() != WL_CONNECTED){
        Serial.print("-");
        delay(500);
    }

    Serial.print("\n Connected to: ");Serial.println(app.WIFI_SSID);

    app.WIFI_IP = WiFi.localIP().toString();
    app.setString("WIFI_IP", app.WIFI_IP.c_str());

    app.WIFI_MAC = WiFi.macAddress();
    app.setString("WIFI_MAC", app.WIFI_MAC.c_str());

    Serial.print("\nWIFI IP:  ");Serial.println(app.WIFI_IP);
    Serial.print("WIFI MAC: ");Serial.println(app.WIFI_MAC);
    Serial.println();



}

void onEventsCallback(WebsocketsEvent event, String data) {
  
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } 
    else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } 
    else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } 
    else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

void onMessageCallback(WebsocketsMessage message) {

    command   = "";
    
    Serial.print("Got Message: ");
    Serial.println(message.data());

    JsonDocument soc_doc;

    command = String(message.data().c_str());

    deserializeJson(soc_doc, command);

    //f_size    = soc_doc["SOCKETS"]["FRAME_SIZE"].as<String>(); //if(f_size.equals("QVGA"))
    //f_quality = soc_doc["SOCKETS"]["QUELITY"].as<String>();


}

bool init_sockets(){

    while (!app.sockets_client.connect(app.websockets_host, app.websockets_port, "/")) {
		delay(500);
		Serial.print(".");
        return false;
	}

    Serial.println("Socket Connected!");

	// run callback when events are occuring
	app.sockets_client.onEvent(onEventsCallback);

	// run callback when messages are received
	app.sockets_client.onMessage(onMessageCallback);

    return true;

}

void start_setup_server(){

    Serial.println("\nSetup server init");
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.ico");
    });

    server.on("/images/logo192.png", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/images/logo192.png");
    });

    server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
    });
 
    server.on("/src/jquery-3.7.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-3.7.1.min.js", "text/javascript");
    });

    server.on("/src/jquery-ui.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-ui.min.js", "text/javascript");
    });

    server.on("/src/jquery-ui.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-ui.min.css", "text/css");
    });
    
    server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
    });

    server.on("/get_setup", HTTP_POST, [](AsyncWebServerRequest *request){

        String response = "";

        if (request->hasParam("QUESTION", true)){

            AsyncWebParameter *p_question = request->getParam("QUESTION", true);
            
        String question = p_question->value();
        
            if(question.equals("GET_SETUP")){

                response = "{";
                response += "\"SETUP_STATUS\":\"" + String(app.SETUP_OK) + "\"";
                response += ",\"AP_IP\":\"" + app.AP_IP + "\"";
                response += ",\"AP_MAC\":\"" + app.AP_MAC + "\"";
                response += ",\"RESPONSE\":\"RESPONSE_OK\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
            else
            {

                response = "{";
                response += "\"SETUP_STATUS\":\"QUESTION_NOT_SETUP\"";
                response += ",\"RESPONSE\":\"RESPONSE_KO\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
        }
        else
        {

            response = "{";
            response += "\"SETUP_STATUS\":\"NO_QUESTION_PARAM\"";
            response += ",\"RESPONSE\":\"RESPONSE_KO\"";
            response += "}";

            request->send(200, "text/plain", response);
        }
    });

    server.on("/get_settings", HTTP_POST, [](AsyncWebServerRequest *request){

        String response = "";

        if (request->hasParam("QUESTION", true)){

            AsyncWebParameter *p_question = request->getParam("QUESTION", true);
            
            String question = p_question->value();
        
            if(question.equals("GET_SETTINGS")){

                response = "{";
                response += "\"SETTINGS_STATUS\":\"1\"";
                response += ",\"AP_SSID\":\"" + app.AP_SSID + "\"";
                response += ",\"AP_PASS\":\"" + app.AP_PASS + "\"";
                response += ",\"RESPONSE\":\"RESPONSE_OK\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
            else
            {

                response = "{";
                response += "\"SETTINGS_STATUS\":\"QUESTION_NOT_SETTINGS\"";
                response += ",\"RESPONSE\":\"RESPONSE_KO\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
        }
        else
        {

            response = "{";
            response += "\"SETTINGS_STATUS\":\"NO_QUESTION_PARAM\"";
            response += ",\"RESPONSE\":\"RESPONSE_KO\"";
            response += "}";

            request->send(200, "text/plain", response);
        }

    });

    server.on("/do_setup", HTTP_POST, [](AsyncWebServerRequest *request){

        String response = "";

        if (request->hasParam("QUESTION", true)){

            AsyncWebParameter *p_question = request->getParam("QUESTION", true);
        String question                = p_question->value();
        
            if(question.equals("DO_SETUP")){

                
                AsyncWebParameter *p_w_setup = request->getParam("WIFI_SETUP", true);
                String w_setup               = p_w_setup->value();

                if(w_setup.equals("1")){

                    Serial.println("Setup device with wifi option");

                    AsyncWebParameter *p_ap_ssid   = request->getParam("AP_SSID", true);
                    AsyncWebParameter *p_ap_pass   = request->getParam("AP_PASS", true);
                    AsyncWebParameter *p_wifi_ssid = request->getParam("WIFI_SSID", true);
                    AsyncWebParameter *p_wifi_pass = request->getParam("WIFI_PASS", true);

                    app.AP_SSID                    = p_ap_ssid->value();
                    app.AP_PASS                    = p_ap_pass->value();
                    app.WIFI_SSID                  = p_wifi_ssid->value();
                    app.WIFI_PASS                  = p_wifi_pass->value();
                    
                    app.setString("AP_SSID", app.AP_SSID.c_str());
                    app.setString("AP_PASS", app.AP_PASS.c_str());
                    app.setBool("WIFI_SETUP", true);
                    app.setString("WIFI_SSID", app.WIFI_SSID.c_str());
                    app.setString("WIFI_PASS", app.WIFI_PASS.c_str());
                    
                    response = "{";
                    response += "\"SETUP_STATUS\":\"" + String(true) + "\"";
                    response += ",\"WIFI_SETUP\":\"" + String(true) + "\"";
                    response += ",\"RESPONSE\":\"RESPONSE_OK\"";
                    response += "}";

                    request->send(200, "text/plain", response);

                    app.setBool("SETUP_OK", true);

                    ESP.restart();
                }
                else
                {
                    Serial.println("Setup device without wifi option");

                    AsyncWebParameter *p_ap_ssid   = request->getParam("AP_SSID", true);
                    AsyncWebParameter *p_ap_pass   = request->getParam("AP_PASS", true);

                    app.AP_SSID                    = p_ap_ssid->value();
                    app.AP_PASS                    = p_ap_pass->value();

                    app.setString("AP_SSID", app.AP_SSID.c_str());
                    app.setString("AP_PASS", app.AP_PASS.c_str());
                    app.setBool("WIFI_SETUP", false);
                    
                    response = "{";
                    response += "\"SETUP_STATUS\":\"" + String(true) + "\"";
                    response += ",\"WIFI_SETUP\":\"" + String(false) + "\"";
                    response += ",\"RESPONSE\":\"RESPONSE_OK\"";
                    response += "}";

                    request->send(200, "text/plain", response);

                    app.setBool("SETUP_OK", true);

                    ESP.restart();
                }
            }
            else
            {

                response = "{";
                response += "\"SETUP_STATUS\":\"QUESTION_NOT_SETUP\"";
                response += ",\"RESPONSE\":\"RESPONSE_KO\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
        }
        else
        {

            response = "{";
            response += "\"SETUP_STATUS\":\"NO_QUESTION_PARAM\"";
            response += ",\"RESPONSE\":\"RESPONSE_KO\"";
            response += "}";

            request->send(200, "text/plain", response);
        }
    });
            
    server.begin();
}

void start_production_server(){

    Serial.println("\nProduction server init");
    
    production_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/chat.html", "text/html");
    });

    production_server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.ico");
    });

    production_server.on("/images/logo192.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/images/logo192.png");
    });

    production_server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
    });
 
    production_server.on("/src/jquery-3.7.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-3.7.1.min.js", "text/javascript");
    });

    production_server.on("/src/jquery-ui.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-ui.min.js", "text/javascript");
    });

    production_server.on("/src/jquery-ui.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/jquery-ui.min.css", "text/css");
    });
    
    production_server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
    });

    production_server.on("/get_settings", HTTP_POST, [](AsyncWebServerRequest *request){

        String response = "";

        if (request->hasParam("QUESTION", true)){

            AsyncWebParameter *p_question = request->getParam("QUESTION", true);
            
        String question = p_question->value();
        
            if(question.equals("GET_SETTINGS")){

                response = "{";
                response += "\"SETTINGS_STATUS\":\"1\"";
                response += ",\"AP_SSID\":\"" + app.AP_SSID + "\"";
                response += ",\"AP_PASS\":\"" + app.AP_PASS + "\"";
                response += ",\"RESPONSE\":\"RESPONSE_OK\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
            else
            {

                response = "{";
                response += "\"SETTINGS_STATUS\":\"QUESTION_NOT_SETTINGS\"";
                response += ",\"RESPONSE\":\"RESPONSE_KO\"";
                response += "}";

                request->send(200, "text/plain", response);
            }
        }
        else
        {

            response = "{";
            response += "\"SETTINGS_STATUS\":\"NO_QUESTION_PARAM\"";
            response += ",\"RESPONSE\":\"RESPONSE_KO\"";
            response += "}";

            request->send(200, "text/plain", response);
        }

    });

            
    production_server.begin();

}

void start_control_server(){

    ControlServerSockets.onEvent(socketEvents);
    Control_server.addHandler(&ControlServerSockets);
    Control_server.addHandler(&ControlServerEvents);

    AsyncCallbackJsonWebHandler *setup_needed = new AsyncCallbackJsonWebHandler("/setup_needed", [](AsyncWebServerRequest *request, JsonVariant &json) {

        JsonDocument data;

        if (json.is<JsonArray>()){
            data = json.as<JsonArray>();
        }
        else if (json.is<JsonObject>())
        {
            data = json.as<JsonObject>();
        }

        if(data.size() > 0){

            String QUESTION        = data["QUESTION"].as<String>();
            String SECURITY        = data["SECURITY"].as<String>();
            String APP_ID          = data["APP_ID"].as<String>();

            Serial.print("\nQUESTION: ");Serial.print(QUESTION);Serial.println("|");
            Serial.print("\nSECURITY: ");Serial.print(SECURITY);Serial.println("|"); // == GRT_SECURITY
            Serial.print("\nAPP_ID:   ");Serial.print(APP_ID);Serial.println("|");

            if(QUESTION.equals("IS_SETUP_NEEDED")){

                String response;

                if(SECURITY.equals("GET_SECURITY")){

                    app.SETUP_OK     = app.getBool("SETUP_OK");
                    app.APP_ID       = app.getString("APP_ID");

                    Serial.print("\nSETUP OK ON DEVICE:             ");Serial.print(app.SETUP_OK);Serial.println("|");
                    Serial.print("\nAPP ID ON DEVICE:               ");Serial.print(app.APP_ID);Serial.println("|");

                    response = "{";
                    response += "\"RESPONSE\":";
                    response += "{\"STATUS\":\"SETUP_OK_CHECK\"";
                    response += ",\"SETUP_OK\":\"" + String(app.SETUP_OK) + "\"";
                    response += ",\"DEFAULT_APP_ID\":\"" + app.APP_ID + "\"}";
                    response += "}";

                    request->send(200, "text/plain", response);

                }
                else
                {
                    response = "{";
                    response += "\"RESPONSE\":";
                    response += "{\"STATUS\":\"SECURITY_ERROR\"";
                    response += ",\"MESSAGE\":\"The security variable comes from app is not the same of this node.\"}";
                    response += "}";

                    request->send(200, "text/plain", response);
                }  

                Serial.print("response: ");Serial.print(response);Serial.println("|");
            }
        }
    });

    AsyncCallbackJsonWebHandler *server_network = new AsyncCallbackJsonWebHandler("/device_network", [](AsyncWebServerRequest *request, JsonVariant &json) {

        JsonDocument data;

        if (json.is<JsonArray>()){
            data = json.as<JsonArray>();
        }
        else if (json.is<JsonObject>())
        {
            data = json.as<JsonObject>();
        }

        if(data.size() > 0){

            String APP_ID         = data["APP_ID"].as<String>(); // final APP ID that will be saved if match passed
            String QUESTION       = data["QUESTION"].as<String>();
            String QUESTION_TYPE  = data["QUESTION_TYPE"].as<String>();

            Serial.print("APP_ID:             ");Serial.print(APP_ID);Serial.println("|");
            Serial.print("QUESTION:           ");Serial.print(QUESTION);Serial.println("|");
            
            bool IS_SETUP_OK    = app.getBool("SETUP_OK");
            bool W_SETUP        = false; // wifi setup made or not
            
            String response;

            if(QUESTION_TYPE.equals("ACCESSPOINT")){

                if(QUESTION.equals("SETUP_ACCESSPOINT")){

                    Serial.println("\n START SETUP ACCESSPOINT \n");

                    String DEFAULT_APP_ID = data["DEFAULT_APP_ID"].as<String>(); // default APP ID to match for setup porposes

                    Serial.print("\n APP_ID FROM APP:    ");Serial.println(DEFAULT_APP_ID);
                    Serial.print("\n APP_ID FROM DEVICE: ");Serial.println(APP_ID);

                    if(!DEFAULT_APP_ID.equals(app.APP_ID)){ // if DEFAULT_APP_ID comming from mobile app != app.APP_ID (default APP Ip saved on flash for setup), then return error to mobile app

                        Serial.println("\n ERROR [DEVICE_SETUP_APP_ID_KO]: APP ID ON DEVICE AND APP ID FROM APP NOT EQUAL \n");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"DEVICE_SETUP_APP_ID_KO\"";
                        response += ",\"MESSAGE\":\"Your device do not have the permiession to connect to this server.\"}";
                        response += "}";

                        request->send(200, "text/plain", response);
                    }
                    else// if DEFAULT_APP_ID comming from mobile app == app.APP_ID (default APP Ip saved on flash for setup), then save the final APP ID comming from mobile app
                    {
                        Serial.println("\n SAVE APP ID FROM APP TO DEVICE MEMORY \n");

                        app.setString("APP_ID", APP_ID.c_str()); // save final APP ID

                        app.APP_ID = app.getString("APP_ID"); // get final APP ID from flash to resend it to mobile app to save it in DB

                        Serial.print("\n SAVED APP ID: ");Serial.println(app.APP_ID);

                        String AP_SSID     = data["AP_SSID"].as<String>();
                        String AP_PASS     = data["AP_PASS"].as<String>();

                        Serial.print("AP_SSID: ");Serial.print(AP_SSID);Serial.println("|");
                        Serial.print("AP_PASS: ");Serial.print(AP_PASS);Serial.println("|");
                        
                        if(!IS_SETUP_OK){ // If setup needed SETUP_OK = false

                            if(app.setString("AP_SSID", AP_SSID.c_str()) && app.setString("AP_PASS", AP_PASS.c_str())){
                                
                                app.AP_SSID       = app.getString("AP_SSID");
                                app.AP_PASS       = app.getString("AP_PASS");
                                app.AP_IP         = app.getString("AP_IP");
                                app.AP_MAC        = app.getString("AP_MAC");
                                app.DEVICE_ID     = app.getString("DEVICE_ID");

                                Serial.print("Saved access point SSID: ");Serial.println(app.AP_SSID);
                                Serial.print("Saved access point PASS: ");Serial.println(app.AP_PASS);
                                Serial.print("Saved AP_IP:             ");Serial.println(app.AP_IP);
                                Serial.print("Saved AP_MAC:            ");Serial.println(app.AP_MAC);

                                response = "{";
                                response += "\"RESPONSE\":";
                                response += "{\"STATUS\":\"SETUP_ACCESS_POINT_OK\"";
                                response += ",\"SETUP_OK\":\"" + String(app.SETUP_OK) + "\"";
                                response += ",\"DEVICE_ID\":\"" + app.DEVICE_ID + "\"";
                                response += ",\"AP_SSID\":\"" + app.AP_SSID + "\"";
                                response += ",\"AP_PASS\":\"" + app.AP_PASS + "\"";
                                response += ",\"AP_IP\":\"" + app.AP_IP + "\"";
                                response += ",\"AP_MAC\":\"" + app.AP_MAC + "\"";
                                response += ",\"MESSAGE\":\"The device network settings upated successfully.\"}";
                                response += "}";
                                
                                request->send(200, "text/plain", response);  
                            }
                            else
                            {

                                if(!app.setString("AP_SSID", AP_SSID.c_str())){

                                    response = "{";
                                    response += "\"RESPONSE\":";
                                    response += "{\"STATUS\":\"DEVICE_SETUP_AP_SSID_KO\"";
                                    response += ",\"MESSAGE\":\"Error while saving device access point PASS.\"}";
                                    response += "}";
                                    request->send(200, "text/plain", response);

                                }
                            
                                if(!app.setString("AP_PASS", AP_PASS.c_str())){

                                    response = "{";
                                    response += "\"RESPONSE\":";
                                    response += "{\"STATUS\":\"DEVICE_SETUP_AP_PASS_KO\"";
                                    response += ",\"MESSAGE\":\"Error while saving device access point PASS.\"}";
                                    response += "}";
                                    request->send(200, "text/plain", response);

                                }
                            }
                        }
                    }
                }
            }

            if(QUESTION_TYPE.equals("PREFS")){

                if(QUESTION.equals("SETUP_DEVICE_PREFS")){

                    if(!APP_ID.equals(app.APP_ID)){

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"DEVICE_SETUP_APP_ID_KO\"";
                        response += ",\"MESSAGE\":\"Your device do not have the permiession to setup this application.\"}";
                        response += "}";

                        request->send(200, "text/plain", response);
                    }
                    else
                    {
    
                        if(!IS_SETUP_OK){ // If setup needed

                            String ID              = data["ID"].as<String>();
                            String DEVICE_NAME     = data["DEVICE_NAME"].as<String>();
                            String AP_SSID         = data["AP_SSID"].as<String>();
                            String AP_PASS         = data["AP_PASS"].as<String>();
                            String AP_CHANNEL      = data["AP_CHANNEL"].as<String>();
                            String AP_HIDDEN       = data["AP_HIDDEN"].as<String>();
                            String AP_MAX_CLIENTS  = data["AP_MAX_CLIENTS"].as<String>();
                            String WIFI_SSID       = data["WIFI_SSID"].as<String>();
                            String WIFI_PASS       = data["WIFI_PASS"].as<String>();
                            //String LORA_ID         = data["LORA_ID"].as<String>();
                            String LORA_BAND       = data["LORA_BAND"].as<String>();
                            String LORA_RX_POWER   = data["LORA_RX_POWER"].as<String>();
                            String LORA_SYNC_WORD  = data["LORA_SYNC_WORD"].as<String>();
                            String LORA_SF         = data["LORA_SF"].as<String>();
                            String LORA_ENABLE_CRS = data["LORA_ENABLE_CRS"].as<String>();
                            String LORA_INVERT_IQ  = data["LORA_ENABLE_INVERT_IQ"].as<String>();

                            if(WIFI_SSID.equals("EMPTY") && WIFI_PASS.equals("EMPTY")){
                                app.WIFI_SETUP = false;
                                app.START_WIFI = false;
                                app.WIFI_IP    = "EMPTY";
                                app.WIFI_DNS_1 = "EMPTY";
                                app.WIFI_DNS_2 = "EMPTY";
                            }

                            if(LORA_BAND.equals("868E6")){
                                app.LORA_BAND = 868E6;
                            }

                            if(LORA_BAND.equals("433E6")){
                                app.LORA_BAND = 433E6;
                            }

                            if(LORA_BAND.equals("915E6")){
                                app.LORA_BAND = 915E6;
                            }

                            if(LORA_ENABLE_CRS == "0"){
                                app.LORA_ENABLE_CRS = false;
                            }
                            else
                            {
                                app.LORA_ENABLE_CRS = true;
                            }

                            if(LORA_INVERT_IQ == "0"){
                                app.LORA_ENABLE_INVERT_IQ = false;
                            }
                            else
                            {
                                app.LORA_ENABLE_INVERT_IQ = true;
                            }
                            
                            
                            Serial.print("ID:          ");Serial.print(ID);Serial.println("|");
                            Serial.print("DEVICE_NAME: ");Serial.print(DEVICE_NAME);Serial.println("|");
                            Serial.print("AP_PASS:     ");Serial.print(AP_PASS);Serial.println("|");

                            if(app.set_uint8("ID", ID.toInt()) && 
                                app.setString("DEVICE_NAME", DEVICE_NAME.c_str()) && 
                                app.setString("AP_SSID", AP_SSID.c_str()) &&
                                app.setString("AP_PASS", AP_PASS.c_str()) &&
                                app.setInt("AP_CHANNEL", AP_CHANNEL.toInt()) && 
                                app.setInt("AP_MAX_CLIENT", AP_MAX_CLIENTS.toInt()) && 
                                app.setInt("AP_SSID_HIDDEN", AP_HIDDEN.toInt()) && 
                                app.setBool("WIFI_SETUP", app.WIFI_SETUP) &&
                                app.setBool("START_WIFI", app.START_WIFI) &&
                                app.setString("WIFI_SSID", WIFI_SSID.c_str()) &&
                                app.setString("WIFI_PASS", WIFI_PASS.c_str()) &&
                                app.setString("WIFI_IP",  app.WIFI_IP.c_str()) &&
                                app.setString("WIFI_DNS_1",  app.WIFI_DNS_1.c_str()) &&
                                app.setString("WIFI_DNS_2",  app.WIFI_DNS_2.c_str()) &&
                                //app.setString("DEVICE_ID", LORA_ID.c_str()) &&
                                app.set_double("LORA_BAND", app.LORA_BAND) &&
                                app.set_int8("LORA_TX_POWER", LORA_RX_POWER.toInt()) &&
                                app.setString("LORA_SYNC_WORD", LORA_SYNC_WORD.c_str()) &&
                                app.setInt("LORA_SF", LORA_SF.toInt()) &&
                                app.setBool("LORA_ENABLE_CRS", app.LORA_ENABLE_CRS) &&
                                app.setBool("LORA_ENABLE_INVERT_IQ", app.LORA_ENABLE_INVERT_IQ)){

                                if(app.setBool("SETUP_OK", true)){

                                    app.SETUP_OK                = app.getBool("SETUP_OK");
                                    app.APP_ID                  = app.getString("APP_ID");
                                    app.APP_VERSION             = app.getString("APP_VERSION");
                                    app.ID                      = app.get_uint8("ID");
                                    app.DEVICE_ID               = app.getString("DEVICE_ID"); // LORA ID
                                    app.DEVICE_NAME             = app.getString("DEVICE_NAME");
                                    app.AP_SSID                 = app.getString("AP_SSID");
                                    app.AP_PASS                 = app.getString("AP_PASS");
                                    app.AP_PASS                 = app.getString("AP_PASS");
                                    app.AP_MAC                  = app.getString("AP_MAC");
                                    app.AP_IP                   = app.getString("AP_IP");
                                    app.AP_HOSTNAME             = app.getString("AP_HOSTNAME");
                                    app.AP_SSID_HIDDEN          = app.getInt("AP_SSID_HIDDEN");
                                    app.AP_CHANNEL              = app.getInt("AP_CHANNEL");
                                    app.AP_MAX_CLIENTS          = app.getInt("AP_MAX_CLIENTS");
                                    app.WIFI_SETUP              = app.getBool("WIFI_SETUP");
                                    app.START_WIFI              = app.getBool("START_WIFI");
                                    app.WIFI_SSID               = app.getString("WIFI_SSID");
                                    app.WIFI_PASS               = app.getString("WIFI_PASS");
                                    app.WIFI_MAC                = app.getString("WIFI_MAC");
                                    app.WIFI_IP                 = app.getString("WIFI_IP");
                                    app.WIFI_DNS_1              = app.getString("WIFI_DNS_1");
                                    app.WIFI_DNS_2              = app.getString("WIFI_DNS_2");
                                    app.LORA_BAND               = app.get_double("LORA_BAND");
                                    app.LORA_TX_POWER           = app.get_int8("LORA_TX_POWER");
                                    app.LORA_SYNC_WORD          = app.getString("LORA_SYNC_WORD");
                                    app.LORA_SF                 = app.getInt("LORA_SF");
                                    app.LORA_ENABLE_CRS         = app.getBool("LORA_ENABLE_CRS");
                                    app.LORA_ENABLE_INVERT_IQ   = app.getBool("LORA_ENABLE_INVERT_IQ");

                                    app.end_spiffs();

                                    response = "{";
                                    response += "\"RESPONSE\":";
                                    response += "{\"STATUS\":\"DEVICE_SETUP_OK\"";
                                    response += ",\"SETUP_OK\":\"" + String(app.SETUP_OK) + "\"";
                                    response += ",\"APP_ID\":\"" + app.APP_ID  + "\"";
                                    response += ",\"APP_VERSION\":\"" + app.APP_VERSION  + "\"";
                                    response += ",\"ID\":\"" + String(app.ID) + "\"";
                                    response += ",\"DEVICE_ID\":\"" + app.DEVICE_ID  + "\"";
                                    response += ",\"DEVICE_NAME\":\"" + app.DEVICE_NAME  + "\"";
                                    response += ",\"AP_SSID\":\"" + app.AP_SSID + "\"";
                                    response += ",\"AP_PASS\":\"" + app.AP_PASS + "\"";
                                    response += ",\"AP_MAC\":\"" + app.AP_MAC + "\"";
                                    response += ",\"AP_IP\":\"" + app.AP_IP + "\"";
                                    response += ",\"AP_HOSTNAME\":\"" + app.AP_HOSTNAME + "\"";
                                    response += ",\"AP_SSID_HIDDEN\":\"" + String(app.AP_SSID_HIDDEN) + "\"";
                                    response += ",\"AP_CHANNEL\":\"" + String(app.AP_CHANNEL) + "\"";
                                    response += ",\"AP_MAX_CLIENTS\":\"" + String(app.AP_MAX_CLIENTS) + "\"";
                                    response += ",\"WIFI_SETUP\":\"" + String(app.WIFI_SETUP) + "\"";
                                    response += ",\"START_WIFI\":\"" + String(app.START_WIFI) + "\"";
                                    response += ",\"WIFI_SSID\":\"" + app.WIFI_SSID + "\"";
                                    response += ",\"WIFI_PASS\":\"" + app.WIFI_PASS + "\"";
                                    response += ",\"WIFI_MAC\":\"" + app.WIFI_MAC + "\"";
                                    response += ",\"WIFI_IP\":\"" + app.WIFI_IP + "\"";
                                    response += ",\"WIFI_DNS_1\":\"" + app.WIFI_DNS_1 + "\"";
                                    response += ",\"WIFI_DNS_2\":\"" + app.WIFI_DNS_2 + "\"";
                                    response += ",\"LORA_BAND\":\"" + String(app.LORA_BAND) + "\"";
                                    response += ",\"LORA_TX_POWER\":\"" + String(app.LORA_TX_POWER) + "\"";
                                    response += ",\"LORA_SYNC_WORD\":\"" + app.LORA_SYNC_WORD + "\"";
                                    response += ",\"LORA_SF\":\"" + String(app.LORA_SF) + "\"";
                                    response += ",\"LORA_ENABLE_CRS\":\"" + String(app.LORA_ENABLE_CRS) + "\"";
                                    response += ",\"LORA_ENABLE_INVERT_IQ\":\"" + String(app.LORA_ENABLE_INVERT_IQ) + "\"";
                                    response += ",\"MESSAGE\":\"The device network settings upated successfully. Device will restart in (10) seconds\"}";
                                    response += "}";

                                    request->send(200, "text/plain", response); 

                                    Serial.print("Saved id:         ");Serial.println(app.ID);
                                    Serial.print("Saved D id:       ");Serial.println(app.DEVICE_ID);
                                    Serial.print("Saved name:       ");Serial.println(app.DEVICE_NAME);
                                    Serial.print("Saved AP PASS:    ");Serial.println(app.AP_PASS);

                                    Serial.println("DEVICE SETUP [OK] -> Restart after 3000 Ms");
                                    delay(3000);

                                    ESP.restart();

                                }
                                else
                                {
                                    response = "{";
                                    response += "\"RESPONSE\":";
                                    response += "{\"STATUS\":\"DEVICE_SETUP_SAVE_SETTING_KO\"";
                                    response += ",\"MESSAGE\":\"Error while saving setting in device memory.\"}";
                                    response += "}";

                                    request->send(200, "text/plain", response);
                                }
                            }
                        }
                    }
                }
            }
        }
    });

    AsyncCallbackJsonWebHandler *device_control = new AsyncCallbackJsonWebHandler("/device_control", [](AsyncWebServerRequest *request, JsonVariant &json) {

        JsonDocument data;

        if (json.is<JsonArray>()){
            data = json.as<JsonArray>();
        }
        else if (json.is<JsonObject>())
        {
            data = json.as<JsonObject>();
        }

        if(data.size() > 0){

            String QUESTION        = data["QUESTION"].as<String>();
            String response;
            
            if(QUESTION.equals("RESET_DEVICE")){

                String SECURITY        = data["SECURITY"].as<String>();
                String WHERE           = data["WHERE"].as<String>();

                Serial.printf("QUESTION FROM APPLICATION: QUESTION [%c] - SECURITY [%c] - WHERE [%c]\n" , QUESTION.c_str(), SECURITY.c_str(), WHERE.c_str());

                if(WHERE.equals("RESET_SETUP_OK")){

                    app.setBool("SETUP_OK", false);

                    response = "{";
                    response += "\"RESPONSE\":";
                    response += "{\"STATUS\":\"RESET_DEVICE_OK\"";
                    response += ",\"RESTART\":\" YES\"}";
                    response += "}";
                    request->send(200, "text/plain", response);

                    delay(2000);

                    ESP.restart();
                }

                if(WHERE.equals("APP_ID_KO")){

                    if(app.setString("APP_ID", SECURITY.c_str())){

                        app.APP_ID = app.getString("APP_ID");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_OK\"";
                        response += ",\"RESTART\":\" NO\"}";
                        response += "}";
                        
                        request->send(200, "text/plain", response);
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_KO\"";
                        response += ",\"RESTART\":\" NO\"}";
                        response += "}";

                        request->send(200, "text/plain", response);
                    }
                }
                
                if(WHERE.equals("NAME_DUPLICATED")){

                    if(app.setString("DEVICE_NAME", "EMPTY")){

                        app.DEVICE_NAME = app.getString("DEVICE_NAME");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_OK\"";
                        response += ",\"RESTART\":\" NO\"}";
                        response += "}";

                        request->send(200, "text/plain", response);
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_KO\"";
                        response += ",\"RESTART\":\" NO\"}";
                        response += "}";

                        request->send(200, "text/plain", response);
                    }
                }

                if(WHERE.equals("ALL_RESET")){

                    if(app.resetConfig()){

                        app.end_spiffs();
                        
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_OK\"";
                        response += ",\"RESTART\":\" NO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);

                        Serial.println("Reset device where [ALL_RESET] passed successfully, Restart after 5000 Ms");

                        delay(5000);

                        ESP.restart();
                        
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"STATUS\":\"RESET_DEVICE_KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);

                        Serial.println("Reset device where [ALL_RESET] failed!, Try again");
                    }
                }
            }
        
            if(QUESTION.equals("RESTART_NETWORK")){
                
                response = "{";
                response += "\"RESPONSE\":";
                response += "{\"STATUS\":\"RESTART_NETWORK_OK\"";
                response += ",\"RESTART\":\" NO\"}";
                response += "}";
                request->send(200, "text/plain", response);
            }

            if(QUESTION.equals("UPDATE_SETTINGS")){

                String ACTION = data["UPDATE_ACTION"].as<String>();
                String VALUE  = data["UPDATE_VALUE"].as<String>();

                if(ACTION.equals("UPDATE_AP_SSID")){

                    String RESTART = data["RESTART_ACTION"].as<String>();

                    if(app.setString("AP_SSID", VALUE.c_str())){

                        app.AP_SSID = app.getString("AP_SSID");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_SSID\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";

                        if(RESTART.equals("RESTART_NOW")){
                            request->send(200, "text/plain", response);
                            delay(3000);
                            ESP.restart();
                        }
                        else
                        {
                            request->send(200, "text/plain", response);
                        }
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_SSID\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);
                    }   
                }

                if(ACTION.equals("UPDATE_AP_IP")){

                    String RESTART = data["RESTART_ACTION"].as<String>();

                    if(app.setString("AP_IP", VALUE.c_str())){

                        app.AP_IP = app.getString("AP_IP");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_IP\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";

                        if(RESTART.equals("RESTART_NOW")){
                            request->send(200, "text/plain", response);
                            delay(3000);
                            ESP.restart();
                        }
                        else
                        {
                            request->send(200, "text/plain", response);
                        }
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_IP\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);
                    }
                }

                if(ACTION.equals("UPDATE_AP_HOSTNAME")){

                    String RESTART = data["RESTART_ACTION"].as<String>();

                    if(app.setString("AP_HOSTNAME", VALUE.c_str())){

                        app.AP_HOSTNAME = app.getString("AP_HOSTNAME");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_HOSTNAME\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";

                        if(RESTART.equals("RESTART_NOW")){
                            request->send(200, "text/plain", response);
                            delay(3000);
                            ESP.restart();
                        }
                        else
                        {
                            request->send(200, "text/plain", response);
                        }
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_HOSTNAME\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);
                    }
                }

                if(ACTION.equals("UPDATE_AP_CHANNEL")){

                    String RESTART = data["RESTART_ACTION"].as<String>();
                    
                    int chan = VALUE.toInt();

                    if(app.setInt("AP_CHANNEL", chan)){

                        app.AP_CHANNEL = app.getInt("AP_CHANNEL");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_CHANNEL\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";

                        if(RESTART.equals("RESTART_NOW")){
                            request->send(200, "text/plain", response);
                            delay(3000);
                            ESP.restart();
                        }
                        else
                        {
                            request->send(200, "text/plain", response);
                        }
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_CHANNEL\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);
                    }
                }

                if(ACTION.equals("UPDATE_AP_MAX_CLIENTS")){

                    String RESTART = data["RESTART_ACTION"].as<String>();

                    Serial.println("VALUE: " + VALUE + " RESTART: " + RESTART);
                    
                    int max = VALUE.toInt();

                    if(app.setInt("AP_MAX_CLIENTS", max)){

                        app.AP_MAX_CLIENTS = app.getInt("AP_MAX_CLIENTS");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_MAX_CLIENTS\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";

                        if(RESTART.equals("RESTART_NOW")){
                            request->send(200, "text/plain", response);
                            delay(3000);
                            ESP.restart();
                        }
                        else
                        {
                            request->send(200, "text/plain", response);
                        }
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_AP_MAX_CLIENTS\"";
                        response += ",\"RESTART\":\"" + RESTART + "\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                        request->send(200, "text/plain", response);
                    }
                }

                if(ACTION.equals("UPDATE_LORA_ID")){
                    
                    if(app.setString("DEVICE_ID", VALUE.c_str())){

                        app.AP_HOSTNAME = app.getString("DEVICE_ID");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_ID\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_ID\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                }

                if(ACTION.equals("UPDATE_LORA_BAND")){
                    
                    double _band;
                    
                    if(VALUE.equals("868")){
                        
                        _band = 868E6;

                        if(app.set_double("LORA_BAND", _band)){

                            app.LORA_BAND = app.get_double("LORA_BAND");

                            response = "{";
                            response += "\"RESPONSE\":";
                            response += "{\"ACTION\":\"UPDATE_LORA_BAND\"";
                            response += ",\"STATUS\":\"OK\"}";
                            response += "}";
                        }
                        else
                        {
                            response = "{";
                            response += "\"RESPONSE\":";
                            response += "{\"ACTION\":\"UPDATE_LORA_BAND\"";
                            response += ",\"STATUS\":\"KO\"}";
                            response += "}";
                        }

                        request->send(200, "text/plain", response);
                    }

                    if(VALUE.equals("915")){
                        
                        _band = 915E6;

                        if(app.set_double("LORA_BAND", _band)){

                            app.LORA_BAND = app.get_double("LORA_BAND");

                            response = "{";
                            response += "\"RESPONSE\":";
                            response += "{\"ACTION\":\"UPDATE_LORA_BAND\"";
                            response += ",\"STATUS\":\" OK\"}";
                            response += "}";
                        }
                        else
                        {
                            response = "{";
                            response += "\"RESPONSE\":";
                            response += "{\"ACTION\":\"UPDATE_LORA_BAND\"";
                            response += ",\"STATUS\":\" KO\"}";
                            response += "}";
                        }

                        request->send(200, "text/plain", response);
                    }
                }

                if(ACTION.equals("UPDATE_LORA_SF")){
                    
                    int sf = VALUE.toInt();

                    if(app.setInt("LORA_SF", sf)){

                        app.LORA_SF = app.getInt("LORA_SF");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_SF\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_SF\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                }

                if(ACTION.equals("UPDATE_LORA_SYNC_WORD")){
                    
                    if(app.setString("LORA_SYNC_WORD", VALUE.c_str())){

                        app.LORA_SYNC_WORD = app.getString("LORA_SYNC_WORD");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_SYNC_WORD\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_SYNC_WORD\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                }

                if(ACTION.equals("UPDATE_LORA_TX_POWER")){
                    
                    int power = VALUE.toInt();
                    
                    if(app.setInt("LORA_TX_POWER", power)){

                        app.LORA_TX_POWER = app.getInt("LORA_TX_POWER");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_TX_POWER\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_TX_POWER\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                }

                if(ACTION.equals("UPDATE_LORA_CRS_ENABLED")){

                    bool crs;
                    Serial.println("CRS BOOL: " +  VALUE);
                    
                    if(VALUE.equals("1")){
                        crs = true;
                    }
                    else
                    {
                        crs = false;
                    }

                    if(app.setBool("LORA_ENABLE_CRS", crs)){

                        app.LORA_ENABLE_CRS = app.getBool("LORA_ENABLE_CRS");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_CRS_ENABLED\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_CRS_ENABLED\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                    
                }

                if(ACTION.equals("UPDATE_LORA_INVERTED_IQ_ENABLED")){
                    
                    bool iq;
                    
                    if(VALUE.equals("1")){
                        iq = true;
                    }
                    else
                    {
                        iq = false;
                    }

                    if(app.setBool("LORA_ENABLE_INVERT_IQ", iq)){

                        app.LORA_ENABLE_INVERT_IQ = app.getBool("LORA_ENABLE_INVERT_IQ");

                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_INVERTED_IQ_ENABLED\"";
                        response += ",\"STATUS\":\"OK\"}";
                        response += "}";
                    }
                    else
                    {
                        response = "{";
                        response += "\"RESPONSE\":";
                        response += "{\"ACTION\":\"UPDATE_LORA_INVERTED_IQ_ENABLED\"";
                        response += ",\"STATUS\":\"KO\"}";
                        response += "}";
                    }

                    request->send(200, "text/plain", response);
                }
            }
        }
    });

    ControlServerEvents.onConnect([](AsyncEventSourceClient *client){
        
        if(client->lastId()){
            printf("\n Client reconnected! Last message ID that it got is: [%u] \n", client->lastId());
        }

        app.EVENT_CONNECTED = true;

        //send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("Event Reconnectd", NULL, millis(), 1000);
    });

    Control_server.addHandler(setup_needed);
    Control_server.addHandler(server_network);
    Control_server.addHandler(device_control);

    addCrossHeader();
    Control_server.begin();

}

t_event_send wifi::send_event(String message, String event, uint32_t id){

    if(app.EVENT_CONNECTED){
        ControlServerEvents.send(message.c_str(), event.c_str(), id);
        Serial.println("\n(send_event) - EVENT SEND ERROR CODE: EVENT_SEND_OK");
        return EVENT_SEND_OK;
    }
    else
    {
        Serial.println("\n(send_event) - EVENT SEND ERROR CODE: EVENT_SEND_NOT_CONNECTED");
        return EVENT_SEND_NOT_CONNECTED;
    }

    Serial.println("\n(send_event) - EVENT SEND ERROR CODE: EVENT_SEND_NOT_CONNECTED");
    return EVENT_SEND_KO;
}

void wifi::send_settings_to_server_calback(){
    
    Serial.println("\nSend settings variables to HTML page\n");

    //server.on("/", HTTP_POST,  attachNodesServerhHandle); /* for nodes attaching */

}

void socketServerListen(){
    app.soc_server.listen(8888);  
}

bool socketServerPoll(){
    return app.soc_server.poll();
}



