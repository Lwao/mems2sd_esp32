        #include "soc/timer_group_struct.h"

        #include "soc/timer_group_reg.h"
        //TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // cancel writing protection
        //TIMERG1.wdt_feed=1; // feeds wdt
        //TIMERG1.wdt_wprotect=0; // restore writing protection
        if(xEventGroupGetBitsFromISR(xEvents) & BIT_(REC_STARTED))
        {
            ESP_LOGI(MEMS_MIC_TAG, "Hello MEMS mic!");
            i2s_read(I2S_PORT_NUM, (void*) inBuffer, I2S_DMA_BUFF_LEN_BYTES, &bytes_read, portMAX_DELAY); // read bytes from mic with i2s
            xQueueSend(xQueueData,&inBuffer,0);
        }
        vTaskDelay(1);
        //taskYIELD();
        //TIMERG0.wdt_wprotect=0;
        
        //if(xSemaphoreTimer!=NULL && xSemaphoreTakeFromISR(xSemaphoreTimer,&xHighPriorityTaskWoken)==pdTRUE)//if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_MIC_READING), pdTRUE, pdFALSE, portMAX_DELAY) & BIT_(ENABLE_MIC_READING))
        //{ 
            //ESP_LOGI(MEMS_MIC_TAG, "Hello MEMS mic!");
            //i2s_read(I2S_PORT_NUM, (void*) inBuffer, I2S_DMA_BUFF_LEN_BYTES, &bytes_read, portMAX_DELAY); // read bytes from mic with i2s
            //memcpy(outBuffer, inBuffer, I2S_DMA_BUFF_LEN_BYTES); // copy input buffer to output buffer
            //xQueueSend(xQueueData,&inBuffer,0);
            //xEventGroupClearBits(xEvents, BIT_(ENABLE_MIC_READING));
            //xEventGroupSetBits(xEvents, BIT_(ENABLE_SDCARD_WRITING)); // notify sd card task to write data
            
        //}
        //vTaskDelay(1);
        //taskYIELD();
        //*/
        /*
        //#include "soc/timer_group_struct.h"
        //#include "soc/timer_group_reg.h"
        //TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
        //TIMERG0.wdt_feed=1;
        //TIMERG0.wdt_wprotect=0;
        if(xSemaphoreTimer!=NULL && xSemaphoreTakeFromISR(xSemaphoreTimer,&xHighPriorityTaskWoken)==pdTRUE) // timer reached counting
        {
            //inMic.cntr++;
            inMic.cntr = esp_timer_get_time();
            inMic.reading = esp_timer_get_time();//gpio_get_level(MIC_DATA_PIN);
            ESP_LOGI(MEMS_MIC_TAG, "Hello MEMS mic!");
            xQueueSend(xQueueData,&inMic,0);
        } else {vTaskDelay(1);}
        */






 ///*
        while(xQueueData!=NULL && xQueueReceive(xQueueData, &outBuffer, 0)==pdTRUE)//if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_SDCARD_WRITING), pdTRUE, pdFALSE, portMAX_DELAY) & BIT_(ENABLE_SDCARD_WRITING)) // wait for data to be read
        {
            ESP_LOGI(SD_CARD_TAG, "Hello SD card!");
            fwrite(outBuffer, I2S_DMA_BUFF_LEN_BYTES, 1, session_file); // write output buffer to sd card current file
            //xEventGroupClearBits(xEvents, BIT_(ENABLE_SDCARD_WRITING)); // clear event flag to reaccess task
            //xEventGroupSetBits(xEvents, BIT_(ENABLE_MIC_READING));
        }
        vTaskDelay(1);
        //*/
        /*
        while(xQueueData!=NULL && xQueueReceive(xQueueData, &inSd, 0)==pdTRUE)
        {
            ESP_LOGI(SD_CARD_TAG, "Hello SD card!");
            fprintf(session_file, "%d,%d\n", inSd.reading, inSd.cntr);
        }
        vTaskDelay(1);
        */