#include "tcp_server.h"


int tcp_server_start(void){
    QTcpServer *tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::Any, TCP_SERVER_PORT)){
        qDebug() << "tcp server start error";
        return -1;
    }
    qDebug() << "tcp server start ok";
    //

    return 0;
}
void tcp_server_close(void);