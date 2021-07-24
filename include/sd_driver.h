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
 *  SD_LIB_INCLUDED:              enable support to SD card SPI communication
 */

#ifndef SD_LIB_INCLUDED 
    #define SD_LIB_INCLUDED // define if SD card libraries were included
    #include "driver/sdspi_host.h"
    #include "driver/spi_common.h"
    #include "sdmmc_cmd.h"
    #include "sdkconfig.h"
#endif

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
    #include <sys/unistd.h>
    #include <sys/stat.h>
#endif //C_POSIX_LIB_INCLUDED

#ifndef _SD_DRIVER_H_
#define _SD_DRIVER_H_

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */


/*
 * Function:  merge_filename 
 * --------------------
 * computes an approximation of pi using:
 *    pi/6 = 1/2 + (1/2 x 3/4) 1/5 (1/2)^3  + (1/2 x 3/4 x 5/6) 1/7 (1/2)^5 +
 *
 *  n: number of terms in the series to sum
 *
 *  returns: the approximate value of pi obtained by suming the first n terms
 *           in the above series
 *           returns zero on error (if n is non-positive)
 */
char* merge_filename(const char *filename);

/**
 * @brief Read the counter value of hardware timer, in unit of a given scale.
 *
 * @param group_num Timer group, 0 for TIMERG0 or 1 for TIMERG1
 * @param timer_num Timer index, 0 for hw_timer[0] & 1 for hw_timer[1]
 * @param time Pointer, type of double*, to accept timer counter value, in seconds.
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
FILE* open_file(const char *filename, char *mode);

/*

*/
void close_file(FILE *file);

/*

*/
void rename_file(const char *actualfname, const char *targetfname);

#endif  // _SD_DRIVER_H_