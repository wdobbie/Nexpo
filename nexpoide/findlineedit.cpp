#include "findlineedit.h"
#include <QKeyEvent>
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>

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

void FindLineEdit::hideEvent(QHideEvent* e)
{
    QLineEdit::hideEvent(e);
    emit hidden();
}

void FindLineEdit::showEvent(QShowEvent* e)
{
    QLineEdit::showEvent(e);
    emit shown();
}

void FindLineEdit::focusInEvent(QFocusEvent* e)
{
    // Select all text when we get focus, unless we got focus from our own completer
    if (!(completer() && completer()->popup()->hasFocus())) {
        // Have to do it via a timer or a subsequent mouse event may deselect the text
        QTimer::singleShot(0, this, SLOT(selectAll()));

    }
    QLineEdit::focusInEvent(e);
}
