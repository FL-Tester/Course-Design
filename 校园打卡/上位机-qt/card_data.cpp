#include "card_data.h"

int qstr_card_data_check(QString str){
    int i;
    for(i=0;i<CARD_DATA_MAX;i++){
        if( str.compare(qstr_card_id[i]) == 0 ){
            return 1;
        }
    }
    return 0;
}

void sql_insert(QString str){
    QSqlQuery query;
    //str 找到在qstr_card_id的index
    int i = 0;
    for (i=0;i<CARD_DATA_MAX;i++){
        if( str.compare(qstr_card_id[i]) == 0 ){
            break;
        }
    }
    //插入qstr_card_id[i] qstr_card_name[i] 系统时间到数据库
    QString sql = QString("insert into nima (name, cardid, time, location) values ('%1', '%2', '%3', '%4')").arg(qstr_card_name[i]).arg(qstr_card_id[i]).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg("北门");
    query.exec(sql);
}

