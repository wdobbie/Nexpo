#include "tabbar.h"
#include <QMouseEvent>

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
}


void TabBar::mouseReleaseEvent(QMouseEvent* event)
{
    // Close tab on middle click
    if (event->button() == Qt::MiddleButton) {
        int idx = tabAt(event->pos());
        if (idx >= 0 && idx < count()) {
            tabCloseRequested(idx);
        }
    }

    QTabBar::mouseReleaseEvent(event);
}
