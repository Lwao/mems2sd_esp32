/**
 * @file config_file.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 22 2022
 */

#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#ifndef ESP_MANAGEMENT_LIBS_INCLUDED
    #define ESP_MANAGEMENT_LIBS_INCLUDED
    #include "esp_err.h" // error codes and helper functions
    #include "esp_log.h" // logging library
    #include "esp_vfs_fat.h" // FAT filesystem support
#endif

#ifndef C_POSIX_LIB_INCLUDED
    #define C_POSIX_LIB_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>
    #include <math.h>
    #include <sys/unistd.h>
    #include <sys/stat.h>
#endif //C_POSIX_LIB_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sd_driver.h"

#define PARSE_CONFIG_TAG  "parse_config"

typedef struct 
{
    int record_file_name_sufix;
    int sampling_rate;
    int record_session_duration;
    char file_name[256];
} config_file_t;

void init_config_file(config_file_t *configurations);
void parse_config_file(sdmmc_host_t* host, sdmmc_card_t** card, config_file_t *configurations);

#endif // _CONFIG_FILE_H_