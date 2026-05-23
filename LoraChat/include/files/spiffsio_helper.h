#pragma once

#include <Arduino.h>


#ifdef __cplusplus
extern "C" {
    #endif /**< _cplusplus */
    
        typedef enum {
            FIO_MOUNT_FAIL,
            FIO_CONFIG_FILE_NOT_EXIST,
            FIO_OK
        } filesio_info_t;

        typedef enum {
            FIO_FILE_CREATE_OK,
            FIO_FILE_CREATE_KO,
            FIO_FILE_CREATE_FILE_EXIST,
            FIO_FAILD_CREATE_FILE,
            FIO_SOURCE_FILE_NOT_EXIST,
            FIO_FILE_NOT_EXIST,
            FIO_FAILD_OPEN_FILE,
            FIO_FAILD_BACKUP_FILE,
            FIO_BACKUP_FILE_OK,
            FIO_OPERATION_OK,
            FIO_OPERATION_FAIL
        } filesio_operations_t;

    #ifdef __cplusplus
}
#endif /**< _cplusplus */