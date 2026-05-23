#ifndef SPIFFSIO_H
#define SPIFFSIO_H

#include <Arduino.h>
#include "files/spiffsio_helper.h"

#define CONFIG_FILE_SIZE 2000
#define CONFIG_FILE_PATH "/config.json"

class SpiffsIo {

    public:
    SpiffsIo();
    ~SpiffsIo();

    filesio_info_t init_spiffs();
    void end_spiffs();

    void init_config();
    bool resetConfig();
    bool fileExiste(const String& path);
    filesio_operations_t createFile(const String& path);
    bool deleteFile(const String& path);
    bool renameFile(const String& pathFrom, const String& pathTo);
    filesio_operations_t copyFile(const String& copyFrom, const String& copyTo);
    filesio_operations_t make_config_backup();
    void printFile(const String& path);

    bool setString(const char *key, const char *value);
    String getString(const char *key);

    bool setInt(const char *key, int value);
    int getInt(const char *key);

    bool set_uint8(const char *key, uint8_t value);
    uint8_t get_uint8(const char *key);

    bool set_int8(const char *key, int8_t value);
    int8_t get_int8(const char *key);

    bool set_double(const char *key, double value);
    double get_double(const char *key);

    bool setLong(const char *key, long value);
    long getLong(const char *key);

    bool setBool(const char *key, bool value);
    long getBool(const char *key);

};

#endif /* SPIFFSIO_H */