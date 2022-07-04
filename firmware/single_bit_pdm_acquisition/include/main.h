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
 *  TIMER_LIB_INCLUDED:           enable hardware timers
 *  SOC_LIB_INCLUDED:             register level access
 */

#include "sd_driver.h"
#include "led_driver.h"
#include "wav_header.h"
#include "config_file.h"
#include "pdm2pcm.h"

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
    #include "driver/i2s.h"
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
    #include "freertos/event_groups.h"
#endif //FREERTOS_LIB_INCLUDED

#ifndef TIMER_LIB_INCLUDED
    #define TIMER_LIB_INCLUDED
    #include "esp_timer.h"
    #include "esp_sleep.h"
#endif //TIMER_LIB_INCLUDED

#ifndef SOC_LIB_INCLUDED
    #define SOC_LIB_INCLUDED
    #include "soc/timer_group_struct.h"
    #include "soc/timer_group_reg.h"
    #include "soc/syscon_reg.h"
    #include "soc/syscon_struct.h"
#endif //SOC_LIB_INCLUDED

/*
 * Define section
 * --------------------
 * Definition of macros to be used globally in the code
 */



// gpio
#define GPIO_INPUT_PIN_SEL1   (1ULL<<BTN_START_END)  // | (1ULL<<ANOTHER_GPIO)
#define ESP_INTR_FLAG_DEFAULT 0

// pins
#define MIC_CLOCK_PIN  GPIO_NUM_21  // gpio 21 - MEMS MIC clock in
#define MIC_DATA_PIN   GPIO_NUM_4   // gpio 4  - MEMS MIC data out
#define BTN_START_END  GPIO_NUM_0   // gpio 0  - button
#define GPIO_OUTPUT_IO GPIO_NUM_16  // gpio 16 - no use

// i2s 
#define I2S_PORT_NUM     0
#define DMA_BUF_COUNT    32
#define DMA_BUF_LEN_SMPL 1024
#define BIT_DEPTH        I2S_BITS_PER_SAMPLE_32BIT
#define DATA_BUFFER_SIZE DMA_BUF_LEN_SMPL*BIT_DEPTH/8
#define NUM_QUEUE_BUF    10

#define OSR                  16 // pdm oversampling rate
#define SAMPLE_RATE_STD_PCM  98000 // sample rate used in esp32 internal pdm2pcm conversion during recording in standard mode
#define SAMPLE_RATE_ULT_PDM  78125 // sample rate used in i2s hack to record raw pdm during recording in ultrasonic mode
#define I2S_CLOCK_RATE       SAMPLE_RATE_ULT_PDM*2*BIT_DEPTH // i2s clock rate for ultrasonic mode (API adjusted to 5MHz)
#define SAMPLE_RATE_ULT_PCM  I2S_CLOCK_RATE/OSR // sample rate of resulting pcm audio after pdm2pcm software conversion

// log flags
#define INIT_SPI_TAG   "init_spi"
#define DEINIT_SPI_TAG "deinit_spi"
#define INIT_SD_TAG    "init_sd"
#define DEINIT_SD_TAG  "deinit_sd"
#define SD_CARD_TAG    "sd_config"
#define MEMS_MIC_TAG   "mems_mic"
#define START_REC_TAG  "start_rec"
#define END_REC_TAG    "end_rec"
#define SETUP_APP_TAG  "setup_app"

// event flags

#define BIT_(shift) (1<<shift)

typedef enum {REC_STARTED,           // flag informing that the recording session already started
             SPI_BUS_FREE} events_t; // flag indicating if there are devices attached to SPI bus or not

/*
 * Global variable declaration section
 * --------------------
 * Initialize global variables to be used in any part of the code
 */

// config input pin - button (GPIO0 commanded by BOOT button)
gpio_config_t in_conf1 = {
    .intr_type    = GPIO_INTR_POSEDGE,   // interrupt on rising edge
    .mode         = GPIO_MODE_INPUT,     // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL1, // bit mask of pins to set (GPIO00)
    .pull_down_en = 1,                   // enable pull-down mode
    .pull_up_en   = 0,                   // disable pull-up mode
};

// i2s acquisition config
i2s_config_t i2s_config_pdm = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM, // master driver | receiving data (RX) | in PDM modulation  
    .sample_rate = 8000,                                  // sample rate (low power mode) clock=64*smpl_rate
    .bits_per_sample = BIT_DEPTH,                         // 16bit resolution per sample
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,         // mono audio configuration
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,    // pcm data format
    .dma_buf_count = DMA_BUF_COUNT,                       // number of buffers, 128 max.
    .dma_buf_len = DMA_BUF_LEN_SMPL,                      // size of each buffer, 1024 max.
    .use_apll = I2S_CLK_APLL,                             // for high accuracy clock applications, use the APLL_CLK clock source, which has the frequency range of 16 ~ 128 MHz
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1              // interrupt level 1
};

// i2s pin config
i2s_pin_config_t i2s_pins_pdm = {
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = MIC_CLOCK_PIN,         // clock pin
    .data_out_num = I2S_PIN_NO_CHANGE,  
    .data_in_num = MIC_DATA_PIN         // data in pin
};

