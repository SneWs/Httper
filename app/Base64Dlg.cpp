#include "Base64Dlg.h"
#include "ui_Base64Dlg.h"

#include <QByteArray>

Base64Dlg::Base64Dlg(Base64Dlg::Mode mode, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Base64Dlg)
    , m_mode(mode)
{
    ui->setupUi(this);

    connect(ui->txtIn, SIGNAL(textChanged()), this, SLOT(onInTextChanged()));
}

Base64Dlg::~Base64Dlg()
{
    delete ui;
}

void Base64Dlg::setContent(const QString& content)
{
    ui->txtIn->setPlainText(content);
}

QString Base64Dlg::getContent() const
{
    return ui->txtOut->toPlainText();
}

void Base64Dlg::onInTextChanged()
{
    if (ui->txtIn->toPlainText().isEmpty())
    {
        ui->txtOut->setPlainText("");
        return;
    }

    if (m_mode == Base64Dlg::Mode::Encode)
        base64Encode();
    else
        base64Decode();
}

void Base64Dlg::base64Encode()
{
    QByteArray data;
    data.append(ui->txtIn->toPlainText());

    ui->txtOut->setPlainText(data.toBase64());
}

void Base64Dlg::base64Decode()
{
    QByteArray data;
    data.append(ui->txtIn->toPlainText());

    ui->txtOut->setPlainText(QByteArray::fromBase64(data));
}
