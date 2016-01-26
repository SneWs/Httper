#include "MainWnd.h"
#include "Settings.h"
#include "SettingsManager.h"

#include <QString>
#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    auto settings = SettingsManager::loadSettings();

    MainWnd w(settings);
    w.applySettings();
    w.show();

    auto status = app.exec();
    if (status == 0)
        w.saveSettings();

    return status;
}
