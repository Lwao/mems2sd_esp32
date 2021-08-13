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

/**
 * @brief Merge filename with appropriate extesion and directory.
 *
 * @param filename base filename without directory or extension
 *
 * @return merged filename
 */
char* merge_filename(const char *filename);

/**
 * @brief Read the counter value of hardware timer, in unit of a given scale.
 *
 * @param filename name of the file to open
 * @param mode mode of opening the file: "r" read; "w" write; "a" append
 *
 * @return pointer to a open file
 */
FILE* open_file(const char *filename, char *mode);

/**
 * @brief Close file when there is no use to it.
 *
 * @param file pointer to file to be closed
 */
void close_file(FILE *file);

/**
 * @brief Rename a file
 *
 * @param actualfname actual filename 
 * @param targetfname target filename name that will substitute de previous  
 */
void rename_file(const char *actualfname, const char *targetfname);

#endif  // _SD_DRIVER_H_