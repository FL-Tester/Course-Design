#pragma once

#include <stdint.h> 
#include <stddef.h> 
#include <string.h> 
#include "esp_wifi.h" 
#include "esp_system.h" 
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h" 
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "addr_from_stdin.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "OLEDDisplay.h"
#include "rc522.h"

#define TCP_SERVER_ADRESS    "192.168.0.107"    // TCP服务器地址
#define TCP_PORT             65001              // TCP服务器端口号
#define mqtt_server          "mqtt://182.61.55.22"
static char *tcp_send_data = "hello world!";   // TCP发送数据

static const char *TAG = "KaiLi-CD"; // for DEBUG
static void display_task(void *arg);  //lcd显示任务
static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data);// rc522事件处理  
static void init_task(void *arg); //初始化任务
static void push_mqtt_task(void *arg); //push mqtt
bool card_id_judge(uint64_t card_id, uint64_t card_correct_id[11]);//判断card_id 与 card_correct_id是否相等
static int tcp_client_task(void *arg); //tcp客户端任务

static rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 22,
};

esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://182.61.55.22",
        //.event_handle = mqtt_event_handler,
        .username = "admin",
        .password = "password",
        .port = 1883,
};

static rc522_handle_t scanner;
SemaphoreHandle_t mutex = NULL;
static TaskHandle_t xtask_display = NULL,  //任务句柄(freertos通知)
                    xtask_pushmqtt = NULL,
                    xtask_tcp_client = NULL;
char card_id_str[20];
uint64_t card_id = 0;
uint64_t card_correct_id[11] = {
    580826361761,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
