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

    // inBuffer  = (char*) calloc(I2S_DMA_BUFF_LEN_BYTES, sizeof(char));
    // outBuffer = (char*) calloc(I2S_DMA_BUFF_LEN_BYTES, sizeof(char));

    // configure gpio pins
    //ESP_ERROR_CHECK(gpio_config(&out_conf));                              // initialize output pin configuration - no use
    ESP_ERROR_CHECK(gpio_config(&in_conf1));                              // initialize input pin 1 configuration - on/off button
    //ESP_ERROR_CHECK(gpio_config(&in_conf2));                              // initialize input pin 2 configuration - mic data in
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));     // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_START_END, ISR_BTN, NULL));  // hook isr handler for specific gpio pin

    // configure timer
    
    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf));                                     // initialize timer
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));                                  // start value for counting
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_COUNT));                          // interrupt value to fire isr
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));                                           // enable timer interruption
    ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_TIMER, NULL, ESP_INTR_FLAG_IRAM)); // associate ISR callback function to timer
    
    // configure i2s
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins));
    ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

    // configure PWM clock - low-power mode
    //ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_low));
    //ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // create queue/semaphores/event groups
    xQueueData        = xQueueCreate(I2S_NUM_BUFF,I2S_DMA_BUFF_LEN_BYTES*sizeof(char)); // 32 bits = 4 bytes
    xEvents           = xEventGroupCreate();
    xSemaphoreBTN_ON  = xSemaphoreCreateBinary();
    xSemaphoreBTN_OFF = xSemaphoreCreateBinary();
    xSemaphoreTimer   = xSemaphoreCreateBinary();
    
    if(xQueueData == NULL){ // tests if queue creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create data queue.\n");
        while(1);
    }
    if(xEvents == NULL){ // tests if event group creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create event group.\n");
        while(1);
    }
    if((xSemaphoreBTN_ON == NULL) || (xSemaphoreBTN_OFF == NULL) || (xSemaphoreTimer == NULL)){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create binary semaphores.\n");
        while(1);
    }  

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART,   "taskSTART", 8192, NULL, configMAX_PRIORITIES-2, &xTaskSTARThandle,   PRO_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskMEMSmic, "taskMIC",   8192, NULL, configMAX_PRIORITIES-3, &xTaskMEMSmicHandle, APP_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskSDcard,  "taskSD",    8192, NULL, configMAX_PRIORITIES-4, &xTaskSDcardHandle,  PRO_CPU_NUM);
    xReturnedTask[3] = xTaskCreatePinnedToCore(vTaskEND,     "taskEND",   8192, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,     PRO_CPU_NUM);

    for(int itr=0; itr<4; itr++) // iterate over tasks 
    {
        if(xReturnedTask[itr] == pdFAIL){ // tests if task creation fails
            ESP_LOGE(SETUP_APP_TAG, "Failed to create task %d.\n", itr);
            while(1);
        }
    }

    // suspend tasks for recording mode
    vTaskSuspend(xTaskMEMSmicHandle);
    vTaskSuspend(xTaskSDcardHandle);
    vTaskSuspend(xTaskENDhandle);

    // set flag informing that the recording is stopped
    xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

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

            // initialize SPI bus and mout SD card
            initialize_spi_bus(&host);
            initialize_sd_card(&host, &card);
            
            // reset RTC 
            settimeofday(&date, NULL); // update time
            
            // open new file in append mode
            while(session_file==NULL){session_file = open_file(fname, "a");}
            //fprintf(session_file, "data,time\n");

            // configure PWM clock - ultrasonic mode
            //ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_ultra));
            //ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_std));
            //ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

            // set flag informing that the recording already started
            xEventGroupSetBits(xEvents, BIT_(REC_STARTED));
            xEventGroupSetBits(xEvents, BIT_(ENABLE_MIC_READING));

            // start i2s
            ESP_ERROR_CHECK(i2s_start(I2S_PORT_NUM));

            ESP_LOGI(START_REC_TAG, "Recording session started.");

            // resume tasks for recording mode
            vTaskResume(xTaskMEMSmicHandle);
            vTaskResume(xTaskSDcardHandle);
            vTaskResume(xTaskENDhandle);
        
            // start timer
            ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));

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
            // pause timer
            ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));

            // suspend tasks for recording mode
            vTaskSuspend(xTaskMEMSmicHandle);

            // stuck in loop while the data queue is not empty
            //while(uxQueueMessagesWaitingFromISR(xQueueData)>0) vTaskDelay(10);

            // suspend tasks for recording mode
            vTaskSuspend(xTaskSDcardHandle);

            // resets queue and discard remaining data to avoid sd card task to access it
            //xQueueReset(xQueueData);

            // configure PWM clock - low-power mode
            //ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_low));
            //ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
            
            ESP_LOGI(END_REC_TAG, "Recording session finished.");

            // i2s stop
            ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

            // close file in use
            close_file(&session_file);
            
            // dismout SD card and free SPI bus (in the given order) 
            deinitialize_sd_card(&card);
            deinitialize_spi_bus(&host);

            // clear flag informing that the recording is stopped
            xEventGroupClearBits(xEvents, BIT_(REC_STARTED));

            // resume task to wait for recording trigger
            vTaskResume(xTaskSTARThandle);

            ESP_LOGI(END_REC_TAG, "Returning to IDLE mode.");

            // locking task
            vTaskSuspend(xTaskENDhandle); // suspend actual task
        } else {vTaskDelay(1);}
    }
}

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
        

