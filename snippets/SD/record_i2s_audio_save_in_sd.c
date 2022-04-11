// i2s configuration
i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM, // Master, RX, PDM
    .sample_rate = 44100, // 44.1kHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16bit
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // mono audio configuration
    .communication_format = I2S_COMM_FORMAT_I2S_MSB, //pcm data format
    .dma_buf_count = 4,                              // number of buffers, 128 max.
    .dma_buf_len = 1024,                             // size of each buffer
    .use_apll = 0,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 // Interrupt level 1
};

i2s_pin_config_t i2s_pins = {
    .ws_io_num = MIC_CLOCK_PIN,
    .data_in_num = MIC_DATA_PIN,
};

char *i2s_buffer;
int i2s_read_len = 100*1024;
size_t bytes_read;

void app_main(void)
{  
    i2s_buffer = (char *) calloc(i2s_read_len, sizeof(char));
   
    xEvents = xEventGroupCreate();
    
    // initialize SPI bus and mout SD card
    initialize_spi_bus(&host);
    initialize_sd_card(&host, &card);

    //vTaskDelay(pdMS_TO_TICKS(1000));

    i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT_NUM, &i2s_pins);

    // open new file in append mode
    while(session_file==NULL){session_file = open_file(fname, "a");}
    //fprintf(session_file, "data,time\n");
    for(int i=0; i<10; i++)
    {
        i2s_read(I2S_PORT_NUM, (void *) i2s_buffer, i2s_read_len, &bytes_read, portMAX_DELAY);
        fwrite(i2s_buffer, i2s_read_len, 1, session_file);
        ESP_LOGI("tag", "Buffer %d.\n", i);
    }
    // close file in use
    close_file(&session_file);
    ESP_LOGI("tag", "File written.\n");

    // dismout SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(&card);
    deinitialize_spi_bus(&host);
    i2s_driver_uninstall(I2S_PORT_NUM);

    vTaskDelete(NULL);
}