#include "tabwidget.h"
#include "tabbar.h"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    TabBar* tabBar = new TabBar(this);
    setTabBar(tabBar);
}
