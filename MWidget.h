#ifndef MWIDGET_H
#define MWIDGET_H

#include <QWidget>

class MWidget : public QWidget
{
    Q_OBJECT
public:
    MWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    { }

signals:
    void onWidgetClosed(MWidget*);

protected:
    virtual void closeEvent(QCloseEvent*) override
    {
        emit onWidgetClosed(this);
    }

};

#endif // MWIDGET_H
