#include "ComboBoxFocusManager.h"

ComboBoxFocusManager::ComboBoxFocusManager(QObject* parent)
    : QObject(parent)
{
    if (parent)
        parent->installEventFilter(this);
}

bool ComboBoxFocusManager::eventFilter(QObject* /*obj*/, QEvent* ev)
{
    if (ev->type() == QEvent::FocusIn)
        emit gainedFocus();
    else if (ev->type() == QEvent::FocusOut)
        emit lostFocus();

    return false;
}

