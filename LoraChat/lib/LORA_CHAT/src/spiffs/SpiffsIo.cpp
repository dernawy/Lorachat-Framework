#include "SpiffsIo.h"
#include <SPIFFS.h>
#include "Application.h"

SpiffsIo::SpiffsIo(){}
SpiffsIo::~SpiffsIo(){
    SPIFFS.end();
}

filesio_info_t SpiffsIo::init_spiffs(){

    if(!SPIFFS.begin()){

        Serial.println("SPIFFS: FIO_MOUNT_FAIL");

        return FIO_MOUNT_FAIL;
    }

    delay(1000);

    if(!SPIFFS.exists(CONFIG_FILE_PATH)){

        Serial.println("SPIFFS: FIO_CONFIG_FILE_NOT_EXIST");

        return FIO_CONFIG_FILE_NOT_EXIST;
    }

    app.init_config();
    
    return FIO_OK;
}

void SpiffsIo::end_spiffs(){
    SPIFFS.end();
}

void SpiffsIo::init_config(){

    Serial.println("\nInit config file\n");

    /* GLOBAL SETTINGS */
    app.APP_ID                  = app.getString("APP_ID");
    app.APP_VERSION             = app.getString("APP_VERSION");
    app.DEVICE_ID               = app.getString("DEVICE_ID");
    app.ID                      = app.get_uint8("ID");
    app.DEVICE_NAME             = app.getString("DEVICE_NAME");
    app.SETUP_OK                = app.getBool("SETUP_OK");
    
    /* WIFI AP SETTINGS */
    app.AP_SSID                 = app.getString("AP_SSID");
    app.AP_PASS                 = app.getString("AP_PASS");
    app.AP_MAC                  = app.getString("AP_MAC");
    app.AP_IP                   = app.getString("AP_IP");
    app.AP_HOSTNAME             = app.getString("AP_HOSTNAME");
    app.AP_SSID_HIDDEN          = app.getInt("AP_SSID_HIDDEN");
    app.AP_CHANNEL              = app.getInt("AP_CHANNEL");
    app.AP_MAX_CLIENTS          = app.getInt("AP_MAX_CLIENTS");

    /* WIFI STA SETTINGS */
    app.WIFI_SETUP              = app.getBool("WIFI_SETUP");
    app.START_WIFI              = app.getBool("START_WIFI");
    app.WIFI_SSID               = app.getString("WIFI_SSID");
    app.WIFI_PASS               = app.getString("WIFI_PASS");
    app.WIFI_DNS_1              = app.getString("WIFI_DNS_1");
    app.WIFI_DNS_2              = app.getString("WIFI_DNS_2");
    app.WIFI_MAC                = app.getString("WIFI_MAC");
    app.WIFI_IP                 = app.getString("WIFI_IP");

    /* LORA SETTINGS */
    app.LORA_BAND               = app.get_double("LORA_BAND");
    app.LORA_TX_POWER           = app.get_int8("LORA_TX_POWER");
    app.LORA_SYNC_WORD          = app.getString("LORA_SYNC_WORD");
    app.LORA_SF                 = app.getInt("LORA_SF");
    app.LORA_ENABLE_CRS         = app.getBool("LORA_ENABLE_CRS");
    app.LORA_ENABLE_INVERT_IQ   = app.getBool("LORA_ENABLE_INVERT_IQ");

    app.websockets_host         = "192.168.4.1";
    app.websockets_port         = 8888;


    Serial.print("\nAPP_ID:      ");Serial.println(app.APP_ID);
    Serial.print("\nID:          ");Serial.println(app.ID);
    Serial.print("\nAPP_VERSION: ");Serial.println(app.APP_VERSION);
    Serial.print("\nDEVICE_ID:   ");Serial.println(app.DEVICE_ID);
    Serial.print("\nSETUP_OK:    ");Serial.println(app.SETUP_OK);
    Serial.print("\nAP_SSID:     ");Serial.println(app.AP_SSID);
    Serial.print("\nAP_PASS:     ");Serial.println(app.AP_PASS);
    Serial.print("\nAP_IP:       ");Serial.println(app.AP_IP);
    Serial.print("\nWIFI_SETUP:  ");Serial.println(app.WIFI_SETUP);
    Serial.print("\nWIFI_SSID:   ");Serial.println(app.WIFI_SSID);
    Serial.print("\nWIFI_PASS:   ");Serial.println(app.WIFI_PASS);
    Serial.print("\nWIFI_IP:     ");Serial.println(app.WIFI_IP);
}

