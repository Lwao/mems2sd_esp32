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
    configurations->record_file_name_sufix = 1;
	configurations->sampling_rate = 8000;
	configurations->record_session_duration = 10;
}

void parse_config_file(sdmmc_host_t* host, sdmmc_card_t** card, config_file_t *configurations)
{
    // initialize SPI bus and mount SD card
    while(initialize_spi_bus(&(*host))!=1){vTaskDelay(100);}
    while(initialize_sd_card(&(*host), &(*card))!=1){vTaskDelay(100);}
    vTaskDelay(100);

    char line[256], *token_name;
	int token_value;
    
    init_config_file(&(*configurations));

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

    close_file(&config_file);

    char* session_fname;
	while(1)
	{
		asprintf(&session_fname, "REC%d.WAV",  configurations->record_file_name_sufix);
		FILE *test_file;
    
    	if ((test_file = open_file(session_fname, "r"))) 
    	{
        	close_file(&test_file);
            ESP_LOGI(PARSE_CONFIG_TAG, "File exists: %s\n", session_fname);
        	//printf("file exists: %s\n", session_fname);
        	configurations->record_file_name_sufix++;
    	}
    	else 
    	{
    		ESP_LOGI(PARSE_CONFIG_TAG, "File does not exist: %s\n", session_fname);
            // printf("File does not exist: %s\n", session_fname);
    		break;
    	}
	}
	free(session_fname);

    printf("%d\n%d\n%d\n", configurations->record_file_name_sufix, configurations->sampling_rate, configurations->record_session_duration);

    // dismount SD card and free SPI bus (in the given order) 
    deinitialize_sd_card(&(*card));
    deinitialize_spi_bus(&(*host));
}