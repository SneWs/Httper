#-------------------------------------------------
#
# Project created by QtCreator 2016-01-02T18:06:06
#
#-------------------------------------------------

QT += core gui network webkitwidgets widgets

!win32 {
    QMAKE_CXXFLAGS_CXX11 = -std=c++11
}

CONFIG += c++11

# Hack for OSX to make C++14 work
macx: {
    QMAKE_CXXFLAGS_CXX11 = -std=c++14 -stdlib=libc++
    CONFIG += c++11
}

TARGET = Httper
TEMPLATE = app

CONFIG(debug, debug|release) {
    DESTDIR = Bin/Debug
    OBJECTS_DIR = Bin/Debug
    MOC_DIR = Bin/Debug
    RCC_DIR = Bin/Debug
    UI_DIR = Bin/Debug
}
else {
    DESTDIR = Bin/Release
    OBJECTS_DIR = Bin/Release
    MOC_DIR = Bin/Release
    RCC_DIR = Bin/Release
    UI_DIR = Bin/Release
}

ICON = ./Images/Httper-icon128x128.icns

SOURCES += main.cpp\
        MainWnd.cpp \
    ComboBoxFocusManager.cpp \
    CookieJar.cpp \
    Settings.cpp \
    SettingsManager.cpp \
    AboutDlg.cpp \
    SettingsDlg.cpp

HEADERS  += MainWnd.h \
    MWidget.h \
    ComboBoxFocusManager.h \
    CookieJar.h \
    Settings.h \
    SettingsManager.h \
    AboutDlg.h \
    SettingsDlg.h

FORMS    += MainWnd.ui \
    AboutDlg.ui \
    SettingsDlg.ui

RESOURCES += \
    Resources.qrc

