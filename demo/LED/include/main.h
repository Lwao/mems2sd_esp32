/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date January 20 2022
 */


#ifndef _MAIN_H_ 
#define _MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define LEDC_GPIO_OUT_RED GPIO_NUM_32
#define LEDC_GPIO_OUT_GREEN GPIO_NUM_33
#define LEDC_GPIO_OUT_BLUE GPIO_NUM_27

#define LEDC_CHANNEL_RED LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE LEDC_CHANNEL_2

#define LDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#define LDC_TIMER LEDC_TIMER_0
#define LDC_DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define LDC_FREQUENCY 5000
#define NUM_LDC 3


#define TIMER_EXPIRATION 5000

#define BIT_(shift) (1<<shift)
#define COMPUTE_DUTY_RES(duty, res) (uint32_t)((float)pow(2,res)*(float)duty/(float)(100)-1)


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

enum events{UPDATE_COLOR}; 

// freertos variables
TaskHandle_t xTaskUpdateColor;
TimerHandle_t xTimer;
EventGroupHandle_t xEvents;

float fixed_duty_cycle[3] = {90, 25, 5};

/**
 * @brief Task to update RGB LED color
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskUpdateColor(void * pvParameters);

/**
 * @brief Software timer callback
 */
void vTimerCbck(TimerHandle_t xTimer);

#endif //_MAIN_H_