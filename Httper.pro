#-------------------------------------------------
#
# Project created by QtCreator 2016-01-02T18:06:06
#
#-------------------------------------------------

QT += core gui network webkitwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = Httper
TEMPLATE = app

ICON = ./Images/Httper-icon128x128.icns

SOURCES += main.cpp\
        MainWnd.cpp

HEADERS  += MainWnd.h \
    MWidget.h

FORMS    += MainWnd.ui

RESOURCES += \
    Resources.qrc
