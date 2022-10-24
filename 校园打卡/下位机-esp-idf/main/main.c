#include "main.h"

void app_main(void){
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(init_task, "init_ask", 1024 * 2, (void *)0, 3, NULL, 0);  //init 通知push_mqtt_task和display_task
    xTaskCreatePinnedToCore(tcp_client_task, "tcp_client_task", 1024 * 5, (void *)0, 2, &xtask_tcp_client, 0);  //tcp客户端任务
    xTaskCreatePinnedToCore(display_task, "display_task", 1024 * 2, (void *)0, 2, &xtask_display, 1);  //lcd显示任务 与rc522中断关联
}

static void init_task(void *arg){
    mutex = xSemaphoreCreateMutex();  //创建互斥锁mutex

    //初始化rc522
    rc522_create(&config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);

    //初始化oled
    // OLEDDisplay_t *oled = OLEDDisplay_init(0 ,0x78, 5,4);
    // OLEDDisplay_clear(oled);   //清屏
    // OLEDDisplay_print(oled, 0, 0, "Welcome to use");

    vTaskDelay(500 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);  //运行完毕删除任务 
}

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data){
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;
    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                printf("your card id is: %llu\n", tag->serial_number);
                card_id = tag->serial_number;
                xTaskNotifyFromISR(xtask_display, 0, eNoAction, NULL); //通知xtaskdisplay
                xTaskNotifyFromISR(xtask_tcp_client, 0, eNoAction, NULL);
            }
        break;
    }
}

static void display_task(void *arg){
	OLEDDisplay_t *oled = OLEDDisplay_init(0 ,0x78, 5,4);
    vTaskDelay(100 / portTICK_PERIOD_MS);
	while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   //阻塞自身

        OLEDDisplay_clear(oled);   //清屏
        sprintf(card_id_str, "%llu", card_id);
        OLEDDisplay_setTextAlignment(oled,TEXT_ALIGN_CENTER); //居中
        OLEDDisplay_drawString(oled,64, 00, "Card ID:");  //显示字符card id
        OLEDDisplay_drawString(oled,64, 20, card_id_str); 
        OLEDDisplay_drawRect(oled, 5, 45, 15, 15);
        OLEDDisplay_drawRect(oled, 67, 45, 15, 15);
        OLEDDisplay_setTextAlignment(oled,TEXT_ALIGN_LEFT);
        OLEDDisplay_drawString(oled, 22, 45, "Success!");  //显示字符card id
        OLEDDisplay_drawString(oled, 84, 45, "Fail!"); 
        OLEDDisplay_display(oled); 
          
        //上互斥锁mutex 
        if (xSemaphoreTake(mutex, 1000) == pdPASS){ 
            if( card_id_judge(card_id , card_correct_id)){
                OLEDDisplay_fillRect(oled, 5, 45, 15, 15);
                OLEDDisplay_display(oled);
            }else{
                OLEDDisplay_fillRect(oled, 67, 45, 15, 15);
                OLEDDisplay_display(oled);
            }
            //释放互斥锁mutex
            xSemaphoreGive(mutex);
        }
        vTaskDelay(250 / portTICK_PERIOD_MS); 
	}
    vTaskDelete(NULL);
}



bool card_id_judge(uint64_t card_id, uint64_t card_correct_id[]){
    for(int i = 0; i < 11; i++){
        if(card_id == card_correct_id[i]){
            return true;
        }
    }
    return false;
}

static int tcp_client_task(void *arg){
    //wifi初始化  配置esp-idf configuration 
    ESP_ERROR_CHECK(nvs_flash_init());  
    ESP_ERROR_CHECK(esp_netif_init());  
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect()); 

    //建立socket
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    int sock = socket(addr_family, SOCK_STREAM, ip_protocol); //tcp socket 建立
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        close(sock);
        return -1;
    }
    //sockaddr_in 地址端口等信息设置
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_ADRESS);
    dest_addr.sin_port = htons(TCP_PORT);
    //连接服务器
    int connect_err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (connect_err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        return -1;
    }
    ESP_LOGE(TAG, "connected to server!");

    //等待通知发送打卡信息
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   //阻塞自身
        //上互斥锁mutex
        if (xSemaphoreTake(mutex, 1000) == pdPASS){ 
            memset(card_id_str, 0, sizeof(card_id_str));
            sprintf(card_id_str, "%llu", card_id);
            //发送打卡信息
            int err = send(sock, card_id_str, strlen(card_id_str), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                close(sock);
                return -1;
            }
            xSemaphoreGive(mutex); //释放互斥锁mutex
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void push_mqtt_task(void *arg){
    //连接mqtt服务器
    esp_mqtt_client_handle_t client_task = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client_task);

    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //阻塞等待rc522_handler通知 后push
        if (xSemaphoreTake(mutex, 1000) == pdPASS){
            if (card_id_judge(card_id , card_correct_id)){
                char card_id_str[20];
                sprintf(card_id_str, "%llu", card_id);
                esp_mqtt_client_publish(client_task, "/card_id", card_id_str, 0, 0, 0);
            }
            xSemaphoreGive(mutex);//释放互斥锁mutex
        }
        //需要subscribe也可以写在这里
            
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}