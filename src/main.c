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

int cntr = 0;
void app_main(void)
{  
    BaseType_t xReturnedTask[4];

    // configure gpio pins
    ESP_ERROR_CHECK(gpio_config(&out_conf));                             // initialize output pin configuration
    ESP_ERROR_CHECK(gpio_config(&in_conf1));                              // initialize input pin 1 configuration
    ESP_ERROR_CHECK(gpio_config(&in_conf2));                              // initialize input pin 2 configuration
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));    // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_INPUT_IO1, ISR_BTN, NULL)); // hook isr handler for specific gpio pin
    //ESP_ERROR_CHECK(gpio_isr_handler_remove(GPIO_INPUT_IO));           // remove isr handler for gpio number

    // configure timer
    ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &timer_conf));                      // initialize timer
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));                   // start value for counting
    ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_COUNT));           // interrupt value to fire isr
    ESP_ERROR_CHECK(timer_enable_intr(TIMER_GROUP_0, TIMER_0));                            // enable timer interruption
    ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_TIMER, NULL, ESP_INTR_FLAG_IRAM)); // associate ISR callback function to timer
    //ESP_ERROR_CHECK(timer_isr_register(TIMER_GROUP_0, TIMER_0, ISR_TIMER, NULL, ESP_INTR_FLAG_IRAM, NULL)); //
    
    // configure PWM clock - low-power mode
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_low));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // set flag informing that the recording is stopped
    flags[REC_STARTED] = 0;

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART,   "tSTART", 8192, NULL, configMAX_PRIORITIES-2, &xTaskSTARThandle,   APP_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskMEMSmic, "tMIC",   8192, NULL, configMAX_PRIORITIES-3, &xTaskMEMSmicHandle, PRO_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskSDcard,  "tSD",    8192, NULL, configMAX_PRIORITIES-4, &xTaskSDcardHandle,  APP_CPU_NUM);
    xReturnedTask[3] = xTaskCreatePinnedToCore(vTaskEND,     "tEND",   8192, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,     APP_CPU_NUM);

    for(int itr=0; itr<4; itr++) // iteeate over tasks 
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

    // create queue
    xQueueData = xQueueCreate(1024,sizeof(long)); // 32 bits = 4 bytes
    if(xQueueData == NULL){ // tests if queue creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create data queue.\n");
        while(1);
    }

    // create binary semaphores
    xSemaphoreBTN_ON = xSemaphoreCreateBinary();
    xSemaphoreBTN_OFF = xSemaphoreCreateBinary();
    xSemaphoreTimer = xSemaphoreCreateBinary();
    
    if(xSemaphoreBTN_ON == NULL){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create binary semaphore for ON button.\n");
        while(1);
    }
    if(xSemaphoreBTN_OFF == NULL){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create binary semaphore for OFF button.\n");
        while(1);
    }
    if(xSemaphoreTimer == NULL){ // tests if semaphore creation fails
        ESP_LOGE(SETUP_APP_TAG, "Failed to create binary semaphore for timer.\n");
        while(1);
    }
    
    ESP_LOGI(SETUP_APP_TAG, "Successful BOOT!");
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
            fprintf(session_file, "Tired, aren't ya?\n");

            // configure PWM clock - ultrasonic mode
            ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_ultra));
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

            // set flag informing that the recording already started
            flags[REC_STARTED] = 1;

            ESP_LOGI(START_REC_TAG, "Recording session started.");

            // resume tasks for recording mode
            vTaskResume(xTaskMEMSmicHandle);
            vTaskResume(xTaskSDcardHandle);
            vTaskResume(xTaskENDhandle);

            // start timer
            ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));

            // locking task
            //xSemaphoreGive(xSemaphoreBTN_ON); // return semaphore
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
            // resets queue and discard remaining data to avoid sd card task to access it
            xQueueReset(xQueueData);

            // pause timer
            ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));

            // suspend tasks for recording mode
            vTaskSuspend(xTaskMEMSmicHandle);
            vTaskSuspend(xTaskSDcardHandle);

            // configure PWM clock - low-power mode
            ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_low));
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
            
            ESP_LOGI(END_REC_TAG, "Recording session finished.");

            // close file in use
            close_file(&session_file);
            
            // dismout SD card and free SPI bus (in the given order) 
            deinitialize_sd_card(&card);
            deinitialize_spi_bus(&host);

            // set flag informing that the recording is stopped
            flags[REC_STARTED] = 0;

            // resume task to wait for recording trigger
            vTaskResume(xTaskSTARThandle);

            ESP_LOGI(END_REC_TAG, "Returning to IDLE mode.");

            // locking task
            //xSemaphoreGive(xSemaphoreBTN_OFF); // return semaphore
            vTaskSuspend(xTaskENDhandle); // suspend actual task
        } else {vTaskDelay(1);}
    }
}

void vTaskMEMSmic(void * pvParameters)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    while(1)
    {
        if(xSemaphoreTimer!=NULL && xSemaphoreTakeFromISR(xSemaphoreTimer,&xHighPriorityTaskWoken)==pdTRUE) // timer reached counting
        {
            ESP_LOGI(MEMS_MIC_TAG, "Hello MEMS mic!\n");
            cntr++;
            xQueueSend(xQueueData,&cntr,0);
            //xSemaphoreGive(xSemaphoreTimer);
        } else {vTaskDelay(1);}
    }
    /*
    unsigned char aux=0;
    unsigned char sending = 42;

    for(int itr=0; itr<8; itr++)
    {
        gpio_set_level(GPIO_OUTPUT_IO, sending & (0x80>>itr)); // x80 = 1000 0000
        vTaskDelay(pdMS_TO_TICKS(1000));
        aux = (aux<<1) | gpio_get_level(GPIO_INPUT_IO); // append 1-bit
        printf("still running: %d\n", aux);
    }
    printf("end running: %d\n", aux); // queue this value
    */
}

void vTaskSDcard(void * pvParameters)
{
    int data;
    while(1)
    {
        if(xQueueData!=NULL && xQueueReceive(xQueueData, (void *) &data, 0)==pdTRUE)
        {
            ESP_LOGI(SD_CARD_TAG, "Hello SD card!");
            fprintf(session_file, "%d\n", data);
            ESP_LOGI(SD_CARD_TAG, "Line written!");
        } else {vTaskDelay(1);}
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
    if(flags[REC_STARTED]){xSemaphoreGiveFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken);} // recording started, so stop recording
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
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
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
    if(flags[SPI_BUS_FREE])
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
        .allocation_unit_size = 16 * 1024 // 
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
    flags[SPI_BUS_FREE] = 0; // spi bus busy
}

void deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(DEINIT_SD_TAG, "Demounting SD card!");
    esp_vfs_fat_sdcard_unmount(mount_point, *card); // unmount partition and disable SDMMC or SPI peripheral
    flags[SPI_BUS_FREE] = 1; // spi bus is free
    ESP_LOGI(DEINIT_SD_TAG, "Card unmounted.");
}


