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
    #include "driver/ledc.h"
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

#ifndef TIMER_LIB_INCLUDED
    #define TIMER_LIB_INCLUDED
    #include "esp_timer.h"
    #include "esp_sleep.h"
#endif //TIMER_LIB_INCLUDED

/*
 * Define section
 * --------------------
 * Definition of macros to be used globally in the code
 */

// TODO: another alternative is to set TIMER_DIVIDER to 1 and the timer need to count up to 20 to fire the isr

// timer
#define TIMER_DIVIDER 2                                //  hardware timer clock divider (TIMER_DIVIDER  = 20/17 -> TIMER_SCALE = 4/4.8MHz
#define TIMER_SCALE   (TIMER_BASE_CLK / TIMER_DIVIDER) // convert counter value to seconds (TIMER_BASE_CLK = 80MHz)
#define TIMER_COUNT   2500                               // 40MHz/TIMER_COUNT

// pins
#define MIC_CLOCK_PIN  GPIO_NUM_2  // gpio 2 - MEMS MIC clock in
#define MIC_DATA_PIN   GPIO_NUM_4  // gpio 4  - MEMS MIC data out
#define BTN_START_END  GPIO_NUM_0  // gpio 0  - button
#define GPIO_OUTPUT_IO GPIO_NUM_16 // gpio 16 - no use

// pwm clock
#define LOW_POWER_MODE_CLOCK  500000      // 351kHz - 815kHz
#define ULTRASONIC_MODE_CLOCK 4000000     // 3.072MHz - 4.8MHz
#define STANDARD_MODE_CLOCK   2000000     // 1.024MHz - 2.475MHz

// gpio
#define GPIO_OUTPUT_PIN_SEL     (1ULL<<GPIO_OUTPUT_IO) // | (1ULL<<ANOTHER_GPIO)
#define GPIO_INPUT_PIN_SEL1     (1ULL<<BTN_START_END)  // | (1ULL<<ANOTHER_GPIO)
#define GPIO_INPUT_PIN_SEL2     (1ULL<<MIC_DATA_PIN)   // | (1ULL<<ANOTHER_GPIO)
#define ESP_INTR_FLAG_DEFAULT 0

// sd card
#define MOUNT_POINT "/sdcard" // SD card mounting directory
#define SPI_DMA_CHAN 1        // DMA channel to be used by the SPI peripheral

// spi bus
#ifndef USE_SPI_MODE
    #define USE_SPI_MODE    // define SPI mode
    #define PIN_NUM_MISO 19 // SDI - Serial Data In
    #define PIN_NUM_MOSI 23 // SDO - Serial Data Out
    #define PIN_NUM_CLK  18 // System clock
    #define PIN_NUM_CS   5  // Chip select
#endif

// log flags
#define INIT_SPI_TAG   "init_spi"
#define DEINIT_SPI_TAG "deinit_spi"
#define INIT_SD_TAG    "init_sd"
#define DEINIT_SD_TAG  "deinit_sd"
#define SD_CARD_TAG    "sd_card"
#define MEMS_MIC_TAG   "mems_mic"
#define START_REC_TAG  "start_rec"
#define END_REC_TAG    "end_rec"
#define SETUP_APP_TAG  "setup_app"

/*
 * Global variable declaration section
 * --------------------
 * Initialize global variables to be used in any part of the code
 */

// config output pin - no use yet
gpio_config_t out_conf = {
    .intr_type    = GPIO_INTR_DISABLE,   // disable interrupt
    .mode         = GPIO_MODE_OUTPUT,    // set as input mode
    .pin_bit_mask = GPIO_OUTPUT_PIN_SEL, // bit mask of pins to set (GPIO16)
    .pull_down_en = 0,                   // disable pull-down mode
    .pull_up_en   = 0,                   // disable pull-up mode
};

// config input pin - button (GPIO0 commanded by BOOT button)
gpio_config_t in_conf1 = {
    .intr_type    = GPIO_INTR_POSEDGE,   // interrupt on rising edge
    .mode         = GPIO_MODE_INPUT,     // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL1, // bit mask of pins to set (GPIO00)
    .pull_down_en = 1,                   // enable pull-down mode
    .pull_up_en   = 0,                   // disable pull-up mode
};

// config input pin - PDM output from mic
gpio_config_t in_conf2 = {
    .intr_type    = GPIO_INTR_DISABLE,   // disable interrupt
    .mode         = GPIO_MODE_INPUT,     // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL2, // bit mask of pins to set (GPIO04)
    .pull_down_en = 1,                   // enable pull-down mode
    .pull_up_en   = 0,                   // disable pull-up mode
};

