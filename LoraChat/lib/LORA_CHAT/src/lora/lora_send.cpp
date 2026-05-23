#include "lora_send.h"

#include "Application.h"
#include "heltec.h"

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6
unsigned int counter = 0;
TaskHandle_t V_LORA_STREAM_TASK;

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

String jsonData = "";

lora_frame_head_t *f_head;
static lora_frame_head_t g_lora_frame_head_default = LORA_FRAME_CONFIG_DEFAULT();

String APP_VER             = "--";
String CLIENT_ID           = "--";
String RSSI                = "--";
String SNR                 = "--";
String MSG_ID              = "--";

String ACTION              = "--";
String SENDER_ID           = "--";
String RECEIVER_ID         = "--";
String TIME                = "--";
JsonObject MESSAGE_OBJECT;
String MESSAGE             = "--";

String MSG_DELIVERY_STATUS = "--";
String response = "";

JsonDocument doc;
DeserializationError error;
t_event_send event_sent;


lora_send::lora_send(/* args */){}
lora_send::~lora_send(){}

void lora_stream_loop(void * arg){

    Serial.println("Lora stream loop started");

    for(;;){

        Heltec.display->clear();
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        Heltec.display->setFont(ArialMT_Plain_10);
        
        Heltec.display->drawString(0, 0, "Sending packet: ");
        Heltec.display->drawString(90, 0, String(counter));
        Heltec.display->display();

        // send packet
        LoRa.beginPacket();

        LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
        
        LoRa.print("hello ");
        LoRa.print(counter);
        LoRa.endPacket();

        counter++;
        digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(10);                       // wait for a second
        digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
        delay(10);                       // wait for a second


        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void lora_send::init_lora(){

    Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, app.LORA_BAND /*long BAND*/);

    Heltec.display->init();
    
    if(app.SETUP_OK) {

        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(20, 0, "LoraChat");
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(35, 20, "Loading ...");

        app.LORA_CONNECTED = true;
    }
    else
    {
        Heltec.display->setFont(ArialMT_Plain_24);
        Heltec.display->drawString(25, 0, "SETUP");
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(20, 25, "New Device");
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(40, 45, "ID:");
        Heltec.display->drawString(60, 45, app.DEVICE_ID);
    }
    
    Heltec.display->display();
   
    delay(3000);
}

uint32_t stringAddrToHex(String addrress){
  
    char array[] = "";

    for(int i = 0; i < addrress.length(); i++){
        array[i] = addrress[i];
    }

    char *endptr;
    uint32_t val;

    val = strtoul(array, &endptr, 16);

    /*if (endptr == array)
    {
        Serial.print("'");
        Serial.print(addrress);
        Serial.println("' does not contain hex digits");
        return -1;
    }
    else
    {
        Serial.print("'");
        Serial.print(addrress);
        Serial.print("' converted = ");
        Serial.println(val);
    }*/
 

    return val;
} 

void lora_receive_task_cb(void * arg){

    

    for(;;){

        if (LoRa.parsePacket() == 0) return;        // if there's no packet, return

        int version       = LoRa.read();
        int security      = LoRa.read();
        app.ACK           = LoRa.read();
        app.TYPE          = LoRa.read();

        /*if(app.ACK){

            if(app.TYPE == LORA_TYPE_ACK){

                app.MSG_ID        = LoRa.read();
                app.LORA_RECEIVER = LoRa.read();
                app.LORA_SENDER   = LoRa.read();

                // read packet header bytes:
                app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

                String incoming = "";

                while (LoRa.available()) {
                    incoming += (char)LoRa.read();
                }

                if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                    Serial.println("error: Ack length does not match length");
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "Ack Length");
                    Heltec.display->drawString(0, 30, "ERROR");
                    Heltec.display->display();
                    return; // skip rest of function
                }

                app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

                Serial.println("\n(onReceive) - Ack Received from:   0x" + String(app.LORA_SENDER, HEX) + " - INT [" + String(app.LORA_SENDER) + "]");
                Serial.println("(onReceive) - Ack Received at:     0x" + String(app.LORA_RECEIVER, HEX) + " - INT [" + String(app.LORA_RECEIVER) + "]");

                // if the recipient isn't this device or broadcast,
                if (app.LORA_RECEIVER != app.LORA_LOCAL_ADDRESS && app.LORA_RECEIVER != 0xFF) {
                    Serial.println("This ack is not for me.");
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "Ack");
                    Heltec.display->drawString(0, 30, "NOT FOR ME");
                    Heltec.display->display();
                    return; // skip rest of function
                }

                // Parse JSON object
                error = deserializeJson(doc, incoming);

                if (error) {
                    Serial.print("deserializeJson() failed: ");
                    Serial.println(error.f_str());
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "des-Json() failed");
                    Heltec.display->drawString(0, 30, String(error.f_str()));
                    Heltec.display->display();
                    return; // skip rest of function
                }

                APP_VER         = doc["APP_VER"].as<String>();

                if(!APP_VER.equals(app.APP_VERSION)){

                    Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                    return;
                }

                ACTION          = doc["ACTION"].as<String>();
                RSSI            = doc["RSSI"].as<String>();
                SNR             = doc["SNR"].as<String>();
                MSG_ID          = doc["MSG_ID"].as<String>();
                SENDER_ID       = doc["SENDER_ID"].as<String>();
                RECEIVER_ID     = doc["RECEIVER_ID"].as<String>();

                Serial.println("\nACK packet Received from: " + String(SENDER_ID));

                Heltec.display->drawString(0, 30, "Ack In: ");
                Heltec.display->drawString(90, 30, String(app.MSG_ID));

                Heltec.display->drawString(0, 45, "From: ");
                Heltec.display->drawString(90, 45, String("0x" + String(app.LORA_SENDER, HEX)));

                String response = "";

                if(ACTION.equals("LORA_DELIVERY_STATUS")){

                    if(MSG_ID.equals(String(app.MSG_ID))){

                        MSG_DELIVERY_STATUS = "MSG_DELIVERY_OK";
                    }
                    else
                    {
                        MSG_DELIVERY_STATUS = "MSG_DELIVERY_KO";
                    }

                    response = "{";
                    response += "\"EVENTS\":";
                    response += "{\"ACTION\":\"" + ACTION + "\"";
                    response += ",\"RESPONSE\":\"EVENTS_SEND_RESPONS\"";
                    response += ",\"DELIVERY_STATUS\":\"" + MSG_DELIVERY_STATUS + "\"";
                    response += ",\"APP_VER\":\"" + APP_VER + "\"";
                    response += ",\"MSG_ID\":\"" + String(app.MSG_ID) + "\"";
                    response += ",\"RSSI\":\"" + RSSI + "\"";
                    response += ",\"SNR\":\"" + SNR + "\"";
                    response += ",\"SENDER_ID\":\"" + SENDER_ID + "\"";
                    response += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
                    response += "}}";

                    event_sent = app.send_event(response, "LORA_ACK", millis());

                    Heltec.display->drawString(0, 60, "Sent to APP: ");

                    if(event_sent != EVENT_SEND_OK){
                        
                        Heltec.display->drawString(100, 60, String("NO"));
                        Heltec.display->display();
                        Serial.println("\n(onReceive) - ACK EVENT SEND ERROR CODE:   " + event_sent);
                        return;
                    }

                    Serial.println("\n(onReceive) - ACK EVENT SENT MESSAGE:   " + response);
            
                    Heltec.display->drawString(100, 60, String("YES"));

                    Serial.println("\nACK packet Received from:   |0x" + String(app.LORA_SENDER, HEX) + "|  MESSAGE DELIVERY STATUS [" + MSG_DELIVERY_STATUS + "]");

                }
            }

            if(app.TYPE == LORA_TYPE_MESSAGE){

                app.MSG_ID        = LoRa.read();
                app.LORA_RECEIVER = LoRa.read();
                app.LORA_SENDER   = LoRa.read();

                Serial.println("\n(onReceive) - Message Received from:   0x" + String(app.LORA_SENDER, HEX) + " - INT [" + String(app.LORA_SENDER) + "]");
                Serial.println("(onReceive) - Message Received at:     0x" + String(app.LORA_RECEIVER, HEX) + " - INT [" + String(app.LORA_RECEIVER) + "]");

                // read packet header bytes:
                app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

                String incoming = "";

                while (LoRa.available()) {
                    incoming += (char)LoRa.read();
                }
                
                Serial.println("OPEN PACKETS: " + incoming);

                if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                    Serial.println("error: message length does not match length");
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "Message Length");
                    Heltec.display->drawString(0, 30, "ERROR");
                    Heltec.display->display();
                    return; // skip rest of function
                }

                app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

                // if the recipient isn't this device or broadcast,
                if (app.LORA_RECEIVER != app.LORA_LOCAL_ADDRESS && app.LORA_RECEIVER != 0xFF) {
                    Serial.println("This message is not for me.");
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "Message");
                    Heltec.display->drawString(0, 30, "NOT FOR ME");
                    Heltec.display->display();
                    return; // skip rest of function
                }

                Heltec.display->drawString(0, 30, "Packet In: ");
                Heltec.display->drawString(90, 30, String(app.MSG_ID));

                Heltec.display->drawString(0, 45, "From: ");
                Heltec.display->drawString(90, 45, String("0x" + String(app.LORA_SENDER, HEX)));

                // Parse JSON object
                error = deserializeJson(doc, incoming);

                if (error) {
                    Serial.print("deserializeJson() failed: ");
                    Serial.println(error.f_str());
                    Heltec.display->clear();
                    Heltec.display->drawString(0, 10, "des-Json() failed");
                    Heltec.display->drawString(0, 30, String(error.f_str()));
                    Heltec.display->display();
                    return; // skip rest of function
                }

                APP_VER         = doc["APP_VER"].as<String>();

                if(!APP_VER.equals(app.APP_VERSION)){

                    Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                    return;
                }

                RSSI            = doc["RSSI"].as<String>();
                SNR             = doc["SNR"].as<String>();
                MSG_ID          = doc["MSG_ID"].as<String>();
                ACTION          = doc["ACTION"].as<String>();

                MESSAGE_OBJECT  = doc["MESSAGE_OBJ"].as<JsonObject>();

                SENDER_ID       = MESSAGE_OBJECT["SENDER_ID"].as<String>();
                RECEIVER_ID     = MESSAGE_OBJECT["RECEIVER_ID"].as<String>();
                TIME            = MESSAGE_OBJECT["TIME"].as<String>();
                MESSAGE         = MESSAGE_OBJECT["MESSAGE"].as<String>();

                Serial.println("\n(onReceive) - MESSAGE packet Received from: " + String(SENDER_ID));

                

                response = "{";
                response += "\"EVENTS\":";
                response += "{\"ACTION\":\"" + ACTION + "\"";
                response += ",\"RESPONSE\":\"EVENTS_SEND_RESPONS\"";
                response += ",\"MESSAGE\":";
                response += "{\"APP_VER\":\"" + APP_VER + "\"";
                response += ",\"MSG_ID\":\"" + String(app.MSG_ID) + "\"";
                response += ",\"RSSI\":\"" + RSSI + "\"";
                response += ",\"SNR\":\"" + SNR + "\"";
                response += ",\"SENDER_ID\":\"" + SENDER_ID + "\"";
                response += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
                response += ",\"TIME\":\"" + TIME + "\"";
                response += ",\"MESSAGE\":\"" + MESSAGE + "\"}";
                response += "}}";

                event_sent = app.send_event(response, "LORA_MESSAGE", millis());

                //Heltec.display->drawString(0, 60, "Sent to APP: ");

                if(event_sent != EVENT_SEND_OK){
                    
                   // Heltec.display->drawString(100, 60, "NO");
                    Heltec.display->display();
                    Serial.println("\n(onReceive) - EVENT SEND ERROR CODE:   " + event_sent);
                    return;
                }

                //Serial.println("\n(onReceive) - EVENT SENT MESSAGE:   " + response);
            
                //Heltec.display->drawString(100, 60, "YES");

                //sendAck(app.ACK, LORA_TYPE_ACK, app.MSG_ID, app.LORA_SENDER, app.LORA_LOCAL_ADDRESS, SENDER_ID, RECEIVER_ID);
        

            }

            Heltec.display->display();
        }*/
      
        


        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

void lora_send::loraReceiveTask(){

    /* Start the main task */
        xTaskCreate(
            lora_receive_task_cb,
            "LORA_RECEIVE",
            100000,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL
        );

}

void onReceive(int packetSize){

    if (packetSize == 0) return;        // if there's no packet, return

    app.IN_VERSION    = LoRa.read();
    app.SECURITY      = LoRa.read();
    app.ACK           = LoRa.read();
    app.TYPE          = LoRa.read();
    

    if(app.IN_VERSION != app.VERSION){
        Serial.print("Lora version error: The Lora version [" + String(app.IN_VERSION) + "] coming from other side of chat is not equal to this device lora version [" + String(app.VERSION) + "]");
        return;
    }

    String APP_VER             = "--";
    String CLIENT_ID           = "--";
    String RSSI                = "--";
    String SNR                 = "--";
    String MSG_ID              = "--";

    String ACTION              = "--";
    String SECURITY            = "--";
    String SENDER_ID           = "--";
    String RECEIVER_ID         = "--";
    String APP_STATUS          = "--";
    String LORA_STATUS         = "--";
    String TIME                = "--";
    JsonObject MESSAGE_OBJECT;
    String MESSAGE             = "--";

    String MSG_DELIVERY_STATUS = "--";

    JsonDocument doc;

    if(app.ACK){

        if(app.TYPE == LORA_TYPE_SEND_APP_STATUS){

            // read packet header bytes:
            app.MSG_ID              = LoRa.read();
            app.LORA_RECEIVER       = LoRa.read();
            app.LORA_SENDER         = LoRa.read();
            app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

            if(app.LORA_RECEIVER != 0xFF || app.LORA_SENDER != 0xFF){
                Serial.println("error: This is not a Broadcasting message - packet comming from [0x" + String(app.LORA_SENDER, HEX) +"] - sent for [0x" + String(app.LORA_RECEIVER, HEX) + "]");
            }

            String incoming = "";

            while (LoRa.available()) {
                incoming += (char)LoRa.read();
            }

            if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                Serial.println("error: Broadcasting message's length does not match length");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Ack Length");
                Heltec.display->drawString(0, 30, "ERROR");
                Heltec.display->display();
                return; // skip rest of function
            }

            DeserializationError error = deserializeJson(doc, incoming);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "des-Json() failed");
                Heltec.display->drawString(0, 30, String(error.f_str()));
                Heltec.display->display();
                return; // skip rest of function
            }

            APP_VER         = doc["APP_VER"].as<String>();

            if(!APP_VER.equals(app.APP_VERSION)){

                Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                return;
            }

            ACTION          = doc["ACTION"].as<String>();    
            SENDER_ID       = doc["SENDER_ID"].as<String>();
            RECEIVER_ID     = doc["RECEIVER_ID"].as<String>();
            RSSI            = doc["RSSI"].as<String>();
            SNR             = doc["SNR"].as<String>();
            APP_STATUS      = doc["APP_STATUS"].as<String>();
            LORA_STATUS     = doc["LORA_STATUS"].as<String>();

            Serial.println("\nAPP_STATUS packet Received");

            Serial.print("\n-MSG ID [" + String(app.MSG_ID) + "]\n-BROADCAST SENDER [" + app.LORA_SENDER + "]\n-BROADCAST RECEIVER [" + app.LORA_RECEIVER + "]\n-SENDER [" + SENDER_ID + "]\n-RECEIVER [" + RECEIVER_ID + "]\n-RSSI [" + RSSI + "]\n-SNR [" + SNR + "]\n-APP STATUS [" + APP_STATUS + "]\n-LORA STATUS [" + LORA_STATUS + "]");

            String response = "";

            if(ACTION.equals("APP_STATUS")){

                response = "{";
                response += "\"EVENTS\":";
                response += "{\"ACTION\":\"" + ACTION + "\"";
                response += ",\"APP_VER\":\"" + APP_VER + "\"";
                response += ",\"MSG_ID\":\"" + String(app.MSG_ID) + "\"";
                response += ",\"RSSI\":\"" + RSSI + "\"";
                response += ",\"SNR\":\"" + SNR + "\"";
                response += ",\"RESPONSE\":\"EVENTS_SEND_RESPONS\"";
                response += ",\"APP_STATUS\":\"" + APP_STATUS + "\"";
                response += ",\"LORA_STATUS\":\"" + LORA_STATUS + "\"";
                response += ",\"SENDER_ID\":\"" + SENDER_ID + "\"";
                response += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
                response += "}}";

                t_event_send event_sent = app.send_event(response, "APP_STATUS", app.MSG_ID);

                if(event_sent != EVENT_SEND_OK){
                    Heltec.display->drawString(0, 60, "Sent to APP: ");
                    Heltec.display->drawString(100, 60, "NO");
                    Heltec.display->display();
                    Serial.println("\n(onReceive) - APP_STATUS EVENT SEND ERROR CODE:   " + event_sent);
                    return;
                }

                Heltec.display->drawString(0, 60, "Sent to APP: ");
                Heltec.display->drawString(100, 60, "YES");
            }

        }

        if(app.TYPE == LORA_TYPE_GET_APP_STATUS){

            app.MSG_ID              = LoRa.read();
            app.LORA_RECEIVER       = LoRa.read();
            app.LORA_SENDER         = LoRa.read();
            app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

            if(app.LORA_RECEIVER != 0xFF || app.LORA_SENDER != 0xFF){
                Serial.println("error: This is not a Broadcasting message - packet comming from [0x" + String(app.LORA_SENDER, HEX) +"] - sent for [0x" + String(app.LORA_RECEIVER, HEX) + "]");
            }

            String incoming = "";

            while (LoRa.available()) {
                incoming += (char)LoRa.read();
            }

            if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                Serial.println("error: Broadcasting message's length does not match length");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Ack Length");
                Heltec.display->drawString(0, 30, "ERROR");
                Heltec.display->display();
                return; // skip rest of function
            }

            DeserializationError error = deserializeJson(doc, incoming);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "des-Json() failed");
                Heltec.display->drawString(0, 30, String(error.f_str()));
                Heltec.display->display();
                return; // skip rest of function
            }

            APP_VER         = doc["APP_VER"].as<String>();

            if(!APP_VER.equals(app.APP_VERSION)){

                Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                return;
            }

            ACTION          = doc["ACTION"].as<String>();    
            SENDER_ID       = doc["SENDER_ID"].as<String>();
            RECEIVER_ID     = doc["RECEIVER_ID"].as<String>();
            
            Serial.println("\nAPP_STATUS packet Received");

            Serial.print("\n-MSG ID [" + String(app.MSG_ID) + "]\n-BROADCAST SENDER [" + app.LORA_SENDER + "]\n-BROADCAST RECEIVER [" + app.LORA_RECEIVER + "]\n-SENDER [" + SENDER_ID + "]\n-RECEIVER [" + RECEIVER_ID + "]");

            String response = "";

            if(ACTION.equals("GET_APP_STATUS")){
                
                app.sendAppStatus(app.APP_CONNECTED_STR, app.LORA_CONNECTED_STR);
            }

        }

        if(app.TYPE == LORA_TYPE_ACK){

            app.MSG_ID        = LoRa.read();
            app.LORA_RECEIVER = LoRa.read();
            app.LORA_SENDER   = LoRa.read();

            // read packet header bytes:
            app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

            String incoming = "";

            while (LoRa.available()) {
                incoming += (char)LoRa.read();
            }

            if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                Serial.println("error: Ack length does not match length");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Ack Length");
                Heltec.display->drawString(0, 30, "ERROR");
                Heltec.display->display();
                return; // skip rest of function
            }

            app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

            Serial.println("\n(onReceive) - Ack Received from:   0x" + String(app.LORA_SENDER, HEX) + " - INT [" + String(app.LORA_SENDER) + "]");
            Serial.println("(onReceive) - Ack Received at:     0x" + String(app.LORA_RECEIVER, HEX) + " - INT [" + String(app.LORA_RECEIVER) + "]");

            // if the recipient isn't this device or broadcast,
            if (app.LORA_RECEIVER != app.LORA_LOCAL_ADDRESS && app.LORA_RECEIVER != 0xFF) {
                Serial.println("This ack is not for me.");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Ack");
                Heltec.display->drawString(0, 30, "NOT FOR ME");
                Heltec.display->display();
                return; // skip rest of function
            }

            // Parse JSON object
            DeserializationError error = deserializeJson(doc, incoming);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "des-Json() failed");
                Heltec.display->drawString(0, 30, String(error.f_str()));
                Heltec.display->display();
                return; // skip rest of function
            }

            APP_VER         = doc["APP_VER"].as<String>();

            if(!APP_VER.equals(app.APP_VERSION)){

                Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                return;
            }

            ACTION          = doc["ACTION"].as<String>();
            MSG_ID          = doc["MSG_ID"].as<String>();
            SENDER_ID       = doc["SENDER_ID"].as<String>();
            RECEIVER_ID     = doc["RECEIVER_ID"].as<String>();
            RSSI            = doc["RSSI"].as<String>();
            SNR             = doc["SNR"].as<String>();

            Serial.println("\nACK packet Received from: " + SENDER_ID);

            Heltec.display->drawString(0, 30, "Ack In: ");
            Heltec.display->drawString(90, 30, String(app.MSG_ID));

            Heltec.display->drawString(0, 45, "From: ");
            Heltec.display->drawString(90, 45, String("0x" + String(app.LORA_SENDER, HEX)));

            String response = "";

            if(ACTION.equals("LORA_DELIVERY_STATUS")){

                if(MSG_ID.equals(String(app.MSG_ID))){

                    MSG_DELIVERY_STATUS = "MSG_DELIVERY_OK";
                }
                else
                {
                    MSG_DELIVERY_STATUS = "MSG_DELIVERY_KO";
                }

                response = "{";
                response += "\"EVENTS\":";
                response += "{\"ACTION\":\"" + ACTION + "\"";
                response += ",\"RESPONSE\":\"EVENTS_SEND_RESPONS\"";
                response += ",\"DELIVERY_STATUS\":\"" + MSG_DELIVERY_STATUS + "\"";
                response += ",\"APP_VER\":\"" + APP_VER + "\"";
                response += ",\"MSG_ID\":\"" + String(app.MSG_ID) + "\"";
                response += ",\"RSSI\":\"" + RSSI + "\"";
                response += ",\"SNR\":\"" + SNR + "\"";
                response += ",\"SENDER_ID\":\"" + SENDER_ID + "\"";
                response += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
                response += "}}";

                t_event_send event_sent = app.send_event(response, "LORA_ACK", app.MSG_ID);

                if(event_sent != EVENT_SEND_OK){
                    Heltec.display->drawString(0, 60, "Sent to APP: ");
                    Heltec.display->drawString(100, 60, String("NO"));
                    Heltec.display->display();
                    Serial.println("\n(onReceive) - ACK EVENT SEND ERROR CODE:   " + event_sent);
                    return;
                }

                Heltec.display->drawString(0, 60, "Sent to APP: ");
                Heltec.display->drawString(100, 60, "YES");

                Serial.println("\nACK packet Received from:   |0x" + String(app.LORA_SENDER, HEX) + "|  MESSAGE DELIVERY STATUS [" + MSG_DELIVERY_STATUS + "]");

            }
        }

        if(app.TYPE == LORA_TYPE_MESSAGE){

            app.MSG_ID        = LoRa.read();
            app.LORA_RECEIVER = LoRa.read();
            app.LORA_SENDER   = LoRa.read();

            Serial.println("\n(onReceive) - Message Received from:   0x" + String(app.LORA_SENDER, HEX) + " - INT [" + String(app.LORA_SENDER) + "]");
            Serial.println("(onReceive) - Message Received at:     0x" + String(app.LORA_RECEIVER, HEX) + " - INT [" + String(app.LORA_RECEIVER) + "]");

            // read packet header bytes:
            app.INCOMING_MSG_LENGTH = LoRa.read();    // incoming msg length

            String incoming = "";

            while (LoRa.available()) {
                incoming += (char)LoRa.read();
            }
            
            Serial.println("OPEN PACKETS: " + incoming);

            if (app.INCOMING_MSG_LENGTH != incoming.length()) {   // check length for error
                Serial.println("error: message length does not match length");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Message Length");
                Heltec.display->drawString(0, 30, "ERROR");
                Heltec.display->display();
                return; // skip rest of function
            }

            app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

            // if the recipient isn't this device or broadcast,
            if (app.LORA_RECEIVER != app.LORA_LOCAL_ADDRESS && app.LORA_RECEIVER != 0xFF) {
                Serial.println("This message is not for me.");
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "Message");
                Heltec.display->drawString(0, 30, "NOT FOR ME");
                Heltec.display->display();
                return; // skip rest of function
            }

            Heltec.display->drawString(0, 30, "Packet In: ");
            Heltec.display->drawString(90, 30, String(app.MSG_ID));

            Heltec.display->drawString(0, 45, "From: ");
            Heltec.display->drawString(90, 45, String("0x" + String(app.LORA_SENDER, HEX)));

            // Parse JSON object
            DeserializationError error = deserializeJson(doc, incoming);

            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                Heltec.display->clear();
                Heltec.display->drawString(0, 10, "des-Json() failed");
                Heltec.display->drawString(0, 30, String(error.f_str()));
                Heltec.display->display();
                return; // skip rest of function
            }

            APP_VER         = doc["APP_VER"].as<String>();

            if(!APP_VER.equals(app.APP_VERSION)){

                Serial.print("Application version error: The application version [" + APP_VER + "] coming from other side of chat is not equal to this device version [" + app.APP_VERSION + "]");

                return;
            }

            RSSI            = doc["RSSI"].as<String>();
            SNR             = doc["SNR"].as<String>();
            MSG_ID          = doc["MSG_ID"].as<String>();
            ACTION          = doc["ACTION"].as<String>();

            MESSAGE_OBJECT  = doc["MESSAGE_OBJ"].as<JsonObject>();

            SENDER_ID       = MESSAGE_OBJECT["SENDER_ID"].as<String>();
            RECEIVER_ID     = MESSAGE_OBJECT["RECEIVER_ID"].as<String>();
            TIME            = MESSAGE_OBJECT["TIME"].as<String>();
            MESSAGE         = MESSAGE_OBJECT["MESSAGE"].as<String>();

            Serial.println("\n(onReceive) - MESSAGE packet Received from: " + String(SENDER_ID));

            String response = "";

            response = "{";
            response += "\"EVENTS\":";
            response += "{\"ACTION\":\"" + ACTION + "\"";
            response += ",\"RESPONSE\":\"EVENTS_SEND_RESPONS\"";
            response += ",\"MESSAGE\":";
            response += "{\"APP_VER\":\"" + APP_VER + "\"";
            response += ",\"MSG_ID\":\"" + String(app.MSG_ID) + "\"";
            response += ",\"RSSI\":\"" + RSSI + "\"";
            response += ",\"SNR\":\"" + SNR + "\"";
            response += ",\"SENDER_ID\":\"" + SENDER_ID + "\"";
            response += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
            response += ",\"TIME\":\"" + TIME + "\"";
            response += ",\"MESSAGE\":\"" + MESSAGE + "\"}";
            response += "}}";

            t_event_send event_sent = app.send_event(response, "LORA_MESSAGE", app.MSG_ID);

            Heltec.display->drawString(0, 60, "Sent to APP: ");

            if(event_sent != EVENT_SEND_OK){
                
                Heltec.display->drawString(100, 60, "NO");
                Heltec.display->display();
                Serial.println("\n(onReceive) - EVENT SEND ERROR CODE:   " + event_sent);
                return;
            }

            Serial.println("\n(onReceive) - EVENT SENT MESSAGE:   " + response);
        
            Heltec.display->drawString(100, 60, "YES");
            
            delay(200);

            sendAck(app.ACK, LORA_TYPE_ACK, app.MSG_ID, app.LORA_SENDER, app.LORA_LOCAL_ADDRESS, SENDER_ID, RECEIVER_ID);
    

        }
    }
      
    Heltec.display->display();
}

