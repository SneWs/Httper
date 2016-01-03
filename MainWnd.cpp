#include "MainWnd.h"
#include "ui_MainWnd.h"

MainWnd::MainWnd()
    : QMainWindow(nullptr)
    , ui(new Ui::MainWnd)
{
    ui->setupUi(this);
}

MainWnd::~MainWnd()
{
    delete ui;
}
