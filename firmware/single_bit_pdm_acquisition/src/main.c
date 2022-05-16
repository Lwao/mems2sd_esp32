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
    // // wav file header initialization
    // memcpy(wav_header.riff, "RIFX", 4);
    // memcpy(wav_header.wave, "WAVE", 4);
    // memcpy(wav_header.fmt,  "fmt ", 4);
    // memcpy(wav_header.data, "data", 4);
  
    // wav_header.chunk_size = 16;                         // size of
    // wav_header.format_tag = 1;                          // PCM
    // wav_header.num_chans = 1;                           // mono
    // wav_header.srate = SAMPLE_RATE;                     // sample rate
    // wav_header.bytes_per_sec = SAMPLE_RATE*BIT_DEPTH/8; // byte rate 
    // wav_header.bytes_per_samp = BIT_DEPTH/8;            // 2-bytes
    // wav_header.bits_per_samp = BIT_DEPTH;               // 16-bits

    // // swap bytes to obey big-endian format 
    // swap_byte_order_long(&wav_header.chunk_size);
    // swap_byte_order_short(&wav_header.format_tag);
    // swap_byte_order_short(&wav_header.num_chans);
    // swap_byte_order_long(&wav_header.srate);
    // swap_byte_order_long(&wav_header.bytes_per_sec);
    // swap_byte_order_short(&wav_header.bytes_per_samp);
    // swap_byte_order_short(&wav_header.bits_per_samp);

    BaseType_t xReturnedTask[3];

    // configure gpio pins
    ESP_ERROR_CHECK(gpio_config(&in_conf1));                              // initialize input pin 1 configuration - on/off button
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));     // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_START_END, ISR_BTN, NULL));  // hook isr handler for specific gpio pin

    // configure i2s
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT_NUM, &i2s_config_i2s, 32, &xQueueData));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT_NUM, &i2s_pins_i2s));
    ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

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

            // initialize SPI bus and mount SD card
            while(initialize_spi_bus(&host)!=1){vTaskDelay(100);}
            while(initialize_sd_card(&host, &card)!=1){vTaskDelay(100);}
            vTaskDelay(100);
            
            // reset RTC 
            settimeofday(&date, NULL); // update time
            
            // open new file in append mode
            while(session_file==NULL) session_file = open_file(fname, "a");

            // write .wav file header to session file
            // fseek(session_file, 0L, SEEK_SET);                                      // seek back to beginning of file
            // fwrite(&wav_header, sizeof(struct wav_header_struct), 1, session_file); // write wav file header
			// fsync(fileno(session_file));                                            // secure data writing

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
    char text[128];
    while(1)
    {
        if(xSemaphoreBTN_OFF!=NULL && xSemaphoreTakeFromISR(xSemaphoreBTN_OFF,&xHighPriorityTaskWoken)==pdTRUE) // button was pressed to turn OFF recording
        {
            // suspend tasks for recording mode
            vTaskSuspend(xTaskRECHandle);

            // i2s stop
            ESP_ERROR_CHECK(i2s_stop(I2S_PORT_NUM));

            // wait to get mutex indicating total end of rec task
            while(xSemaphoreTake(xMutex,portMAX_DELAY)==pdFALSE);

            ESP_LOGI(END_REC_TAG, "Recording session finished.");

            // // finish .wav file format structure
            // fseek(session_file, 0L, SEEK_END);                               // seek to end of session file
            // wav_header.flength = ftell(session_file);                        // get file size
            // wav_header.dlength = wav_header.flength-44;                      // get data size (file size minus wav file header size)
            // swap_byte_order_long(&wav_header.flength);                       // swap byte order to invert endianness
            // swap_byte_order_long(&wav_header.dlength);                       // swap byte order to invert endianness
            // close_file(&session_file);                                       // close file that was open in append mode
            
            // while(session_file==NULL) session_file = open_file(fname, "r+"); // re-open file in read-write mode
            // fgets(text, 127, session_file);                                  // get initial sample text from file header

            // fseek(session_file, 4, SEEK_SET);                                // offset file pointer to end of "RIFX"            
            // fputc((wav_header.flength >> 24) & 255, session_file);           // distribute bytes o file length to specific position
            // fputc((wav_header.flength >> 16) & 255, session_file);           // distribute bytes o file length to specific position
            // fputc((wav_header.flength >> 8) & 255, session_file);            // distribute bytes o file length to specific position
            // fputc((wav_header.flength >> 0) & 255, session_file);            // distribute bytes o file length to specific position
            
            // fseek(session_file, 40, SEEK_SET);                               // offset file pointer to end of "data"            
            // fputc((wav_header.dlength >> 24) & 255, session_file);           // distribute bytes o data length to specific position
            // fputc((wav_header.dlength >> 16) & 255, session_file);           // distribute bytes o data length to specific position
            // fputc((wav_header.dlength >> 8) & 255, session_file);            // distribute bytes o data length to specific position
            // fputc((wav_header.dlength >> 0) & 255, session_file);            // distribute bytes o data length to specific position
            
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
        while(
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

/*
 * Peripheral section
 * --------------------
 * Declaration of functions responsible to (de)initializa peripherals such as SPI bus or SD card host
 */

int initialize_spi_bus(sdmmc_host_t* host)
{
    ESP_LOGI(INIT_SPI_TAG, "Initializing SPI bus!");

    sdmmc_host_t host_temp = SDSPI_HOST_DEFAULT();
    // host_temp.max_freq_khz = SDMMC_FREQ_PROBING;//100;
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
        return 0;
    }

    return 1; // initialization complete
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

int initialize_sd_card(sdmmc_host_t* host, sdmmc_card_t** card)
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
        return 0;
    }

    // card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);
    xEventGroupClearBits(xEvents, BIT_(SPI_BUS_FREE)); // spi bus busy

    return 1; // initialization complete
}

void deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(DEINIT_SD_TAG, "Demounting SD card!");
    esp_vfs_fat_sdcard_unmount(mount_point, *card); // unmount partition and disable SDMMC or SPI peripheral
    xEventGroupSetBits(xEvents, BIT_(SPI_BUS_FREE)); // spi bus is free
    ESP_LOGI(DEINIT_SD_TAG, "Card unmounted.");
}

void swap_byte_order_short(short* s)
{
    (*s) = ((*s) >> 8) |
           ((*s) << 8);
}

void swap_byte_order_long(long* l)
{
    (*l) = ((*l) >> 24) |
           (((*l)<<8) & 0x00FF0000) |
           (((*l)>>8) & 0x0000FF00) |
           ((*l) << 24);
}
