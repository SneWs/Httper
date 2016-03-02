#include "SettingsDlg.h"
#include "ui_SettingsDlg.h"

SettingsDlg::SettingsDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDlg)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint |
        Qt::CustomizeWindowHint);

    connect(ui->btnCancel, SIGNAL(clicked(bool)), this, SLOT(onCancelButtonClicked()));
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(onSaveButtonClicked()));
}

SettingsDlg::~SettingsDlg()
{
    delete ui;
}

void SettingsDlg::onSaveButtonClicked()
{
    setResult(QDialog::Rejected);
    close();
}

void SettingsDlg::onCancelButtonClicked()
{
    setResult(QDialog::Accepted);
    close();
}
