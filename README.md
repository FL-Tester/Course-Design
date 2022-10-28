# Course-Design
 课程设计

## 校园打卡系统
### 要求
自由组合，8-10人一组，每组推选组长1人；可以在下面的参考选题中自由选题或自拟题目。
基于校园卡的学生考勤系统
（1）任务设计制作
	基于校园卡的学生考勤系统。
（2）要求
	1. 以校园卡（RFID卡）作为身份识别卡，刷卡进行签到。
	2. 刷卡信息可以传输至WEB服务器（传输方式自拟，可以是有线的，也可以是无线的）进行验证；验证结果可以返回至客服端进行显示（验证成功或失败）。
	3. 编写B/S结构的应用程序，可以通过网页浏览器实时查看考勤信息。
	4. 电路制作时，必须注明本组成员学号与姓名。
### 选题修改
基于校园卡的学生考勤系统（不变）
1. 以校园卡（RFID卡）作为身份识别卡，刷卡进行签到。
2. 刷卡信息可以传输至私有服务器数据库和客户端（gec6818）。
	板子通过sql插入传到数据库或者tcp到qt小型数据库中
3. 在开发板屏幕或者上位机（可以是开发板qt程序中）中实时查看考勤信息。
	qt开发/纯c画图ui
4. 电路制作时，必须注明本组成员学号与姓名。

### 芯片选型 && 外设
#### 主控 
ESP32
gec6818开发板(Cortex-A8 S5PV210)
#### 外设
RC522(SPI) 
OLED(SSD1306)(I2C)

### 下位机实现
#### 开发环境
esp-idf + vscode
#### rcc522
参考：https://github.com/abobija/esp-idf-rc522
展示rc522事件处理部分代码
```c
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
```
主要利用FreeRTOS中的任务通知机制
当rc522中断触发 就会通知在conditionwait(FreeRTOS的通知)的两个任务

#### oled(0.96寸)
参考：https://github.com/bkgoodman/ESP32-IDF-OLED-SSD1306
不展示代码

#### tcp客户端建立
标准的tcp客户端建立 手写
```c
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
```
#### FreeRTOS使用
创建了三个任务
分别是初始化任务，tcp客户端任务，显示任务
```c
void app_main(void){
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreatePinnedToCore(init_task, "init_ask", 1024 * 2, (void *)0, 3, NULL, 0);  //init 通知push_mqtt_task和display_task
    xTaskCreatePinnedToCore(tcp_client_task, "tcp_client_task", 1024 * 5, (void *)0, 2, &xtask_tcp_client, 0);  //tcp客户端任务
    xTaskCreatePinnedToCore(display_task, "display_task", 1024 * 2, (void *)0, 2, &xtask_display, 1);  //lcd显示任务 与rc522中断关联
}
```
### 上位机实现
#### 开发环境
qt5.7 + vscode 
#### tcp服务端设置
甚至不用写标准的c流程 用到了qt内置的类 QTcpServer
connect()中用到了lambda 只能说lambda非常强大
```CPP
QTcpServer *tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::Any, TCP_SERVER_PORT)){
        qDebug() << "TCP Server start error";
    }else{
        qDebug() << "TCP Server start ok";
    }
    //消息处理
    connect(tcpServer, &QTcpServer::newConnection, [=](){
        QTcpSocket *tcpSocket = tcpServer->nextPendingConnection();
        connect(tcpSocket, &QTcpSocket::readyRead, [=](){
            QByteArray array = tcpSocket->readAll();
            textEdit->clear();//textEdit清空
            QString str_buf = QString(array);
            if( qstr_card_data_check(str_buf) ){
                qDebug() << "card_id check ok";
                sql_insert(str_buf);
                model->setQuery("select name, cardid, time, location from nima");//刷新数据
                view_SqLite->setModel(model);//显示数据
                textEdit->append(array);
            }else{
                qDebug() << "card_data_check error";
            }
            qDebug() << array;
        });
    });
```
#### sqlite数据库
连接的是默认的数据库 
```cpp
QSqlDatabase database;
QSqlDatabase::removeDatabase("qt_sql_default_connection");
if (QSqlDatabase::contains("qt_sql_default_connection")){ // 
    database = QSqlDatabase::database("qt_sql_default_connection");
}else{
    database = QSqlDatabase::addDatabase("QSQLITE"); 
    database.setDatabaseName("MyDataBase.db"); 
    database.setUserName("fengle");
    database.setPassword("123456");
}
```
操作的方法都是sql语句 不表
### pcb制作
暂时没有

### 结果展示
界面没有设计 qt能跑就行
![](/res/card_result.jpg)

<video id="video" controls="" preload="none">
    <source id="mp4" src="/res/card.mp4" type="video/mp4">
</video>






