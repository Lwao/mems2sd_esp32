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

#ifndef SD_LIB_INCLUDED 
    #define SD_LIB_INCLUDED // define if SD card libraries were included
    #include "driver/sdspi_host.h"
    #include "driver/spi_common.h"
    #include "sdmmc_cmd.h"
    #include "sdkconfig.h"
#endif //SD_LIB_INCLUDED

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

char* merge_filename(char *filename)
{
    char* bar = "/";
    char* fileformat = "";
    char *name = malloc(strlen(MOUNT_POINT)+strlen(bar)+strlen(filename)+strlen(fileformat));
    strcpy(name, MOUNT_POINT);
    strcat(name, bar); 
    strcat(name, filename);
    //strcat(name, fileformat);
    return name;
}

FILE* open_file(char *filename, char *mode)
{
    FILE* f;
    
    if(filename != NULL)
    {
        // char * name = merge_filename(filename);        

        // Open file
        ESP_LOGD(SD_DRIVER_TAG, "Opening file.");
        f = fopen(filename, mode);
        free(filename);
        if (f == NULL) {
            ESP_LOGD(SD_DRIVER_TAG, "Failed to open file.");
            return NULL;
        }
        ESP_LOGD(SD_DRIVER_TAG, "File opened.");
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
    ESP_LOGD(SD_DRIVER_TAG, "File closed.");
    return;
}

void rename_file(char *actualfname, char *targetfname)
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
        ESP_LOGD(SD_DRIVER_TAG, "Rename failed.");
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

int initialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(INIT_SPI_TAG, "Initializing SPI bus!");

    sdmmc_host_t host_temp = SDSPI_HOST_DEFAULT();
    // host_temp.max_freq_khz = SDMMC_FREQ_PROBING;//100;
    //host_temp.command_timeout_ms = 100;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 8192,
    };

    esp_err_t ret = spi_bus_initialize(host_temp.slot, &bus_cfg, SPI_DMA_CHAN);
    *host = host_temp;

    if (ret != ESP_OK) {
        ESP_LOGE(INIT_SPI_TAG, "Failed to initialize SPI bus.");
        return 0;
    }

    return 1; // initialization complete
}

int deinitialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(DEINIT_SPI_TAG, "Deinitializing SPI bus!");
    
    esp_err_t ret = spi_bus_free((*host).slot); //deinitialize the bus after all devices are removed
    if(ret != ESP_OK) {
        ESP_LOGI(DEINIT_SPI_TAG, "SPI not freed.");
        return 0;
    }
    
    ESP_LOGI(DEINIT_SPI_TAG, "SPI bus freed.");
    return 1;
}

int initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card)
{
    ESP_LOGI(INIT_SD_TAG, "Initializing SD card!");

    // options for mounting the filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, // format if mount failed
        .max_files = 5, // max number of files
        .allocation_unit_size = 16 * 1024, // 
    };
    
    // options for initialize SD SPI device 
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT(); // init with default pin mapping
    slot_config.gpio_cs = PIN_NUM_CS; // map CS pin
    slot_config.host_id = (*host).slot; // associate SPI bus
    // slot_config.gpio_cd // if there is card detect (CD) signal
    // slot_config.gpio_wp // if there is write protect (WP) signal

    esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &(*host), &slot_config, &mount_config, card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(INIT_SD_TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(INIT_SD_TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return 0;
    }

    // card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);

    return 1; // initialization complete
}

int deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(DEINIT_SD_TAG, "Demounting SD card!");
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(MOUNT_POINT, *card); // unmount partition and disable SDMMC or SPI peripheral
    
    if(ret != ESP_OK) {
        ESP_LOGI(DEINIT_SD_TAG, "Card still mounted.");
        return 0;
    }
    
    ESP_LOGI(DEINIT_SD_TAG, "Card unmounted.");
    return 1;
}