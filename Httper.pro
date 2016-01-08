#-------------------------------------------------
#
# Project created by QtCreator 2016-01-02T18:06:06
#
#-------------------------------------------------

QT += core gui network webkitwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS_CXX11 = -std=c++14
CONFIG += c++11

# Hack for OSX to make C++14 work
macx: {
    QMAKE_CXXFLAGS_CXX11 = -std=c++14 -stdlib=libc++
    CONFIG += c++11
}

TARGET = Httper
TEMPLATE = app

ICON = ./Images/Httper-icon128x128.icns

SOURCES += main.cpp\
        MainWnd.cpp \
    ComboBoxFocusManager.cpp \
    CookieJar.cpp \
    Settings.cpp \
    SettingsManager.cpp \
    AboutDlg.cpp

HEADERS  += MainWnd.h \
    MWidget.h \
    ComboBoxFocusManager.h \
    CookieJar.h \
    Settings.h \
    SettingsManager.h \
    AboutDlg.h

FORMS    += MainWnd.ui \
    AboutDlg.ui

RESOURCES += \
    Resources.qrc

