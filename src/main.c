/**
 * @file main.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 24 2021
 */

#include "main.h"

void app_main(void)
{  
    // initialize SPI bus and mout SD card
    initialize_spi_bus(&host);
    initialize_sd_card(&host, &card);
    
    // open new file in append mode
    session_file = open_file(fname, "a");
    fprintf(session_file, "hello world!\n");

    // close file in use
    close_file(session_file);
    ESP_LOGI(TAG, "File written.\n");

    // dismout SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(&card);
    deinitialize_spi_bus(&host);
    /*
    BaseType_t xReturnedTask[4];

    // configure gpio pins
    gpio_config(&out_conf);
    gpio_config(&in_conf);
    
    // set flag informing that the recording is stopped
    flag_rec_started = 0;

    // create tasks
    xReturnedTask[0] = xTaskCreate(vTaskSTART, "taskSTART", configMINIMAL_STACK_SIZE, NULL, 0, &xTaskSTARThandle);
    xReturnedTask[1] = xTaskCreate(vTaskMEMSmic, "taskMEMSmic", configMINIMAL_STACK_SIZE, NULL, 0, &xTaskMEMSmicHandle);
    xReturnedTask[2] = xTaskCreate(vTaskSDcard, "taskSDcard", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskSDcardHandle);
    xReturnedTask[2] = xTaskCreate(vTaskEND, "taskEND", configMINIMAL_STACK_SIZE, NULL, 0, &xTaskENDhandle);

    for(int itr=0; itr<4; itr++) // tests if task creation fails
    {
        if(xReturnedTask[itr] == pdFAIL){
            ESP_LOGE(TAG, "Failed to create task %d.\n", itr);
            while(1);
        }
    }

    // create queue
    xQueueData = xQueueCreate(1,sizeof(long)); // 32 bits = 4 bytes
    if(xQueueData == NULL){ // tests if queue creation fails
        ESP_LOGE(TAG, "Failed to create data queue.\n");
        while(1);
    }

    // create binary semaphores
    xSemaphoreBTN_ON = xSemaphoreCreateBinary();
    xSemaphoreBTN_OFF = xSemaphoreCreateBinary();
    
    if(xSemaphoreBTN_ON == NULL){ // tests if semaphore creation fails
        ESP_LOGE(TAG, "Failed to create binary semaphore for ON button.\n");
        while(1);
    }
    if(xSemaphoreBTN_OFF == NULL){ // tests if semaphore creation fails
        ESP_LOGE(TAG, "Failed to create binary semaphore for OFF button.\n");
        while(1);
    }
    */
    /*
    unsigned char aux=0;
    unsigned char sending = 42;
    gpio_config(&out_conf);
    gpio_config(&in_conf);


    for(int itr=0; itr<8; itr++)
    {
        gpio_set_level(GPIO_OUTPUT_IO, sending & (0x80>>itr)); // x80 = 1000 0000
        vTaskDelay(pdMS_TO_TICKS(1000));
        aux = (aux<<1) | gpio_get_level(GPIO_INPUT_IO); // append 1-bit
        printf("still running: %d\n", aux);
    }
    printf("end running: %d\n", aux);
    */
}

void vTaskSTART(void * pvParameters)
{
    while(1)
    {
        // button was pressed to turn ON recording
        xSemaphoreTake(xSemaphoreBTN_ON,portMAX_DELAY); 

        // initialize SPI bus and mout SD card
        initialize_spi_bus(&host);
        initialize_sd_card(&host, &card);

        // resume tasks for recording mode
        vTaskResume(xTaskMEMSmicHandle);
        vTaskResume(xTaskSDcardHandle);
        
        // reset RTC 
        struct tm date = {// call struct with date data
            .tm_sec = 0, // current date (to be fecthed from NTP)
        };
        settimeofday(&date, NULL); // update time

        // open new file in append mode
        session_file = open_file(fname, "a");
        ESP_LOGI(TAG, "File open.\n");

        // set flag informing that the recording already started
        flag_rec_started = 1;
    }
}

void vTaskEND(void * pvParameters)
{
    while(1)
    {
        // button was pressed to turn OFF recording
        xSemaphoreTake(xSemaphoreBTN_OFF,portMAX_DELAY);

        // close file in use
        close_file(session_file);
        ESP_LOGI(TAG, "File written.\n");
        
        // dismout SD card and free SPI bus (in the given order) 
        deinitialize_sd_card(&card);
        deinitialize_spi_bus(&host);

        // suspend tasks for recording mode
        vTaskSuspend(xTaskMEMSmicHandle);
        vTaskSuspend(xTaskSDcardHandle);
        
        // set flag informing that the recording is stopped
        flag_rec_started = 0;
    }
}

void initialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(TAG, "Initializing SPI bus!");

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
        ESP_LOGE(TAG, "Failed to initialize SPI bus.");
        return;
    }
}

void deinitialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(TAG, "Deinitializing SPI bus!");
    if(flag_spi_bus_free)
    {
        spi_bus_free((*host).slot); //deinitialize the bus after all devices are removed
        ESP_LOGI(TAG, "SPI bus freed.");
    } else {ESP_LOGI(TAG, "SPI not freed.");}
}

void initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card)
{
    ESP_LOGI(TAG, "Initializing SD card!");

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
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);
    flag_spi_bus_free = 0; // spi bus busy
}

void deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(TAG, "Deinitializing SPI bus!");
    esp_vfs_fat_sdcard_unmount(mount_point, *card); // unmount partition and disable SDMMC or SPI peripheral
    flag_spi_bus_free = 1; // spi bus free
    ESP_LOGI(TAG, "Card unmounted.");
}

void IRAM_ATTR ISR_BTN()
{
  BaseType_t xHighPriorityTaskWoken = pdTRUE;
  if(flag_rec_started){xSemaphoreGiveFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken);}
  else{xSemaphoreGiveFromISR(xSemaphoreBTN_ON,&xHighPriorityTaskWoken);}
  if(xHighPriorityTaskWoken == pdTRUE){portYIELD_FROM_ISR();}
}

