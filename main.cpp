#include "MainWnd.h"
#include "Settings.h"
#include "SettingsManager.h"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    auto settings = SettingsManager::loadSettings();

    MainWnd w(settings);

    w.loadSettings();
    w.show();

    auto status = a.exec();
    if (status == 0)
        w.saveSettings();

    return status;
}
