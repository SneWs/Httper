#include "MainWnd.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    MainWnd w;

    w.loadSettings();
    w.show();
    w.saveSettings();

    return a.exec();
}
