/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date January 20 2022
 */

#include "main.h"



void app_main()
{
    // configure ledc
    // periph_module_enable(PERIPH_LEDC_MODULE);
    // ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    // ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // configure i2s
    periph_module_enable(PERIPH_I2S0_MODULE);
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config, 32, &xQueueData));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins));
    ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));

    vTaskDelay(pdMS_TO_TICKS(3000));

    i2s_set_sample_rates(I2S_PORT_NUM, SAMPLE_RATE);
    
    vTaskDelete(NULL);
}



