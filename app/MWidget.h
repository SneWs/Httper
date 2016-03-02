#ifndef MWIDGET_H
#define MWIDGET_H

#include <QWidget>
#include <QKeyEvent>

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

    virtual void keyPressEvent(QKeyEvent* ev) override
    {
        if (ev->modifiers() == Qt::ControlModifier)
        {
            if (ev->key() == Qt::Key_W)
                close();
        }
        else if (ev->key() == Qt::Key_Escape)
            close();
    }

};

#endif // MWIDGET_H
