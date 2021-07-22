/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "sd_driver.h"

#define TIMER_DIVIDER         16 //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   1 // sample test interval for the first timer
#define TEST_WITHOUT_RELOAD   0 // testing will be done without auto reload
#define TEST_WITH_RELOAD      1 // testing will be done with auto reload

#ifndef RTC_LIB_INCLUDED
#define RTC_LIB_INCLUDED
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
#endif

#ifndef SD_LIB_INCLUDED 
#define SD_LIB_INCLUDED // define if SD card libraries were included
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#endif

#ifndef FREERTOS_LIB_INCLUDED
#define FREERTOS_LIB_INCLUDED
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#endif

#ifndef _STAT_H_
#define _STAT_H_ // define if stat.h library was defined
#include <sys/stat.h>
#endif

#ifndef ADC_LIB_INCLUDED
#define ADC_LIB_INCLUDED
#include <driver/adc.h>
#include "esp_adc_cal.h"
#endif

static const char *TAG = "example";  // ESP log tag
#define MOUNT_POINT "/sdcard"        // SD card mounting point
#define SPI_DMA_CHAN 1               // DMA channel to be used by the SPI peripheral
#define USE_SPI_MODE                 // define SPI mode
#define VREF 1100
#define SAMPLING_TIME 500

#ifdef USE_SPI_MODE
#define PIN_NUM_MISO 19 // SDi - Serial Data In
#define PIN_NUM_MOSI 23 // SDO - Serial Data Out
#define PIN_NUM_CLK  18 // System clock
#define PIN_NUM_CS   5  // Chip select
#endif

// global variables


static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     // ADC channel in use   
static const adc_bits_width_t width = ADC_WIDTH_BIT_12; // ADC bit width 
static const adc_atten_t atten = ADC_ATTEN_DB_0;        // 0dB range attenuation
static const adc_unit_t unit = ADC_UNIT_1;              // using ADC1

sdmmc_card_t* card;
sdmmc_host_t host;
FILE* f;
const char mount_point[] = MOUNT_POINT;

TaskHandle_t xTaskADCHandle;
xQueueHandle timer_queue;

typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

struct tm date; // call struct with date data

const char* fname1 = "ex_sd_1";
const char* fname2 = "ex_sd_2";
const char* fname3 = "ex_sd_3";


// functions prototypes
void init_spi_bus();
void sd_card_init();
void sd_card_end();
void taskADC(void * pvParameters);
void check_efuse(void);
void print_char_val_type(esp_adc_cal_value_t val_type);
void timer_config(int timer_idx, bool auto_reload, double timer_interval_sec);
void IRAM_ATTR timerISR (void *param);

uint32_t adcBufferTime;
uint32_t adcBufferSample;

void app_main(void)
{
    // RTC 
    date.tm_sec = 0;//1623365971; // current date (to be fecthed from NTP)
    settimeofday(&date, NULL); // update time
    // Timer
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));
    timer_config(TIMER_0, TEST_WITHOUT_RELOAD, TIMER_INTERVAL0_SEC);
    // ADC
    xTaskCreate(taskADC, "TaskADC", configMINIMAL_STACK_SIZE+1024, NULL, 5, &xTaskADCHandle);
    //check_efuse();
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, VREF, adc_chars);
    //print_char_val_type(val_type);

    
    sd_card_init();
    //sd_card_end();
}

void init_spi_bus()
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host_temp = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host_temp.slot, &bus_cfg, SPI_DMA_CHAN);
    host = host_temp;
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }
}

void sd_card_init()
{
    esp_err_t ret;

    // Options for mounting the filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
    #else
        .format_if_mount_failed = false,
    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.

    init_spi_bus();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

void sd_card_end()
{
    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}

void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void timer_config(int timer_idx, bool auto_reload, double timer_interval_sec){
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timerISR,
                       (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

void IRAM_ATTR timerISR (void *param){
    timer_spinlock_take(TIMER_GROUP_0);
    int timer_idx = (int) param;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t timer_intr = timer_group_get_intr_status_in_isr(TIMER_GROUP_0);
    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, timer_idx);

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if (timer_intr & TIMER_INTR_T0) {
        evt.type = TEST_WITHOUT_RELOAD;
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
        timer_counter_value += (uint64_t) (TIMER_INTERVAL0_SEC * TIMER_SCALE);
        timer_group_set_alarm_value_in_isr(TIMER_GROUP_0, timer_idx, timer_counter_value);
    } else if (timer_intr & TIMER_INTR_T1) {
        evt.type = TEST_WITH_RELOAD;
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
    } else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, timer_idx);

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_queue, &evt, NULL);
    timer_spinlock_give(TIMER_GROUP_0);
}

void taskADC(void * pvParameters){

  while(1){
    timer_event_t evt;
    xQueueReceive(timer_queue, &evt, portMAX_DELAY);
    
    adcBufferTime = esp_timer_get_time();
    adcBufferSample = adc1_get_raw((adc1_channel_t)channel);
    f = open_file(fname2, "a");
    fprintf(f, "%d %d %d\n", (int32_t) time(NULL), adcBufferTime, adcBufferSample);
    ESP_LOGI(TAG, "File written");
    close_file(f);
  }

}