void lora_send::loraRecive(int packetSize){
    onReceive(packetSize);
}

void getDeliveryStatus(){

}

void sendAck(int ack, int type, int msg_id, byte dest_addr, byte src_addr, String sender, String receiver){

    init_send_settings();

    LoRa.beginPacket();
    
    LoRa.write(app.VERSION);   // add lora version
    LoRa.write(0);             // add security enabled
    LoRa.write(ack);           // add ack packet enabled
    LoRa.write(type);          // add packet type
    LoRa.write(msg_id);        // add MSG ID
    LoRa.write(dest_addr);     // add receiver addr
    LoRa.write(src_addr);      // add sender addr

    String response = "";

    response = "{";
    response += "\"ACTION\":\"LORA_DELIVERY_STATUS\"";
    response += ",\"APP_VER\":\"" + app.APP_VERSION + "\"";
    response += ",\"RSSI\":\"" + String(LoRa.packetRssi()) + "\"";
    response += ",\"SNR\":\"" + String(LoRa.packetSnr()) + "\"";
    response += ",\"MSG_ID\":\"" + String(msg_id) + "\"";
    response += ",\"SENDER_ID\":\"" + sender + "\"";
    response += ",\"RECEIVER_ID\":\"" + receiver + "\"";
    response += "}";

    LoRa.write(response.length());      // add payload length
    LoRa.print(response);               // add payload

    LoRa.endPacket();

    Serial.print("\n(sendAck) - OBJECT_SENT:         ");Serial.println(response);
    Serial.println("(sendAck) - Ack sent from:   0x" + String(src_addr, HEX) + " - INT [" + String(src_addr) + "]");
    Serial.println("(sendAck) - Ack sent to:     0x" + String(dest_addr, HEX) + " - INT [" + String(dest_addr) + "]");

    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    
    Heltec.display->drawString(0, 0, "Ack Out: ");
    Heltec.display->drawString(90, 0, String(msg_id));
    
    Heltec.display->drawString(0, 15, "Sent To: ");
    Heltec.display->drawString(90, 15, String("0x" + String(dest_addr, HEX)));

}

