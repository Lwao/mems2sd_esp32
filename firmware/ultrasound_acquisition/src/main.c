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
    BaseType_t xReturnedTask[4];

    // configure ldc
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    for (int ch=0; ch<NUM_LDC; ch++) ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[ch]));
    change_color(&ledc_channel, &ledc_timer, WHITE_COLOR);

    // configure gpio pins
    ESP_ERROR_CHECK(gpio_config(&in_conf1));                              // initialize input pin 1 configuration - on/off button
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));     // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_START_END, ISR_BTN, NULL));  // hook isr handler for specific gpio pin

    // configure i2s
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config_i2s, 32, &xQueueEvent));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins_i2s));
    ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

    // parse configuration file from SD card
    parse_config_file(&host, &card, &configurations);

    // create semaphores/event groups
    xQueueData        = xQueueCreate(NUM_QUEUE_BUF,DMA_BUF_LEN_SMPL*sizeof(long)); 
    xEvents           = xEventGroupCreate();
    xMutexREC         = xSemaphoreCreateMutex();
    xMutexDSP         = xSemaphoreCreateMutex();
    xSemaphoreBTN_ON  = xSemaphoreCreateBinary();
    xSemaphoreBTN_OFF = xSemaphoreCreateBinary();

    if(configurations.record_session_duration>0)
    {
        xTimerInSession = xTimerCreate("TimerInSession", 
                                        pdMS_TO_TICKS(configurations.record_session_duration*1000),
                                        pdFALSE, 
                                        (void*)0, 
                                        vTimerInSessionCbck);
        xTimerStop(xTimerInSession,0);
        if(xTimerInSession == NULL){ // tests if timer creation fails
            ESP_LOGE(SETUP_APP_TAG, "Failed to create in session timer.");
            while(1);
        }
        ESP_LOGI(SETUP_APP_TAG, "Record session duration will be timed.");
    }

    if(configurations.interval_between_record_session>0)
    {
        xTimerOutSession = xTimerCreate("TimerOutSession", 
                                        pdMS_TO_TICKS(configurations.interval_between_record_session*1000), 
                                        pdFALSE, 
                                        (void*)0, 
                                        vTimerOutSessionCbck);
        xTimerStop(xTimerOutSession,0);
        if(xTimerOutSession == NULL){ // tests if timer creation fails
            ESP_LOGE(SETUP_APP_TAG, "Failed to create out session timer.");
            while(1);
        }
        ESP_LOGI(SETUP_APP_TAG, "Interval between record session will be timed.");
    }

    if((xQueueEvent == NULL) || (xQueueData == NULL)){ // tests if queue creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create data queue.");
        while(1);
    }
    if(xEvents == NULL){ // tests if event group creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create event group.");
        while(1);
    }
    if((xSemaphoreBTN_ON == NULL) || (xSemaphoreBTN_OFF == NULL) || (xMutexREC == NULL) || (xMutexDSP == NULL)){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create semaphores.");
        while(1);
    }  

    // set flag informing that the recording is stopped
    xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART, "taskSTART", 8192/2, NULL, configMAX_PRIORITIES-2, &xTaskSTARThandle, PRO_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskEND,   "taskEND",   8192/2, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,   PRO_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskREC,   "taskREC",   8192/4, NULL, configMAX_PRIORITIES-3, &xTaskRECHandle,   APP_CPU_NUM);
    xReturnedTask[3] = xTaskCreatePinnedToCore(vTaskDSP,   "taskDSP",   8192/4, NULL, configMAX_PRIORITIES-3, &xTaskDSPHandle,   PRO_CPU_NUM);
   
    for(int itr=0; itr<4; itr++) // iterate over tasks 
    {
        if(xReturnedTask[itr] == pdFAIL){ // tests if task creation fails
            ESP_LOGE(SETUP_APP_TAG, "Failed to create task %d.", itr);
            while(1);
        }
    }

    // suspend tasks for recording mode
    vTaskSuspend(xTaskRECHandle);
    vTaskSuspend(xTaskDSPHandle);
    vTaskSuspend(xTaskENDhandle);

    ESP_LOGI(SETUP_APP_TAG, "Successful BOOT!");

    // start out session timer to time-in when to start recording
    if(configurations.interval_between_record_session>0) 
    {
        xTimerReset(xTimerOutSession,0); 
        xTimerStart(xTimerOutSession,0); 
    }

    change_color(&ledc_channel, &ledc_timer, BLUE_COLOR);

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
            change_color(&ledc_channel, &ledc_timer, GREEN_COLOR);

            ESP_LOGI(START_REC_TAG, "Starting record session.");
            
            // initialize SPI bus and mount SD card
            while(initialize_spi_bus(&host)!=1){vTaskDelay(100);}
            while(initialize_sd_card(&host, &card)!=1){vTaskDelay(100);}
            vTaskDelay(100);
            
            // reset RTC 
            settimeofday(&date, NULL); // update time
            
            // open new file in append mode
            // while(session_file==NULL) session_file = open_file(configurations.file_name, "a");
            while(session_file==NULL) session_file = open_file("rec.wav", "a");

            // write first part of .wav header
            init_wav_header(&session_file, &wav_header, (int)SAMPLE_RATE_ULT_PCM, BIT_DEPTH);

            // set flag informing that the recording already started
            xEventGroupSetBits(xEvents, BIT_(REC_STARTED));

            // start i2s
            ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));        // start i2s clocking mic to low-power mode
            vTaskDelay(100);                                 // structural delay to changes take place
            i2s_set_sample_rates(I2S_PORT_NUM, SAMPLE_RATE_ULT_PDM); // change mic to ultrasonic mode

            ESP_LOGI(START_REC_TAG, "Recording session started.");

            // start in session timer to time-in when to stop recording
            if(configurations.record_session_duration>0) 
            {
                xTimerReset(xTimerInSession,0); 
                xTimerStart(xTimerInSession,0); 
            }
            
            change_color(&ledc_channel, &ledc_timer, configurations.recording_color);

            init_app_cic(&cic);
            init_app_fir(&fir);

            // resume tasks for recording mode
            vTaskResume(xTaskDSPHandle);
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
            change_color(&ledc_channel, &ledc_timer, RED_COLOR);

            ESP_LOGI(END_REC_TAG, "Stoping recording...");

            // i2s stop
            ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

            // wait to get mutex indicating total end of rec task
            while(xSemaphoreTake(xMutexREC,portMAX_DELAY)==pdFALSE);
            while(xSemaphoreTake(xMutexDSP,portMAX_DELAY)==pdFALSE);

            // suspend tasks for recording mode
            vTaskSuspend(xTaskRECHandle);
            vTaskSuspend(xTaskDSPHandle);

            ESP_LOGI(END_REC_TAG, "Recording session finished.");

            // finish to write the .wav header by extracting size of payload
            // finish_wav_header(&session_file, &wav_header, configurations.file_name);
            finish_wav_header(&session_file, &wav_header, "rec.wav");

            // close file in use
            close_file(&session_file);

            // engage next file name
            // get_file_name(&configurations);
            
            // dismount SD card and free SPI bus (in the given order) 
            deinitialize_sd_card(&card);
            deinitialize_spi_bus(&host);

            // clear flag informing that the recording is stopped
            xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

            // resume task to wait for recording trigger
            vTaskResume(xTaskSTARThandle);

            ESP_LOGI(END_REC_TAG, "Returning to IDLE mode.");

            xSemaphoreGive(xMutexREC);
            xSemaphoreGive(xMutexDSP);

            // start out session timer to time-in when to start recording
            if(configurations.interval_between_record_session>0) 
            {
                xTimerReset(xTimerOutSession,0); 
                xTimerStart(xTimerOutSession,0); 
            }

            change_color(&ledc_channel, &ledc_timer, BLUE_COLOR);

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
            (xQueueEvent!=NULL)                               &&
            (xQueueReceive(xQueueEvent, &i2s_evt, 0)==pdTRUE) &&
            (i2s_evt.type == I2S_EVENT_RX_DONE)               &&
            (xMutexREC!=NULL)                                 &&
            (xSemaphoreTake(xMutexREC,portMAX_DELAY)==pdTRUE)
        ) // wait for data to be read
        {
            // ESP_LOGI(SD_CARD_TAG, "Hello REC task!");
            i2s_read(I2S_PORT_NUM, (void*) input_buffer, DATA_BUFFER_SIZE, &bytes_read, portMAX_DELAY); // read bytes from DMA

            // enqueue data for next task
            xQueueSend(xQueueData,&input_buffer,portMAX_DELAY);

            // fwrite(input_buffer, bytes_read, 1, session_file); // write buffer to sd card current file
			// fsync(fileno(session_file));
            xSemaphoreGive(xMutexREC);
        }
        vTaskDelay(1);
        // if (evt.type == I2S_EVENT_RX_Q_OVF) printf("RX data dropped\n");
    }
}