bool SpiffsIo::resetConfig(){

    Serial.println("resetConfig(): Application Control start reset device");

    if(app.setString("APP_ID", "DEFV1-LORA-CHAT-0000"));
    delay(200);
    if(app.set_uint8("ID", 0));
    delay(200);
    if(app.setString("DEVICE_ID", "0xFD"));
    delay(200);
    if(app.setString("APP_VERSION", "LORA-CHAT-V1"));
    delay(200);
    if(app.setString("DEVICE_NAME", "EMPTY"));
    delay(200);
    if(app.setBool("SETUP_OK", false));
    delay(200);
    if(app.setString("AP_SSID", "LoraChat"));
    delay(200);
    if(app.setString("AP_PASS", "LoraChat"));
    delay(200);
    if(app.setString("AP_MAC", "EMPTY"));
    delay(200);
    if(app.setString("AP_IP", "EMPTY"));
    delay(200);
    if(app.setString("AP_HOSTNAME", "cyrenaica"));
    delay(200);
    if(app.setInt("AP_CHANNEL", 1));
    delay(200);
    if(app.setInt("AP_SSID_HIDDEN", 0));
    delay(200);
    if(app.setInt("AP_MAX_CLIENT", 1));
    delay(200);
    if(app.setBool("WIFI_SETUP", false));
    delay(200);
    if(app.setBool("START_WIFI", false));
    delay(200);
    if(app.setString("WIFI_SSID", "EMPTY"));
    delay(200);
    if(app.setString("WIFI_PASS", "EMPTY"));
    delay(200);
    if(app.setString("WIFI_MAC", "EMPTY"));
    delay(200);
    if(app.setString("WIFI_IP", "EMPTY"));
    delay(200);
    if(app.setString("WIFI_DNS_1", "EMPTY"));
    delay(200);
    if(app.setString("WIFI_DNS_2", "EMPTY"));
    delay(200);
    if(app.set_double("LORA_BAND", 868E6));
    delay(200);
    if(app.set_int8("LORA_TX_POWER", 14));
    delay(200);
    if(app.setString("LORA_SYNC_WORD", "PRIVATE"));
    delay(200);
    if(app.setInt("LORA_SF", 7));
    delay(200);
    if(app.setBool("LORA_ENABLE_CRS", true));
    delay(200);
    
    if(app.setBool("LORA_ENABLE_INVERT_IQ", false)){

        delay(200);

        init_config();

        delay(200);

        return true;

    }

    return false;
    
}

bool SpiffsIo::fileExiste(const String& path){
    return SPIFFS.exists(path);
}

filesio_operations_t SpiffsIo::createFile(const String& path){

    if(!fileExiste(path)){

        File file = SPIFFS.open(path, FILE_READ);

        if(!file){
            file.flush();
            file.close();
            
            return FIO_FILE_CREATE_KO;
        }

        file.flush();
        file.close();

        return FIO_OPERATION_OK;
    }
    else
    {
        return FIO_FILE_CREATE_FILE_EXIST;
    }

    return FIO_OPERATION_FAIL;

}

bool SpiffsIo::deleteFile(const String& path){
    return SPIFFS.remove(path);
}

bool SpiffsIo::renameFile(const String& pathFrom, const String& pathTo){
    return SPIFFS.rename(pathFrom, pathTo);
}

filesio_operations_t SpiffsIo::copyFile(const String& copyFrom, const String& copyTo){

    if(!fileExiste(copyFrom)){
        return FIO_SOURCE_FILE_NOT_EXIST;
    }

    File f_from = SPIFFS.open(copyFrom , FILE_READ);

    if(!f_from){
        return FIO_FAILD_OPEN_FILE;
    }

    if(fileExiste(copyTo)){
        deleteFile(copyTo);
    }

    File f_to   = SPIFFS.open(copyTo, FILE_WRITE);

    if(!f_to){
        return FIO_FAILD_CREATE_FILE;
    }

    char buffer[4096];

    while( f_from.available() ) {
        size_t read_bytes = f_from.readBytes( buffer, 4096 );
        f_to.write( (const uint8_t *)buffer, read_bytes ); 
    }

    return FIO_OPERATION_OK;
}

void SpiffsIo::printFile(const String& path){

    if(path == CONFIG_FILE_PATH){

        File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
        JsonDocument document;
        JsonObject object = document.to<JsonObject>();

        Serial.println("Config File Content:");
            
        deserializeJson(document, configFile);
        configFile.close();

        String table;
        serializeJson(object, table);
            
        Serial.println();    
        Serial.println(table);
        Serial.println();
    }
}

bool SpiffsIo::setString(const char *key, const char *value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

String SpiffsIo::getString(const char *key){

    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<String>();
}

bool SpiffsIo::setInt(const char *key, int value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

int SpiffsIo::getInt(const char *key){


    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<int>();
}

bool SpiffsIo::set_uint8(const char *key, uint8_t value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

uint8_t SpiffsIo::get_uint8(const char *key){
    

    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<uint8_t>();
}

bool SpiffsIo::set_int8(const char *key, int8_t value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

int8_t SpiffsIo::get_int8(const char *key){
    

    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<int8_t>();
}

bool SpiffsIo::set_double(const char *key, double value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

double SpiffsIo::get_double(const char *key){
    

    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<double>();
}


bool SpiffsIo::setLong(const char *key, long value){

    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

long SpiffsIo::getLong(const char *key){
    
   

    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<long>();
}

bool SpiffsIo::setBool(const char *key, bool value){
   
    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    deserializeJson(doc, configFile);
    configFile.close();

    doc[key] = value;

    configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");

    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        configFile.close();
        return false;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
        configFile.close();
        return false;
    }
    
    configFile.close();

    return true;
}

long SpiffsIo::getBool(const char *key){


    File configFile   = SPIFFS.open(CONFIG_FILE_PATH, "r");
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();
    deserializeJson(doc, configFile);
    configFile.flush();
    configFile.close();
    
    return object[key].as<bool>();

}