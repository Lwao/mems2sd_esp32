/**
 * @file sd_driver.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 24 2021
 */

/*
 * Include section
 * --------------------
 * Importing all necessary libraries for the good maintenance of the code
 */

#include "sd_driver.h"

/*
 * Define section
 * --------------------
 * Definition of macros to be used gloablly in the code
 */

#ifndef MOUNT_POINT
    #define MOUNT_POINT "/sdcard"
#endif

/*
 * Global variable declaration section
 * --------------------
 * Initialize global variables to be used in any part of the code
 */


/*
 * Function definition section
 * --------------------
 * Define early prototyped functions execution code
 */

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
        ESP_LOGI(SD_DRIVER_TAG, "Opening file.");
        f = fopen(name, mode);
        free(name);
        if (f == NULL) {
            ESP_LOGE(SD_DRIVER_TAG, "Failed to open file.");
            return NULL;
        }
        ESP_LOGI(SD_DRIVER_TAG, "File opened.");
        return f;
    } else 
    {
        ESP_LOGE(SD_DRIVER_TAG, "Invalid file name.");
        return NULL;
    }
}

void close_file(FILE **file)
{
    if(*file != NULL){
        fclose(*file);
        *file = NULL;
    }
    ESP_LOGI(SD_DRIVER_TAG, "File closed.");
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
    ESP_LOGI(SD_DRIVER_TAG, "Renaming file.");
    if (rename(targetName, actualName) != 0) {
        ESP_LOGE(SD_DRIVER_TAG, "Rename failed.");
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
