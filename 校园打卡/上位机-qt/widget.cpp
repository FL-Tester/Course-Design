#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Widget){
    ui->setupUi(this);
    setWindowTitle("主窗口"); //设置窗口标题
    resize(800,480); //设置窗口大小
    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap(":/new/prefix1/res/5.jpg"))); //设置背景
    this->setPalette(palette);
    //创建QTableView控件 大小600x300 位置(100,100)
    QTableView *view_SqLite = new QTableView(this);
    view_SqLite->setGeometry(200,0,600,480);
    //create database for sqlite 
    QSqlDatabase database;
    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    if (QSqlDatabase::contains("qt_sql_default_connection")){ // 
        database = QSqlDatabase::database("qt_sql_default_connection");
    }
    else{
        database = QSqlDatabase::addDatabase("QSQLITE"); 
        database.setDatabaseName("MyDataBase.db"); 
        database.setUserName("fengle");
        database.setPassword("123456");
    }
    //open test
    if (!database.open()){
        qDebug() << "Error: Failed to connect database." << database.lastError();
    }
    else{
        qDebug() << "Database open successfully";
    }
    //数据表创建
    QSqlQuery query;
    //创建cardrecord表 字段 keyid name cardid time location
    query.exec("create table nima(keyid int primary key, name varchar(20), cardid varchar(20), time datetime, location varchar(20))");
    //清除 nima表中的数据
    query.exec("delete from nima");
    //插入数据 
    QSqlQueryModel *model = new QSqlQueryModel(view_SqLite);
    //查询数据显示居中
    model->setQuery("select name, cardid, time, location from nima");
    model->setHeaderData(0, Qt::Horizontal, "名字");
    model->setHeaderData(1, Qt::Horizontal, "卡号");
    model->setHeaderData(2, Qt::Horizontal, "时间");  
    model->setHeaderData(3, Qt::Horizontal, "地点");
    view_SqLite->setModel(model);
    view_SqLite->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    view_SqLite->setColumnWidth(0, 80);
    view_SqLite->setColumnWidth(1, 170);
    view_SqLite->setColumnWidth(2, 220);
    view_SqLite->setColumnWidth(3, 70);
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setGeometry(0,240,200,240);
    //创建tcp server
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
            }
            else
            {
                qDebug() << "card_data_check error";
            }
            qDebug() << array;
        });
    });
}


Widget::~Widget(){
    delete ui;
}
