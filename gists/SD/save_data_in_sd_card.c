void app_main(void)
{
	// initialize SPI bus and mout SD card
    initialize_spi_bus(&host);
    initialize_sd_card(&host, &card);
    
    // open new file in append mode
    while(session_file==NULL){session_file = open_file(fname, "a");}
    for(int i = 0; i<100; i++)
    {
        data++;
        fprintf(session_file, "%d!\n", data);
    }
    

    // close file in use
    close_file(&session_file);
    ESP_LOGI("tag", "File written.\n");

    // dismout SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(&card);
    deinitialize_spi_bus(&host);
}

/*
--- Available filters and text transformations: colorize, debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at http://bit.ly/pio-monitor-filters
--- Miniterm on COM3  115200,8,N,1 ---
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
J␂␒��ѹesp32: SPI Speed      : 40MHz␛[0m
␛[0;32mI (43) boot.esp32: SPI Mode       : DIO␛[0m
␛[0;32mI (47) boot.esp32: SPI Flash Size : 4MB␛[0m
␛[0;32mI (52) boot: Enabling RNG early entropy source...␛[0m
␛[0;32mI (57) boot: Partition Table:␛[0m
␛[0;32mI (61) boot: ## Label            Usage          Type ST Offset   Length␛[0m
␛[0;32mI (68) boot:  0 nvs              WiFi data        01 02 00009000 00006000␛[0m
␛[0;32mI (76) boot:  1 phy_init         RF data          01 01 0000f000 00001000␛[0m
␛[0;32mI (83) boot:  2 factory          factory app      00 00 00010000 00100000␛[0m
␛[0;32mI (91) boot: End of partition table␛[0m
␛[0;32mI (95) boot_comm: chip revision: 1, min. application chip revision: 0␛[0m    
␛[0;32mI (102) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=0b408h ( 46088) map␛[0m
␛[0;32mI (128) esp_image: segment 1: paddr=0001b430 vaddr=3ffb0000 size=02a58h ( 10840) load␛[0m
␛[0;32mI (133) esp_image: segment 2: paddr=0001de90 vaddr=40080000 size=02188h (  8584) load␛[0m
␛[0;32mI (138) esp_image: segment 3: paddr=00020020 vaddr=400d0020 size=1e024h (122916) map␛[0m
␛[0;32mI (189) esp_image: segment 4: paddr=0003e04c vaddr=40082188 size=0a3ach ( 41900) load␛[0m
␛[0;32mI (207) esp_image: segment 5: paddr=00048400 vaddr=50000000 size=00010h (    16) load␛[0m
␛[0;32mI (214) boot: Loaded app from partition at offset 0x10000␛[0m
␛[0;32mI (214) boot: Disabling RNG early entropy source...␛[0m      
␛[0;32mI (227) cpu_start: Pro cpu up.␛[0m
␛[0;32mI (227) cpu_start: Starting app cpu, entry point is 0x4008226c␛[0m
␛[0;32mI (0) cpu_start: App cpu up.␛[0m
␛[0;32mI (241) cpu_start: Pro cpu start user code␛[0m
␛[0;32mI (241) cpu_start: cpu freq: 160000000␛[0m
␛[0;32mI (241) cpu_start: Application information:␛[0m       
␛[0;32mI (246) cpu_start: Project name:     mems2sd_esp32␛[0m
␛[0;32mI (251) cpu_start: App version:      c3f1665-dirty␛[0m
␛[0;32mI (257) cpu_start: Compile time:     Aug 13 2021 00:36:47␛[0m
␛[0;32mI (263) cpu_start: ELF file SHA256:  64176f3c12f706c1...␛[0m 
␛[0;32mI (269) cpu_start: ESP-IDF:          4.3.0␛[0m
␛[0;32mI (274) heap_init: Initializing. RAM available for dynamic allocation:␛[0m
␛[0;32mI (281) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM␛[0m
␛[0;32mI (287) heap_init: At 3FFB3348 len 0002CCB8 (179 KiB): DRAM␛[0m
␛[0;32mI (293) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM␛[0m
␛[0;32mI (299) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM␛[0m
␛[0;32mI (306) heap_init: At 4008C534 len 00013ACC (78 KiB): IRAM␛[0m
␛[0;32mI (313) spi_flash: detected chip: generic␛[0m
␛[0;32mI (317) spi_flash: flash io: dio␛[0m
␛[0;32mI (322) cpu_start: Starting scheduler on PRO CPU.␛[0m
␛[0;32mI (0) cpu_start: Starting scheduler on APP CPU.␛[0m
␛[0;32mI (331) init_spi: Initializing SPI bus!␛[0m
␛[0;32mI (341) init_sd: Initializing SD card!␛[0m
␛[0;32mI (341) gpio: GPIO[5]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 ␛[0m
␛[0;32mI (351) sdspi_transaction: cmd=52, R1 response: command not supported␛[0m
␛[0;32mI (401) sdspi_transaction: cmd=5, R1 response: command not supported␛[0m
Name: SD16G
Type: SDHC/SDXC
Speed: 20 MHz
Size: 14772MB
␛[0;32mI (411) sd_driver: Opening file.␛[0m
␛[0;32mI (411) sd_driver: File opened.␛[0m
␛[0;32mI (421) sd_driver: File closed.␛[0m
␛[0;32mI (421) tag: File written.
␛[0m
␛[0;32mI (421) deinit_sd: Demounting SD card!␛[0m
␛[0;32mI (431) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 ␛[0m
␛[0;32mI (431) deinit_sd: Card unmounted.␛[0m
␛[0;32mI (441) deinit_spi: Deinitializing SPI bus!␛[0m
␛[0;32mI (441) gpio: GPIO[23]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 ␛[0m
␛[0;32mI (451) gpio: GPIO[19]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 ␛[0m
␛[0;32mI (461) gpio: GPIO[18]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 ␛[0m
␛[0;32mI (471) deinit_spi: SPI bus freed.␛[0m
*/