void init_send_settings(){

    Serial.print("CRS:       ");Serial.println(app.LORA_ENABLE_CRS);
    Serial.print("SF:        ");Serial.println(app.LORA_SF);
    Serial.print("IQ:        ");Serial.println(app.LORA_ENABLE_INVERT_IQ);
    Serial.print("SYNC WORD: ");Serial.println(app.LORA_SYNC_WORD);
    Serial.print("TX POWER:  ");Serial.println(app.LORA_TX_POWER);

    if(app.LORA_ENABLE_CRS){
        LoRa.enableCrc();
    }
    else
    {
        LoRa.disableCrc();
    }
    
    LoRa.setSpreadingFactor(app.LORA_SF);

    if(app.LORA_ENABLE_INVERT_IQ){
        LoRa.enableInvertIQ();
    }
    else
    {
        LoRa.disableInvertIQ();
    }

    if(app.LORA_SYNC_WORD.equals("PRIVATE")){
        LoRa.setSyncWord(LORA_PRIVATE_SYNCWORD);
    }
    else
    {
        LoRa.setSyncWord(LORA_PUBLIC_SYNCWORD);
    }
    
    LoRa.setTxPower(app.LORA_TX_POWER, RF_PACONFIG_PASELECT_PABOOST);
}

String parseAppMessage(JsonDocument app_doc){

    String json = "";

    /** Start of variables coming from application by sockets for (ALL) types */
    String ACK             = app_doc["ACK"].as<String>();  // If ACK enabled
    String TYPE            = app_doc["TYPE"].as<String>(); // Packet type
    String SENDER_ID       = app_doc["SENDER_ID"].as<String>();
    String RECEIVER_ID     = app_doc["RECEIVER_ID"].as<String>();
    /** End of variables coming from application by sockets for (ALL) types */

    /** Start of variables coming from application by sockets just for (LORA_MESSAGE) type */
    String TIME            = "--";
    String MESSAGE         = "--";
    /** End of variables coming from application by sockets just for (LORA_MESSAGE) type */

    app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);
    app.LORA_RECEIVER      = stringAddrToHex(RECEIVER_ID);

    if(ACK.equals("true")){
        app.ACK = true;
    }
    else
    {
        app.ACK = false;
    }

    if(TYPE.equals("LORA_TYPE_ACK")){
        app.TYPE        = LORA_TYPE_ACK;
       
    }
    else if(TYPE.equals("LORA_TYPE_DELIVERY_STATUS")){
        app.TYPE        = LORA_TYPE_DELIVERY_STATUS; 
    }
    else if(TYPE.equals("LORA_TYPE_MESSAGE")){
        app.TYPE = LORA_TYPE_MESSAGE;

        TIME            = app_doc["TIME"].as<String>();
        MESSAGE         = app_doc["MESSAGE"].as<String>();

        json = "{\"APP_VER\":\"" + app.APP_VERSION + "\"";
        json += ",\"RSSI\":\"" + String(LoRa.packetRssi()) + "\"";
        json += ",\"SNR\":\"" + String(LoRa.packetSnr()) + "\"";
        json += ",\"MSG_ID\":\"" + String(counter) + "\"";
        json += ",\"ACTION\":\"SEND_MESSAGE\"";
        json += ",\"MESSAGE_OBJ\":";
        json += "{\"SENDER_ID\":\"" + SENDER_ID + "\"";
        json += ",\"RECEIVER_ID\":\"" + RECEIVER_ID + "\"";
        json += ",\"TIME\":\"" + TIME + "\"";
        json += ",\"MESSAGE\":\"" + MESSAGE + "\"}";
        json += "}";
    }
    else
    {
        app.TYPE = LORA_TYPE_DATA;
    }
    
    return json;
}

