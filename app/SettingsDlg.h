#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <QDialog>

namespace Ui
{
class SettingsDlg;
}

class SettingsDlg : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDlg(QWidget* parent = nullptr);
    ~SettingsDlg();

protected slots:
    void onSaveButtonClicked();
    void onCancelButtonClicked();

private:
    Ui::SettingsDlg* ui;
};

#endif // SETTINGSDLG_H
