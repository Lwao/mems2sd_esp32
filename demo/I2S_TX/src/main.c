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
    // fill data buffer
    for(int ii=0; ii<DMA_BUF_LEN_SMPL; ii++)
    {
        // if(ii%2) data[ii] = 0x11111111;
        // else data[ii] = 0xEEEEEEEE;
        // if(ii%2) data[ii] = 0x1111CCCC;
        // else data[ii] = 0xEEEE5555;
        if(ii%2) data[ii] = 0x01234567;
        else data[ii] = 0x89ABCDEF;
    }

    // configure i2s
    periph_module_enable(PERIPH_I2S0_MODULE);
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins));
    ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));

    xTaskCreatePinnedToCore(vTaskTX, "task_tx", 8192, NULL, configMAX_PRIORITIES-1, &xTaskTX, PRO_CPU_NUM);

    vTaskDelete(NULL);
}

void vTaskTX(void * pvParameters)
{
    while(1)
    {
        i2s_write(I2S_PORT_NUM, (void*) data, DATA_BUFFER_SIZE, &bytes_written, 10);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

