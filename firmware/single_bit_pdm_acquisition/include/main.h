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

/* Sample rate
0  -> 16kHz   = 16000; 
1  -> 32kHz   = 32000;
2  -> 44.1kHz = 44100; 
3  -> 96kHz   = 96000;
4  -> 112kHz  = 112000;
5  -> 128kHz  = 128000;
6  -> 144kHz  = 144000;
7  -> 160kHz  = 160000;
8  -> 176kHz  = 176000;
9  -> 192kHz  = 192000;
10 -> 250kHz  = 250000;
*/
#define SAMPLE_RATE_PDM2PCM 44100
#define SAMPLE_RATE_PDM 62500 // 140000->4.8MHz, 62500->2MHz
#define SAMPLE_RATE SAMPLE_RATE_PDM
/* Bit depth
0 -> 8-bits  = I2S_BITS_PER_SAMPLE_8BIT ;
1 -> 16-bits = I2S_BITS_PER_SAMPLE_16BIT;
*/
#define BIT_DEPTH I2S_BITS_PER_SAMPLE_16BIT
#define DATA_BUFFER_SIZE DMA_BUF_LEN_SMPL*BIT_DEPTH/8

// gpio
#define GPIO_INPUT_PIN_SEL1   (1ULL<<BTN_START_END)  // | (1ULL<<ANOTHER_GPIO)
#define ESP_INTR_FLAG_DEFAULT 0

// sd card
#define MOUNT_POINT "/sdcard" // SD card mounting directory
#define SPI_DMA_CHAN 1        // DMA channel to be used by the SPI peripheral

// #define CONFIG_FREERTOS_HZ 100

// spi bus
#ifndef USE_SPI_MODE
    #define USE_SPI_MODE    // define SPI mode
    #define PIN_NUM_MISO 19 // SDI - Serial Data In
    #define PIN_NUM_MOSI 23 // SDO - Serial Data Out
    #define PIN_NUM_CLK  18 // System clock
    #define PIN_NUM_CS   22 // Chip select
#endif

// pins
#define MIC_CLOCK_PIN  GPIO_NUM_21  // gpio 21 - MEMS MIC clock in
#define MIC_DATA_PIN   GPIO_NUM_4   // gpio 4  - MEMS MIC data out
#define BTN_START_END  GPIO_NUM_0   // gpio 0  - button
#define GPIO_OUTPUT_IO GPIO_NUM_16  // gpio 16 - no use

// i2s 
#define I2S_PORT_NUM     0
#define DMA_BUF_COUNT    64
#define DMA_BUF_LEN_SMPL 1024

// freetros
#define configTICK_RATE_HZ 1000

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

// event flags

#define BIT_(shift) (1<<shift)

enum events{REC_STARTED,   // flag informing that the recording session already started
            SPI_BUS_FREE}; // flag indicating if there are devices attached to SPI bus or not

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
    .use_apll = 1,                                        // for high accuracy clock applications, use the APLL_CLK clock source, which has the frequency range of 16 ~ 128 MHz
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
    .sample_rate = 16000,                                 // sample rate (low power mode) clock=2*16*smpl_rate
    .bits_per_sample = BIT_DEPTH,                         // 16bit resolution per sample
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,         // mono audio configuration (LL Layer defaults right-channel-second(LSB))
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,    // pcm data format
    .dma_buf_count = DMA_BUF_COUNT,                       // number of buffers, 128 max.
    .dma_buf_len = DMA_BUF_LEN_SMPL,                      // size of each buffer, 1024 max.
    .use_apll = 1,                                        // for high accuracy clock applications, use the APLL_CLK clock source, which has the frequency range of 16 ~ 128 MHz
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1              // interrupt level 1
};

// i2s pin config
i2s_pin_config_t i2s_pins_i2s = {
    .bck_io_num = MIC_CLOCK_PIN,        // clock pin
    .ws_io_num = I2S_PIN_NO_CHANGE,         
    .data_out_num = I2S_PIN_NO_CHANGE,  
    .data_in_num = MIC_DATA_PIN         // data in pin
};


// wav file header https://forum.arduino.cc/t/creating-a-wav-file-header/314260/4
struct wav_header_struct {
    char  riff[4];        /* "RIFF" little endian and "RIFX" big endian */
    long  flength;        /* file length in bytes                       */
    char  wave[4];        /* "WAVE"                                     */
    char  fmt[4];         /* "fmt "                                     */
    long  chunk_size;     /* size of FMT chunk in bytes (usually 16)    */
    short format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM    */
    short num_chans;      /* 1=mono, 2=stereo                           */
    long  srate;          /* Sampling rate in samples per second        */
    long  bytes_per_sec;  /* bytes per second = srate*bytes_per_samp    */
    short bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo             */
    short bits_per_samp;  /* Number of bits per sample                  */
    char  data[4];        /* "data"                                     */
    long  dlength;        /* data length in bytes (filelength - 44)     */
} wav_header;

struct timeval date = {// struct with date data
    .tv_sec = 0, // current date in seconds (to be fecthed from NTP)
};

size_t bytes_read; // number of bytes read by i2s_read
char dataBuffer[DATA_BUFFER_SIZE]; // data buffer to store DMA_BUF_LEN_SMPL samples from i2s

// sd card variables
sdmmc_card_t* card;
sdmmc_host_t host;
FILE* session_file = NULL;

const char mount_point[] = MOUNT_POINT; // sd card mounting point
const char* fname = "rec.wav"; // standard session file name

// freertos variables
TaskHandle_t xTaskRECHandle;   // get data from mic and save into sd card [task_handle]
TaskHandle_t xTaskSTARThandle; // starting routine [task_handle]
TaskHandle_t xTaskENDhandle;   // ending routine [task_handle]

QueueHandle_t xQueueData;            // data queue for transfering microphone data to sd card [queue_handle]
EventGroupHandle_t xEvents;          // event group to handle event flags [event_group_handle]
SemaphoreHandle_t xMutex;            // mutex to allow end of recording only when the spi interface finishes to write into sd card [sempaphore_handle]
SemaphoreHandle_t xSemaphoreBTN_ON;  // semaphore to interpret button as start button interrupt [semaphore_handle]
SemaphoreHandle_t xSemaphoreBTN_OFF; // semaphore to interpret button as end button interrupt [semaphore_handle]
SemaphoreHandle_t xSemaphoreTimer;   // semaphore to interpret timer got interrupt [semaphore_handle]

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

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
 */
void deinitialize_spi_bus(sdmmc_host_t* host);

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
 */
void deinitialize_sd_card(sdmmc_card_t** card);

/**
 * @brief Task to acquire and save audio data into SD card
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskREC(void * pvParameters);

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
 * @brief Byte swap to compensate reversed endianness of ESP32 I2S peripheral (short).
 * 
 * @param s short integer to revert
 */
void swap_byte_order_short(short* s);

/**
 * @brief Byte swap to compensate reversed endianness of ESP32 I2S peripheral (long).
 * 
 * @param l long integer to revert
 */
void swap_byte_order_long(long* l);

#endif //_MAIN_H_