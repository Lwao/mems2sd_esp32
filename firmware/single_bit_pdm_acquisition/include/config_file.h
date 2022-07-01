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
#include "led_driver.h"

#define PARSE_CONFIG_TAG  "parse_config"

typedef struct 
{
    int record_file_name_sufix;
    int sampling_rate;
    int bit_depth;
    int record_session_duration;
    int interval_between_record_session;
    colors_t recording_color;
    char* file_name;
} config_file_t;

/**
 * @brief Initialize config file structure with default data.
 * 
 * @param configurations config file structure to be manipulated.
 */
void init_config_file(config_file_t *configurations);

/**
 * @brief Initialize config file structure with default data.
 * 
 * @param host pointer to SPI bus host
 * @param card pointer to SD card host
 * @param configurations config file structure to be manipulated.
 */
void parse_config_file(sdmmc_host_t* host, sdmmc_card_t** card, config_file_t *configurations);

/**
 * @brief Get file name based in the SD card file system current existing file to prevent overwriting.
 * 
 * @param configurations config file structure to be manipulated.
 */
void get_file_name(config_file_t *configurations);
#endif // _CONFIG_FILE_H_