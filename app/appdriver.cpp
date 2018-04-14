/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       松下伺服驱动调试助手
*******************************************************************************/
#include "appdriver.h"

typedef int (AppDriver::*pClass)(void);
QMap<int, pClass> taskMap;

AppDriver::AppDriver(QWidget *parent)
    : QMainWindow(parent)
{
    initUI();
    initParam();
}

AppDriver::~AppDriver()
{
}

void AppDriver::initUI()
{
    initSkin();
    initTitle();
    initLayout();
    initDevPort();
    initDevDisplay();
}

void AppDriver::initSkin()
{
    QFile file;
    QString qss;
    file.setFileName(":/qss_black.css");
    file.open(QFile::ReadOnly);
    qss = QLatin1String(file.readAll());
    qApp->setStyleSheet(qss);
}

void AppDriver::initTitle()
{
    char s_month[5];
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int month, day, year;

    sscanf((__DATE__), "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month)-month_names)/3+1;

    QDate dt;
    dt.setDate(year, month, day);
    static const QTime tt = QTime::fromString(__TIME__, "hh:mm:ss");

    QDateTime t(dt, tt);
    QString verNumb = QString("V-0.1.%1").arg(t.toString("yyMMdd-hhmm"));

    this->setWindowTitle(tr("松下伺服驱动调试助手%1").arg(verNumb));
}

void AppDriver::initParam()
{
    com = NULL;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(taskThread()));
    timer->start(10);

    taskMap[0] = &AppDriver::getRead;
    taskMap[1] = &AppDriver::getHand;
    taskMap[2] = &AppDriver::putHand;
    taskMap[3] = &AppDriver::getConf;
    taskMap[4] = &AppDriver::putConf;
    taskMap[5] = &AppDriver::getData;
    taskMap[6] = &AppDriver::putData;

    currMap = 0;
    isStart = 0;
    timeOut = 0;
    timeRep = 0;
}

void AppDriver::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;

    text = new QTextBrowser(this);
    layout->addWidget(text);

    btnLayout = new QHBoxLayout;
    layout->addLayout(btnLayout);

    layout->addWidget(new QLabel(tr("松下伺服驱动调试助手 by link"), this));

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    this->setCentralWidget(frame);
    this->resize(800, 600);
}

void AppDriver::initDevPort()
{
    QStringList com;
#ifndef __linux__
    QString path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM";
    QSettings *settings = new QSettings(path, QSettings::NativeFormat);
    QStringList key = settings->allKeys();
    HKEY hKey;
    int ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                             0, KEY_READ, &hKey);
    if (ret != 0) {
        qDebug() << "Cannot open regedit!";
    } else {
        for (int i=0; i < key.size(); i++) {
            wchar_t name[256];
            DWORD ikey = sizeof(name);
            char kvalue[256];
            DWORD t = sizeof(kvalue);
            DWORD type;
            QString tmpName;
            ret = ::RegEnumValue(hKey, i, LPWSTR(name), &ikey, 0, &type,
                                 reinterpret_cast<BYTE*>(kvalue), &t);
            if (ret == 0) {
                for (int j = 0; j < static_cast<int>(t); j++) {
                    if (kvalue[j] != 0x00) {
                        tmpName.append(kvalue[j]);
                    }
                }
                com << tmpName;
            }
        }
        RegCloseKey(hKey);
    }
#else
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        com << info.portName();
    }
#endif
    com.sort();
    for (int i=0; i < com.size(); i++) {
        if (com.at(i).size() > 4) {
            com.move(i, com.size()-1);
        }
    }
    btnLayout->addWidget(new QLabel(tr("串口:"), this));
    boxDevPort = new QComboBox(this);
    boxDevPort->addItems(com);
    boxDevPort->setView(new QListView);
    boxDevPort->setMinimumSize(77, 35);
    btnLayout->addWidget(boxDevPort);
    btnLayout->addStretch();
}

void AppDriver::initDevDisplay()
{
    btnLayout->addStretch();
    btnLayout->addWidget(new QLabel(tr("转速:"), this));

    speed = new QLineEdit(this);
    speed->setFixedHeight(35);
    btnLayout->addWidget(speed);

    btnLayout->addWidget(new QLabel(tr("转矩:"), this));

    torque = new QLineEdit(this);
    torque->setFixedHeight(35);
    btnLayout->addWidget(torque);

    QPushButton *btnCurrStop = new QPushButton(this);
    btnCurrStop->setText(tr("读取"));
    btnCurrStop->setMinimumSize(97, 35);
    connect(btnCurrStop, SIGNAL(clicked(bool)), this, SLOT(initTask()));
    btnLayout->addWidget(btnCurrStop);
}

int AppDriver::getRead()
{
    if (isStart == QMessageBox::Yes) {
        isStart = QMessageBox::No;
        return QMessageBox::Apply;
    } else {
        return QMessageBox::Reset;
    }
}

int AppDriver::getHand()
{
    return writeMsg(QByteArray::fromHex("0005"));
}