void vTaskDSP(void * pvParameters)
{
    while(1)
    {
        if(
            (xQueueData!=NULL)                                         &&
            (xQueueReceive(xQueueData, &processing_buffer, 0)==pdTRUE) &&
            (xMutexDSP!=NULL)                                          &&
            (xSemaphoreTake(xMutexDSP,portMAX_DELAY)==pdTRUE)
        ) // wait for data to be read
        {
            // ESP_LOGI(SD_CARD_TAG, "Hello DSP task!");
            process_app_cic(&cic, &processing_buffer, &output_buffer);
            process_app_fir(&fir, fir_coeffs, &output_buffer);
            fwrite(output_buffer, 4*DMA_BUF_LEN_SMPL, 1, session_file); // write buffer to sd card current file
			fsync(fileno(session_file));
            xSemaphoreGive(xMutexDSP);
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

static void vTimerInSessionCbck()
{ 
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    if((xEventGroupGetBitsFromISR(xEvents) & BIT_(REC_STARTED)))      // recording is started
    xSemaphoreGiveFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken); // so stop recording
    xTimerStop(xTimerInSession,0); 
    portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

static void vTimerOutSessionCbck()
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    if(!(xEventGroupGetBitsFromISR(xEvents) & BIT_(REC_STARTED)))    // recording is stopped, 
    xSemaphoreGiveFromISR(xSemaphoreBTN_ON,&xHighPriorityTaskWoken); // so start recording
    xTimerStop(xTimerOutSession,0); 
    portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}
