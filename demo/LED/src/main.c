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
    

    xEvents = xEventGroupCreate();
    xTimer  = xTimerCreate("Timer",pdMS_TO_TICKS(TIMER_EXPIRATION),pdTRUE,(void*)0,vTimerCbck);

    xTaskCreatePinnedToCore(vTaskUpdateColor, 
                            "TASK_UPDATE_COLOR",   
                            configMINIMAL_STACK_SIZE+1024, 
                            NULL, 
                            configMAX_PRIORITIES-1, 
                            &xTaskUpdateColor, 
                            PRO_CPU_NUM);

    xTimerStart(xTimer,0);
    
    vTaskDelete(NULL);
}



void vTaskUpdateColor(void * pvParameters)
{
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(UPDATE_COLOR), pdTRUE, pdTRUE, portMAX_DELAY) & BIT_(UPDATE_COLOR))
        {
            ESP_LOGI("UPDATE_COLOR", "Hello World!");
        } else{vTaskDelay(1);}
    }
}

void vTimerCbck(TimerHandle_t xTimer){xEventGroupSetBits(xEvents, BIT_(UPDATE_COLOR));}
