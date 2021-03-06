#ifndef SCRIPTSTATUSWIDGET_H
#define SCRIPTSTATUSWIDGET_H

#include <QFrame>

namespace Ui {
class ScriptStatusWidget;
}

class ScriptStatusWidget : public QFrame
{
    Q_OBJECT

public:
    explicit ScriptStatusWidget(QWidget *parent = 0);
    ~ScriptStatusWidget();

public slots:
    void setName(const QString&);
    void setElapsed(double);

private:
    Ui::ScriptStatusWidget *ui;
    double m_lastElapsedValue;
};

#endif // SCRIPTSTATUSWIDGET_H