// read from microphone timer config (APB is the fafault clock source)
timer_config_t timer_conf = {
    .divider     = TIMER_DIVIDER,
    .counter_dir = TIMER_COUNT_UP,
    .counter_en  = TIMER_PAUSE,
    .alarm_en    = TIMER_ALARM_EN,
    .auto_reload = 1,
}; 

// microphone low-power mode PWM clock config
ledc_timer_config_t ledc_timer_low = {
    .speed_mode      = LEDC_HIGH_SPEED_MODE,
    .timer_num       = LEDC_TIMER_0,
    .duty_resolution = 2,
    .freq_hz         = LOW_POWER_MODE_CLOCK
};

// microphone ultrasonic mode PWM clock config
ledc_timer_config_t ledc_timer_ultra = {
    .speed_mode      = LEDC_HIGH_SPEED_MODE,
    .timer_num       = LEDC_TIMER_0,
    .duty_resolution = 2,
    .freq_hz         = ULTRASONIC_MODE_CLOCK
};

// microphone standard mode PWM clock config
ledc_timer_config_t ledc_timer_std = {
    .speed_mode      = LEDC_HIGH_SPEED_MODE,
    .timer_num       = LEDC_TIMER_0,
    .duty_resolution = 2,
    .freq_hz         = STANDARD_MODE_CLOCK
};
 
// PWM clock channel connected to microphone 
ledc_channel_config_t ledc_channel = {
    .channel    = LEDC_CHANNEL_0,
    .gpio_num   = MIC_CLOCK_PIN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 2
};

struct timeval date = {// call struct with date data
    .tv_sec = 0, // current date (to be fecthed from NTP)
};

// sd card variables
sdmmc_card_t* card;
sdmmc_host_t host;
FILE* session_file = NULL;

const char mount_point[] = MOUNT_POINT;
const char* fname = "EX_SD";

// freertos variables
TaskHandle_t xTaskMEMSmicHandle; // mems microphone [task_handle]
TaskHandle_t xTaskSDcardHandle; // micro sd card [task_handle]
TaskHandle_t xTaskSTARThandle; // starting routine [task_handle]
TaskHandle_t xTaskENDhandle; // ending routine [task_handle]

QueueHandle_t xQueueData; // data queue for transfering microphone data to sd card [queue_handle]
SemaphoreHandle_t xSemaphoreBTN_ON; // semaphore to interpret button as start button interrupt [semaphore_handle]
SemaphoreHandle_t xSemaphoreBTN_OFF; // semaphore to interpret button as end button interrupt [semaphore_handle]
SemaphoreHandle_t xSemaphoreTimer; // semaphore to interpret timer got interrupt [semaphore_handle]

// flags
_Bool flags[2] = {0};

// enum containing flags id for flags array
enum flag_id{SPI_BUS_FREE, // flag indicating if there are devices attached to SPI bus or not
            REC_STARTED};  // flag informing that the recording session already started

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

/**
 * @brief Initialize SPI bus
 * 
 * @param host pointer to SPI bus host
 */
void initialize_spi_bus(sdmmc_host_t* host);

/**
 * @brief Deinitialize SPI bus
 * 
 * @param host pointer to SPI bus host
 */
void deinitialize_spi_bus(sdmmc_host_t* host);

/**
 * @brief Mount SD card filesystem
 * 
 * @param host pointer to SPI bus host
 * @param card pointer to SD card host
 */
void initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card);

/**
 * @brief Unmount SD card filesystem
 * 
 * @param card pointer to SD card host
 */
void deinitialize_sd_card(sdmmc_card_t** card);

/**
 * @brief Task to capture data from MEMS microphone
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskMEMSmic(void * pvParameters);

/**
 * @brief Task to save acquired data in SD card
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskSDcard(void * pvParameters);

/**
 * @brief Task to configure START of recording when receives command to it
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskSTART(void * pvParameters);

/**
 * @brief Task to configure END of recording when receives command to it
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskEND(void * pvParameters);

/**
 * @brief Interrupt service routine for button pressed (associated with BOOT button a.k.a GPIO0)
 */
static void IRAM_ATTR ISR_BTN();

/**
 * @brief Interrupt service routine for MEMS microphone timer
 */
static bool IRAM_ATTR ISR_TIMER();

#endif //_MAIN_H_