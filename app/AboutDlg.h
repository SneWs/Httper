#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include "ui_AboutDlg.h"

#include <QDialog>
#include <memory>

class AboutDlg : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDlg(QWidget *parent = nullptr);

private:
    std::unique_ptr<Ui::AboutDlg> ui;
};

#endif // ABOUTDLG_H
