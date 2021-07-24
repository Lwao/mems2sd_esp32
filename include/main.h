/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 24 2021
 */

#ifndef _MAIN_H_ 
#define _MAIN_H_

/*
 * Include section
 * --------------------
 * Importing all necessary libraries for the good maintenance of the code
 *
 *  C_POSIX_LIB_INCLUDED:         standard functionalities
 *  RTC_LIB_INCLUDED:             allow system to deal with RTC (no NTP available)
 *  ESP_MANAGEMENT_LIBS_INCLUDED: log messages, error codes and SD filesystem support
 *  DRIVERS_INCLUDED:             basic esp drivers and peripherals
 *  SD_LIB_INCLUDED:              enable support to SD card SPI communication
 *  FREERTOS_LIB_INCLUDED:        enable freeRTOS support
 */

#include "sd_driver.h"

#ifndef C_POSIX_LIB_INCLUDED
    #define C_POSIX_LIB_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/unistd.h>
    #include <sys/stat.h>
#endif //C_POSIX_LIB_INCLUDED

#ifndef RTC_LIB_INCLUDED
    #define RTC_LIB_INCLUDED
    #include <esp_system.h>
    #include <time.h>
    #include <sys/time.h>
#endif //RTC_LIB_INCLUDED

#ifndef ESP_MANAGEMENT_LIBS_INCLUDED
    #define ESP_MANAGEMENT_LIBS_INCLUDED
    #include "esp_err.h" // error codes and helper functions
    #include "esp_log.h" // logging library
    #include "esp_vfs_fat.h" // FAT filesystem support
#endif //ESP_MANAGEMENT_LIBS_INCLUDED

#ifndef DRIVERS_INCLUDED
    #define DRIVERS_INCLUDED
    #include "driver/gpio.h"
    #include "driver/periph_ctrl.h"
    #include "driver/timer.h"
#endif //DRIVERS_INCLUDED

#ifndef SD_LIB_INCLUDED 
    #define SD_LIB_INCLUDED // define if SD card libraries were included
    #include "driver/sdspi_host.h"
    #include "driver/spi_common.h"
    #include "sdmmc_cmd.h"
    #include "sdkconfig.h"
#endif //SD_LIB_INCLUDED

#ifndef FREERTOS_LIB_INCLUDED
    #define FREERTOS_LIB_INCLUDED
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
    #include "freertos/semphr.h"
#endif //FREERTOS_LIB_INCLUDED

/*
 * Define section
 * --------------------
 * Definition of macros to be used gloablly in the code
 */
// TIMER_BASE_CLK = 80MHz
// TIMER_DIVIDER  = 20   // max=17 -> 4.7MHz 
// TIMER_SCALE    = 4MHz // max=4.8MHz
#define TIMER_DIVIDER         20 //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   1 // sample test interval for the first timer
#define TEST_WITHOUT_RELOAD   0 // testing will be done without auto reload
#define TEST_WITH_RELOAD      1 // testing will be done with auto reload

#define GPIO_OUTPUT_IO       GPIO_NUM_18 
#define GPIO_INPUT_IO        GPIO_NUM_4 
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO) // | (1ULL<<ANOTHER_GPIO)
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO)  // | (1ULL<<ANOTHER_GPIO)

#define MOUNT_POINT "/sdcard"        // SD card mounting point
#define SPI_DMA_CHAN 1               // DMA channel to be used by the SPI peripheral

#define VREF 1100
#define SAMPLING_TIME 500

#ifndef USE_SPI_MODE
    #define USE_SPI_MODE    // define SPI mode
    #define PIN_NUM_MISO 19 // SDI - Serial Data In
    #define PIN_NUM_MOSI 23 // SDO - Serial Data Out
    #define PIN_NUM_CLK  18 // System clock
    #define PIN_NUM_CS   5  // Chip select
#endif

/*
 * Global variable declaration section
 * --------------------
 * Initialize global variables to be used in any part of the code
 */

static const char *TAG = "example"; // ESP log tag

// config output pin
gpio_config_t out_conf = {
    .intr_type = GPIO_INTR_DISABLE, // disable interrupt
    .mode = GPIO_MODE_OUTPUT, // set as input mode
    .pin_bit_mask = GPIO_OUTPUT_PIN_SEL, // bit mask of pins to set (GPIO18)
    .pull_down_en = 0, // disable pull-down mode
    .pull_up_en = 0, // disable pull-up mode
};


// config input pin
gpio_config_t in_conf = {
    .intr_type = GPIO_INTR_DISABLE, // disable interrupt
    .mode = GPIO_MODE_INPUT, // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL, // bit mask of pins to set (GPIO18)
    .pull_down_en = 1, // enable pull-down mode
    .pull_up_en = 0, // disable pull-up mode
};


// sd card variables
sdmmc_card_t* card;
sdmmc_host_t host;
FILE* session_file;

const char mount_point[] = MOUNT_POINT;
const char* fname = "ex_mems2sd";

// freertos variables
TaskHandle_t xTaskMEMSmicHandle; // mems microphone [task_handle]
TaskHandle_t xTaskSDcardHandle; // micro sd card [task_handle]
TaskHandle_t xTaskSTARThandle; // starting routine [task_handle]
TaskHandle_t xTaskENDhandle; // ending routine [task_handle]

QueueHandle_t xQueueData; // data queue for transfering microphone data to sd card [queue_handle]
SemaphoreHandle_t xSemaphoreBTN_ON; // semaphore to interpret button as start button [semaphore_handle]
SemaphoreHandle_t xSemaphoreBTN_OFF; // semaphore to interpret button as end button [semaphore_handle]

// flags
_Bool flag_spi_bus_free = 0;     // flag if there are devices attached to SPI bus or not
_Bool flag_rec_started = 0; // flag informing that the recording already started

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

// functions prototypes
void initialize_spi_bus();
void init_spi_bus();
void sd_card_init();
void sd_card_end();

void vTaskMEMSmic(void * pvParameters);
void vTaskSDcard(void * pvParameters);
void vTaskSTART(void * pvParameters);
void vTaskEND(void * pvParameters);


#endif //_MAIN_H_