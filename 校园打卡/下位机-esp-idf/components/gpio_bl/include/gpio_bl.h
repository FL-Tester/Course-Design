#pragma once 
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GREEN_LED        12
#define RED_LED          13
#define BEEP             14
#define RESET            0
#define SET              1


void card_success(); //card success
void card_fail(); //card fail
int gpio_bl_init(void); // init gpio