int AppDriver::putHand()
{
    int ret = QMessageBox::Retry;
    if (com->bytesAvailable() >= 1) {
        QByteArray msg = com->readAll();
        QString tstring = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        text->insertPlainText(tr("[%1] ").arg(tstring));
        text->insertPlainText("com recv: ");
        text->insertPlainText(msg.toHex());
        text->insertPlainText("\n");
        qDebug() << "com recv:" << msg.toHex();
        ret = QMessageBox::Apply;
    } else {
        ret = QMessageBox::Retry;
    }
    return ret;
}

int AppDriver::getConf()
{
    return writeMsg(QByteArray::fromHex("0001926D00"));
}

int AppDriver::putConf()
{
    int ret = QMessageBox::Retry;
    if (com->bytesAvailable() >= 2) {
        QByteArray msg = com->readAll();
        QString tstring = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        text->insertPlainText(tr("[%1] ").arg(tstring));
        text->insertPlainText("com recv: ");
        text->insertPlainText(msg.toHex());
        text->insertPlainText("\n");
        qDebug() << "com recv:" << msg.toHex();
        ret = QMessageBox::Apply;
    }
    return ret;
}

int AppDriver::getData()
{
    return writeMsg(QByteArray::fromHex("0400"));
}

int AppDriver::putData()
{
    int ret = QMessageBox::Retry;
    if (com->bytesAvailable() >= 7) {
        QByteArray msg = com->readAll();
        QString tstring = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        text->insertPlainText(tr("[%1] ").arg(tstring));
        text->insertPlainText("com recv: ");
        text->insertPlainText(msg.toHex());
        text->insertPlainText("\n");
        qDebug() << "com recv:" << msg.toHex();
        int ss = quint8(msg.at(3)) + quint8(msg.at(4))*256;
        ss = (ss > 0x8000) ? (ss-0x10000) : ss;
        speed->setText(QString::number(ss));
        int tt = quint8(msg.at(5)) + quint8(msg.at(6))*256;
        tt = (tt > 0x8000) ? (tt-0x10000) : tt;
        torque->setText(QString::number(tt/2000.0));
        ret = QMessageBox::Apply;
    }
    return ret;
}

int AppDriver::writeMsg(QByteArray msg)
{
    tmpByte.clear();
    if (com == NULL || !com->isOpen()) {
        lastError = tr("串口未打开");
        return QMessageBox::Abort;
    }
    if (com->bytesAvailable() > 0) {
        com->readAll();
    }
    if (com->write(msg) != msg.size()) {
        lastError = tr("串口写入失败");
        return QMessageBox::Abort;
    }
    QString tstring = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    text->insertPlainText(tr("[%1] ").arg(tstring));
    text->insertPlainText("com send: ");
    text->insertPlainText(msg.toHex());
    text->insertPlainText("\n");
    qDebug() << "com send:" << msg.toHex();
    return QMessageBox::Apply;
}

int AppDriver::initTask()
{
    if (com != NULL && com->isOpen()) {
        com->close();
    }
    com = new QSerialPort(boxDevPort->currentText(), this);
    if (com->open(QIODevice::ReadWrite)) {
        com->setBaudRate(9600);
        com->setDataBits(QSerialPort::Data8);
        com->setParity(QSerialPort::NoParity);
        com->setStopBits(QSerialPort::OneStop);
        com->setFlowControl(QSerialPort::NoFlowControl);
        com->setDataTerminalReady(true);
        com->setRequestToSend(false);
        isStart = QMessageBox::Yes;
        text->clear();
    } else {
        QMessageBox::warning(this, tr("警告"), tr("串口打开失败"), QMessageBox::Ok);
        qDebug() << "com open error";
    }
    return QMessageBox::Apply;
}

int AppDriver::taskThread()
{
    int ret = QMessageBox::Abort;
    if (taskMap.keys().contains(currMap))
        ret = (this->*taskMap[currMap])();
    switch (ret) {
    case QMessageBox::Apply:
        currMap = (taskMap.keys().contains(currMap+1)) ? (currMap+1) : 0;
        timeOut = 0;
        break;
    case QMessageBox::Retry:
        timeOut++;
        if (timeOut%30 == 0) {  // 300ms无正确应答,重复发送
            currMap = (currMap <= 0) ? 0 : (currMap-1);
            timeRep++;
            if (timeRep >= 3) {  // 重复3次后无应答,发出警告并退出
                ret = QMessageBox::Abort;
                lastError = tr("通信无应答");
            }
        }
        break;
    default:
        break;
    }
    if (ret == QMessageBox::Abort) {
        QMessageBox::StandardButtons button = QMessageBox::Abort | QMessageBox::Retry;
        button = QMessageBox::warning(this, tr("警告"), lastError, button);
        if (button == QMessageBox::Abort) {
            currMap = 0;
        }
        timeOut = 0;
        timeRep = 0;
    }
    return ret;
}
