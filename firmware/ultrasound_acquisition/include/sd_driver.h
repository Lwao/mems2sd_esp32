/**
 * @file sd_driver.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 24 2021
 */


/*
 * Include section
 * --------------------
 * Importing all necessary libraries for the good maintenance of the code
 *
 *  C_POSIX_LIB_INCLUDED:         standard functionalities
 *  ESP_MANAGEMENT_LIBS_INCLUDED: log messages, error codes and SD filesystem support
 */

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

#define SD_DRIVER_TAG "sd_driver"
#define INIT_SPI_TAG   "init_spi"
#define DEINIT_SPI_TAG "deinit_spi"
#define INIT_SD_TAG    "init_sd"
#define DEINIT_SD_TAG  "deinit_sd"

// sd card
#define MOUNT_POINT "/sdcard" // SD card mounting directory
#define SPI_DMA_CHAN 1        // DMA channel to be used by the SPI peripheral

// spi bus
#ifndef USE_SPI_MODE
    #define USE_SPI_MODE    // define SPI mode
    #define PIN_NUM_MISO 19 // SDI - Serial Data In
    #define PIN_NUM_MOSI 23 // SDO - Serial Data Out
    #define PIN_NUM_CLK  18 // System clock
    #define PIN_NUM_CS   22 // Chip select
#endif

#ifndef _SD_DRIVER_H_
#define _SD_DRIVER_H_

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

/**
 * @brief Merge filename with appropriate extesion and directory.
 *
 * @param filename base filename without directory or extension
 *
 * @return merged filename
 */
char* merge_filename(char *filename);

/**
 * @brief Read the counter value of hardware timer, in unit of a given scale.
 *
 * @param filename name of the file to open
 * @param mode mode of opening the file: "r" read; "w" write; "a" append
 *
 * @return pointer to a open file
 */
FILE* open_file(char *filename, char *mode);

/**
 * @brief Close file when there is no use to it.
 *
 * @param file pointer to file to be closed
 */
void close_file(FILE **file);

/**
 * @brief Rename a file
 *
 * @param actualfname actual filename 
 * @param targetfname target filename name that will substitute de previous  
 */
void rename_file(char *actualfname, char *targetfname);

/**
 * @brief Initialize SPI bus
 * 
 * @param host pointer to SPI bus host
 * 
 * @return Error code
 */
int initialize_spi_bus(sdmmc_host_t* host);

/**
 * @brief Deinitialize SPI bus
 * 
 * @param host pointer to SPI bus host
 * 
 * @return Error code
 */
int deinitialize_spi_bus(sdmmc_host_t* host);

/**
 * @brief Mount SD card filesystem
 * 
 * @param host pointer to SPI bus host
 * @param card pointer to SD card host
 * 
 * @return Error code
 */
int initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card);

/**
 * @brief Unmount SD card filesystem
 * 
 * @param card pointer to SD card host
 * 
 * @return Error code
 */
int deinitialize_sd_card(sdmmc_card_t** card);


#endif  // _SD_DRIVER_H_