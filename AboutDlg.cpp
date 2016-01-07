#include "AboutDlg.h"

AboutDlg::AboutDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
    ui->lblIcon->setPixmap(QPixmap(":/AppIcon.png"));
}
