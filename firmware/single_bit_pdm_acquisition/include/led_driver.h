/**
 * @file led_driver.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 27 2021
 */


#ifndef _LED_DRIVER_H_ 
#define _LED_DRIVER_H_ 

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

#include "driver/ledc.h"
#include "driver/gpio.h"

#define LED_TAG  "led_config"

#define LEDC_GPIO_OUT_RED GPIO_NUM_32
#define LEDC_GPIO_OUT_GREEN GPIO_NUM_33
#define LEDC_GPIO_OUT_BLUE GPIO_NUM_27

#define LEDC_CHANNEL_RED LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE LEDC_CHANNEL_2

#define LDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#define LDC_TIMER LEDC_TIMER_0
#define LDC_DUTY_RESOLUTION LEDC_TIMER_12_BIT
#define LDC_FREQUENCY 5000
#define NUM_LDC 3

#define LED_STD_LEVEL 0xFC0
#define LED_OFF_LEVEL 0xFFFF

#define BIT_(shift) (1<<shift)

// flags to access led color
typedef enum {RED_COLOR, GREEN_COLOR, BLUE_COLOR, MAGENTA_COLOR, YELLOW_COLOR, CYAN_COLOR, WHITE_COLOR, OFF_COLOR} colors_t;
typedef enum {RED_CH, GREEN_CH, BLUE_CH} led_channel_t;

/**
 * @brief Change color of current attached to LDC peripheral RGB LED.
 * 
 * According to which stage of the firmware is running, the RGB LED should assume the following colors:
 *  - Setup: white;
 *  - IDLE mode: blue;
 *  - Start recording: green;
 *  - End recording: red;
 *  - Recording: off;
 * 
 * @param ledc_channel array of ledc channels containing all colors channels.
 * @param ledc_time ledc timer.
 * @param color color to which be turned on in the RGB LED.
 */
void change_color(ledc_channel_config_t (*ledc_channel)[NUM_LDC], ledc_timer_config_t *ledc_timer, colors_t color);

#endif // _LED_DRIVER_H_