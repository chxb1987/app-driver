/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       松下伺服驱动调试助手
*******************************************************************************/
#ifndef APPDRIVER_H
#define APPDRIVER_H

#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QSettings>
#include <QDateTime>
#include <QListView>
#include <QLineEdit>
#include <QComboBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QTextBrowser>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#ifndef __linux__
#include <qt_windows.h>
#endif

class AppDriver : public QMainWindow
{
    Q_OBJECT

public:
    explicit AppDriver(QWidget *parent = 0);
    ~AppDriver();
private slots:
    void initUI();
    void initSkin();
    void initTitle();
    void initParam();
    void initLayout();
    void initDevPort();
    void initDevDisplay();
    int getRead();
    int getHand();
    int putHand();
    int getConf();
    int putConf();
    int getData();
    int putData();
    int writeMsg(QByteArray msg);
    int initTask();
    int taskThread();
private:
    QTextBrowser *text;
    QHBoxLayout *btnLayout;
    QComboBox *boxDevPort;
    QLineEdit *torque;
    QLineEdit *speed;
    QSerialPort *com;
    QTimer *timer;
    QByteArray tmpByte;
    int timeOut;
    int timeRep;
    int currTask;
    int taskStart;
    int currMap;
    int isStart;
    QString lastError;
};

#endif // APPDRIVER_H
