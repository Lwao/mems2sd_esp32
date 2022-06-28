/**
 * @file led_driver.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 27 2021
 */

#include "led_driver.h"

void init_led()
{
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
    float fixed_duty_cycle[3] = {90, 25, 5};

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    for (int ch=0; ch<NUM_LDC; ch++) ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[ch]));

    //ledc_set_freq(LEDC_MODE, LEDC_TIMER, 2500);
    for (int ch=0; ch<NUM_LDC; ch++) 
    {
        ESP_ERROR_CHECK(ledc_set_duty(ledc_channel[ch].speed_mode, 
                                    ledc_channel[ch].channel, 
                                    COMPUTE_DUTY_RES(fixed_duty_cycle[ch], ledc_timer.duty_resolution)));
        ESP_ERROR_CHECK(ledc_update_duty(ledc_channel[ch].speed_mode, 
                                        ledc_channel[ch].channel));
    }
}



