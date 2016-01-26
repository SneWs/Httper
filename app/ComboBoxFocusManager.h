#ifndef COMBOBOX_FOCUS_MANAGER_H__
#define COMBOBOX_FOCUS_MANAGER_H__

#include <QObject>
#include <QEvent>

class ComboBoxFocusManager : public QObject
{
    Q_OBJECT
public:
    explicit ComboBoxFocusManager(QObject* parent);
    virtual bool eventFilter(QObject* /*obj*/, QEvent* ev) override;

signals:
    void gainedFocus();
    void lostFocus();
};

#endif
