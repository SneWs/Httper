#ifndef MAINWND_H
#define MAINWND_H

#include <QMainWindow>

namespace Ui
{
class MainWnd;
}

class MainWnd : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWnd();
    ~MainWnd();

private:
    Ui::MainWnd* ui;
};

#endif // MAINWND_H