lora_send_t lora_send::sendMessage(JsonDocument app_doc){

    uint8_t g_frame_head = 0;

    init_send_settings();

    jsonData = parseAppMessage(app_doc);

    Serial.print("\n(sendMessage) - OBJECT_SENT:         ");Serial.println(jsonData);
    Serial.println("(sendMessage) - Message sent from:   0x" + String(app.LORA_LOCAL_ADDRESS, HEX) + " - INT [" + String(app.LORA_LOCAL_ADDRESS) + "]");
    Serial.println("(sendMessage) - Message sent to:       0x" + String(app.LORA_RECEIVER, HEX) + " - INT [" + String(app.LORA_RECEIVER) + "]");

    app.MSG_ID = counter;

    LoRa.beginPacket();
    LoRa.write(app.VERSION);            // add lora version
    LoRa.write(0);                      // add security
    LoRa.write(1);                      // add ack
    LoRa.write(app.TYPE);               // add packet type
    LoRa.write(app.MSG_ID);             // add message ID
    LoRa.write(app.LORA_RECEIVER);      // add destination address
    LoRa.write(app.LORA_LOCAL_ADDRESS); // add sender address

    LoRa.write(jsonData.length());      // add payload length
    LoRa.print(jsonData);               // add payload

    if(LoRa.endPacket() == 1){

        counter++;

        Heltec.display->clear();
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        Heltec.display->setFont(ArialMT_Plain_10);
        
        Heltec.display->drawString(0, 0, "Packet Out: ");
        Heltec.display->drawString(90, 0, String(counter));
        
        Heltec.display->drawString(0, 15, "Sent To: ");
        Heltec.display->drawString(90, 15, String("0x" + String(app.LORA_RECEIVER, HEX)));
        
        digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(10);                 // wait for a second
        digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
        delay(10);                 // wait for a second  

        return LORA_SEND_OK;
    }
    
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    
    Heltec.display->drawString(0, 0, "Packet Out: ");
    Heltec.display->drawString(90, 0, String(counter));
    
    Heltec.display->drawString(0, 15, "Error: ");
    Heltec.display->drawString(90, 15, "Send failed");

    return LORA_SEND_FAIL;
    
}

