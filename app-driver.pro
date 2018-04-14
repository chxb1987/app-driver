#-------------------------------------------------
#
# Project created by QtCreator 2018-04-12T15:24:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = app-driver
TEMPLATE = app

RC_FILE += qrc/appsource.rc

HEADERS += \
    app/appdriver.h

SOURCES += \
    app/appdriver.cpp \
    app/main.cpp

RESOURCES += \
    qrc/appsource.qrc

