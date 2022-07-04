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
    configurations->bit_depth = 16;
	configurations->record_session_duration = -1;
    configurations->interval_between_record_session = -1;
    configurations->recording_color = OFF_COLOR;
    configurations->file_name = (char*) malloc(256 * sizeof(char));
    // if(asprintf(&(*configurations).file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
    // ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing test session file name.");
    if(sprintf(configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
    ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing test session file name.");
}

void parse_config_file(sdmmc_host_t* host, sdmmc_card_t** card, config_file_t *configurations)
{
    ESP_LOGI(PARSE_CONFIG_TAG, "Parsing config file.");

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
            if(strcmp(token_name, "bit_depth")==0) configurations->bit_depth = token_value;
            if(strcmp(token_name, "record_session_duration")==0) configurations->record_session_duration = token_value;
            if(strcmp(token_name, "interval_between_record_session")==0) configurations->interval_between_record_session = token_value;
            if(strcmp(token_name, "recording_color")==0) configurations->recording_color = (colors_t)token_value;
        }
    }
    ESP_LOGI(PARSE_CONFIG_TAG, "Config file parsed.");

    close_file(&config_file);

    get_file_name(configurations);
    
    ESP_LOGI(PARSE_CONFIG_TAG, "Record file name sufix: %d", configurations->record_file_name_sufix);
    ESP_LOGI(PARSE_CONFIG_TAG, "Sample rate: %dHz", configurations->sampling_rate);
    ESP_LOGI(PARSE_CONFIG_TAG, "Bit depth: %d-bits", configurations->bit_depth);
    ESP_LOGI(PARSE_CONFIG_TAG, "Record session duration: %ds", configurations->record_session_duration);
    ESP_LOGI(PARSE_CONFIG_TAG, "Interval between record session: %ds", configurations->interval_between_record_session);
    ESP_LOGI(PARSE_CONFIG_TAG, "Recording color: %d", configurations->recording_color);

    // dismount SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(card);
    deinitialize_spi_bus(host);
    vTaskDelay(100);
}

void get_file_name(config_file_t *configurations)
{
    FILE *test_file;
    // free(configurations->file_name);
	while(1)
	{
        // if(sprintf(configurations->file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
        // ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing test session file name.");
        if(asprintf(&(*configurations).file_name, "rec%d.wav",  configurations->record_file_name_sufix)==-1)
        ESP_LOGE(PARSE_CONFIG_TAG, "Error initializing test session file name.");
    
    	if ((test_file = open_file(configurations->file_name, "r"))) 
    	{
        	ESP_LOGI(PARSE_CONFIG_TAG, "File exists: %s", configurations->file_name);
            close_file(&test_file);
            free(configurations->file_name);
        	configurations->record_file_name_sufix++;
    	}
    	else 
    	{
    		ESP_LOGI(PARSE_CONFIG_TAG, "File does not exist: %s", configurations->file_name);
    		break;
    	}   
        // free(configurations->file_name);
	}
    close_file(&test_file);

    ESP_LOGI(PARSE_CONFIG_TAG, "Engage session file name as: %s", configurations->file_name);
}