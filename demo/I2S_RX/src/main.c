/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 4 2022
 */

#include "main.h"

void app_main()
{
    // configure i2s
    periph_module_enable(PERIPH_I2S0_MODULE);
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config, 32, &xQueueData));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins));
    ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));

    periph_module_enable(PERIPH_I2S1_MODULE);
    ESP_ERROR_CHECK(i2s_driver_install(1, &i2s_config_mimic, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(1, &i2s_pins_mimic));
    ESP_ERROR_CHECK(i2s_start(1));

    xTaskCreatePinnedToCore(vTaskRX, "task_rx", 8192, NULL, configMAX_PRIORITIES-1, &xTaskRX, PRO_CPU_NUM);

    vTaskDelete(NULL);
}

void vTaskRX(void * pvParameters)
{
    i2s_event_t i2s_evt;
    while(1)
    {
        while(
            (xQueueReceive(xQueueData, &i2s_evt, 0)==pdTRUE) &&
            (i2s_evt.type == I2S_EVENT_RX_DONE)              
        ) 
        {
            i2s_read(I2S_PORT_NUM, (void*) data, DATA_BUFFER_SIZE, &bytes_read, portMAX_DELAY); 
        }
        printf("%lx %lx %lx %lx\n", data[0], data[1], data[2], data[3]);
        // printf("%x %x %x %x\n", data[0], data[1], data[2], data[3]);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

