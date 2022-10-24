#-------------------------------------------------
#
# Project created by QtCreator 2022-10-22T17:36:39
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = demo
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    tcp_server.cpp \
    card_data.cpp

HEADERS  += widget.h \
    tcp_server.h \
    card_data.h

FORMS    += widget.ui

RESOURCES += \
    res.qrc
