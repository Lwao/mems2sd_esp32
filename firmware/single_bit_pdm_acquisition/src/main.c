/**
 * @file main.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 24 2021
 */

#include "main.h"

/*
 * Main section
 * --------------------
 * Main function with general single time configuration in BOOT time
 */

void app_main(void)
{  
    BaseType_t xReturnedTask[3];

    // configure gpio pins
    ESP_ERROR_CHECK(gpio_config(&in_conf1));                              // initialize input pin 1 configuration - on/off button
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));     // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_START_END, ISR_BTN, NULL));  // hook isr handler for specific gpio pin

    // configure i2s
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config_i2s, 32, &xQueueData));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins_i2s));
    ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

    // init_config_file(&configurations);
    // parse_config_file(&host, &card, &configurations);

    // create semaphores/event groups
    xEvents           = xEventGroupCreate();
    xMutex            = xSemaphoreCreateMutex();
    xSemaphoreBTN_ON  = xSemaphoreCreateBinary();
    xSemaphoreBTN_OFF = xSemaphoreCreateBinary();
    
    if(xQueueData == NULL){ // tests if queue creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create data queue.\n");
        while(1);
    }
    if(xEvents == NULL){ // tests if event group creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create event group.\n");
        while(1);
    }
    if((xSemaphoreBTN_ON == NULL) || (xSemaphoreBTN_OFF == NULL) || (xMutex == NULL)){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create semaphores.\n");
        while(1);
    }  

    // set flag informing that the recording is stopped
    xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART, "taskSTART", 8192, NULL, configMAX_PRIORITIES-2, &xTaskSTARThandle, PRO_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskEND,   "taskEND",   8192, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,   PRO_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskREC,   "taskREC",   8192, NULL, configMAX_PRIORITIES-3, &xTaskRECHandle,   APP_CPU_NUM);
   
    for(int itr=0; itr<3; itr++) // iterate over tasks 
    {
        if(xReturnedTask[itr] == pdFAIL){ // tests if task creation fails
            ESP_LOGE(SETUP_APP_TAG, "Failed to create task %d.\n", itr);
            while(1);
        }
    }

    // suspend tasks for recording mode
    vTaskSuspend(xTaskRECHandle);
    vTaskSuspend(xTaskENDhandle);
    // vTaskSuspend(xTaskSTARThandle);

    ESP_LOGI(SETUP_APP_TAG, "Successful BOOT!");
    vTaskDelete(NULL);
}

/*
 * freeRTOS section
 * --------------------
 * Declaration of freeRTOS tasks
 */

void vTaskSTART(void * pvParameters)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    while(1)
    {
        if(xSemaphoreBTN_ON!=NULL && xSemaphoreTakeFromISR(xSemaphoreBTN_ON,&xHighPriorityTaskWoken)==pdTRUE) // button was pressed to turn ON recording
        {
            ESP_LOGI(START_REC_TAG, "Starting record session.");

            // initialize SPI bus and mount SD card
            while(initialize_spi_bus(&host)!=1){vTaskDelay(100);}
            while(initialize_sd_card(&host, &card)!=1){vTaskDelay(100);}
            vTaskDelay(100);
            
            // reset RTC 
            settimeofday(&date, NULL); // update time
            
            // open new file in append mode
            while(session_file==NULL) session_file = open_file(fname, "a");

            // write first part of .wav header
            init_wav_header(&session_file, &wav_header, SAMPLE_RATE, BIT_DEPTH);

            // set flag informing that the recording already started
            xEventGroupSetBits(xEvents, BIT_(REC_STARTED));

            // start i2s
            ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));        // start i2s clocking mic to low-power mode
            vTaskDelay(100);                                 // structural delay to changes take place
            i2s_set_sample_rates(I2S_PORT_NUM, SAMPLE_RATE); // change mic to ultrasonic mode

            ESP_LOGI(START_REC_TAG, "Recording session started.");

            // resume tasks for recording mode
            vTaskResume(xTaskRECHandle);
            vTaskResume(xTaskENDhandle);
        
            // locking task
            vTaskSuspend(xTaskSTARThandle); // suspend actual task
        } else {vTaskDelay(1);}
    }
}

void vTaskEND(void * pvParameters)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    while(1)
    {
        if(xSemaphoreBTN_OFF!=NULL && xSemaphoreTakeFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken)==pdTRUE) // button was pressed to turn OFF recording
        {
            ESP_LOGI(END_REC_TAG, "Stoping recording...");

            // i2s stop
            ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

            // wait to get mutex indicating total end of rec task
            while(xSemaphoreTake(xMutex,portMAX_DELAY)==pdFALSE);

            // suspend tasks for recording mode
            vTaskSuspend(xTaskRECHandle);

            ESP_LOGI(END_REC_TAG, "Recording session finished.");

            // finish to write the .wav header by extracting size of payload
            finish_wav_header(&session_file, &wav_header, fname);

            // close file in use
            close_file(&session_file);
            
            // dismount SD card and free SPI bus (in the given order) 
            deinitialize_sd_card(&card);
            deinitialize_spi_bus(&host);

            // clear flag informing that the recording is stopped
            xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

            // resume task to wait for recording trigger
            vTaskResume(xTaskSTARThandle);

            ESP_LOGI(END_REC_TAG, "Returning to IDLE mode.");

            xSemaphoreGive(xMutex);

            // locking task
            vTaskSuspend(xTaskENDhandle); // suspend actual task
        } else {vTaskDelay(1);}
    }
}

void vTaskREC(void * pvParameters)
{
    i2s_event_t i2s_evt;
    while(1)
    {
        if(
            (xQueueData!=NULL)                               &&
            (xQueueReceive(xQueueData, &i2s_evt, 0)==pdTRUE) &&
            (i2s_evt.type == I2S_EVENT_RX_DONE)              &&
            (xMutex!=NULL)                                   &&
            (xSemaphoreTake(xMutex,portMAX_DELAY)==pdTRUE)
        ) // wait for data to be read
        {
            // ESP_LOGI(SD_CARD_TAG, "Hello SD card!");
            i2s_read(I2S_PORT_NUM, (void*) dataBuffer, DATA_BUFFER_SIZE, &bytes_read, portMAX_DELAY); // read bytes from DMA
            fwrite(dataBuffer, bytes_read, 1, session_file); // write buffer to sd card current file
			fsync(fileno(session_file));
            xSemaphoreGive(xMutex);
            //printf("%lx %lx %lx %lx\n", dataBuffer[0], dataBuffer[1], dataBuffer[2], dataBuffer[3]);
        }
        vTaskDelay(1);
        // if (evt.type == I2S_EVENT_RX_Q_OVF) printf("RX data dropped\n");
    }
}

/*
 * ISR section
 * --------------------
 * Declaration of interrupt service routines from push-buttons, timers, etc.
 */

static void IRAM_ATTR ISR_BTN()
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    if(xEventGroupGetBitsFromISR(xEvents) & BIT_(REC_STARTED)){xSemaphoreGiveFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken);} // recording started, so stop recording
    else{xSemaphoreGiveFromISR(xSemaphoreBTN_ON,&xHighPriorityTaskWoken);} // recording stopd, so start recording
    portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}