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
    void setElapsedSeconds(double);

private:
    Ui::ScriptStatusWidget *ui;
};

#endif // SCRIPTSTATUSWIDGET_H