void vTaskMEMSmic(void * pvParameters)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_MIC_READING), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_MIC_READING))//if(xSemaphoreTimer!=NULL && xSemaphoreTakeFromISR(xSemaphoreTimer,&xHighPriorityTaskWoken)==pdTRUE)
        {
            TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // cancel writing protection
            TIMERG1.wdt_feed=1; // feeds wdt
            TIMERG1.wdt_wprotect=0; // restore writing protection
            ESP_LOGI(MEMS_MIC_TAG, "Hello MEMS mic!");
            i2s_read(I2S_PORT_NUM, (void*) inBuffer, I2S_DMA_BUFF_LEN_BYTES, &bytes_read, portMAX_DELAY); // read bytes from mic with i2s
            //for(int i=0; i<I2S_DMA_BUFF_LEN_BYTES; i++){inBuffer[i]= (char) i;}
            xQueueSend(xQueueData,&inBuffer,0);
        } else{vTaskDelay(1);}
    }
}

void vTaskSDcard(void * pvParameters)
{
    while(1)
    {
        while(xQueueData!=NULL && xQueueReceive(xQueueData, &outBuffer, 0)==pdTRUE) // wait for data to be read
        {
            ESP_LOGI(SD_CARD_TAG, "Hello SD card!");
            fwrite(outBuffer, I2S_DMA_BUFF_LEN_BYTES, 1, session_file); // write output buffer to sd card current file
        }
        vTaskDelay(1);
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

static bool IRAM_ATTR ISR_TIMER()
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphoreTimer,&xHighPriorityTaskWoken);
    return xHighPriorityTaskWoken == pdTRUE;
}

/*
 * Peripheral section
 * --------------------
 * Declaration of functions responsible to (de)initializa peripherals such as SPI bus or SD card host
 */

void initialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(INIT_SPI_TAG, "Initializing SPI bus!");

    sdmmc_host_t host_temp = SDSPI_HOST_DEFAULT();
    //host_temp.max_freq_khz = SDMMC_FREQ_PROBING;//100;
    //host_temp.command_timeout_ms = 100;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 8192,
    };

    esp_err_t ret = spi_bus_initialize(host_temp.slot, &bus_cfg, SPI_DMA_CHAN);
    *host = host_temp;

    if (ret != ESP_OK) {
        ESP_LOGE(INIT_SPI_TAG, "Failed to initialize SPI bus.");
        return;
    }
}

void deinitialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(DEINIT_SPI_TAG, "Deinitializing SPI bus!");
    if(xEventGroupGetBits(xEvents) & BIT_(SPI_BUS_FREE)) // spi bus is free
    {
        spi_bus_free((*host).slot); //deinitialize the bus after all devices are removed
        ESP_LOGI(DEINIT_SPI_TAG, "SPI bus freed.");
    } else {ESP_LOGI(DEINIT_SPI_TAG, "SPI not freed.");}
}

void initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card)
{
    ESP_LOGI(INIT_SD_TAG, "Initializing SD card!");

    // options for mounting the filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, // format if mount failed
        .max_files = 5, // max number of files
        .allocation_unit_size = 16 * 1024, // 
    };
    
    // options for initialize SD SPI device 
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT(); // init with default pin mapping
    slot_config.gpio_cs = PIN_NUM_CS; // map CS pin
    slot_config.host_id = (*host).slot; // associate SPI bus
    // slot_config.gpio_cd // if there is card detect (CD) signal
    // slot_config.gpio_wp // if there is write protect (WP) signal

    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &(*host), &slot_config, &mount_config, card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(INIT_SD_TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(INIT_SD_TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);
    xEventGroupClearBits(xEvents, BIT_(SPI_BUS_FREE)); // spi bus busy
}

void deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(DEINIT_SD_TAG, "Demounting SD card!");
    esp_vfs_fat_sdcard_unmount(mount_point, *card); // unmount partition and disable SDMMC or SPI peripheral
    xEventGroupSetBits(xEvents, BIT_(SPI_BUS_FREE)); // spi bus is free
    ESP_LOGI(DEINIT_SD_TAG, "Card unmounted.");
}


