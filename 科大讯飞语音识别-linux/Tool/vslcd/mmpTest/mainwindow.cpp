#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <linux/input.h>
#include <QDebug>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    efile = ::open("/dev/ubuntu_event",O_RDWR);
    if(efile<0)
    {
        qDebug()<<"open event fail";
    }

    file.setFileName("/dev/ubuntu_lcd");
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug()<<"open lcd fail";
    }
    mmp = file.map(0,800*480*4);
    this->startTimer(100);
}

MainWindow::~MainWindow()
{
    delete ui;
    file.unmap(mmp);
    ::close(efile);
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    QImage image(mmp,800,480, QImage::Format_RGB32);
    ui->label->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    struct input_event data;
    data.type=3;
    data.code=0;
    data.value = e->pos().x();
    write(efile, &data, sizeof(data));
    usleep(10000);
    data.type=3;
    data.code=1;
    data.value = e->pos().y();
    write(efile, &data, sizeof(data));
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    struct input_event data;
    data.type=3;
    data.code=0;
    data.value = e->pos().x();
    write(efile, &data, sizeof(data));
    usleep(10000);
    data.type=3;
    data.code=1;
    data.value = e->pos().y();
    write(efile, &data, sizeof(data));
}
