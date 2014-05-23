#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>

class QMouseEvent;

class TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = 0);

signals:

public slots:

protected:
    void mouseReleaseEvent(QMouseEvent *);
};

#endif // TABBAR_H
