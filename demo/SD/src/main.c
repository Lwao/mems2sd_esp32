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
    // initialize SPI bus and mout SD card
    while(initialize_spi_bus(&host)!=1){vTaskDelay(100);}
    while(initialize_sd_card(&host, &card)!=1){vTaskDelay(100);}

    vTaskDelay(1000);
    
    // open new file in append mode
    int data=0;
    session_file = open_file(fname, "a");
    for(int i = 0; i<100; i++)
    {
        data++;
        fprintf(session_file, "%d!\n", data);
        // fwrite(data, 4, 1, session_file);
        // fsync(fileno(session_file));
    }
    

    // close file in use
    close_file(&session_file);
    ESP_LOGI(SETUP_APP_TAG, "File written.\n");

    // dismout SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(&card);
    deinitialize_spi_bus(&host);
}

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
    spi_bus_free((*host).slot); //deinitialize the bus after all devices are removed
    ESP_LOGI(DEINIT_SPI_TAG, "SPI bus freed.");
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

    return 1; // initialization complete
}

void deinitialize_sd_card(sdmmc_card_t** card)
{
    ESP_LOGI(DEINIT_SD_TAG, "Demounting SD card!");
    esp_vfs_fat_sdcard_unmount(mount_point, *card); // unmount partition and disable SDMMC or SPI peripheral
    ESP_LOGI(DEINIT_SD_TAG, "Card unmounted.");
}