void lora_send::openStream(String app_status, String lora_status){

    Serial.println("Send OPEN STREAM Lora packet");

    if(app.LORA_ENABLE_CRS){
        LoRa.enableCrc();
    }
    else
    {
        LoRa.disableCrc();
    }
    
    LoRa.setSpreadingFactor(app.LORA_SF);

    if(app.LORA_ENABLE_INVERT_IQ){
        LoRa.enableInvertIQ();
    }
    else
    {
        LoRa.disableInvertIQ();
    }

    if(app.LORA_SYNC_WORD.equals("PRIVATE")){
        LoRa.setSyncWord(LORA_PRIVATE_SYNCWORD);
    }
    else
    {
        LoRa.setSyncWord(LORA_PUBLIC_SYNCWORD);
    }
    
    LoRa.setTxPower(app.LORA_TX_POWER, RF_PACONFIG_PASELECT_PABOOST);

    app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

    jsonData = "{\"APP_VER\":\"" + app.APP_VERSION + "\"";
    jsonData += ",\"ACTION\":\"BROADCASTING_ENTRANCE\"";
    jsonData += ",\"SENDER_ID\":\"" + String(app.LORA_LOCAL_ADDRESS, HEX) + "\"";
    jsonData += ",\"RECEIVER_ID\":\"" + String(app.LORA_BROADCAST_ID, HEX) + "\"";
    jsonData += ",\"RSSI\":\"" + String(LoRa.packetRssi()) + "\"";
    jsonData += ",\"SNR\":\"" + String(LoRa.packetSnr()) + "\"";
    jsonData += ",\"APP_STATUS\":\"" + app_status + "\"";
    jsonData += ",\"LORA_STATUS\":\"" + lora_status + "\"";
    jsonData += ",\"MSG\":\"Lora Closed\"";
    jsonData += "}";

    LoRa.beginPacket();
    LoRa.write(app.VERSION);            // add lora version
    LoRa.write(0);                      // add security
    LoRa.write(1);                      // add ack
    LoRa.write(LORA_TYPE_OPEN_STREAM);  // add packet type
    LoRa.write(0);                      // add message ID
    LoRa.write(app.LORA_BROADCAST_ID);  // add destination address
    LoRa.write(app.LORA_BROADCAST_ID);  // add sender address
    LoRa.write(jsonData.length());      // add payload length
    LoRa.print(jsonData);               // add payload
    counter++;
    LoRa.endPacket();
    
    onReceive(LoRa.parsePacket());
}

