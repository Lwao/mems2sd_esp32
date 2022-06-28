/**
 * @file config_file.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date June 22 2022
 */

#include "config_file.h"

void init_config_file(config_file_t *configurations)
{
    configurations->record_file_name_sufix = 0;
	configurations->sampling_rate = 8000;
	configurations->record_session_duration = 10;
    // if(asprintf(&configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
    if(sprintf(configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
    ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing session file name.");
}

void parse_config_file(sdmmc_host_t* host, sdmmc_card_t** card, config_file_t *configurations)
{
    // initialize SPI bus and mount SD card
    while(initialize_spi_bus(host)!=1){vTaskDelay(100);}
    while(initialize_sd_card(host, card)!=1){vTaskDelay(100);}
    vTaskDelay(100);

    char line[256], *token_name;
	int token_value;
    
    init_config_file(configurations);

    FILE* config_file = open_file("config.txt", "r");
    
    if(config_file)
    {
        while (fgets(line, sizeof(line), config_file))
        {
            token_name = strtok(line, "=");
            token_value = atoi(strtok(NULL, ";"));
            
            if(strcmp(token_name, "record_file_name_sufix")==0) configurations->record_file_name_sufix = token_value;
            if(strcmp(token_name, "sampling_rate")==0) configurations->sampling_rate = token_value;
            if(strcmp(token_name, "record_session_duration")==0) configurations->record_session_duration = token_value;
        }
    }
    ESP_LOGI(PARSE_CONFIG_TAG, "Config file parsed.");

    close_file(&config_file);

    // char* session_fname;
	while(1)
	{
		// if(asprintf(&configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
        if(sprintf(configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
        ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing test session file name.");
        
		FILE *test_file;
    
    	if ((test_file = open_file(configurations->file_name, "r"))) 
    	{
        	ESP_LOGI(PARSE_CONFIG_TAG, "File exists: %s", configurations->file_name);
            close_file(&test_file);
        	configurations->record_file_name_sufix++;
    	}
    	else 
    	{
    		ESP_LOGI(PARSE_CONFIG_TAG, "File does not exist: %s", configurations->file_name);
    		break;
    	}   
	}
    // free(session_fname);

    ESP_LOGI(PARSE_CONFIG_TAG, "Record file name sufix: %d", configurations->record_file_name_sufix);
    ESP_LOGI(PARSE_CONFIG_TAG, "Sample rate: %dHz", configurations->sampling_rate);
    ESP_LOGI(PARSE_CONFIG_TAG, "Record session duration: %d s", configurations->record_session_duration);
    ESP_LOGI(PARSE_CONFIG_TAG, "Session file name: %s", configurations->file_name);

    // dismount SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(card);
    deinitialize_spi_bus(host);
    vTaskDelay(100);
}