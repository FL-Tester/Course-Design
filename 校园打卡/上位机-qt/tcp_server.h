#ifndef TCP_SERVER_H
#define TCP_SERVER_H

//qt socket
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>


#define TCP_SERVER_PORT 65001
int tcp_server_start(void);


#endif // TCP_SERVER_H

