#include "driver/gpio.h"

#define GPIO_OUTPUT_IO       GPIO_NUM_18 
#define GPIO_INPUT_IO        GPIO_NUM_4 
#define GPIO_OUTPUT_PIN_SEL (1ULL<<GPIO_OUTPUT_IO)
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO)

// config output pin
gpio_config_t out_conf = {
    .intr_type = GPIO_INTR_DISABLE, // disable interrupt
    .mode = GPIO_MODE_OUTPUT, // set as input mode
    .pin_bit_mask = GPIO_OUTPUT_PIN_SEL, // bit mask of pins to set (GPIO18)
    .pull_down_en = 0, // disable pull-down mode
    .pull_up_en = 0, // disable pull-up mode
};


// config input pin
gpio_config_t in_conf = {
    .intr_type = GPIO_INTR_DISABLE, // disable interrupt
    .mode = GPIO_MODE_INPUT, // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL, // bit mask of pins to set (GPIO18)
    .pull_down_en = 1, // enable pull-down mode
    .pull_up_en = 0, // disable pull-up mode
};



void app_main(void)
{
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
}