// i2s acquisition config
i2s_config_t i2s_config_i2s = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX,                // master driver | receiving data (RX)   
    .sample_rate = 8000,                                  // sample rate (low power mode) clock=2*bit_depth*smpl_rate
    .bits_per_sample = BIT_DEPTH,                         // 16bit resolution per sample
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,         // mono audio configuration (LL Layer defaults right-channel-second(LSB))
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,    // pcm data format
    .dma_buf_count = DMA_BUF_COUNT,                       // number of buffers, 128 max.
    .dma_buf_len = DMA_BUF_LEN_SMPL,                      // size of each buffer, 1024 max.
    .use_apll = 0,//I2S_CLK_APLL,                             // for high accuracy clock applications, use the APLL_CLK clock source, which has the frequency range of 16 ~ 128 MHz
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // interrupt level 1
};

// i2s pin config
i2s_pin_config_t i2s_pins_i2s = {
    .bck_io_num = MIC_CLOCK_PIN,        // clock pin
    .ws_io_num = I2S_PIN_NO_CHANGE,         
    .data_out_num = I2S_PIN_NO_CHANGE,  
    .data_in_num = MIC_DATA_PIN         // data in pin
};

// Prepare and then apply the LEDC PWM timer configuration
ledc_timer_config_t ledc_timer = {
    .speed_mode       = LDC_SPEED_MODE,
    .timer_num        = LDC_TIMER,
    .duty_resolution  = LDC_DUTY_RESOLUTION ,
    .freq_hz          = LDC_FREQUENCY, 
    .clk_cfg          = LEDC_AUTO_CLK
};


// Prepare and then apply the LEDC PWM channel configuration
ledc_channel_config_t ledc_channel[NUM_LDC] = {
    { // red
    .speed_mode     = LDC_SPEED_MODE,
    .channel        = LEDC_CHANNEL_RED,
    .timer_sel      = LDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = LEDC_GPIO_OUT_RED,
    .duty           = 0, 
    .hpoint         = 0
    },
    { // green
    .speed_mode     = LDC_SPEED_MODE,
    .channel        = LEDC_CHANNEL_GREEN,
    .timer_sel      = LDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = LEDC_GPIO_OUT_GREEN,
    .duty           = 0, 
    .hpoint         = 0
    },
    { // blue
    .speed_mode     = LDC_SPEED_MODE,
    .channel        = LEDC_CHANNEL_BLUE,
    .timer_sel      = LDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = LEDC_GPIO_OUT_BLUE,
    .duty           = 0, 
    .hpoint         = 0
    }
};

size_t bytes_read; // number of bytes read by i2s_read
long input_buffer[DMA_BUF_LEN_SMPL], processing_buffer[DMA_BUF_LEN_SMPL]; // data buffer to store DMA_BUF_LEN_SMPL samples from i2s
short output_buffer[2*DMA_BUF_LEN_SMPL]; // 

// sd card variables
sdmmc_card_t* card;
sdmmc_host_t host;
FILE* session_file = NULL;

char mount_point[] = MOUNT_POINT; // sd card mounting point
char *fname;

// freertos variables
TaskHandle_t xTaskDSPHandle;   // process PDM data from mic [task_handle]
TaskHandle_t xTaskRECHandle;   // get data from mic and save into sd card [task_handle]
TaskHandle_t xTaskSTARThandle; // starting routine [task_handle]
TaskHandle_t xTaskENDhandle;   // ending routine [task_handle]

QueueHandle_t xQueueData, xQueueEvent;                 // data and event queue for transfering microphone data to sd card [queue_handle]
EventGroupHandle_t xEvents;                            // event group to handle event flags [event_group_handle]
SemaphoreHandle_t xMutexREC, xMutexDSP;                // mutex to allow lock recording and processing tasks [semaphore_handle]
SemaphoreHandle_t xSemaphoreBTN_ON, xSemaphoreBTN_OFF; // semaphore to interpret button on/off state interrupt [semaphore_handle]
TimerHandle_t xTimerInSession, xTimerOutSession;       // software timer to count in-session and out-session time [timer_handle]

portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

struct timeval date = {// struct with date data
    .tv_sec = 0, // current date in seconds (to be fecthed from NTP)
};

config_file_t configurations;
wav_header_t wav_header;
app_cic_t cic;
app_fir_t fir;

short fir_coeffs[FIR_ORDER] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    1, -1, -1, 4, 0, -9, 4, 34, 34, 4, -9, 0, 4, -1, -1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

/**
 * @brief Task to process input buffer data.
 *
 * @param pvParameters freeRTOS task parameters.
 */
void vTaskDSP(void * pvParameters);

/**
 * @brief Task to acquire and save audio data into SD card.
 *
 * @param pvParameters freeRTOS task parameters.
 */
void vTaskREC(void * pvParameters);

/**
 * @brief Task to configure START of recording when receives command to it.
 *
 * @param pvParameters freeRTOS task parameters.
 */
void vTaskSTART(void * pvParameters);

/**
 * @brief Task to configure END of recording when receives command to it.
 *
 * @param pvParameters freeRTOS task parameters.
 */
void vTaskEND(void * pvParameters);

/**
 * @brief Interrupt service routine for button pressed (associated with BOOT button a.k.a GPIO0)
 */
static void IRAM_ATTR ISR_BTN();

/**
 * @brief Timer callback to stop recording. 
 */
static void vTimerInSessionCbck();

/**
 * @brief Timer callback to start recording. 
 */
static void vTimerOutSessionCbck();

#endif //_MAIN_H_