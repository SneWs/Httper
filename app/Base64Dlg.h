#ifndef BASE64DLG_H
#define BASE64DLG_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui
{
class Base64Dlg;
}

class Base64Dlg : public QDialog
{
    Q_OBJECT
public:
    enum class Mode
    {
        Encode,
        Decode
    };

    explicit Base64Dlg(Base64Dlg::Mode mode, QWidget* parent = nullptr);
    ~Base64Dlg();

    void setContent(const QString& content);
    QString getContent() const;

signals:
    void onDialogClosed(QDialog*);

protected:
    virtual void closeEvent(QCloseEvent*) override
    {
        emit onDialogClosed(this);
    }

    virtual void keyPressEvent(QKeyEvent* ev) override
    {
        if (ev->key() == Qt::Key_Escape)
            close();
    }

private slots:
    void onInTextChanged();

private:
    void base64Encode();
    void base64Decode();

private:
    Ui::Base64Dlg* ui;
    Base64Dlg::Mode m_mode;
};

#endif // BASE64DLG_H
