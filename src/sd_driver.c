#include "sd_driver.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"

#ifndef SD_LIB_INCLUDED 
#define SD_LIB_INCLUDED // define if SD card libraries were included
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#endif

#ifndef _STAT_H_
#define _STAT_H_ // define if stat.h library was defined
#include <sys/stat.h>
#endif

#ifndef MOUNT_POINT
#define MOUNT_POINT "/sdcard"
#endif

static const char *TAG = "example";  // ESP log tag

char* merge_filename(const char *filename)
{
    const char* bar = "/";
    const char* fileformat = ".txt";
    char *name = malloc(strlen(MOUNT_POINT)+strlen(bar)+strlen(filename)+strlen(fileformat));
    strcpy(name, MOUNT_POINT);
    strcat(name, bar); 
    strcat(name, filename);
    strcat(name, fileformat);
    return name;
}

FILE* open_file(const char *filename, char *mode)
{
    FILE* f;
    
    if(filename != NULL)
    {
        char * name = merge_filename(filename);        

        // Open file
        ESP_LOGI(TAG, "Opening file");
        f = fopen(name, mode);
        free(name);
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file");
            return NULL;
        }
        return f;
    } else 
    {
        ESP_LOGE(TAG, "Invalid file name");
        return NULL;
    }
}

void close_file(FILE *file)
{
    fclose(file);
    ESP_LOGI(TAG, "File closed");
    return;
}

void rename_file(const char *actualfname, const char *targetfname)
{
    char * actualName = merge_filename(actualfname); 
    char * targetName = merge_filename(targetfname); 
    // Check if destination file exists before renaming
    struct stat st;
    if (stat(targetName, &st) == 0) {
        // Delete it if it exists
        unlink(targetName);
    }
    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename(targetName, actualName) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        free(targetName);
        free(actualName);
        return;
    } else
    {
        free(targetName);
        free(actualName);
        return;
    }
    return;
}
