#include "gpio_bl.h"

gpio_config_t beepled_io_conf = {
    .pin_bit_mask = (
        1ULL << GREEN_LED  | 
        1ULL << RED_LED    | 
        1ULL << BEEP 
    ),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = 0,
    .pull_down_en = 0,
    .intr_type = GPIO_INTR_DISABLE, //
};

//gpio init
int gpio_bl_init(void)
{
    if (gpio_config(&beepled_io_conf) == ESP_OK) {
        printf("BL GPIO init failed ");
        return -1;
    }
    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(RED_LED, 0);
    gpio_set_level(BEEP, 1);   //低电平有效蜂鸣器
    return 0;
}

//card success
void card_success()
{
    gpio_set_level(GREEN_LED, 1);
    gpio_set_level(BEEP, RESET);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(BEEP, SET);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(BEEP, RESET);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(BEEP, SET);
    gpio_set_level(GREEN_LED, 0);
}

//card fail
void card_fail(){
    gpio_set_level(RED_LED, 1);
    gpio_set_level(BEEP, RESET);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(BEEP, SET);
    gpio_set_level(RED_LED, 0);
}