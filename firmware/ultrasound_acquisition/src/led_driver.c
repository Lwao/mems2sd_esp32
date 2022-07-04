/**
 * @file led_driver.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 27 2021
 */

#include "led_driver.h"

void change_color(ledc_channel_config_t (*ledc_channel)[NUM_LDC], ledc_timer_config_t *ledc_timer, colors_t color)
{
    // set duty cycle according to color
    switch(color)
    {
        case RED_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_OFF_LEVEL));
            break;

        case GREEN_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_OFF_LEVEL));
            break;

        case BLUE_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_STD_LEVEL));
            break;

        case MAGENTA_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_STD_LEVEL));
            break;

        case YELLOW_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_OFF_LEVEL));
            break;

        case CYAN_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_STD_LEVEL));
            break;

        case WHITE_COLOR:
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_STD_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_STD_LEVEL));
            break;

        default: // OFF_COLOR
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[RED_CH].speed_mode, (*ledc_channel)[RED_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[GREEN_CH].speed_mode, (*ledc_channel)[GREEN_CH].channel, LED_OFF_LEVEL));
            ESP_ERROR_CHECK(ledc_set_duty((*ledc_channel)[BLUE_CH].speed_mode, (*ledc_channel)[BLUE_CH].channel, LED_OFF_LEVEL));
            break;
    }

    // update duty cycle
    for(int ch=0; ch<NUM_LDC; ch++) ESP_ERROR_CHECK(ledc_update_duty((*ledc_channel)[ch].speed_mode, (*ledc_channel)[ch].channel));
}



