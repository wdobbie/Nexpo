#include "findlineedit.h"
#include <QKeyEvent>

FindLineEdit::FindLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
}


void FindLineEdit::keyPressEvent(QKeyEvent* event)
{
    switch(event->key()) {
    case Qt::Key_Escape:
        emit escapePressed();
        break;
    default:
        QLineEdit::keyPressEvent(event);
    }
}

void FindLineEdit::hideEvent(QHideEvent *)
{
    emit hidden();
}

void FindLineEdit::showEvent(QShowEvent *)
{
    emit shown();
}