void lora_send::sendAppStatus(String app_status, String lora_status){

    Serial.println("Send APP STATUS Lora packet");

    if(app.LORA_ENABLE_CRS){
        LoRa.enableCrc();
    }
    else
    {
        LoRa.disableCrc();
    }
    
    LoRa.setSpreadingFactor(app.LORA_SF);

    if(app.LORA_ENABLE_INVERT_IQ){
        LoRa.enableInvertIQ();
    }
    else
    {
        LoRa.disableInvertIQ();
    }

    if(app.LORA_SYNC_WORD.equals("PRIVATE")){
        LoRa.setSyncWord(LORA_PRIVATE_SYNCWORD);
    }
    else
    {
        LoRa.setSyncWord(LORA_PUBLIC_SYNCWORD);
    }
    
    LoRa.setTxPower(app.LORA_TX_POWER, RF_PACONFIG_PASELECT_PABOOST);

    app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

    jsonData = "{\"APP_VER\":\"" + app.APP_VERSION + "\"";
    jsonData += ",\"ACTION\":\"APP_STATUS\"";
    jsonData += ",\"SENDER_ID\":\"" + String(app.LORA_LOCAL_ADDRESS, HEX) + "\"";
    jsonData += ",\"RECEIVER_ID\":\"" + String(app.LORA_BROADCAST_ID, HEX) + "\"";
    jsonData += ",\"RSSI\":\"" + String(LoRa.packetRssi()) + "\"";
    jsonData += ",\"SNR\":\"" + String(LoRa.packetSnr()) + "\"";
    jsonData += ",\"APP_STATUS\":\"" + app_status + "\"";
    jsonData += ",\"LORA_STATUS\":\"" + lora_status + "\"";
    jsonData += "}";

    LoRa.beginPacket();
    LoRa.write(app.VERSION);                // add lora version
    LoRa.write(0);                          // add security
    LoRa.write(1);                          // add ack
    LoRa.write(LORA_TYPE_SEND_APP_STATUS);  // add packet type
    LoRa.write(0);                          // add message ID
    LoRa.write(app.LORA_BROADCAST_ID);      // add destination address
    LoRa.write(app.LORA_BROADCAST_ID);      // add sender address
    LoRa.write(jsonData.length());          // add payload length
    LoRa.print(jsonData);                   // add payload
    counter++;
    LoRa.endPacket();
    
    onReceive(LoRa.parsePacket());
}

