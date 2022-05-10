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
#include <stdlib.h>
#include <math.h>

#include "driver/i2s.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

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
#define SAMPLE_RATE 100000
/* Bit depth
0 -> 8-bits  = I2S_BITS_PER_SAMPLE_8BIT ;
1 -> 16-bits = I2S_BITS_PER_SAMPLE_16BIT;
*/
#define BIT_DEPTH I2S_BITS_PER_SAMPLE_16BIT
#define DATA_BUFFER_SIZE DMA_BUF_LEN_SMPL*BIT_DEPTH/8

// i2s 
#define I2S_PORT_NUM     0
#define DMA_BUF_COUNT    64
#define DMA_BUF_LEN_SMPL 1024

// pins
#define MIC_CLOCK_PIN  GPIO_NUM_21  // gpio 21 - MEMS MIC clock in
#define MIC_DATA_PIN   GPIO_NUM_4   // gpio 4  - MEMS MIC data out

// ledc
#define LEDC_GPIO_OUT GPIO_NUM_21
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LDC_TIMER LEDC_TIMER_0
#define LDC_DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define LDC_DUTY_CYCLE 50
#define LDC_FREQUENCY 4000000

#define COMPUTE_DUTY_RES(duty, res) (uint32_t)((float)pow(2,res)*(float)duty/(float)(100)-1)

// i2s acquisition config
i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM, // master driver | receiving data (RX) | in PDM modulation  
    .sample_rate = 8000,                           // sample rate
    .bits_per_sample = BIT_DEPTH,                         // 16bit resolution per sample
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,         // mono audio configuration
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,    // pcm data format
    .dma_buf_count = DMA_BUF_COUNT,                       // number of buffers, 128 max.
    .dma_buf_len = DMA_BUF_LEN_SMPL,                      // size of each buffer, 1024 max.
    .use_apll = 1,                                        // for high accuracy clock applications, use the APLL_CLK clock source, which has the frequency range of 16 ~ 128 MHz
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // interrupt level 1
    .fixed_mclk = 0
};

// i2s pin config
i2s_pin_config_t i2s_pins = {
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = MIC_CLOCK_PIN,         // clock pin
    .data_out_num = I2S_PIN_NO_CHANGE,  
    .data_in_num = MIC_DATA_PIN         // data in pin
};

// ledc timer config
ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_1_BIT,
    .freq_hz = LDC_FREQUENCY,
    .speed_mode = LDC_SPEED_MODE,
    .timer_num = LDC_TIMER
};

//  ledc channel config
ledc_channel_config_t ledc_channel = {
    .channel    = LEDC_CHANNEL,
    .duty       = 1,
    .gpio_num   = LEDC_GPIO_OUT,
    .speed_mode = LDC_SPEED_MODE,
    .timer_sel  = LDC_TIMER 
};


QueueHandle_t xQueueData;

#endif //_MAIN_H_