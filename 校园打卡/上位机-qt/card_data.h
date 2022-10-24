#ifndef CARD_DATA_H
#define CARD_DATA_H

#include <QList>
#include <QString>
#include <iostream>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDateTime>
using namespace std;
#define CARD_DATA_MAX 10
const QString qstr_card_id[CARD_DATA_MAX] = {
    "580826361761", 
    "406629893939", 
    "406629893941",
    "406629893942", 
    "406629893943", 
    "406629893944", 
    "406629893945", 
    "406629893946", 
    "406629893947", 
    "406629893948" 
};

const QString qstr_card_name[CARD_DATA_MAX] = {
    "张小三", 
    "李大四", 
    "王大五",
    "赵老六", 
    "孙可七", 
    "周里八", 
    "吴十九", 
    "郑十三", 
    "钱十一", 
    "孙十二" 
};

int qstr_card_data_check(QString str);
void sql_insert(QString str);

#endif