void lora_send::getAppStatus(){

    Serial.println("Send APP STATUS Lora packet");

    if(app.LORA_ENABLE_CRS){
        LoRa.enableCrc();
    }
    else
    {
        LoRa.disableCrc();
    }
    
    LoRa.setSpreadingFactor(app.LORA_SF);

    if(app.LORA_ENABLE_INVERT_IQ){
        LoRa.enableInvertIQ();
    }
    else
    {
        LoRa.disableInvertIQ();
    }

    if(app.LORA_SYNC_WORD.equals("PRIVATE")){
        LoRa.setSyncWord(LORA_PRIVATE_SYNCWORD);
    }
    else
    {
        LoRa.setSyncWord(LORA_PUBLIC_SYNCWORD);
    }
    
    LoRa.setTxPower(app.LORA_TX_POWER, RF_PACONFIG_PASELECT_PABOOST);

    app.LORA_LOCAL_ADDRESS = stringAddrToHex(app.DEVICE_ID);

    jsonData = "{\"APP_VER\":\"" + app.APP_VERSION + "\"";
    jsonData += ",\"ACTION\":\"GET_APP_STATUS\"";
    jsonData += ",\"SENDER_ID\":\"" + String(app.LORA_LOCAL_ADDRESS, HEX) + "\"";
    jsonData += ",\"RECEIVER_ID\":\"" + String(app.LORA_BROADCAST_ID, HEX) + "\"";
    jsonData += "}";

    LoRa.beginPacket();
    LoRa.write(app.VERSION);                // add lora version
    LoRa.write(0);                          // add security
    LoRa.write(1);                          // add ack
    LoRa.write(LORA_TYPE_GET_APP_STATUS);  // add packet type
    LoRa.write(0);                          // add message ID
    LoRa.write(app.LORA_BROADCAST_ID);      // add destination address
    LoRa.write(app.LORA_BROADCAST_ID);      // add sender address
    LoRa.write(jsonData.length());          // add payload length
    LoRa.print(jsonData);                   // add payload
    counter++;
    LoRa.endPacket();
    
    onReceive(LoRa.parsePacket());
}


